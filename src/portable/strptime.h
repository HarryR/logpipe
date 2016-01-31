#ifndef PORTABLE_STRPTIME_H_
#define PORTABLE_STRPTIME_H_
#include <time.h>

const char * netbsd_strptime(const char *buf, const char *fmt, struct tm *tm);

// Can force usage of netbsd_strptime if PORTABLE_STRPTIME is defined
#if !defined(HAVE_STRPTIME) || defined(PORTABLE_STRPTIME)
static inline const char * portable_strptime(const char *buf, const char *fmt, struct tm *tm) {
	return netbsd_strptime(buf, fmt, tm);	
}
#else
static inline const char * portable_strptime(const char *buf, const char *fmt, struct tm *tm) {
	return strptime(buf, fmt, tm);	
}
#endif

#endif
