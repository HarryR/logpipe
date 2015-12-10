#include "config.h"
#include "parser.h"

#include <time.h>

#define SAVE_LINE_STR(field) { line->field.ptr = line->ts; line->field.len = line->p - line->ts; }

%%{
  machine parser_apacheclf;

  alphtype unsigned char;
  access line->;
  variable p line->p;
  variable pe line->pe;
  variable eof line->eof;

  # Registers the mark action, but delegates its implementation to the host 
  # language
  action mark { line->ts = line->p; }

  escaped_char = ( '\\x' xdigit{2} | '\\' ascii | [^"] ) ;

  # Define the various components of a log entry
  #
  # > and % denote entering and leaving actions respectively, so this machine
  #   will mark and emit each of the following token types as it sees them
  client_ip = [0-9\.]+
            >mark %{ SAVE_LINE_STR(client_ip) };

  timestamp = [^\]]+
            >mark %{ SAVE_LINE_STR(timestamp) };

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
    client_ip       space
    '-'             space
    '-'             space
    '[' timestamp ']'    space
    '"' req_verb space req_path space "HTTP/" req_ver '"' space
    resp_status          space
    resp_size            space
    '"' req_referrer '"'     space
    '"' req_agent '"'   '\n'
  );

  main := line $err{ return 0; };

  write data;

}%%

int logline_parse_apacheclf(logline_t *line) {
    %% write init;  
    %% write exec;
    logline_parse_timestamp_apacheclf(line);
    return 1;
}

