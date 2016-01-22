#include <syslog.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "logpipe-module.h"
#include "querystring.h"
#include "url.h"


typedef struct {
	int level;
	str_t ident;
} modsyslog_ctx_t;


/**
 * Parses a comma separated list of syslog options
 */
static int parse_option(str_t *str) {
	int logopt = 0;
	unsigned char *ptr = str_ptr(str);
	size_t len = str_len(str);
	while( len ) {
		str_t prev = {ptr, len};
		unsigned char *next = str_pos(&prev, ',');
		str_t current = {ptr, next == NULL ? len : (size_t)(next - ptr)};

		if( str_caseeq_cstr(&current, "cons") ) {
			logopt |= LOG_CONS;
		}
		if( str_caseeq_cstr(&current, "ndelay") ) {
			logopt |= LOG_NDELAY;
		}
		if( str_caseeq_cstr(&current, "odelay") ) {
			logopt |= LOG_ODELAY;
		}
		if( str_caseeq_cstr(&current, "perror") ) {
			logopt |= LOG_PERROR;
		}
		if( str_caseeq_cstr(&current, "pid") ) {
			logopt |= LOG_PID;
		}

		if( next == NULL ) {
			break;
		}

		len -= (current.len + 1);
		ptr = &next[1];
	}
	return logopt;
}


static int parse_facility(str_t *str) {
	if( str ) {		
		if( str_caseeq_cstr(str, "auth") ) return LOG_AUTH;
		if( str_caseeq_cstr(str, "authpriv") ) return LOG_AUTHPRIV;
		if( str_caseeq_cstr(str, "cron") ) return LOG_CRON;
		if( str_caseeq_cstr(str, "daemon") ) return LOG_DAEMON;
		if( str_caseeq_cstr(str, "ftp") ) return LOG_FTP;
		if( str_caseeq_cstr(str, "kern") ) return LOG_KERN;
		if( str_caseeq_cstr(str, "local0") ) return LOG_LOCAL0;
		if( str_caseeq_cstr(str, "local1") ) return LOG_LOCAL1;
		if( str_caseeq_cstr(str, "local2") ) return LOG_LOCAL2;
		if( str_caseeq_cstr(str, "local3") ) return LOG_LOCAL3;
		if( str_caseeq_cstr(str, "local4") ) return LOG_LOCAL4;
		if( str_caseeq_cstr(str, "local5") ) return LOG_LOCAL5;
		if( str_caseeq_cstr(str, "local6") ) return LOG_LOCAL6;
		if( str_caseeq_cstr(str, "local7") ) return LOG_LOCAL7;
		if( str_caseeq_cstr(str, "lpr") ) return LOG_LPR;
		if( str_caseeq_cstr(str, "mail") ) return LOG_MAIL;
		if( str_caseeq_cstr(str, "news") ) return LOG_NEWS;
		if( str_caseeq_cstr(str, "syslog") ) return LOG_SYSLOG;
		if( str_caseeq_cstr(str, "uucp") ) return LOG_UUCP;
	}
	return LOG_LOCAL0;
}


static int parse_level(str_t *str) {
	if( str ) {
		if( str_caseeq_cstr(str, "emerg") ) return LOG_EMERG;
		if( str_caseeq_cstr(str, "emergency") ) return LOG_EMERG;
		if( str_caseeq_cstr(str, "alert") ) return LOG_ALERT;
		if( str_caseeq_cstr(str, "crit") ) return LOG_CRIT;
		if( str_caseeq_cstr(str, "critical") ) return LOG_CRIT;
		if( str_caseeq_cstr(str, "err") ) return LOG_ERR;
		if( str_caseeq_cstr(str, "error") ) return LOG_ERR;
		if( str_caseeq_cstr(str, "warning") ) return LOG_WARNING;
		if( str_caseeq_cstr(str, "warn") ) return LOG_WARNING;
		if( str_caseeq_cstr(str, "notice") ) return LOG_NOTICE;
		if( str_caseeq_cstr(str, "info") ) return LOG_INFO;
		if( str_caseeq_cstr(str, "debug") ) return LOG_DEBUG;
	}
	return LOG_INFO;
}


static int start_syslog(void **ctx, str_t *str, logmeta_t *meta) {
	modsyslog_ctx_t *new_ctx = malloc(sizeof(modsyslog_ctx_t));
	new_ctx->level = LOG_INFO;
	str_init(&new_ctx->ident);
	*ctx = new_ctx;
	if( ! str ) {
		return 1;
	}

	php_url *url = php_url_parse((const char*)str->ptr);
	if( ! url ) {
		return 1;
	}

	if( url->query ) {
		str_t str = {(unsigned char *)url->query, strlen(url->query)};
		pair_t *query = querystring_parse(&str);
		if( query ) {
			// Sensible defaults
			int logopt = LOG_NDELAY;
			int facility = LOG_LOCAL0;			

			pair_t *facility_pair = strpair_bykey_cstr(query, "facility");
			if( facility_pair ) {
				facility = parse_facility(&facility_pair->val);
			}

			pair_t *level_pair = strpair_bykey_cstr(query, "level");
			if( level_pair ) {
				new_ctx->level = parse_level(&level_pair->val);
			}

			pair_t *ident_pair = strpair_bykey_cstr(query, "ident");
			if( ident_pair ) {
				new_ctx->ident = str_clone(&ident_pair->val);
			}

			pair_t *option_pair = strpair_bykey_cstr(query, "option");
			if( option_pair ) {
				logopt = parse_option(&option_pair->val);
			}

			strpair_clear(query);
			openlog((char*)str_ptr(&new_ctx->ident), logopt, facility);
		}
	}
	php_url_free(url);
	return 1;
}


static int stop_syslog(void *_ctx, str_t *str, logmeta_t *meta) {
	modsyslog_ctx_t *ctx = _ctx;
	closelog();
	str_clear(&ctx->ident);
	free(ctx);
	return 1;
}


static int run_syslog(void *_ctx, str_t *str, logmeta_t *meta) {
	modsyslog_ctx_t *ctx = _ctx;
	if( ! str_len(str) ) {
		return 0;
	}
	syslog(ctx->level, "%.*s", (int)str_len(str), str_ptr(str));
	return 1;
}


const logmod_t mod_syslog = {
	"syslog", &start_syslog, &run_syslog, &stop_syslog
};
