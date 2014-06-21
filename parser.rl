#define _XOPEN_SOURCE 700
#define _BSD_SOURCE
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <unistd.h>

#include "base64.h"
#include "md5.h"
#include "json.h"
#include "jv-utils.h"
#include "url.h"

#define SAVE_LINE_STR(field) { line->field.ptr = line->ts; line->field.len = line->p - line->ts; }

#define MAX_LINE_LENGTH 4096

static const char default_rowtype[] = "apacheclf";
static const char default_index_fmt[] = "logstash-%Y.%m.%d";

typedef struct {
    unsigned char* ptr;
    size_t len;
} str_t;

typedef struct {
    str_t key;
    str_t val;
} pair_t;

typedef struct {
    int cs;
    unsigned char *p, *pe, *ts, *eof;

    struct tm utc_timestamp;

    str_t raw;

    str_t timestamp;
    str_t host;
    str_t req_verb;
    str_t req_path;
    str_t req_ver;
    str_t resp_status;
    str_t resp_size;
    str_t req_referrer;
    str_t req_agent;
} logline_t;

typedef struct {
    const char* rowtype;
    const char* index_fmt;
    pair_t* extra;
    size_t extra_cnt;
} logopt_t;

%%{
  machine parser;

  alphtype unsigned char;
  access line->;
  variable p line->p;
  variable pe line->pe;
  variable eof line->eof;

  # Registers the mark action, but delegates its implementation to the host 
  # language
  action mark { line->ts = line->p; }

  escaped_char = ( '\\x' xdigit{2} | '\\' ascii | [^"] ) ;

  # Define the various components of a log entry
  #
  # > and % denote entering and leaving actions respectively, so this machine
  #   will mark and emit each of the following token types as it sees them
  client_ip = [0-9\.]+
            >mark %{ SAVE_LINE_STR(host) };

  timestamp = [^\]]+
            >mark %{ SAVE_LINE_STR(timestamp) };

  req_verb = [A-Z]+
            >mark %{ SAVE_LINE_STR(req_verb) };

  req_path = [^ ]+
            >mark %{ SAVE_LINE_STR(req_path) };

  req_ver  = [0-9\.]+
            >mark %{ SAVE_LINE_STR(req_ver) };

  resp_status  = digit+
            >mark %{ SAVE_LINE_STR(resp_status) };

  resp_size = (digit+ >mark %{ SAVE_LINE_STR(resp_size) } | '-' );

  req_referrer = escaped_char*
            >mark %{ SAVE_LINE_STR(req_referrer) };

  req_agent = escaped_char*
            >mark %{ SAVE_LINE_STR(req_agent) };

  # Assemble the components to define a single line
  line = (
    client_ip       space
    '-'             space
    '-'             space
    '[' timestamp ']'    space
    '"' req_verb space req_path space "HTTP/" req_ver '"' space
    resp_status          space
    resp_size            space
    '"' req_referrer '"'     space
    '"' req_agent '"'   '\n'
  );

  main := line $err{ return 0; };

  write data;

}%%

static int jp_callback(void *userdata, const char *s, uint32_t length) {
    return fwrite(s, length, 1, (FILE*)userdata);
}

static int json_print_key(json_printer *printer, const char *key) {
    return json_print_raw(printer, JSON_KEY, key, strlen(key));
}

static void logline_parse_timestamp( logline_t *line ) {
    struct tm local_timestamp;
    strptime((char*)line->timestamp.ptr, "%d/%b/%Y:%H:%M:%S %z", &local_timestamp);
    long int gmtoff = local_timestamp.tm_gmtoff;
    time_t actual_time = timegm(&local_timestamp) - gmtoff;
    gmtime_r(&actual_time, &line->utc_timestamp);
}

static void print_keystr(json_printer *jp, char *key, str_t *str) {
    json_print_key(jp, key);
    json_print_raw(jp, JSON_STRING, (char*)str->ptr, str->len);
}

static void print_keystr2(json_printer *jp, char *key, char* str) {
    json_print_key(jp, key);
    json_print_raw(jp, JSON_STRING, str, strlen(str));
}

static void print_keyraw(json_printer *jp, char *key, str_t *str) {
    json_print_key(jp, key);
    json_print_raw(jp, JSON_INT, (char*)str->ptr, str->len);
}

static int logline_parse(logline_t *line, unsigned char* buf, size_t buf_sz) {
    memset(line, 0, sizeof(*line));
    line->p = buf;
    line->eof = line->pe = buf + buf_sz;
    line->ts = line->p;
    line->raw.ptr = (unsigned char*)buf;
    line->raw.len = buf_sz;

    %% write init;  
    %% write exec;

    logline_parse_timestamp(line);

    return 1;
}

static void logline_print(logline_t* line, logopt_t *opt, FILE* fd) {
    json_printer jp;

    md5_state_t ctx;
    unsigned char md5[16];
    char b64[BASE64_SIZE(sizeof(md5))];
    md5_init(&ctx);
    md5_append(&ctx, line->raw.ptr, line->raw.len);
    md5_finish(&ctx, md5);
    char* x = base64_encode(b64, sizeof(b64), md5, 16);
    assert(x != NULL);
    b64[22] = 0;

    json_print_init(&jp, jp_callback, fd); 
    json_print_raw(&jp, JSON_OBJECT_BEGIN, NULL, 0);
    json_print_key(&jp, "index");

        json_print_raw(&jp, JSON_OBJECT_BEGIN, NULL, 0);
        json_print_key(&jp, "_id");
        json_print_raw(&jp, JSON_STRING, b64, 22);

        json_print_key(&jp, "_type");
        json_print_raw(&jp, JSON_STRING, opt->rowtype, strlen(opt->rowtype));

        char index_name[0xff]; // XXX: how to know size of strftime result in advance?
        strftime(index_name, sizeof(index_name), opt->index_fmt, &line->utc_timestamp);
        json_print_key(&jp, "_index");
        json_print_raw(&jp, JSON_STRING, index_name, strlen(index_name));

        // Timestamp compatible with logstash
        // This format should be auto-detected by elasticsearch
        char timestamp[50];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", &line->utc_timestamp);
        json_print_key(&jp, "@timestamp");
        json_print_raw(&jp, JSON_STRING, timestamp, strlen(timestamp));

        print_keystr(&jp, "client_ip", &line->host);

        // "req": {...}
        json_print_key(&jp, "req");
            json_print_raw(&jp, JSON_OBJECT_BEGIN, NULL, 0);
            // TODO: hostname
            print_keystr(&jp, "verb", &line->req_verb);
            print_keystr(&jp, "path", &line->req_path);
            print_keystr(&jp, "ver", &line->req_ver);
            if( line->req_referrer.len ) {
                print_keystr(&jp, "referrer", &line->req_referrer);
            }
            if( line->req_agent.len ) {
                print_keystr(&jp, "agent", &line->req_agent);
            }
            json_print_raw(&jp, JSON_OBJECT_END, NULL, 0);

        // "referrer": {...}
        if( line->req_referrer.len ) {
            json_print_key(&jp, "referrer");
                json_print_raw(&jp, JSON_OBJECT_BEGIN, NULL, 0);
                php_url *url = php_url_parse_ex((char*)line->req_referrer.ptr, line->req_referrer.len);
                if( url != NULL ) {
                    if( url->scheme ) print_keystr2(&jp, "scheme", url->scheme);
                    if( url->user) print_keystr2(&jp, "user", url->user);
                    if( url->pass) print_keystr2(&jp, "pass", url->pass);
                    if( url->host) print_keystr2(&jp, "host", url->host);
                    if( url->path) print_keystr2(&jp, "path", url->path);
                    if( url->query) print_keystr2(&jp, "query", url->query);
                    if( url->fragment) print_keystr2(&jp, "fragment", url->fragment);
                    php_url_free(url);
                }
                else {
                    print_keystr(&jp, "_raw", &line->req_referrer);
                }
                json_print_raw(&jp, JSON_OBJECT_END, NULL, 0);
        }

        // "resp": {...}
        json_print_key(&jp, "resp");
            json_print_raw(&jp, JSON_OBJECT_BEGIN, NULL, 0);
            print_keyraw(&jp, "status", &line->resp_status);
            if( line->resp_size.len ) {
                print_keyraw(&jp, "size", &line->resp_size);
            }
            json_print_raw(&jp, JSON_OBJECT_END, NULL, 0);

        // Add extra fields to every record (specified via -a on cmdline)
        for( size_t i = 0; i < opt->extra_cnt; i++ ) {
            pair_t *pair = &opt->extra[i];
            print_keystr(&jp, (char*)pair->key.ptr, &pair->val);
        }

        json_print_raw(&jp, JSON_OBJECT_END, NULL, 0);

    json_print_raw(&jp, JSON_OBJECT_END, NULL, 0);
    json_print_free(&jp);
    fwrite("\n", 1, 1, fd);
}



static void show_help(char *program) {
    fprintf(stderr, "Usage: %s <options>\n\n", program);
    fprintf(stderr, "  -h           - Show help\n\n");
    fprintf(stderr, "  -t <type>    - value of \"_type\" field\n");
    fprintf(stderr, "                 default: \"%s\"\n\n", default_rowtype);
    fprintf(stderr, "  -i <fmt>     - index name format, uses strftime\n");
    fprintf(stderr, "                 default: \"%s\"\n\n", default_index_fmt);
    fprintf(stderr, "  -a <key=val> - Add extra keypairs to JSON output\n\n");
}

static int logopt_init(logopt_t *opt, int argc, char **argv) {
    memset(opt, 0, sizeof(*opt));
    char c;
    char *key;
    char *val;
    pair_t *pair;

    while( (c = getopt(argc, argv, "ht:i:a:")) != -1 ) {
        switch( c) {
        case 'h':
            return 0; 
        case 't':
            opt->rowtype = optarg;
            break;

        case 'i':
            opt->index_fmt = optarg;
            break;

        case 'a':
            key = optarg;
            val = strchr(key, '=');
            if( val == NULL ) {
                fprintf(stderr, "Error parsing extra: '%s'\n", optarg);
                return 0;
            }
            val[0] = 0;
            val++;
            if( strlen(val) == 0 || strlen(key) == 0 ) {
                fprintf(stderr, "Error parsing extra: '%s'\n", optarg);
                return 0;
            }

            opt->extra_cnt += 1;
            opt->extra = realloc(opt->extra, sizeof(pair_t) * opt->extra_cnt);
            pair = &opt->extra[opt->extra_cnt - 1];
            pair->key.ptr = (unsigned char*)key;
            pair->key.len = strlen(key);
            pair->val.ptr = (unsigned char*)val;
            pair->val.len = strlen(key);
            break; 
        }
    }
    return 1;
}



int main(int argc, char **argv) {
  char buf[MAX_LINE_LENGTH];
  size_t buf_sz;
  logline_t line;
  logopt_t opts;

  if( ! logopt_init(&opts, argc, argv) ) {
    show_help(argv[0]);
    return 1;
  }

  if( opts.rowtype == NULL ) {
    opts.rowtype = default_rowtype;
  }

  if( opts.index_fmt == NULL ) {
    opts.index_fmt = default_index_fmt;
  }
 
  while(fgets(buf, MAX_LINE_LENGTH, stdin) != NULL) {
    buf_sz = strlen(buf);
    if( logline_parse(&line, (unsigned char*)buf, buf_sz) ) {
        logline_print(&line, &opts, stdout);
    }
    else {
        fwrite(buf, buf_sz, 1, stderr);
    }
  }

  return 0;
}
