#include "mod.h"

static 
int jp_callback (void *ctx, const char *data, uint32_t len) {
    return str_append(ctx, data, len) >= 0;
}

static int
logstash_print(void *ctx, str_t *str, logline_t *line) {
    json_printer jp;
    str_clear(str);
    json_print_init(&jp, jp_callback, str); 
    json_print_raw(&jp, JSON_OBJECT_BEGIN, NULL, 0);
    json_print_key(&jp, "index");

        json_print_raw(&jp, JSON_OBJECT_BEGIN, NULL, 0);

		logline_print_id(line, &jp, "_id");
        print_keystr2(&jp, "_type", "logstash");

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

        print_optkeystr(&jp, "client_ip", &line->client_ip);

        /* "req": {...} */
        json_print_key(&jp, "req");
            json_print_raw(&jp, JSON_OBJECT_BEGIN, NULL, 0);
            print_optkeystr(&jp, "verb", &line->req_verb);
            print_optkeystr(&jp, "path", &line->req_path);
            print_optkeystr(&jp, "ver", &line->req_ver);
            print_optkeystr(&jp, "agent", &line->req_agent);            
            json_print_raw(&jp, JSON_OBJECT_END, NULL, 0);

        /* "referrer": {...} */
        if( line->req_referrer.len ) {
            json_print_key(&jp, "referrer");
                json_print_raw(&jp, JSON_OBJECT_BEGIN, NULL, 0);
                php_url *url = php_url_parse_ex((char*)line->req_referrer.ptr, line->req_referrer.len);
                if( url != NULL ) {
                    print_optkeystr2(&jp, "scheme", url->scheme);
                    print_optkeystr2(&jp, "user", url->user);
                    print_optkeystr2(&jp, "pass", url->pass);
                    print_optkeystr2(&jp, "host", url->host);
                    print_optkeystr2(&jp, "path", url->path);
                    print_optkeystr2(&jp, "query", url->query);
                    print_optkeystr2(&jp, "fragment", url->fragment);
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

        json_print_raw(&jp, JSON_OBJECT_END, NULL, 0);

    json_print_raw(&jp, JSON_OBJECT_END, NULL, 0);
    json_print_free(&jp);
    return 1;
}
const logmod_t mod_print_logstash = {
	"print.logstash", NULL, logstash_print, NULL
};
