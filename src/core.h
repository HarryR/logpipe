#ifndef CORE_H_
#define CORE_H_

#include "config.h"

//#define _XOPEN_SOURCE 700
//#define _BSD_SOURCE
#include <time.h>

#include "json.h"

typedef struct {
    unsigned char* ptr;
    size_t len;
} str_t;

typedef struct {
    str_t key;
    str_t val;
} pair_t;

typedef struct {    
    const char* format_in;
    const char* format_out;
    const char* rowtype;
    const char* index_fmt;
    pair_t* extra;
    size_t extra_cnt;
} logopt_t;


int logopt_init(logopt_t *opt, int argc, char **argv);

int str_append(str_t *str, const char *s, int length);
int str_append_str(str_t *str, str_t *b);
void str_init(str_t *str);
void str_free(str_t *str);

/*
void logline_hyperstats_print(logline_t *line, logopt_t *opt, FILE *fd);
void logline_logstash_print(logline_t* line, logopt_t *opt, FILE* fd);

typedef void (*logline_print_fn_t)(logline_t *line, logopt_t *opt, FILE *fd);

int logline_input_stdin(logopt_t *ctx, str_t *line);

void logline_print_extra(logopt_t *opt, json_printer *jp);
void logline_print_id(logline_t *line, json_printer *jp, const char* key);
void logline_print_splitpath(json_printer *jp, char *path, size_t len);
void logline_make_md5(logline_t *line);
*/
#endif

