#ifndef LOG_H_
#define LOG_H_

#include "str.h"
#include <time.h>

typedef struct {    
	unsigned char md5[16];
    struct tm utc_timestamp;

    str_t timestamp;
    str_t client_ip;
    str_t client_identity;
    str_t client_auth;

    str_t req_verb;
    str_t req_path;
    str_t req_ver;

    // Cache response code, e.g. 'TCP_HIT'
    str_t resp_cache;

    // Response status code (e.g '200')
    str_t resp_status;
    str_t resp_size;

    str_t req_referrer;
    str_t req_agent;

    // Squid options 
    str_t duration;     // request duration
    str_t resp_bytes;  // total bytes transferred
    str_t heir_code;    // Hierarchy code
    str_t mime_type;    // Hierarchy code
} logline_t;

typedef int (*logmod_fn_t)(void *ctx, str_t *str, logline_t *line);

typedef struct {
    const char *name;
    logmod_fn_t init_fn;
    logmod_fn_t run_fn;
    logmod_fn_t free_fn;
} logmod_t;

typedef struct {
    const logmod_t *mod;
    void *ctx;
} logstep_t;

const logmod_t *step_findmod(const logmod_t **mods, const char *name);
void steps_free(logstep_t *steps);
logstep_t *steps_new(const logmod_t **mods, int argc, const char **argv);
int steps_init(logstep_t *steps, str_t *str, logline_t *line);
int steps_run(logstep_t *steps, str_t *str, logline_t *line);
void line_init(logline_t *line, str_t *str);
void line_free(logline_t *line);
void line_parse_timestamp_apacheclf( logline_t *line );
void line_parse_timestamp_epoch_secs( logline_t *line );

#endif
