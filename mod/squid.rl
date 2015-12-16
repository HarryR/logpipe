#include <time.h>

#include "mod.h"

#define PRINT_FIELD(field) { printf(" " #field ": %d \"%.*s\"\n", (int)line->field.len, (int)line->field.len, line->field.ptr); }

static void save_str(str_t *field, const char *data, int len) {
  if( len == 0 || (len == 1 && data[0] == '-') ) {
    return;
  }
  str_append(field, data, len);
}
#define SAVE_LINE_STR(field) { save_str(&line->field, (const char*)ts, p - ts); }

%%{
  machine parser_squid;

  alphtype unsigned char;

  # Registers the mark action, but delegates its implementation to the host 
  # language
  action mark { ts = p; }

  utc_timestamp_ms = ( [0-9]+ '.' [0-9]{3} )
            >mark %{
              SAVE_LINE_STR(timestamp)              
            };

  duration = [0-9]+
            >mark %{ SAVE_LINE_STR(duration) };

  client_ip = [0-9\.:]+
            >mark %{ SAVE_LINE_STR(client_ip) };

  result_code_cache = [A-Z_]+
          >mark %{ SAVE_LINE_STR(resp_cache) };

  result_code_status = [0-9]{3}
          >mark %{ SAVE_LINE_STR(resp_status) };

  result_code = (result_code_cache '/' result_code_status) ;  

  # The size is the amount of data delivered to the client.
  # Mind that this does not constitute the net object size,
  # as headers are also counted. Also, failed requests may
  # deliver an error page, the size of which is also logged here.
  bytes = [0-9]+
            >mark %{ SAVE_LINE_STR(resp_bytes) };

  # request method - e.g. GET POST, ICP_QUERY
  req_verb = [a-zA-Z:\-_]+
            >mark %{ SAVE_LINE_STR(req_verb) };

  # URL - This column contains the URL requested.
  req_path = [^ ]+
            >mark %{ SAVE_LINE_STR(req_path) };

  # user
  # The eighth column may contain the user identity for the
  # requesting client. This may be sourced from one of HTTP
  # authentication, an external ACL helper, TLS authentication,
  # or IDENT lookup (RFC 931) - checked in that order with the
  # first to present information displayed. If no user identity
  # is available a "-" will be logged.
  client_auth = [^ ]+
            >mark %{ SAVE_LINE_STR(client_auth) };

  heir_code = [^ ]+
            >mark %{ SAVE_LINE_STR(heir_code) };

  mime_type = [^ ]+
            >mark %{ SAVE_LINE_STR(mime_type) };

  # The squid log format consists of 10 fields
  # "%9d.%03d %6d %s %s/%03d %d %s %s %s %s%s/%s %s"
  line = (
    utc_timestamp_ms       space+
    duration    space+
    client_ip        space+
    result_code space+
    bytes space+ <:
    req_verb space+
    req_path space+
    client_auth space+
    heir_code space+
    mime_type
  );


  main := line %{
    str_ptime_epoch_secs(&line->timestamp, &line->utc_timestamp);

    return str_len(&line->timestamp)
        && str_len(&line->duration)
        && str_len(&line->client_ip)
        && str_len(&line->resp_cache)
        && str_len(&line->resp_status)
        && str_len(&line->resp_bytes)
        && str_len(&line->req_verb)
        && str_len(&line->req_path)
        && str_len(&line->heir_code);
  };

  write data;

}%%

static int parse_squid(void *ctx, str_t *str, logline_t *line) {
    int cs;
    unsigned char *p, *pe, *ts, *eof;
    line_free(line);
    line_init(line, str);

    p = str->ptr;
    eof = pe = str->ptr + str->len;
    ts = p;

    %% write init;
    %% write exec;

    return 1;
}

const logmod_t mod_parse_squid = {
  "parse.squid", NULL, parse_squid, NULL
};


static int print_squid(void *ctx, str_t *str, logline_t *line) {
  char timestamp[50];
  int have_timestmap = strftime(timestamp, sizeof(timestamp), "%s", &line->utc_timestamp) > 0;

  str_clear(str);

  // timestamp, UTC epoch + 3 digit milliseconds
  // XXX: 'struct tm' doesn't have millis, information loss!
  if( have_timestmap ) {
    str_append(str, timestamp, strlen(timestamp));
    str_append(str, ".000", 4);
  }
  else {
    str_append(str, "-", 1);
  }
  str_append(str, " ", 1);

  // duration
  if( ! str_len(&line->duration) ) {
    str_append(str, "-", 1);
  }
  else {
    str_append_str(str, &line->duration);
  }
  str_append(str, " ", 1);

  // client_ip
  if( ! str_len(&line->client_ip) ) {
    str_append(str, "-", 1);
  }
  else {
    str_append_str(str, &line->client_ip);
  }
  str_append(str, " ", 1);

  if( ! str_len(&line->resp_cache) ) {
    str_append(str, "-", 1);
  }
  else {
    str_append_str(str, &line->resp_cache);
  }
  str_append(str, "/", 1);
  if( ! str_len(&line->resp_status) ) {
    str_append(str, "-", 1);
  }
  else {
    str_append_str(str, &line->resp_status);
  }
  str_append(str, " ", 1);

  if( ! str_len(&line->resp_bytes) ) {
    str_append(str, "-", 1);
  }
  else {
    str_append_str(str, &line->resp_bytes);
  }
  str_append(str, " ", 1);

  if( ! str_len(&line->req_verb) ) {
    str_append(str, "-", 1);
  }
  else {
    str_append_str(str, &line->req_verb);
  }
  str_append(str, " ", 1);

  if( ! str_len(&line->req_path) ) {
    str_append(str, "-", 1);
  }
  else {
    str_append_str(str, &line->req_path);
  }
  str_append(str, " ", 1);

  if( ! str_len(&line->client_auth) ) {
    str_append(str, "-", 1);
  }
  else {
    str_append_str(str, &line->client_auth);
  }
  str_append(str, " ", 1);

  if( ! str_len(&line->heir_code) ) {
    str_append(str, "-", 1);
  }
  else {
    str_append_str(str, &line->heir_code);
  }
  str_append(str, " ", 1);

  if( ! str_len(&line->mime_type) ) {
    str_append(str, "-", 1);
  }
  else {
    str_append_str(str, &line->mime_type);
  }
  str_append(str, " ", 1);

  return 1;
}

const logmod_t mod_print_squid = {
  "print.squid", NULL, print_squid, NULL
};