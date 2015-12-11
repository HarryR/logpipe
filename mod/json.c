#include "mod.h"

#include "jv-utils.h"
#include "url.h"
#include "base64.h"

#include <assert.h>

static int json_print_key(json_printer *printer, const char *key) {
    return json_print_raw(printer, JSON_KEY, key, strlen(key));
}


static void print_keystr(json_printer *jp, char *key, str_t *str) {
    json_print_key(jp, key);
    json_print_raw(jp, JSON_STRING, (char*)str->ptr, str->len);
}


static void print_keystr2(json_printer *jp, char *key, char* str) {
    json_print_key(jp, key);
    json_print_raw(jp, JSON_STRING, str, strlen(str));
}


static void print_keyraw(json_printer *jp, char *key, str_t *str) {
    json_print_key(jp, key);
    json_print_raw(jp, JSON_INT, (char*)str->ptr, str->len);
}

void logline_print_extra(logopt_t *opt, json_printer *jp) {
		size_t i;
        /* Add extra fields to every record (specified via -a on cmdline) */
        for( i = 0; i < opt->extra_cnt; i++ ) {
            pair_t *pair = &opt->extra[i];
            print_keystr(jp, (char*)pair->key.ptr, &pair->val);
        }
}


void logline_print_id(logline_t *line, json_printer *jp, const char* key) {
    char b64[BASE64_SIZE(16)];
    base64_encode(b64, sizeof(b64), line->md5, 16);
    b64[22] = 0;
    json_print_key(jp, key);
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

static void hyperstats_print(void *ctx, str_t *str, logline_t *line) {
	json_printer jp;
	str_free(str);
	json_print_init(&jp, str_append, str); 
	json_print_raw(&jp, JSON_OBJECT_BEGIN, NULL, 0);

	// "_id": "pJW+IFHwFiWp/GH..."
	logline_print_id(line, &jp, "_id");

	// "facets":{
	json_print_key(&jp, "facets");
	json_print_raw(&jp, JSON_OBJECT_BEGIN, NULL, 0);
		//logline_print_extra(opt, &jp);

		// "timestamp": [2014, 03, 27, 23]
		json_print_key(&jp, "timestamp");
		json_print_raw(&jp, JSON_ARRAY_BEGIN, NULL, 0);
			char ts_year[5], ts_month[3], ts_day[3], ts_hour[3];
			strftime(ts_year, sizeof(ts_year), "%Y", &line->utc_timestamp);
			json_print_raw(&jp, JSON_INT, ts_year, 4);
			strftime(ts_month, sizeof(ts_month), "%m", &line->utc_timestamp);
			json_print_raw(&jp, JSON_INT, ts_month, 2);
			strftime(ts_day, sizeof(ts_day), "%d", &line->utc_timestamp);
			json_print_raw(&jp, JSON_INT, ts_day, 2);
			strftime(ts_hour, sizeof(ts_hour), "%H", &line->utc_timestamp);
			json_print_raw(&jp, JSON_INT, ts_hour, 2);
		json_print_raw(&jp, JSON_ARRAY_END, NULL, 0);

        print_keystr(&jp, "req_method", &line->req_verb);
        print_keystr(&jp, "req_version", &line->req_ver);
	
		// "req_path": ["/derp", "/test.php"]
		json_print_key(&jp, "req_path");
		logline_print_splitpath(&jp, (char*)line->req_path.ptr, line->req_path.len);

        php_url *url = php_url_parse_ex((char*)line->req_referrer.ptr, line->req_referrer.len);
		if( url != NULL ) {
			if( url->scheme ) print_keystr2(&jp, "ref_scheme", url->scheme);
            if( url->host) print_keystr2(&jp, "ref_host", url->host);
            if( url->path) {
				json_print_key(&jp, "ref_path");
				logline_print_splitpath(&jp, url->path, strlen(url->path));
			}
			php_url_free(url);
		}

        print_keyraw(&jp, "resp_status", &line->resp_status);
	
	json_print_raw(&jp, JSON_OBJECT_END, NULL, 0);
	// }

	// "values": {
	json_print_key(&jp, "values");
    json_print_raw(&jp, JSON_OBJECT_BEGIN, NULL, 0);
		json_print_key(&jp, "hits");
    	json_print_raw(&jp, JSON_INT, "1", 1);

    	if( line->resp_size.len ) {
    		print_keyraw(&jp, "bytes", &line->resp_size);
    	}
	json_print_raw(&jp, JSON_OBJECT_END, NULL, 0);
	// }

	json_print_raw(&jp, JSON_OBJECT_END, NULL, 0);
    json_print_free(&jp);
}
const logmod_t mod_print_hyperstats = {
	"print.hyperstats", NULL, hyperstats_print, NULL
};


static int
logstash_print(void *ctx, str_t *str, logline_t *line) {
    json_printer jp;
    str_free(str);
    json_print_init(&jp, str_append, str); 
    json_print_raw(&jp, JSON_OBJECT_BEGIN, NULL, 0);
    json_print_key(&jp, "index");

        json_print_raw(&jp, JSON_OBJECT_BEGIN, NULL, 0);

		logline_print_id(line, &jp, "_id");

		/*
        json_print_key(&jp, "_type");
        json_print_raw(&jp, JSON_STRING, opt->rowtype, strlen(opt->rowtype));
        */

        /*
        char index_name[0xff]; // XXX: how to know size of strftime result in advance?
        strftime(index_name, sizeof(index_name), opt->index_fmt, &line->utc_timestamp);
        json_print_key(&jp, "_index");
        json_print_raw(&jp, JSON_STRING, index_name, strlen(index_name));
        */

        /* Timestamp compatible with logstash
         * This format should be auto-detected by elasticsearch
		 */
        char timestamp[50];
        memset(timestamp, 0, sizeof(timestamp));
        strftime(timestamp, sizeof(timestamp), "logstash-%Y.%m.%d", &line->utc_timestamp);
        json_print_key(&jp, "_index");
        json_print_raw(&jp, JSON_STRING, timestamp, strlen(timestamp));

        memset(timestamp, 0, sizeof(timestamp));
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", &line->utc_timestamp);
        json_print_key(&jp, "@timestamp");
        json_print_raw(&jp, JSON_STRING, timestamp, strlen(timestamp));

        print_keystr(&jp, "client_ip", &line->client_ip);

        /* "req": {...} */
        json_print_key(&jp, "req");
            json_print_raw(&jp, JSON_OBJECT_BEGIN, NULL, 0);
            print_keystr(&jp, "verb", &line->req_verb);
            print_keystr(&jp, "path", &line->req_path);
            print_keystr(&jp, "ver", &line->req_ver);
            if( line->req_agent.len ) {
                print_keystr(&jp, "agent", &line->req_agent);
            }
            json_print_raw(&jp, JSON_OBJECT_END, NULL, 0);

        /* "referrer": {...} */
        if( line->req_referrer.len ) {
            json_print_key(&jp, "referrer");
                json_print_raw(&jp, JSON_OBJECT_BEGIN, NULL, 0);
                php_url *url = php_url_parse_ex((char*)line->req_referrer.ptr, line->req_referrer.len);
                if( url != NULL ) {
                    if( url->scheme ) print_keystr2(&jp, "scheme", url->scheme);
                    if( url->user) print_keystr2(&jp, "user", url->user);
                    if( url->pass) print_keystr2(&jp, "pass", url->pass);
                    if( url->host) print_keystr2(&jp, "host", url->host);
                    if( url->path) print_keystr2(&jp, "path", url->path);
                    if( url->query) print_keystr2(&jp, "query", url->query);
                    if( url->fragment) print_keystr2(&jp, "fragment", url->fragment);
                    php_url_free(url);
                }
                else {
                    print_keystr(&jp, "_raw", &line->req_referrer);
                }
                json_print_raw(&jp, JSON_OBJECT_END, NULL, 0);
        }

        /* "resp": {...} */
        json_print_key(&jp, "resp");
            json_print_raw(&jp, JSON_OBJECT_BEGIN, NULL, 0);
            print_keyraw(&jp, "status", &line->resp_status);
            if( line->resp_size.len ) {
                print_keyraw(&jp, "size", &line->resp_size);
            }
            json_print_raw(&jp, JSON_OBJECT_END, NULL, 0);

		//logline_print_extra(opt, &jp);

        json_print_raw(&jp, JSON_OBJECT_END, NULL, 0);

    json_print_raw(&jp, JSON_OBJECT_END, NULL, 0);
    json_print_free(&jp);
    return 1;
}
const logmod_t mod_print_logstash = {
	"print.logstash", NULL, logstash_print, NULL
};
