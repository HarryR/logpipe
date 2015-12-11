#include <stdio.h>

#include "mod.h"

#define MAX_LINE_LENGTH 8192

static int run_FILE_read(FILE *ctx, str_t *str, logline_t *line) {
	char buf[MAX_LINE_LENGTH];
	str_free(str);
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

static int init_FILE_stdin(FILE **ctx, str_t *str, logline_t *line) {
	*ctx = stdin;
	return 1;
}
static int init_FILE_stdout(FILE **ctx, str_t *str, logline_t *line) {
	*ctx = stdout;
	return 1;
}
static int init_FILE_stderr(FILE **ctx, str_t *str, logline_t *line) {
	*ctx = stderr;
	return 1;
}

static int run_FILE_write(FILE *ctx, str_t *str, logline_t *line) {
	if( str && str->ptr && str->len ) {
		fwrite(str->ptr, str->len, 1, ctx);
		if( str->ptr[str->len-1] == '\n' ) {
			str->ptr[str->len-1] = 0;
		}
		fwrite("\n", 1, 1, ctx);
	}
	return 1;
}

static int stop_FILE(FILE *ctx, str_t *str, logline_t *line) {
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