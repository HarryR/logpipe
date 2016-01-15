#include "logpipe-module.h"
#include "jsonutils.h"
#include "url.h"

static 
int jp_callback (void *ctx, const char *data, uint32_t len) {
    return str_append(ctx, data, len) >= 0;
}

static int
logstash_print(void *ctx, str_t *str, logmeta_t *meta) {
    json_printer jp;
    str_clear(str);
    json_print_init(&jp, jp_callback, str); 
    json_print_raw(&jp, JSON_OBJECT_BEGIN, NULL, 0);
    json_print_key(&jp, "index");

        json_print_raw(&jp, JSON_OBJECT_BEGIN, NULL, 0);

		logline_print_id(meta, &jp, "_id");
        print_keystr2(&jp, "_type", "logstash");

        /* Timestamp compatible with logstash
         * This format should be auto-detected by elasticsearch
		 */
        char timestamp[50];
        memset(timestamp, 0, sizeof(timestamp));
        strftime(timestamp, sizeof(timestamp), "logstash-%Y.%m.%d", &meta->utc_timestamp);
        json_print_key(&jp, "_index");
        json_print_raw(&jp, JSON_STRING, timestamp, strlen(timestamp));

        memset(timestamp, 0, sizeof(timestamp));
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", &meta->utc_timestamp);
        json_print_key(&jp, "@timestamp");
        json_print_raw(&jp, JSON_STRING, timestamp, strlen(timestamp));

        print_optkeystr(&jp, "client_ip", logmeta_field(meta, LOGPIPE_C_IP));

        /* "req": {...} */
        json_print_key(&jp, "req");
            json_print_raw(&jp, JSON_OBJECT_BEGIN, NULL, 0);
            print_optkeystr(&jp, "verb", logmeta_field(meta, LOGPIPE_CS_METHOD));
            print_optkeystr(&jp, "path", logmeta_field(meta, LOGPIPE_CS_URI_STEM));
            print_optkeystr(&jp, "agent", logmeta_field(meta, LOGPIPE_CS_USER_AGENT));
            json_print_raw(&jp, JSON_OBJECT_END, NULL, 0);

        /* "referrer": {...} */
        str_t *referrer = logmeta_field(meta, LOGPIPE_CS_REFERER);
        if( str_len(referrer) ) {
            json_print_key(&jp, "referrer");
                json_print_raw(&jp, JSON_OBJECT_BEGIN, NULL, 0);
                php_url *url = php_url_parse_ex((char*)referrer->ptr, referrer->len);
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
                    print_keystr(&jp, "_raw", referrer);
                }
                json_print_raw(&jp, JSON_OBJECT_END, NULL, 0);
        }

        /* "resp": {...} */
        json_print_key(&jp, "resp");
            json_print_raw(&jp, JSON_OBJECT_BEGIN, NULL, 0);
            print_keyraw(&jp, "status", logmeta_field(meta, LOGPIPE_SC_STATUS));
            str_t *resp_size = logmeta_field(meta, LOGPIPE_BYTES);
            if( str_len(resp_size) ) {
                print_keyraw(&jp, "size", resp_size);
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
