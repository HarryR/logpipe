#include "mod.h"
#include "base64.h"

#include <assert.h>

int json_print_key(json_printer *printer, const char *key) {
    return json_print_raw(printer, JSON_KEY, key, strlen(key));
}

void print_keystr(json_printer *jp, char *key, str_t *str) {
    json_print_key(jp, key);
    json_print_raw(jp, JSON_STRING, (char*)str->ptr, str->len);
}


void print_keystr2(json_printer *jp, char *key, char* str) {
    json_print_key(jp, key);
    json_print_raw(jp, JSON_STRING, str, strlen(str));
}


void print_keyraw(json_printer *jp, char *key, str_t *str) {
    json_print_key(jp, key);
    json_print_raw(jp, JSON_INT, (char*)str->ptr, str->len);
}

void print_strraw(json_printer *jp, str_t *str) {
    json_print_raw(jp, JSON_STRING, (char*)str->ptr, str->len);
}


void logline_print_id(logline_t *line, json_printer *jp, const char* key) {
    char b64[BASE64_SIZE(16)];
    base64_encode(b64, sizeof(b64), line->md5, 16);
    b64[22] = 0;
    if( key ) {
        json_print_key(jp, key);
    }
    json_print_raw(jp, JSON_STRING, b64, 22);
}


/**
 * Outputs a filesystem path as a JSON array
 *
 * Example:
 *   /derp/123.txt = ["/derp","/123.txt"]
 *   / = ["/"]
 */
void logline_print_splitpath(json_printer *jp, char *path, size_t len) {
	assert( path != NULL );
	assert( len > 0 );
	assert( jp != NULL );

	int is_first = 1;

	json_print_raw(jp, JSON_ARRAY_BEGIN, NULL, 0);
	while (len > 0) {
		char *slash = memchr(path + 1, '/', len - 1);	
		if( slash ) {
			size_t slash_ofs = slash - path;
			// Ignore double slashes
			if( slash_ofs > 1 ) {
				json_print_raw(jp, JSON_STRING, path, slash_ofs);
			}
			path = slash;
			len -= slash_ofs;
		}
		else {
			// Allow "/" if it's the first component, ignore it if it's the last 
			if( is_first || (path[0] != '/' && len == 1) ) {
				json_print_raw(jp, JSON_STRING, path, len);
			}
			len -= len;
		}
		is_first = 0;
	}
	json_print_raw(jp, JSON_ARRAY_END, NULL, 0);	
}