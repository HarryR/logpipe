#include "mod.h"

#include "json.h"
#include "jv-utils.h"
#include "url.h"
#include "base64.h"

#include <assert.h>

/*
static void logline_print_extra(logopt_t *opt, json_printer *jp) {
		size_t i;
        // Add extra fields to every record (specified via -a on cmdline)
        for( i = 0; i < opt->extra_cnt; i++ ) {
            pair_t *pair = &opt->extra[i];
            print_keystr(jp, (char*)pair->key.ptr, &pair->val);
        }
}
*/

static 
int jp_callback (void *ctx, const char *data, uint32_t len) {
    return str_append(ctx, data, len) >= 0;
}

static int hyperstats_print(void *ctx, str_t *str, logline_t *line) {
	json_printer jp;
	str_clear(str);
	json_print_init(&jp, jp_callback, str); 
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

		if( line->req_verb.len ) {
        	print_keystr(&jp, "req_method", &line->req_verb);
		}
		if( line->req_ver.len ) {
        	print_keystr(&jp, "req_version", &line->req_ver);
		}
		if( line->resp_status.len ) {
        	print_keyraw(&jp, "resp_status", &line->resp_status);
		}
	
		if( line->req_path.len ) {			
			// "req_path": ["/derp", "/test.php"]
			json_print_key(&jp, "req_path");
			json_print_splitpath(&jp, (char*)line->req_path.ptr, line->req_path.len);
		}

		if( line->req_referrer.len ) {			
	        php_url *url = php_url_parse_ex((char*)line->req_referrer.ptr, line->req_referrer.len);
			if( url != NULL ) {
				if( url->scheme ) print_keystr2(&jp, "ref_scheme", url->scheme);
	            if( url->host) print_keystr2(&jp, "ref_host", url->host);
	            if( url->path) {
					json_print_key(&jp, "ref_path");
					json_print_splitpath(&jp, url->path, strlen(url->path));
				}
				php_url_free(url);
			}
		}
	
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
    return 1;
}
const logmod_t mod_print_hyperstats = {
	"print.hyperstats", NULL, hyperstats_print, NULL
};


