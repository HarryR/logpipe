#ifndef _CMC_STRPTIME_H
#define _CMC_STRPTIME_H
#include <time.h>

const char * strptime(const char *buf, const char *fmt, struct tm *tm);

#endif