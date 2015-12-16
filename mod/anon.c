#include "mod.h"

#include <stdint.h>

/*
 * The deterministic source of randomness allows for
 * fuzzing and anonymising the output in a way which
 * is reproducable and testable, not strong random!...
 */

static uint16_t lfsr = 0xD359u;

uint16_t get_lfsr() {
    unsigned lsb = lfsr & 1;   /* Get LSB (i.e., the output bit). */
    lfsr >>= 1;                /* Shift register */
    lfsr ^= (-lsb) & 0xB400u;  /* If the output bit is 1, apply toggle mask.
                                * The value has 1 at bits corresponding
                                * to taps, 0 elsewhere. */
    return lfsr;
}

static int rotXb(int c, int basis){
  c = (((c-basis)+get_lfsr())%26)+basis;
  return c;
}

static int rotXn(int c, int basis){
  c = (((c-basis)+get_lfsr())%10)+basis;
  return c;
}

static char rotX(char c){
  if('a' <= c && c <= 'z'){
    return rotXb(c, 'a');
  } else if ('A' <= c && c <= 'Z') {
    return rotXb(c, 'A');
  } else if ('0' <= c && c <= '9') {
  	return rotXn(c, '0');
  } else {
    return c;
  }
}

static void anon_blank(str_t *str) {
  if( str && str->len && get_lfsr() % 4 ) {
    str_clear(str);
  }
}

static void anon_str(str_t *str) {
	if( ! str->ptr || str->len < 1 ) {
		return;
	}

	size_t i;
	for( i = 0; i < str->len; i++ ) {
		str->ptr[i] = rotX(str->ptr[i]);
	}
}


static int
debug_anon(void *ctx, str_t *str, logline_t *line) {
	anon_str(&line->client_ip);
	anon_str(&line->client_identity);
	anon_str(&line->client_auth);
	anon_str(&line->req_path);
	anon_str(&line->req_referrer);
	anon_str(&line->req_agent);
	line->utc_timestamp.tm_sec = get_lfsr() % 60;
	line->utc_timestamp.tm_mon = get_lfsr() % 12;
	line->utc_timestamp.tm_year = 110;
	return 1;
}

const logmod_t mod_debug_anon = {
	"debug.anon", NULL, debug_anon, NULL
};


static int
debug_randblank(void *ctx, str_t *str, logline_t *line) {
  anon_blank(&line->timestamp);
  anon_blank(&line->client_ip);
  anon_blank(&line->client_identity);
  anon_blank(&line->client_auth);
  anon_blank(&line->req_verb);
  anon_blank(&line->req_path);
  anon_blank(&line->req_ver);
  anon_blank(&line->req_referrer);
  anon_blank(&line->req_agent);
  anon_blank(&line->duration);
  anon_blank(&line->resp_bytes);
  anon_blank(&line->resp_cache);
  anon_blank(&line->heir_code);
  anon_blank(&line->mime_type);
  return 1;
}

const logmod_t mod_debug_randblank = {
  "debug.randblank", NULL, debug_randblank, NULL
};