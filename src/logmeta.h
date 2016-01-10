#ifndef LOGMETA_H_
#define LOGMETA_H_

#include "logpipe.h"
#include "str.h"

typedef struct {
	unsigned char md5[16];
    struct tm utc_timestamp;
	str_t fields[LOGPIPE_FIELDS_END];
} logmeta_t;

void logmeta_init(logmeta_t *meta);
void logmeta_clear(logmeta_t *meta);
void logmeta_hash(logmeta_t *meta, const str_t *buf);
void logmeta_field_clear(logmeta_t *meta, logpipe_field_t field);
void logmeta_field_set(logmeta_t *meta, logpipe_field_t field, const str_t *str);
struct tm *logmeta_timestamp(logmeta_t *meta);
str_t *logmeta_field(logmeta_t *meta, logpipe_field_t field);
int logmeta_field_isempty(logmeta_t *meta, logpipe_field_t field);
const char *logmeta_field_cstr(logmeta_t *meta, logpipe_field_t field);

#endif
