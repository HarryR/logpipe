#include "core.h"
#include <unistd.h>

static const char default_rowtype[] = "logstash";
static const char default_index_fmt[] = "logstash-%Y.%m.%d";

int logopt_init(logopt_t *opt, int argc, char **argv) {
    memset(opt, 0, sizeof(*opt));
    char c;
    char *key;
    char *val;
    pair_t *pair;

    opt->rowtype = default_rowtype;
    opt->index_fmt = default_index_fmt;

    while( (c = getopt(argc, argv, "ht:i:k:a:o:")) != -1 ) {
        switch( c) {
        case 'h':
            return 0; 

        case 't':
            opt->rowtype = optarg;
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
    printf("Optind: %d", optind);
    return 1;
}
