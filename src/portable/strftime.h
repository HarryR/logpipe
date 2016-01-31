#ifndef PORTABLE_STRFTIME_H_
#define PORTABLE_STRFTIME_H_

#include <time.h>

size_t netbsd_strftime(char *s, size_t maxsize, const char *format, const struct tm *t);

#if !defined(HAVE_STRFTIME) || defined(PORTABLE_STRFTIME)
static inline size_t portable_strftime(char *s, size_t maxsize, const char *format, const struct tm *t) {
	return netbsd_strftime(s, maxsize, format, t);
}
#else
static inline size_t portable_strftime(char *s, size_t maxsize, const char *format, const struct tm *t) {
	return strftime(s, maxsize, format, t);
}
#endif

#endif
