#include "mod.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

const logmod_t *builtin_mods[] = {
  &mod_reset_str,
  &mod_reset_line,
  &mod_reset_both,
  &mod_stdin,
  &mod_stderr,
  &mod_stdout,
  &mod_parse_apacheclf,
  &mod_parse_clfjson,
  &mod_print_logstash,
  &mod_print_hyperstats,
  &mod_print_clfjson,
  &mod_print_apacheclf,
  &mod_syslog,
  &mod_debug_line,
  NULL
};

static void show_mods() {
  int i = 0;
  fprintf(stderr, "Mods:\n\n");
  while( builtin_mods[i] ) {
    const logmod_t *mod = builtin_mods[i];
    if( ! mod->name ) {
      break;
    }
    fprintf(stderr, " - %s\n", mod->name);
    i++;
  }
  fprintf(stderr, "\n");
}

static void show_help(char *program) {
    fprintf(stderr, "Usage: %s <steps> ...\n\n", program);
    show_mods();
}

int main(int argc, char **argv) {  
  logstep_t *steps = steps_new(argc-1, (const char **)&argv[1]);
  if( ! steps ) {
    show_help(argv[0]);
    exit(EXIT_FAILURE);
  }

  str_t str;
  logline_t line;

  if( ! steps_init(steps, &str, &line) ) {
    show_help(argv[0]);
    exit(EXIT_FAILURE);
  }

  while( steps_run(steps, &str, &line) ) {
    // run is synchronous, it will process one message
    // by performaning each step in sequence.
  }

  steps_free(steps);
  line_free(&line);
  str_clear(&str);

  exit(EXIT_SUCCESS);
}
