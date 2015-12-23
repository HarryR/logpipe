#include "logmeta.h"
#include "md5.h"
#include <assert.h>
#include <string.h>

void logmeta_init(logmeta_t *meta) {
	memset(meta, 0, sizeof(*meta));
}

void logmeta_clear(logmeta_t *meta) {
	int i;
	for( i = 0; i < LOGPIPE_FIELDS_END; i++ ) {
		logmeta_field_clear(meta, i);
	}
	memset(meta, 0, sizeof(*meta));
}

void logmeta_hash(logmeta_t *meta, str_t *str) {	
	memset(meta->md5, 0, sizeof(meta->md5));
	if( str && str->ptr ) {
    	md5_state_t ctx;
    	md5_init(&ctx);
    	md5_append(&ctx, str->ptr, str->len);
    	md5_finish(&ctx, meta->md5);
	}
}

str_t *logmeta_field(logmeta_t *meta, logpipe_field_t field) {
	assert( field >= 0 );
	assert( field < LOGPIPE_FIELDS_END );
	return &meta->fields[field];
}

void logmeta_field_clear(logmeta_t *meta, logpipe_field_t field) {
	str_t *field_str = logmeta_field(meta, field);
	str_clear(field_str);
}

void logmeta_field_set(logmeta_t *meta, logpipe_field_t field, str_t *str) {
	str_t *field_str = logmeta_field(meta, field);
	str_clear(field_str);
	str_append_str(field_str, str);
}

int logmeta_field_isempty(logmeta_t *meta, logpipe_field_t field) {
	str_t *str = logmeta_field(meta, field);
	return str_isempty(str);
}

struct tm *logmeta_timestamp(logmeta_t *meta) {
	return &meta->utc_timestamp;
}