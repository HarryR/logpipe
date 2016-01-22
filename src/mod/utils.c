#include <stdio.h>
#include <string.h>

#include "logpipe-module.h"

#define MAX_LINE_LENGTH 8192

static int reset_str(void *ctx, str_t *str, logmeta_t *meta) {
	if( str ) {
		str_clear(str);
	}
	return 1;
}
const logmod_t mod_reset_str = {
  "reset.str", NULL, reset_str, NULL
};


static int reset_line(void *ctx, str_t *str, logmeta_t *meta) {
	if( meta ) {		
		logmeta_clear(meta);
	}
	return 1;
}
const logmod_t mod_reset_line = {
  "reset.line", NULL, reset_line, NULL
};


static int reset_both(void *ctx, str_t *str, logmeta_t *meta) {
	return reset_line(ctx, str, meta) || reset_str(ctx, str, meta);
}
const logmod_t mod_reset_both = {
  "reset", NULL, reset_both, NULL
};

#define PRINT_FIELD(field) { printf(" " #field ": %d \"%.*s\"\n", (int)logmeta_field(meta, field)->len, (int)logmeta_field(meta, field)->len, logmeta_field(meta, field)->ptr); }
static int debug_line(void *ctx, str_t *str, logmeta_t *meta) {
	printf("\n");
	PRINT_FIELD(LOGPIPE_TIMESTAMP);
	PRINT_FIELD(LOGPIPE_C_IP);
	PRINT_FIELD(LOGPIPE_CS_IDENT);
	PRINT_FIELD(LOGPIPE_CS_USERNAME);
	PRINT_FIELD(LOGPIPE_CS_METHOD);
	PRINT_FIELD(LOGPIPE_CS_URI_STEM);
	PRINT_FIELD(LOGPIPE_CS_HTTP_VERSION);
	PRINT_FIELD(LOGPIPE_SC_STATUS);
	PRINT_FIELD(LOGPIPE_BYTES);
	PRINT_FIELD(LOGPIPE_CS_REFERER);
	PRINT_FIELD(LOGPIPE_CS_USER_AGENT);
	PRINT_FIELD(LOGPIPE_TIME_TAKEN);
	PRINT_FIELD(LOGPIPE_SC_CACHE);
	PRINT_FIELD(LOGPIPE_HIER_CODE);
	PRINT_FIELD(LOGPIPE_SC_CONTENT_TYPE);
	return 1;
}
#undef PRINT_FIELD
const logmod_t mod_debug_line = {
  "debug.line", NULL, debug_line, NULL
};

static int run_FILE_read(void *ctx, str_t *str, logmeta_t *meta) {
	char buf[MAX_LINE_LENGTH];
	str_clear(str);
	if( ! fgets(buf, MAX_LINE_LENGTH, ctx) ) {
		return -1;
	}
	size_t len = strlen(buf);
	if( buf[len-1] == '\n' ) {
		buf[len-1] = 0;
		len -= 1;
	}
	str_append(str, buf, len);
	return 1;
}

static int init_FILE_stdin(void **ctx, str_t *str, logmeta_t *meta) {
	*ctx = stdin;
	return 1;
}
static int init_FILE_stdout(void **ctx, str_t *str, logmeta_t *meta) {
	*ctx = stdout;
	return 1;
}
static int init_FILE_stderr(void **ctx, str_t *str, logmeta_t *meta) {
	*ctx = stderr;
	return 1;
}

static int run_FILE_write(void *ctx, str_t *str, logmeta_t *meta) {
	if( ! str_isempty(str) ) {
		if( fwrite(str_ptr(str), str_len(str), 1, ctx) != 1 ) {
			if( feof(ctx) ) {
				return -1;
			}
			// XXX: if we fail to write... halt the pipeline?
			return 0;
		}
		if( str_char(str, str_len(str)) != '\n' ) {
			fwrite("\n", 1, 1, ctx);
		}
	}
	return 1;
}

static int stop_FILE(void *ctx, str_t *str, logmeta_t *meta) {
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