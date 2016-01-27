#include <time.h>
#include <string.h>

#include "logpipe-module.h"


static void save_str(logmeta_t *meta, logpipe_field_t field, const char *data, size_t len) {
  str_t *field_str = logmeta_field(meta, field);  
  if( len == 0 || (len == 1 && data[0] == '-') ) {
    return;
  }
  str_append(field_str, data, len);
}
#define SAVE_LINE_STR(field) { save_str(meta, field, (const char*)ts, (size_t)(p - ts)); }

%%{
  machine parser_apacheclf;

  alphtype unsigned char;

  # Registers the mark action, but delegates its implementation to the host 
  # language
  action mark { ts = p; }

  escaped_char = ( '\\x' xdigit{2} | '\\' ascii | [^"] ) ;

  # Define the various components of a log entry
  #
  # > and % denote entering and leaving actions respectively, so this machine
  #   will mark and emit each of the following token types as it sees them
  client_ip = [0-9\.:]+
            >mark %{ SAVE_LINE_STR(LOGPIPE_C_IP) };

  timestamp = [^\]]+
            >mark %{
              SAVE_LINE_STR(LOGPIPE_TIMESTAMP)              
              str_ptime_rfc1123(logmeta_field(meta, LOGPIPE_TIMESTAMP), &meta->utc_timestamp);
            };

  client_identity = '-' | [^ ]+
            >mark %{ SAVE_LINE_STR(LOGPIPE_CS_IDENT) };

  client_auth =  '-' | [^ ]+
            >mark %{ SAVE_LINE_STR(LOGPIPE_CS_USERNAME) };

  req_verb = [A-Z]+
            >mark %{ SAVE_LINE_STR(LOGPIPE_CS_METHOD) };

  req_path = [^ ]+
            >mark %{ SAVE_LINE_STR(LOGPIPE_CS_URI_STEM) };

  req_ver  = [0-9\.]+
            >mark %{ SAVE_LINE_STR(LOGPIPE_CS_HTTP_VERSION) };

  resp_status  = digit+
            >mark %{ SAVE_LINE_STR(LOGPIPE_SC_STATUS) };

  resp_size = (digit+ >mark %{ SAVE_LINE_STR(LOGPIPE_BYTES) } | '-' );

  req_referrer = escaped_char*
            >mark %{ SAVE_LINE_STR(LOGPIPE_CS_REFERER) };

  req_agent = escaped_char*
            >mark %{ SAVE_LINE_STR(LOGPIPE_CS_USER_AGENT) };

  # Assemble the components to define a single line
  line = (
    client_ip       space+
    client_identity    space+
    client_auth        space+ 
    '[' timestamp ']'    space+ <:
    # GET /path HTTP/1.0
    '"'+ (space* req_verb space+ (req_path . space+ "HTTP/") req_ver space* '"') space+ <:
    resp_status          space+
    resp_size            space+  <:
    # either agent or both agent and referrer are optional
    # as per the Common Logging spec
    '"' req_referrer '"'     space+
    '"' req_agent '"'
  );

  main := line $err{
    return ! logmeta_field_isempty(meta, LOGPIPE_C_IP)
        && ! logmeta_field_isempty(meta, LOGPIPE_TIMESTAMP)
        && ! logmeta_field_isempty(meta, LOGPIPE_CS_METHOD)
        && ! logmeta_field_isempty(meta, LOGPIPE_CS_URI_STEM)
        && ! logmeta_field_isempty(meta, LOGPIPE_CS_HTTP_VERSION)
        && ! logmeta_field_isempty(meta, LOGPIPE_SC_STATUS);
  };

  write data;

}%%

static int parse_apacheclf(void *ctx, str_t *str, logmeta_t *meta) {
    int cs;
    unsigned char *p, *pe, *ts, *eof;
    logmeta_clear(meta);
    logmeta_hash(meta, str);

    p = str->ptr;
    eof = pe = str->ptr + str->len;
    ts = p;

    %% write init;
    %% write exec;

    return 1;
}

const logmod_t mod_parse_apacheclf = {
  "parse.apacheclf", NULL, parse_apacheclf, NULL
};


static void print_field_or_dash(str_t *buf, logmeta_t *meta, logpipe_field_t field, const char *after) {
  str_t *field_str = logmeta_field(meta, field);
  if( str_isempty(field_str) ) {
    str_append(buf, "-", 1);
  }
  else {
    str_append_str(buf, field_str);
  }
  if( after ) {
    str_append(buf, after, 1);
  }
}


static int print_apacheclf(void *ctx, str_t *str, logmeta_t *meta) {
  char timestamp[50];
  memset(timestamp, 0, sizeof(timestamp));
  strftime(timestamp, sizeof(timestamp), "%d/%b/%Y:%H:%M:%S %z", &meta->utc_timestamp);

  str_clear(str);

  print_field_or_dash(str, meta, LOGPIPE_C_IP, " ");
  print_field_or_dash(str, meta, LOGPIPE_CS_IDENT, " ");
  print_field_or_dash(str, meta, LOGPIPE_CS_USERNAME, " ");

  // timestamp
  str_append(str, "[", 1);
  str_append(str, timestamp, strlen(timestamp));
  str_append(str, "] ", 2);

  // "GET ... HTTP/1."
  str_append(str, "\"", 1);
  print_field_or_dash(str, meta, LOGPIPE_CS_METHOD, " ");
  print_field_or_dash(str, meta, LOGPIPE_CS_URI_STEM, " ");
  str_append(str, "HTTP/", 5);
  print_field_or_dash(str, meta, LOGPIPE_CS_HTTP_VERSION, NULL);
  str_append(str, "\" ", 2);

  print_field_or_dash(str, meta, LOGPIPE_SC_STATUS, " ");
  print_field_or_dash(str, meta, LOGPIPE_BYTES, NULL);

  if( ! logmeta_field_isempty(meta, LOGPIPE_CS_REFERER) ) {
    str_append(str, " \"", 1);
    str_append_str(str, logmeta_field(meta, LOGPIPE_CS_REFERER));
    str_append(str, "\"", 1);
    if( ! logmeta_field_isempty(meta, LOGPIPE_CS_USER_AGENT) ) {
      str_append(str, " \"", 2);
      str_append_str(str, logmeta_field(meta, LOGPIPE_CS_USER_AGENT));
      str_append(str, "\"", 1);
    }
  }

  return 1;
}

const logmod_t mod_print_apacheclf = {
  "print.apacheclf", NULL, print_apacheclf, NULL
};
