#include <time.h>

#include "mod.h"

#define SAVE_LINE_STR(field) { str_append(&line->field, (const char*)ts, p - ts); }

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
            >mark %{ SAVE_LINE_STR(client_ip) };

  timestamp = [^\]]+
            >mark %{
              SAVE_LINE_STR(timestamp)
              line_parse_timestamp_apacheclf(line);
            };

  client_identity = [^ ]+ | '-'
            >mark %{ SAVE_LINE_STR(client_identity) };

  client_auth = [^ ]+ | '-'
            >mark %{ SAVE_LINE_STR(client_auth) };

  req_verb = [A-Z]+
            >mark %{ SAVE_LINE_STR(req_verb) };

  req_path = [^ ]+
            >mark %{ SAVE_LINE_STR(req_path) };

  req_ver  = [0-9\.]+
            >mark %{ SAVE_LINE_STR(req_ver) };

  resp_status  = digit+
            >mark %{ SAVE_LINE_STR(resp_status) };

  resp_size = (digit+ >mark %{ SAVE_LINE_STR(resp_size) } | '-' );

  req_referrer = escaped_char*
            >mark %{ SAVE_LINE_STR(req_referrer) };

  req_agent = escaped_char*
            >mark %{ SAVE_LINE_STR(req_agent) };

  # Assemble the components to define a single line
  line = (
    client_ip       space+
    client_identity    space+
    client_auth        space+
    '[' timestamp ']'    space+
    # GET /path HTTP/1.0
    '"'+ (space* req_verb space+) (req_path space+) ("HTTP/" req_ver space* '"') space+
    resp_status          space+
    resp_size            space+
    # either agent or both agent and referrer are optional
    # as per the Common Logging spec
    '"' req_referrer '"'     space+
    '"' req_agent '"'
    ( '\n' )?
  );

  main := line $err{
    if( line->client_ip.len
        && line->timestamp.len
        && line->req_verb.len
        && line->req_path.len
        && line->req_ver.len
        && line->resp_status.len) {
        return 1;
      }

    #if 0
    // Used when debugging the parser...
    fprintf(stderr, "ip:%d ts:%d vrb:%d pth:%d ver:%d sts:%d siz:%d ref:%d agt:%d\n",
        line->client_ip.len, 
        line->timestamp.len,
        line->req_verb.len,
        line->req_path.len,
        line->req_ver.len,
        line->resp_status.len,
        line->resp_size.len,
        line->req_referrer.len,
        line->req_agent.len
    );
    #endif
    return 0;
  };

  write data;

}%%

static int parse_apacheclf(void *ctx, str_t *str, logline_t *line) {
    int cs;
    unsigned char *p, *pe, *ts, *eof;
    line_free(line);
    line_init(line, NULL);

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


static int print_apacheclf(void *ctx, str_t *str, logline_t *line) {
  char timestamp[50];
  memset(timestamp, 0, sizeof(timestamp));
  strftime(timestamp, sizeof(timestamp), "%d/%b/%Y:%H:%M:%S %z", &line->utc_timestamp);

  str_clear(str);

  // ip
  if( ! line->client_ip.len ) {
    str_append(str, "-", 1);
  }
  else {
    str_append_str(str, &line->client_ip);
  }
  str_append(str, " ", 1);

  // ident
  if( ! line->client_identity.len ) {
    str_append(str, "-", 1);
  }
  else {
    str_append_str(str, &line->client_identity);
  }
  str_append(str, " ", 1);

  // auth
  if( ! line->client_auth.len ) {
    str_append(str, "-", 1);
  }
  else {
    str_append_str(str, &line->client_auth);
  }
  str_append(str, " ", 1);

  // timestamp
  str_append(str, "[", 1);
  str_append(str, timestamp, strlen(timestamp));
  str_append(str, "] ", 2);

  // "GET ... HTTP/1."
  str_append(str, "\"", 1);
  if( ! line->req_verb.len ) {
    str_append(str, "ERR", 1);
  }
  else {
    str_append_str(str, &line->req_verb);
  }
  str_append(str, " ", 1);

  // Path
  if( ! line->req_path.len ) {
    str_append(str, "?", 1);
  }
  else {
    str_append_str(str, &line->req_path);
  }
  str_append(str, " ", 1);

  // HTTP ver
  str_append(str, "HTTP/", 5);
  if( ! line->req_ver.len ) {
    str_append(str, "?.?", 3);
  }
  else {
    str_append_str(str, &line->req_ver);
  }
  str_append(str, "\" ", 2);

  if( ! line->resp_status.len ) {
    str_append(str, "-", 1);
  }
  else {
    str_append_str(str, &line->resp_status);
  }
  str_append(str, " ", 1);

  if( ! line->resp_size.len ) {
    str_append(str, "-", 1);
  }
  else {
    str_append_str(str, &line->resp_size);
  }
  str_append(str, " ", 1);

  if( line->req_referrer.len ) {
    str_append(str, "\"", 1);
    str_append_str(str, &line->req_referrer);
    str_append(str, "\"", 1);
    if( line->req_agent.len ) {
      str_append(str, " \"", 2);
      str_append_str(str, &line->req_agent);
      str_append(str, "\"", 1);
    }
  }

  return 1;
}

const logmod_t mod_print_apacheclf = {
  "print.apacheclf", NULL, print_apacheclf, NULL
};