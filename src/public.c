#include "logpipe.h"
#include "steps.h"

#include "logpipe-module.h"

#include <stdlib.h>
#include <assert.h>

struct logpipe {
	int is_stopping;
	logsteps_t steps;
	str_t buf;
	logline_t meta;
};


logpipe_t *logpipe_new(void) {
	logpipe_t *val = malloc(sizeof(*val));
	if( ! val ) {
		return val;
	}
	val->is_stopping = 0;
	str_init(&val->buf);
	line_init(&val->meta, NULL);
	logsteps_init(&val->steps);
	return val;
}


void logpipe_destroy(logpipe_t *pipe) {
	assert( pipe );
	pipe->is_stopping = 1;
	logsteps_free(&pipe->steps);
	line_free(&pipe->meta);
	str_clear(&pipe->buf);
	free(pipe);
}


int logpipe_step(logpipe_t *pipe) {
	assert( pipe );
	return logsteps_step(&pipe->steps, &pipe->buf, &pipe->meta);
}


void logpipe_restart(logpipe_t *pipe) {
	logsteps_restart(&pipe->steps);
}


int logpipe_run(logpipe_t *pipe) {
	int step;
	assert( pipe );
	logsteps_restart(&pipe->steps);
	while( (step = logpipe_step(pipe)) > 0 ) {
		/* do nothing... will reduce to zero */
	}
	return step;
}


int logpipe_run_forever(logpipe_t *pipe) {
	assert( pipe );
	while( ! pipe->is_stopping ) {
		logpipe_run(pipe);
	}
	return pipe->is_stopping;
}


int logpipe_steps_add(logpipe_t *pipe, const char *format) {
	assert( pipe );
	return logsteps_add(&pipe->steps, format, builtin_mods);
}


int logpipe_steps_count(const logpipe_t *pipe) {
	assert( pipe );
	return logsteps_count(&pipe->steps);
}


const char *logpipe_buf_get(logpipe_t *pipe, int *len) {
	assert( pipe );
	if( len ) {
		*len = str_len(&pipe->buf);
	}
	return str_ptr(&pipe->buf);
}


void logpipe_buf_set(logpipe_t *pipe, const char *str, int len) {
	assert( pipe );
	assert( len >= 0 );
	str_clear(&pipe->buf);
	str_append(&pipe->buf, str, len);
}