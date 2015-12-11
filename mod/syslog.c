#include "mod.h"

#include <syslog.h>

static int init_syslog(void *ctx, str_t *str, logline_t *line) {
	return 1;
}

static int run_syslog(void *ctx, str_t *str, logline_t *line) {
	syslog(LOG_ERR, "%.*", str->len, str->ptr);
	return 1;
}

const logmod_t mod_syslog = {
	"syslog", NULL, &run_syslog, NULL
};