#define USE_LOGPIPE_MODULES
#include "logpipe-module.h"
#include "steps.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>


static const logmod_t *logsteps_findmod(const char *name) {
  const logmod_t *mod = NULL;
  const char *name_end = name;
  assert( name != NULL );

  while( isalnum(*name_end) && *name_end != '.' ) {
    name_end++;    
  }
  if( name == name_end ) {
      return NULL;
  }

  int i = 0;
  while( (mod = builtin_mods[i]) ) {
    if( ! mod->name ) break;
    size_t len = name_end - name;
    if( strlen(mod->name) > len ) {
      len = strlen(mod->name);
    }
    if( ! strncmp(name, mod->name, len) ) {
      return mod;
    }
    i++;
  }
  return NULL;
}


void logsteps_init(logsteps_t *steps) {
	assert( steps != NULL );
	memset(steps, 0, sizeof(*steps));
}


void logsteps_free(logsteps_t *steps) {
	size_t idx;
	assert( steps != NULL );
	for( idx = 0; idx < steps->count; idx++ ) {
		logstep_t *step = &steps->steps[idx];
		if( step->mod->free_fn ) {
			step->mod->free_fn(step->ctx, NULL, NULL);
		}
	}
	free(steps->steps);
	memset(steps, 0, sizeof(*steps));
}


int logsteps_step(logsteps_t *steps, void *arg_A, void *arg_B, int *error) {
	assert( steps != NULL );
	size_t idx = steps->idx;
	if( idx >= steps->count ) {
		return -1;
	}
	logstep_t *step = &steps->steps[idx];
	const logmod_t *mod = step->mod;
	if( mod && mod->run_fn ) {
		int tmp_ret = mod->run_fn(step->ctx, arg_A, arg_B);
		if( error ) {
			*error = tmp_ret;
		}
	}
	steps->idx += 1;
	return steps->count - steps->idx;
}


int logsteps_add(logsteps_t *steps, const char *format) {
	const logmod_t *mod = logsteps_findmod(format);
	assert( steps != NULL );
	if( ! mod ) {
		return 0;
	}
	size_t idx = steps->count;
	steps->steps = realloc(steps->steps, sizeof(logstep_t) * (idx + 1));
	steps->steps[idx].mod = mod;
	if( mod->init_fn ) {
		int mod_retn = 	mod->init_fn(&steps->steps[idx].ctx, NULL, NULL);
		if( mod_retn <= 0 ) {
			return mod_retn;
		}
	}
	steps->count += 1;
	return steps->count;
}


int logsteps_count(const logsteps_t *steps) {
	assert( steps != NULL );
	return steps->count;
}


int logsteps_idx(const logsteps_t *steps) {
	assert( steps != NULL );
	return steps->idx;
}


void logsteps_restart(logsteps_t *steps) {
	assert( steps != NULL );
	steps->idx = 0;
}