#include "logpipe-module.h"
#include "jsonutils.h"
#include "jv-utils.h"
#include "url.h"
#include "base64.h"

#include <assert.h>

static 
int jp_callback (void *ctx, const char *data, size_t len) {
    return str_append(ctx, data, len) > 0;
}

static int hyperstats_print(void *ctx, str_t *str, logmeta_t *meta) {
	json_printer jp;
	str_clear(str);
	json_print_init(&jp, jp_callback, str); 
	json_print_raw(&jp, JSON_OBJECT_BEGIN, NULL, 0);

	// "_id": "pJW+IFHwFiWp/GH..."
	logline_print_id(meta, &jp, "_id");

	// "facets":{
	json_print_key(&jp, "facets");
	json_print_raw(&jp, JSON_OBJECT_BEGIN, NULL, 0);
		//logline_print_extra(opt, &jp);

		// "timestamp": [2014, 03, 27, 23]
		json_print_key(&jp, "timestamp");
		json_print_raw(&jp, JSON_ARRAY_BEGIN, NULL, 0);
			char ts_year[5], ts_month[3], ts_day[3], ts_hour[3];
			strftime(ts_year, sizeof(ts_year), "%Y", logmeta_timestamp(meta));
			json_print_raw(&jp, JSON_INT, ts_year, 4);
			strftime(ts_month, sizeof(ts_month), "%m", logmeta_timestamp(meta));
			json_print_raw(&jp, JSON_INT, ts_month, 2);
			strftime(ts_day, sizeof(ts_day), "%d", logmeta_timestamp(meta));
			json_print_raw(&jp, JSON_INT, ts_day, 2);
			strftime(ts_hour, sizeof(ts_hour), "%H", logmeta_timestamp(meta));
			json_print_raw(&jp, JSON_INT, ts_hour, 2);
		json_print_raw(&jp, JSON_ARRAY_END, NULL, 0);

		if( ! logmeta_field_isempty(meta, LOGPIPE_CS_METHOD) ) {
        	print_keystr(&jp, "req_method", logmeta_field(meta, LOGPIPE_CS_METHOD));
		}
		if( ! logmeta_field_isempty(meta, LOGPIPE_SC_STATUS) ) {
        	print_keystr(&jp, "resp_status", logmeta_field(meta, LOGPIPE_SC_STATUS));
		}
	
		str_t *req_path = logmeta_field(meta, LOGPIPE_CS_URI_STEM);
		if( req_path->len ) {			
			// "req_path": ["/derp", "/test.php"]
			json_print_key(&jp, "req_path");
			json_print_splitpath(&jp, (char*)req_path->ptr, req_path->len);
		}

		str_t *req_referrer = logmeta_field(meta, LOGPIPE_CS_REFERER);
		if( req_referrer->len ) {			
	        php_url *url = php_url_parse_ex((char*)req_referrer->ptr, req_referrer->len);
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

    	if( ! logmeta_field_isempty(meta, LOGPIPE_BYTES) ) {
    		print_keyraw(&jp, "bytes", logmeta_field(meta, LOGPIPE_BYTES));
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


