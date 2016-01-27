#ifndef STEPS_H__
#define STEPS_H__

#include <sys/types.h>

#include "logpipe-module.h"

typedef struct {
    const logmod_t *mod;
    void *ctx;
} logstep_t;

typedef struct {
    size_t count;
    size_t idx;
    logstep_t *steps;
} logsteps_t;

void logsteps_init(logsteps_t *steps);
void logsteps_free(logsteps_t *steps);
int logsteps_step(logsteps_t *steps, void *arg_A, void *arg_B);
size_t logsteps_add(logsteps_t *steps, const char *format);
size_t logsteps_count(const logsteps_t *steps);
size_t logsteps_idx(const logsteps_t *steps);
void logsteps_restart(logsteps_t *steps);

#endif
