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

#endif
