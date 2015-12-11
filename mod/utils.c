#include <stdio.h>

#include "mod.h"

#define MAX_LINE_LENGTH 8192

static int reset_str(void *ctx, str_t *str, logline_t *line) {
	if( str ) {
		str_free(str);
	}
	return 1;
}
const logmod_t mod_reset_str = {
  "reset.str", NULL, reset_str, NULL
};


static int reset_line(void *ctx, str_t *str, logline_t *line) {
	if( line ) {		
		str_free(&line->raw);
		memset(line, 0, sizeof(*line));
	}
	return 1;
}
const logmod_t mod_reset_line = {
  "reset.line", NULL, reset_line, NULL
};


static int reset_both(void *ctx, str_t *str, logline_t *line) {
	return reset_line(ctx, str, line) || reset_str(ctx, str, line);
}
const logmod_t mod_reset_both = {
  "reset", NULL, reset_both, NULL
};