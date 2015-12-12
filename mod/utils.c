#include <stdio.h>
#include <string.h>
#include <syslog.h>

#include "mod.h"

#define MAX_LINE_LENGTH 8192

static int run_syslog(void *ctx, str_t *str, logline_t *line) {
	syslog(LOG_ERR, "%.*s", (int)str->len, str->ptr);
	return 1;
}

const logmod_t mod_syslog = {
	"syslog", NULL, &run_syslog, NULL
};


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
	printf("\n");
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
	return 1;
}
#undef PRINT_FIELD
const logmod_t mod_debug_line = {
  "debug.line", NULL, debug_line, NULL
};

static int run_FILE_read(void *ctx, str_t *str, logline_t *line) {
	char buf[MAX_LINE_LENGTH];
	str_clear(str);
	if( ! fgets(buf, MAX_LINE_LENGTH, ctx) ) {
		return 0;
	}
	size_t len = strlen(buf);
	if( buf[len-1] == '\n' ) {
		buf[len-1] = 0;
	}
	str_append(str, buf, len);
	return 1;
}

static int init_FILE_stdin(void *ctx, str_t *str, logline_t *line) {
	*((FILE**)ctx) = stdin;
	return 1;
}
static int init_FILE_stdout(void *ctx, str_t *str, logline_t *line) {
	*((FILE**)ctx) = stdout;
	return 1;
}
static int init_FILE_stderr(void *ctx, str_t *str, logline_t *line) {
	*((FILE**)ctx) = stderr;
	return 1;
}

static int run_FILE_write(void *ctx, str_t *str, logline_t *line) {
	if( str && str->ptr && str->len ) {
		fwrite(str->ptr, str->len, 1, ctx);
		if( str->ptr[str->len] != '\n' ) {
			fwrite("\n", 1, 1, ctx);
		}
	}
	return 1;
}

static int stop_FILE(void *ctx, str_t *str, logline_t *line) {
	if( ctx ) {
		fclose(ctx);
	}
	return 1;
}

const logmod_t mod_stdin = {
	"stdin", &init_FILE_stdin, &run_FILE_read, &stop_FILE
};

const logmod_t mod_stderr = {
	"stderr", &init_FILE_stderr, &run_FILE_write, &stop_FILE
};

const logmod_t mod_stdout = {
	"stdout", &init_FILE_stdout, &run_FILE_write, &stop_FILE
};