#ifndef CORE_H_
#define CORE_H_

#define _XOPEN_SOURCE 700
#define _BSD_SOURCE
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
    int cs;
    unsigned char *p, *pe, *ts, *eof;

	unsigned char md5[16];
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
	const char* format;
    const char* rowtype;
    const char* index_fmt;
    pair_t* extra;
    size_t extra_cnt;
} logopt_t;

void logline_hyperstats_print(logline_t *line, logopt_t *opt, FILE *fd);
void logline_logstash_print(logline_t* line, logopt_t *opt, FILE* fd);

void logline_print_extra(logopt_t *opt, json_printer *jp);
void logline_print_id(logline_t *line, json_printer *jp, const char* key);
void logline_print_splitpath(json_printer *jp, char *path, size_t len);
void logline_make_md5(logline_t *line);

#endif

