#ifndef MOD_H_
#define MOD_H_

#include "config.h"
#include "log.h"
#include "jsonutils.h"
#include "url.h"

extern const logmod_t *builtin_mods[];

extern const logmod_t mod_stdin;
extern const logmod_t mod_stdout;
extern const logmod_t mod_stderr;

extern const logmod_t mod_reset_both;
extern const logmod_t mod_reset_line;
extern const logmod_t mod_reset_str;

extern const logmod_t mod_parse_apacheclf;
extern const logmod_t mod_print_apacheclf;

extern const logmod_t mod_print_hyperstats;
extern const logmod_t mod_print_logstash;
extern const logmod_t mod_parse_clfjson;
extern const logmod_t mod_syslog;
extern const logmod_t mod_print_clfjson;
extern const logmod_t mod_debug_line;
//extern const logmod_t mod_;

#endif