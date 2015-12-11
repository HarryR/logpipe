#include "mod.h"

#include <ctype.h>

const logmod_t *step_findmod(const char *name) {
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
    if( ! strncmp(name, mod->name, name_end - name) ) {
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

logstep_t *steps_new(int argc, const char **argv) {
    logstep_t *steps = NULL;
    size_t count = 0;
    int i;

    for( i = 0; i < argc; i++ ) {
      const logmod_t *mod = step_findmod(argv[i]);
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
    str_free(&line->raw);
  }
}

void line_init(logline_t *line, str_t *str) {
  line_free(line);
  memset(line, 0, sizeof(*line));
  if( str && str->ptr ) {
    str_append_str(&line->raw, str);
    logline_make_md5(line);
  }
}

int steps_init(logstep_t *steps, str_t *str, logline_t *line) {
  int ret = 0;
  logstep_t *step = &steps[ret];
  str_init(str);
  line_init(line, str);
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
            fprintf(stderr, "!%s: %s\n", mod->name, str->ptr);
          }
          break;
      }
      ret += 1;
      step = &steps[ret];
    }
  }
  str_free(str);
  return ret;
}