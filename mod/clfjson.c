#include "mod.h"
#include "jv-utils.h"
#include "json.h"

static 
int jp_callback (void *ctx, const char *data, uint32_t len) {
    return str_append(ctx, data, len) >= 0;
}

static int
print_clfjson(void *ctx, str_t *str, logline_t *line) {
    json_printer jp;
    str_clear(str);
    json_print_init(&jp, jp_callback, str); 

    char timestamp[50];
    memset(timestamp, 0, sizeof(timestamp));
    strftime(timestamp, sizeof(timestamp), "%d/%b/%Y:%H:%M:%S %z", &line->utc_timestamp);

    json_print_raw(&jp, JSON_ARRAY_BEGIN, NULL, 0);    
        print_strraw(&jp, &line->client_ip);
        print_strraw(&jp, &line->client_identity);
        print_strraw(&jp, &line->client_auth);
        json_print_raw(&jp, JSON_STRING, timestamp, strlen(timestamp));
        print_strraw(&jp, &line->req_verb);
        print_strraw(&jp, &line->req_path);
        print_strraw(&jp, &line->req_ver);
        print_strraw(&jp, &line->resp_status);
        print_strraw(&jp, &line->resp_size);
        if( line->req_referrer.len ) {
            print_strraw(&jp, &line->req_referrer);
            if( line->req_agent.len ) {
                print_strraw(&jp, &line->req_agent);
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
	logline_t *line;
} clfjson_state_t;

#define SAVE_LINE_STR(field) { line->field.ptr = data; line->field.length; }

typedef struct {
	str_t *str;
	int token;
	int opt;
} clfjson_step_t;

static int
clfjson_state(void *_state, int type, const char *data, uint32_t length) {
	clfjson_state_t *state = _state;
	logline_t *line = state->line;
	if( state->finished || state->errored ) {
		return 1;
	}
	clfjson_step_t steps[] = {
		{NULL, JSON_ARRAY_BEGIN, 0},
		{&line->client_ip, JSON_STRING, 0},
		{&line->client_identity, JSON_STRING, 0},
		{&line->client_auth, JSON_STRING, 0},
		{&line->timestamp, JSON_STRING, 0},
		{&line->req_verb, JSON_STRING, 0},
		{&line->req_path, JSON_STRING, 0},
		{&line->req_ver, JSON_STRING, 0},
		{&line->resp_status, JSON_STRING, 0},
		{&line->resp_size, JSON_STRING, 0},
		{&line->req_referrer, JSON_STRING, 1},
		{&line->req_agent, JSON_STRING, 1},
		{NULL, JSON_ARRAY_END, 0},
	};	
	clfjson_step_t *step = &steps[state->i];
	if( ! step->opt ) {
		// Got array end during optional string...
		if( state->i > 0 && type == JSON_ARRAY_END ) {			
			state->finished = 1;
			return 0;
		}
		if( type != step->token ) {
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
		if( step->token != type ) {
			state->errored = 1;
			state->i++;
			return 1;
		}		
	}
	if( step->str ) {
		// Append to raw buffer, separated by \0, save in ptr
		str_append(step->str, data, length);
	}
	state->i++;
	return 0;
}

static int
parse_clfjson(void *ctx, str_t *str, logline_t *line) {
	clfjson_state_t state = {0, 0, 0, str, line};
	json_parser parser;
	uint32_t processed = 0;
	line_free(line);
	line_init(line, str);
	json_parser_init(&parser, NULL, clfjson_state, &state);
	json_parser_string(&parser, (const char *)str->ptr, str->len, &processed);
	line_parse_timestamp_apacheclf(line);
	json_parser_free(&parser);
	return line->client_ip.len
        && line->timestamp.len
        && line->req_verb.len
        && line->req_path.len
        && line->req_ver.len
        && line->resp_status.len;
}

const logmod_t mod_parse_clfjson = {
	"parse.clfjson", NULL, parse_clfjson, NULL
};