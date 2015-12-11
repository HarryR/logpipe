#include <stdio.h>
#include <string.h>

#include "mod.h"

#define MAX_LINE_LENGTH 8192

static int reset_str(void *ctx, str_t *str, logline_t *line) {
	if( str ) {
		str_clear(str);
	}
	return 1;
}
const logmod_t mod_reset_str = {
  "reset.str", NULL, reset_str, NULL
};


static int reset_line(void *ctx, str_t *str, logline_t *line) {
	if( line ) {		
		line_free(line);
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

#define PRINT_FIELD(field) { printf(" " #field ": %d \"%.*s\"\n", (int)line->field.len, (int)line->field.len, line->field.ptr); }
static int debug_line(void *ctx, str_t *str, logline_t *line) {
	printf("line %p\n", line);
	PRINT_FIELD(timestamp);
	PRINT_FIELD(client_ip);
	PRINT_FIELD(client_identity);
	PRINT_FIELD(client_auth);
	PRINT_FIELD(req_verb);
	PRINT_FIELD(req_path);
	PRINT_FIELD(req_ver);
	PRINT_FIELD(resp_status);
	PRINT_FIELD(resp_size);
	PRINT_FIELD(req_referrer);
	PRINT_FIELD(req_agent);
	PRINT_FIELD(duration);
	PRINT_FIELD(total_bytes);
	PRINT_FIELD(result_code);
	PRINT_FIELD(heir_code);
	PRINT_FIELD(mime_type);
	printf("\n");
	return 0;
}
#undef PRINT_FIELD
const logmod_t mod_debug_line = {
  "debug.line", NULL, debug_line, NULL
};