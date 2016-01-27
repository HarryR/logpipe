#include "logpipe-module.h"
#include "jsonutils.h"
#include "jv-utils.h"
#include "json.h"

static 
int jp_callback (void *ctx, const char *data, size_t len) {
    return str_append(ctx, data, len) > 0;
}

static int
print_clfjson(void *ctx, str_t *str, logmeta_t *meta) {
    json_printer jp;
    str_clear(str);
    json_print_init(&jp, jp_callback, str); 

    char timestamp[50];
    memset(timestamp, 0, sizeof(timestamp));
    strftime(timestamp, sizeof(timestamp), "%d/%b/%Y:%H:%M:%S %z", &meta->utc_timestamp);

    json_print_raw(&jp, JSON_ARRAY_BEGIN, NULL, 0);    
        print_strraw_or_null(&jp, logmeta_field(meta, LOGPIPE_C_IP));
        print_strraw_or_null(&jp, logmeta_field(meta, LOGPIPE_CS_IDENT));
        print_strraw_or_null(&jp, logmeta_field(meta, LOGPIPE_CS_USERNAME));
        json_print_raw(&jp, JSON_STRING, timestamp, strlen(timestamp));
        print_strraw_or_null(&jp, logmeta_field(meta, LOGPIPE_CS_METHOD));
        print_strraw_or_null(&jp, logmeta_field(meta, LOGPIPE_CS_URI_STEM));
        print_strraw_or_null(&jp, logmeta_field(meta, LOGPIPE_SC_STATUS));
        print_strraw_or_null(&jp, logmeta_field(meta, LOGPIPE_BYTES));
        if( ! logmeta_field_isempty(meta, LOGPIPE_CS_REFERER) ) {
            print_strraw_or_null(&jp, logmeta_field(meta, LOGPIPE_CS_REFERER));
            if( ! logmeta_field_isempty(meta, LOGPIPE_CS_USER_AGENT) ) {
                print_strraw_or_null(&jp, logmeta_field(meta, LOGPIPE_CS_USER_AGENT));
            }
        }
    json_print_raw(&jp, JSON_ARRAY_END, NULL, 0);    
    json_print_free(&jp);
    return 1;
}
const logmod_t mod_print_clfjson = {
    "print.clfjson", NULL, print_clfjson, NULL
};


typedef struct {
	int i;
	int finished;
	int errored;
	str_t *str;
	logmeta_t *meta;
} clfjson_state_t;

typedef struct {
	str_t *str;
	int token;
	int opt;
} clfjson_step_t;

static int
clfjson_state(void *_state, int type, const char *data, size_t length) {
	clfjson_state_t *state = _state;
	logmeta_t *meta = state->meta;
	if( state->finished || state->errored ) {
		return 1;
	}
	clfjson_step_t steps[] = {
		{NULL, JSON_ARRAY_BEGIN, 0},
		{logmeta_field(meta, LOGPIPE_C_IP), JSON_STRING, 0},
		{logmeta_field(meta, LOGPIPE_CS_IDENT), JSON_STRING, 0},
		{logmeta_field(meta, LOGPIPE_CS_USERNAME), JSON_STRING, 0},
		{logmeta_field(meta, LOGPIPE_TIMESTAMP), JSON_STRING, 0},
		{logmeta_field(meta, LOGPIPE_CS_METHOD), JSON_STRING, 0},
		{logmeta_field(meta, LOGPIPE_CS_URI_STEM), JSON_STRING, 0},
		{logmeta_field(meta, LOGPIPE_SC_STATUS), JSON_STRING, 0},
		{logmeta_field(meta, LOGPIPE_BYTES), JSON_STRING, 0},
		{logmeta_field(meta, LOGPIPE_CS_REFERER), JSON_STRING, 1},
		{logmeta_field(meta, LOGPIPE_CS_USER_AGENT), JSON_STRING, 1},
		{NULL, JSON_ARRAY_END, 0},
	};	
	clfjson_step_t *step = &steps[state->i];
	if( ! step->opt ) {
		// Got array end during optional string...
		if( state->i > 0 && type == JSON_ARRAY_END ) {			
			state->finished = 1;
			return 0;
		}
		if( type != step->token && type != JSON_NULL ) {
			state->errored = 1;
			state->i++;
			return 1;
		}
	}
	else {
		if( type == JSON_ARRAY_END ) {
			state->finished = 1;
			return 0;
		}
		if( step->token != type && type != JSON_NULL ) {
			state->errored = 1;
			state->i++;
			return 1;
		}		
	}
	if( step->str ) {
		// Append to raw buffer, separated by \0, save in ptr
		if( type == JSON_NULL ) {
			str_clear(step->str);
		}
		str_append(step->str, data, length);
	}
	state->i++;
	return 0;
}

static int
parse_clfjson(void *ctx, str_t *str, logmeta_t *meta) {
	clfjson_state_t state = {0, 0, 0, str, meta};
	json_parser parser;
	size_t processed = 0;
	logmeta_clear(meta);
	logmeta_hash(meta, str);
	json_parser_init(&parser, NULL, clfjson_state, &state);
	json_parser_string(&parser, (const char *)str->ptr, str->len, &processed);
	str_ptime_rfc1123(logmeta_field(meta, LOGPIPE_TIMESTAMP), &meta->utc_timestamp);
	json_parser_free(&parser);
	return ! logmeta_field_isempty(meta, LOGPIPE_C_IP)
		&& ! logmeta_field_isempty(meta, LOGPIPE_TIMESTAMP)
		&& ! logmeta_field_isempty(meta, LOGPIPE_CS_METHOD)
		&& ! logmeta_field_isempty(meta, LOGPIPE_CS_URI_STEM)
		&& ! logmeta_field_isempty(meta, LOGPIPE_SC_STATUS);
}

const logmod_t mod_parse_clfjson = {
	"parse.clfjson", NULL, parse_clfjson, NULL
};
