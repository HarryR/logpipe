#ifndef TIMEGM_H_
#define TIMEGM_H_

#include "config.h"

#ifndef HAVE_TIMEGM
#include <time.h>
time_t timegm(struct tm *tm);
#endif

#endif

