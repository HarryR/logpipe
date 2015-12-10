#include "parser.h"
#include <assert.h>

void logline_parse_timestamp_apacheclf( logline_t *line ) {
    struct tm local_timestamp;
    strptime((char*)line->timestamp.ptr, "%d/%b/%Y:%H:%M:%S %z", &local_timestamp);
    long int gmtoff = local_timestamp.tm_gmtoff;
    time_t actual_time = timegm(&local_timestamp) - gmtoff;
    gmtime_r(&actual_time, &line->utc_timestamp);
}

void logline_line_init(logline_t *line, unsigned char* buf, size_t buf_sz) {
	assert(line);
	assert(buf);
	assert(buf_sz);
    memset(line, 0, sizeof(*line));
    line->p = buf;
    line->eof = line->pe = buf + buf_sz;
    line->ts = line->p;
    line->raw.ptr = (unsigned char*)buf;
    line->raw.len = buf_sz;
  	logline_make_md5(line);
}