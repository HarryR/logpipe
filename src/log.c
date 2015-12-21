#include "logpipe-module.h"
#include "md5.h"
#include <string.h>


void line_free(logline_t *line) {
  if( line ) {
    // Common Log Format options
    str_clear(&line->timestamp);
    str_clear(&line->client_ip);
    str_clear(&line->client_identity);
    str_clear(&line->client_auth);
    str_clear(&line->req_verb);
    str_clear(&line->req_path);
    str_clear(&line->req_ver);
    str_clear(&line->resp_status);
    str_clear(&line->resp_size);
    str_clear(&line->req_referrer);
    str_clear(&line->req_agent);

    // Squid options 
    str_clear(&line->duration);
    str_clear(&line->resp_cache);
    str_clear(&line->hier_code);
    str_clear(&line->mime_type);
  }
}

void line_init(logline_t *line, str_t *str) {
  memset(line, 0, sizeof(*line));
  if( str && str->ptr ) {
    md5_state_t ctx;
    md5_init(&ctx);
    md5_append(&ctx, str->ptr, str->len);
    md5_finish(&ctx, line->md5);
  }
}
