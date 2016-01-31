#include "str.h"

%%{
	machine strpair_split;
	alphtype unsigned char;
	action mark { ts = p; }
	value = (any - space)+
		  >{ str_init(&val);
		  	 val.ptr = p; }
		  %{ val.len = (size_t)(p - val.ptr);
		     output = strpair_add(output, NULL, &val); }
		  ;
	main := space* value ( space+ value )* space* ;
	write data;
}%%

pair_t *strpair_split(const str_t *buf) {
	int cs;
    unsigned char *p, *pe, *eof;
    pair_t *output = NULL;
    str_t val;

    p = buf->ptr;
    eof = pe = buf->ptr + buf->len;

    %% write init;
    %% write exec;

    return output;
}
