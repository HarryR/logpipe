#include "log.h"
#include "mod.h"
#include "md5.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const logmod_t *step_findmod(const logmod_t **mods, const char *name) {
  const logmod_t *mod = NULL;
  const char *name_end = name;
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

void steps_free(logstep_t *steps) {
  if( steps ) {
    int i = 0;
    logstep_t *step = &steps[0];
    while( step ) {
      if( ! step->mod || ! step->mod->name ) {
        break;
      }
      if( step->mod->free_fn ) {
        step->mod->free_fn(step->ctx, NULL, NULL);
      }
      i+= 1;
      step = &steps[i];
    }
    free(steps);
  }
}

logstep_t *steps_new(const logmod_t **mods, int argc, const char **argv) {
    logstep_t *steps = NULL;
    size_t count = 0;
    int i;

    for( i = 0; i < argc; i++ ) {
      const logmod_t *mod = step_findmod(mods, argv[i]);
      if( ! mod ) {
        fprintf(stderr, "Error: unknown step '%s'\n", argv[i]);
        return NULL;
      }

      count += 1;
      steps = realloc(steps, sizeof(logstep_t) * (count+1));
      if( ! steps ) {
        return NULL;
      }

      steps[i].mod = mod;
      if( mod->init_fn ) {
        mod->init_fn(&steps[i].ctx, NULL, NULL);
        if( ! mod->init_fn(&steps[i].ctx, NULL, NULL) ) {
          fprintf(stderr, "Error: cannot init step '%s'\n", argv[i]);
          steps_free(steps);
          return NULL; 
        }
      }
    }
    if( count ) {
      memset(&steps[count], 0, sizeof(*steps));
    }
    return steps;
}

void line_free(logline_t *line) {
  if( line ) {
    // Common Log Format options
    str_clear(&line->timestamp);
    str_clear(&line->client_ip);
    str_clear(&line->client_identity);
    str_clear(&line->client_auth);
    str_clear(&line->req_verb);
    str_clear(&line->req_path);
    str_clear(&line->req_ver);
    str_clear(&line->resp_status);
    str_clear(&line->resp_size);
    str_clear(&line->req_referrer);
    str_clear(&line->req_agent);

    // Squid options 
    str_clear(&line->duration);
    str_clear(&line->resp_bytes);
    str_clear(&line->resp_cache);
    str_clear(&line->heir_code);
    str_clear(&line->mime_type);
  }
}

void line_init(logline_t *line, str_t *str) {
  memset(line, 0, sizeof(*line));
  if( str && str->ptr ) {
    md5_state_t ctx;
    md5_init(&ctx);
    md5_append(&ctx, str->ptr, str->len);
    md5_finish(&ctx, line->md5);
  }
}

int steps_init(logstep_t *steps, str_t *str, logline_t *line) {
  int ret = 0;
  logstep_t *step = &steps[ret];
  str_init(str);
  line_init(line, NULL);
  while( step && step->mod ) {
    const logmod_t *mod = step->mod;
    if( mod->init_fn ) {      
      mod->init_fn(step->ctx, str, line);
    }
    ret += 1;
    step = &steps[ret];
  }
  return ret;
}

int steps_run(logstep_t *steps, str_t *str, logline_t *line) {
  int ret = 0;
  logstep_t *step = &steps[0];
  while( step && step->mod ) {
    const logmod_t *mod = step->mod;
    if( mod->run_fn ) {      
      if( ! mod->run_fn(step->ctx, str, line) ) {
          if( str->ptr ) {
            // mod->name, ret, 
            fprintf(stderr, "%.*s\n",
                    (int)str->len, str->ptr);
          }
          break;
      }
      ret += 1;
      step = &steps[ret];
    }
  }
  str_clear(str);
  return ret;
}

void line_parse_timestamp_epoch_secs( logline_t *line ) {
  if( ! str_isempty(&line->timestamp) ) {
    char *pos = str_rpos(&line->timestamp, '.');
    if( pos ) {
      *pos = 0;
    }    
    str_ptime(&line->timestamp, "%s", &line->utc_timestamp);
    if( pos ) {
      *pos = '.';
    }
  }
  else {
    memset(&line->utc_timestamp, 0, sizeof(line->utc_timestamp));
  }
}