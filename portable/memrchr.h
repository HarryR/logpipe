#ifndef MEMRCHR_H_
#define MEMRCHR_H_

#ifndef HAVE_MEMRCHR
#include <stddef.h>
void* memrchr(const void *s, int c, size_t n);
#endif

#endif

