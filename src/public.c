#include "logpipe.h"
#include "steps.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

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
	logmeta_init(&pipe->meta);
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


int logpipe_step(logpipe_t *pipe) {
	assert( pipe );
	return logsteps_step(&pipe->steps, &pipe->buf, &pipe->meta);
}


void logpipe_restart(logpipe_t *pipe) {
	logsteps_restart(&pipe->steps);
}


int logpipe_run(logpipe_t *pipe) {
	int idx = 0;
	assert( pipe );
	logsteps_restart(&pipe->steps);
	while( logpipe_steps_index(pipe) < logpipe_steps_count(pipe) ) {
		int ret_code = logpipe_step(pipe);
		if( ret_code <= 0 ) {
			// logpipe_step returns negative if the pipeline
			// must be permanently halted
			if( ret_code < 0 ) {
				return -1;
			}
			// Otherwise end up returning whichever step index
			// that the pipeline got up to.
			break;
		}
		idx += 1;
	}
	return idx;
}


int logpipe_run_forever(logpipe_t *pipe) {
	assert( pipe );		
	if( logpipe_steps_count(pipe) ) {
		while( ! pipe->is_stopping ) {
			int ret_code = logpipe_run(pipe);
			if( ret_code < 0 ) {
				break;
			}
		}
	}
	return pipe->is_stopping;
}


size_t logpipe_steps_add(logpipe_t *pipe, const char *format) {
	assert( pipe );
	return logsteps_add(&pipe->steps, format);
}


size_t logpipe_steps_count(const logpipe_t *pipe) {
	assert( pipe );
	return logsteps_count(&pipe->steps);
}


const char *logpipe_buf_get(logpipe_t *pipe, size_t *len) {
	assert( pipe );
	if( len ) {
		*len = str_len(&pipe->buf);
	}
	return (char*)str_ptr(&pipe->buf);
}


size_t logpipe_steps_index(const logpipe_t *pipe) {
	assert( pipe );
	return logsteps_idx(&pipe->steps);
}


void logpipe_buf_set(logpipe_t *pipe, const char *str, size_t len) {
	assert( pipe );
	assert( len >= 0 );
	str_clear(&pipe->buf);
	str_append(&pipe->buf, str, len);
}


int logpipe_test(int result, const char *steps_cstr, const char *input, const char *output) {
	if( ! steps_cstr ) return 0;
	const str_t steps_str = {steps_cstr, strlen(steps_cstr)};
	pair_t *steps = strpair_split(&steps_str);
	if( ! strpair_count(steps) ) return 0;

	logpipe_t *pipe = logpipe_new();
	if( ! pipe ) return 0;

	pair_t *steps_iter = steps;
	while( steps_iter ) {
		if( logpipe_steps_add(pipe, (const char*)steps_iter->val.ptr) <= 0 ) {
			logpipe_destroy(pipe);
			return 0;
		}
		steps_iter = strpair_next(steps_iter);
	}

	if( input ) {
		logpipe_buf_set(pipe, input, strlen(input));
	}

	int is_ok = 1;
	int run_result = logpipe_run(pipe);
	if( result == run_result ) {
		size_t out_sz = 0;
		const char *out = logpipe_buf_get(pipe, &out_sz);
		if( output == NULL ) {
			is_ok = (out == NULL) && (out_sz == 0);
		}
		else if( out_sz != strlen(output) ) {
			is_ok = 0;
		}
		else {
			is_ok = strncmp(out, output, out_sz) == 0;
		}
	}
	else {
		is_ok = 0;
	}

	logpipe_destroy(pipe);
	return is_ok;
}
