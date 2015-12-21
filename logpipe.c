#include "logpipe.h"
#include "logpipe-module.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


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
    fprintf(stderr, "Usage: %s <step> [step ...]\n\n", program);
    show_mods();
}


static logpipe_t *logpipe = NULL;
static void do_cleanup (void) {
  logpipe_destroy(logpipe);
}


int main(int argc, char **argv) {
  if( argc < 2 ) {
    show_help(argv[0]);
    exit(EXIT_FAILURE);
  }

  logpipe = logpipe_new();
  int i;
  atexit(do_cleanup);
  for( i = 1; i < argc; i++ ) {
    if( logpipe_steps_add(logpipe, argv[i]) <= 0 ) {
      fprintf(stderr, "Error initialising module:\n  %s\n", argv[i]);
      exit(EXIT_FAILURE);
    }
  }

  logpipe_run_forever(logpipe);

  exit(EXIT_SUCCESS);
}
