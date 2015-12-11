#ifndef PARSER_H_
#define PARSER_H_

#include "core.h"

typedef int (*logline_parse_fn_t)(logline_t *line);
void logline_line_init(logline_t *line, unsigned char* buf, size_t buf_sz);
int logline_parse_apacheclf(logline_t *line);
void logline_parse_timestamp_apacheclf( logline_t *line );

#endif

