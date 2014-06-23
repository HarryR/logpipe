#include "parser.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX_LINE_LENGTH 4096

static const char default_rowtype[] = "apacheclf";
static const char default_format[] = "logstash";
static const char default_index_fmt[] = "logstash-%Y.%m.%d";

static void show_help(char *program) {
    fprintf(stderr, "Usage: %s <options>\n\n", program);
    fprintf(stderr, "  -h           - Show help\n\n");
	fprintf(stderr, "  -f <format>  - Output format\n");
	fprintf(stderr, "                 default: \"%s\"\n", default_format);
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

    while( (c = getopt(argc, argv, "ht:i:a:f:")) != -1 ) {
        switch( c) {
        case 'h':
            return 0; 

        case 't':
            opt->rowtype = optarg;
            break;

		case 'f':
			opt->format = optarg;
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
  int format;

  if( ! logopt_init(&opts, argc, argv) ) {
    show_help(argv[0]);
    return 1;
  }

  if( opts.format == NULL ) {
	opts.format = default_format;
  }

  if( opts.rowtype == NULL ) {
    opts.rowtype = default_rowtype;
  }

  if( opts.index_fmt == NULL ) {
    opts.index_fmt = default_index_fmt;
  }

  if( strcmp("logstash", opts.format) == 0 ) {
    format = 1; 
  }
  else if( strcmp("hyperstats", opts.format) == 0 ) {
    format = 2;
  }
  else {
    fprintf(stderr, "Error: unknown output format '%s'\n", opts.format);
    show_help(argv[0]);
    return 1;
  }
 
  while(fgets(buf, MAX_LINE_LENGTH, stdin) != NULL) {
    buf_sz = strlen(buf);
    if( logline_parse(&line, (unsigned char*)buf, buf_sz) ) {
        if( format == 1 ) {
        	logline_logstash_print(&line, &opts, stdout);
		}
		else if(  format == 2 ) {
			logline_hyperstats_print(&line, &opts, stdout);
		}
    }
    else {
        fwrite(buf, buf_sz, 1, stderr);
    }
  }

  return 0;
}
