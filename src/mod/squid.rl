#include <time.h>
#include <string.h>

#include "logpipe-module.h"

#define PRINT_FIELD(field) { printf(" " #field ": %d \"%.*s\"\n", (int)logmeta_field(field)->len, (int)logmeta_field(field)->len, logmeta_field(field)->ptr); }

static void save_str(logmeta_t *meta, logpipe_field_t field, const char *data, int len) {
  str_t *field_str = logmeta_field(meta, field);
  if( len == 0 || (len == 1 && data[0] == '-') ) {
    return;
  }
  str_append(field_str, data, len);
}
#define SAVE_LINE_STR(field) { save_str(meta, field, (const char*)ts, p - ts); }

%%{
  machine parser_squid;

  alphtype unsigned char;

  # Registers the mark action, but delegates its implementation to the host 
  # language
  action mark { ts = p; }

  utc_timestamp_ms = ( [0-9]+ '.' [0-9]{3} )
            >mark %{
              SAVE_LINE_STR(LOGPIPE_TIMESTAMP)              
            };

  duration = [0-9]+
            >mark %{ SAVE_LINE_STR(LOGPIPE_TIME_TAKEN) };

  client_ip = [0-9\.:]+
            >mark %{ SAVE_LINE_STR(LOGPIPE_C_IP) };

  result_code_cache = [A-Z_]+
          >mark %{ SAVE_LINE_STR(LOGPIPE_SC_CACHE) };

  result_code_status = [0-9]{3}
          >mark %{ SAVE_LINE_STR(LOGPIPE_SC_STATUS) };

  result_code = (result_code_cache '/' result_code_status) ;  

  # The size is the amount of data delivered to the client.
  # Mind that this does not constitute the net object size,
  # as headers are also counted. Also, failed requests may
  # deliver an error page, the size of which is also logged here.
  bytes = [0-9]+
            >mark %{ SAVE_LINE_STR(LOGPIPE_BYTES) };

  # request method - e.g. GET POST, ICP_QUERY
  req_verb = [a-zA-Z:\-_]+
            >mark %{ SAVE_LINE_STR(LOGPIPE_CS_METHOD) };

  # URL - This column contains the URL requested.
  req_path = [^ ]+
            >mark %{ SAVE_LINE_STR(LOGPIPE_CS_URI_STEM) };

  # user
  # The eighth column may contain the user identity for the
  # requesting client. This may be sourced from one of HTTP
  # authentication, an external ACL helper, TLS authentication,
  # or IDENT lookup (RFC 931) - checked in that order with the
  # first to present information displayed. If no user identity
  # is available a "-" will be logged.
  client_auth = [^ ]+
            >mark %{ SAVE_LINE_STR(LOGPIPE_CS_USERNAME) };

  hier_code = [^ ]+
            >mark %{ SAVE_LINE_STR(LOGPIPE_HIER_CODE) };

  mime_type = [^ ]+
            >mark %{ SAVE_LINE_STR(LOGPIPE_SC_CONTENT_TYPE) };

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
    hier_code space+
    mime_type
  );


  main := line %{
    str_ptime_epoch_secs(logmeta_field(meta, LOGPIPE_TIMESTAMP), &meta->utc_timestamp);

    return ! logmeta_field_isempty(meta, LOGPIPE_TIMESTAMP)
        && ! logmeta_field_isempty(meta, LOGPIPE_TIME_TAKEN)
        && ! logmeta_field_isempty(meta, LOGPIPE_C_IP)
        && ! logmeta_field_isempty(meta, LOGPIPE_SC_CACHE)
        && ! logmeta_field_isempty(meta, LOGPIPE_SC_STATUS)
        && ! logmeta_field_isempty(meta, LOGPIPE_BYTES)
        && ! logmeta_field_isempty(meta, LOGPIPE_CS_METHOD)
        && ! logmeta_field_isempty(meta, LOGPIPE_CS_URI_STEM);
  };

  write data;

}%%

static int parse_squid(void *ctx, str_t *str, logmeta_t *meta) {
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

const logmod_t mod_parse_squid = {
  "parse.squid", NULL, parse_squid, NULL
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


static int print_squid(void *ctx, str_t *str, logmeta_t *meta) {
  char timestamp[50];
  int have_timestmap = strftime(timestamp, sizeof(timestamp), "%s", &meta->utc_timestamp) > 0;

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

  print_field_or_dash(str, meta, LOGPIPE_TIME_TAKEN, " ");
  print_field_or_dash(str, meta, LOGPIPE_C_IP, " ");
  print_field_or_dash(str, meta, LOGPIPE_SC_CACHE, "/");
  print_field_or_dash(str, meta, LOGPIPE_SC_STATUS, " ");
  print_field_or_dash(str, meta, LOGPIPE_BYTES, " ");
  print_field_or_dash(str, meta, LOGPIPE_CS_METHOD, " ");
  print_field_or_dash(str, meta, LOGPIPE_CS_URI_STEM, " ");
  print_field_or_dash(str, meta, LOGPIPE_CS_USERNAME, " ");
  print_field_or_dash(str, meta, LOGPIPE_HIER_CODE, " ");
  print_field_or_dash(str, meta, LOGPIPE_SC_CONTENT_TYPE, " ");

  return 1;
}

const logmod_t mod_print_squid = {
  "print.squid", NULL, print_squid, NULL
};


/*
Squid sends a number of commands to the log daemon. These are sent in the first byte of each input line:

  L<data>\n - logfile data
  R\n - rotate file
  T\n - truncate file
  O\n - re-open file
  F\n - flush file
  r<n>\n - set rotate count to <n>
  b<n>\n - 1 = buffer output, 0 = don't buffer output
 */
static int squid_logfile_daemon(void *ctx, str_t *buf, logmeta_t *meta) {
  if( ! str_len(buf) ) {
    return 0;
  }
  unsigned char *ptr = buf->ptr;
  size_t len = buf->len;

  // In some situations (e.g. daemon queue overflow under high load),
  // it is possible for the buffer to fill up with many L characters
  // but without any line data. Skip all preceeding 'L' characters
  while( *ptr == 'L' ) {
    // Replace L characters with spaces
    *ptr = ' ';
    len--;
    ptr++;
  }
  if( ! len ) {
    return 0;
  }

  // The next character may be a control byte, if it is - discard line
  switch( *ptr ) {
  case 0:
  case '\n':
  case 'R':
  case 'T':
  case 'O':
  case 'F':
  case 'b':
  case 'r':
    return 0;
  }

  // Line can be processed, spaces at beginning of line will be skipped
  return 1;
}

const logmod_t mod_squid_logfile_daemon = {
  "squid.logfile_daemon", NULL, squid_logfile_daemon, NULL
};