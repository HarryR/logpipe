#include "mod.h"
#include "md5.h"
#include <assert.h>

void logline_make_md5(logline_t *line) {
    md5_state_t ctx;
    md5_init(&ctx);
    md5_append(&ctx, line->raw.ptr, line->raw.len);
    md5_finish(&ctx, line->md5);
}

// [18/Sep/2011:19:18:28 -0400]
void logline_parse_timestamp_apacheclf( logline_t *line ) {
    struct tm local_timestamp;
    strptime((char*)line->timestamp.ptr, "%d/%b/%Y:%H:%M:%S %z", &local_timestamp);
    long int gmtoff = local_timestamp.tm_gmtoff;
    time_t actual_time = timegm(&local_timestamp) - gmtoff;
    gmtime_r(&actual_time, &line->utc_timestamp);
}

