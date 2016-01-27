#include "querystring.h"
#include "url.h"

%%{
	machine parse_querystring;
	alphtype unsigned char;
	action mark { ts = p; }

	# Implements a parser for query strings as defined in:
	# https://en.wikipedia.org/wiki/Query_string

	key = [^=]+
		>{ key.ptr = p; }
		%{ key.len = (size_t)(p - key.ptr); }
		;

	value = [^&]+
		>{ val.ptr = p; }
		%{ val.len = (size_t)(p - val.ptr); }
		;

	pair = (key "=" value)
		   >{ str_init(&key);
		   	  str_init(&val); }
		   %{ 
		   		if( str_len(&key) ) {
		   			key.len = php_url_decode((char*)key.ptr, key.len);
		      		val.len = php_url_decode((char*)val.ptr, val.len);
		      		output = strpair_add(output, &key, &val);
		   		}
		   	}
		   	;

	main := pair ( "&" pair )* ;

	write data;
}%%

pair_t *querystring_parse(str_t *buf) {
	// XXX: the input buffer will be mangled during parsing
	//      because of the php_url_decode stage
	int cs;
    unsigned char *p, *pe, *eof;
    pair_t *output = NULL;

    str_t key;
    str_t val;

    p = buf->ptr;
    eof = pe = buf->ptr + buf->len;

    %% write init;
    %% write exec;

    return output;
}
