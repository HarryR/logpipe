#include "logpipe.h"
#include "steps.h"

#include <stdlib.h>
#include <assert.h>

struct logpipe {
	int is_stopping;
	logsteps_t steps;
	str_t buf;
	logmeta_t meta;
};


logpipe_t *logpipe_new(void) {
	logpipe_t *pipe = malloc(sizeof(*pipe));
	if( ! pipe ) {
		return pipe;
	}
	pipe->is_stopping = 0;
	str_init(&pipe->buf);
	logmeta_clear(&pipe->meta);
	logsteps_init(&pipe->steps);
	return pipe;
}


void logpipe_destroy(logpipe_t *pipe) {
	assert( pipe );
	pipe->is_stopping = 1;
	logsteps_free(&pipe->steps);
	logmeta_clear(&pipe->meta);
	str_clear(&pipe->buf);
	free(pipe);
}


int logpipe_step(logpipe_t *pipe, int *error) {
	assert( pipe );
	return logsteps_step(&pipe->steps, &pipe->buf, &pipe->meta, error);
}


void logpipe_restart(logpipe_t *pipe) {
	logsteps_restart(&pipe->steps);
}


int logpipe_run(logpipe_t *pipe, int *error) {
	int idx = 0;
	int step;
	assert( pipe );
	*error = 0;
	logsteps_restart(&pipe->steps);
	if( logpipe_steps_count(pipe) ) {		
		while( (step = logpipe_step(pipe, error)) > 0 ) {
			if( step < 0 || *error <= 0 ) {
				break;
			}
			idx += 1;
			/* step will be zero when it reaches the last step */
		}
		idx += 1;
	}
	if( error ) {
		*error = 0;
	}
	return idx;
}


int logpipe_run_forever(logpipe_t *pipe) {
	assert( pipe );		
	if( logpipe_steps_count(pipe) ) {
		while( ! pipe->is_stopping ) {
			int error = 0;
			int step = logpipe_run(pipe, &error);
			if( step != logpipe_steps_count(pipe) || error ) {
				break;
			}
		}
	}
	return pipe->is_stopping;
}


int logpipe_steps_add(logpipe_t *pipe, const char *format) {
	assert( pipe );
	return logsteps_add(&pipe->steps, format);
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


int logpipe_steps_index(const logpipe_t *pipe) {
	assert( pipe );
	if( logsteps_count(&pipe->steps) == 0 ) {
		return -1;
	}
	return logsteps_idx(&pipe->steps);
}


void logpipe_buf_set(logpipe_t *pipe, const char *str, int len) {
	assert( pipe );
	assert( len >= 0 );
	str_clear(&pipe->buf);
	str_append(&pipe->buf, str, len);
}