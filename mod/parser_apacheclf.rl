#include <time.h>

#include "mod.h"

#define SAVE_LINE_STR(field) { line->field.ptr = ts; line->field.len = p - ts; }

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
              logline_parse_timestamp_apacheclf(line);
            };

  client_identity = [^ ]+ | '-'
            >mark %{ SAVE_LINE_STR(client_identity) };

  client_auth = [^ ]+ | '-'
            >mark %{ SAVE_LINE_STR(client_auth) };

  req_verb = [A-Z]+
            >mark %{ SAVE_LINE_STR(req_verb) };

  req_path = any*
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
    '"'+ space* req_verb space+ req_path space+ "HTTP/" req_ver+ space* '"' space+
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

    line_init(line, str);
    p = line->raw.ptr;
    eof = pe = line->raw.ptr + line->raw.len;
    ts = p;
    %% write init;
    %% write exec;    
    return 1;
}

const logmod_t mod_parse_apacheclf = {
  "parse.apacheclf", NULL, parse_apacheclf, NULL
};

