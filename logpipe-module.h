#ifndef LOGPIPE_MODULES_H_
#define LOGPIPE_MODULES_H_

#include "config.h"
#include "src/str.h"
#include "src/logmeta.h"

typedef int (*logpipe_init_fn_t)(void **ctx, str_t *buf, logmeta_t *meta);
typedef int (*logpipe_run_fn_t)(void *ctx, str_t *buf, logmeta_t *meta);
typedef int (*logpipe_free_fn_t)(void *ctx, str_t *buf, logmeta_t *meta);

typedef struct {
    const char *name;
    logpipe_init_fn_t init_fn;
    logpipe_run_fn_t run_fn;
    logpipe_free_fn_t free_fn;
} logmod_t;

extern const logmod_t mod_stdin;
extern const logmod_t mod_stdout;
extern const logmod_t mod_stderr;

extern const logmod_t mod_reset_both;
extern const logmod_t mod_reset_line;
extern const logmod_t mod_reset_str;

extern const logmod_t mod_parse_apacheclf;
extern const logmod_t mod_print_apacheclf;

extern const logmod_t mod_parse_squid;
extern const logmod_t mod_print_squid;
extern const logmod_t mod_squid_logfile_daemon;

extern const logmod_t mod_print_hyperstats;
extern const logmod_t mod_print_logstash;
extern const logmod_t mod_parse_clfjson;
extern const logmod_t mod_print_clfjson;
extern const logmod_t mod_debug_line;
extern const logmod_t mod_debug_anon;
extern const logmod_t mod_debug_randblank;

#ifdef HAVE_SYSLOG
extern const logmod_t mod_syslog;
#endif

//extern const logmod_t mod_;

static const logmod_t *builtin_mods[] = {
  &mod_reset_str,
  &mod_reset_line,
  &mod_reset_both,
  &mod_stdin,
  &mod_stderr,
  &mod_stdout,
  &mod_parse_apacheclf,
  &mod_print_apacheclf,
  &mod_parse_squid,
  &mod_print_squid,
  &mod_squid_logfile_daemon,
  &mod_parse_clfjson,
  &mod_print_clfjson,
  &mod_print_logstash,
  &mod_print_hyperstats,
  &mod_debug_line,
  &mod_debug_anon,
  &mod_debug_randblank,

  #ifdef HAVE_SYSLOG
  &mod_syslog,
  #endif

  // &mod_...,
  0
};

#endif
