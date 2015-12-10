#include "parser.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX_LINE_LENGTH 8192

static const char default_rowtype[] = "apacheclf";
static const char default_format_out[] = "logstash";
static const char default_format_in[] = "apacheclf";
static const char default_index_fmt[] = "logstash-%Y.%m.%d";

static void show_help(char *program) {
    fprintf(stderr, "Usage: %s <options>\n\n", program);
    fprintf(stderr, "  -h           - Show help\n\n");
    fprintf(stderr, "  -i <input>   - Input format: apacheclf, wikimedia\n");
    fprintf(stderr, "                 default: \"%s\"\n\n", default_format_in);
    fprintf(stderr, "  -o <output>  - Output format: logstash, hyperstats\n");
    fprintf(stderr, "                 default: \"%s\"\n\n", default_format_out);
    fprintf(stderr, "  -t <type>    - value of \"_type\" field\n");
    fprintf(stderr, "                 default: \"%s\"\n\n", default_rowtype);
    fprintf(stderr, "  -k <fmt>     - index key format, uses strftime\n");
    fprintf(stderr, "                 default: \"%s\"\n\n", default_index_fmt);
    fprintf(stderr, "  -a <key=val> - Add extra keypairs to JSON output\n\n");
}


static int logopt_init(logopt_t *opt, int argc, char **argv) {
    memset(opt, 0, sizeof(*opt));
    char c;
    char *key;
    char *val;
    pair_t *pair;

    while( (c = getopt(argc, argv, "ht:i:k:a:o:")) != -1 ) {
        switch( c) {
        case 'h':
            return 0; 

        case 't':
            opt->rowtype = optarg;
            break;

        case 'o':
          opt->format_out = optarg;
        break;

        case 'i':
            opt->format_in = optarg;
            break;

        case 'k':
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
            pair->val.len = strlen(val);
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

  if( opts.format_out == NULL ) {
    opts.format_out = default_format_out;
  }

  if( opts.format_in == NULL ) {
    opts.format_in = default_format_in;
  }

  if( opts.rowtype == NULL ) {
    opts.rowtype = default_rowtype;
  }

  if( opts.index_fmt == NULL ) {
    opts.index_fmt = default_index_fmt;
  }

  logstash_parse_fn_t parse_fn;
  if( strcmp("apacheclf", opts.format_in) == 0 ) {
    parse_fn = logline_parse_apacheclf;
  }
  else {
    fprintf(stderr, "Error: unknown input format '%s'\n", opts.format_in);
    show_help(argv[0]);
    return 1;
  }

  logstash_print_fn_t print_fn;
  if( strcmp("logstash", opts.format_out) == 0 ) {
    print_fn = logline_logstash_print;
  }
  else if( strcmp("hyperstats", opts.format_out) == 0 ) {
    print_fn = logline_hyperstats_print;
  }
  else {
    fprintf(stderr, "Error: unknown output format '%s'\n", opts.format_out);
    show_help(argv[0]);
    return 1;
  }
 
  /* Reads input lines, outputs parsed lines to stdout
   * and unparsable lines to stderr.
   */
  while(fgets(buf, MAX_LINE_LENGTH, stdin) != NULL) {
    buf_sz = strlen(buf);
    logline_line_init(&line, (unsigned char*)buf, buf_sz);
    if( parse_fn(&line) ) {
      print_fn(&line, &opts, stdout);
    }
    else {
        fwrite(buf, buf_sz, 1, stderr);
    }
  }

  return 0;
}
