#include "logpipe-module.h"

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

static void anon_blank(logmeta_t *meta, logpipe_field_t field) {
  str_t *str = logmeta_field(meta, field);
  if( str && str->len && get_lfsr() % 4 ) {
    str_clear(str);
  }
}

static void anon_str(logmeta_t *meta, logpipe_field_t field) {
  str_t *str = logmeta_field(meta, field);
	if( ! str->ptr || str->len < 1 ) {
		return;
	}

	size_t i;
	for( i = 0; i < str->len; i++ ) {
		str->ptr[i] = rotX(str->ptr[i]);
	}
}


static int
debug_anon(void *ctx, str_t *str, logmeta_t *meta) {
	anon_str(meta, LOGPIPE_C_IP);
	anon_str(meta, LOGPIPE_CS_IDENT);
	anon_str(meta, LOGPIPE_CS_USERNAME);
	anon_str(meta, LOGPIPE_CS_URI_STEM);
	anon_str(meta, LOGPIPE_CS_REFERER);
	anon_str(meta, LOGPIPE_CS_USER_AGENT);
	meta->utc_timestamp.tm_sec = get_lfsr() % 60;
	meta->utc_timestamp.tm_mon = get_lfsr() % 12;
	meta->utc_timestamp.tm_year = 110;
	return 1;
}

const logmod_t mod_debug_anon = {
	"debug.anon", NULL, (logmod_fn_t)debug_anon, NULL
};


static int
debug_randblank(void *ctx, str_t *str, logmeta_t *meta) {
  int i;
  for( i = 0; i  < LOGPIPE_FIELDS_END; i++ ) {
    anon_blank(meta, i);
  }
  return 1;
}

const logmod_t mod_debug_randblank = {
  "debug.randblank", NULL, debug_randblank, NULL
};