#ifndef CORE_H_
#define CORE_H_

#include <sys/types.h>

typedef struct {
    unsigned char* ptr;
    size_t len;
} str_t;

typedef struct {
    str_t key;
    str_t val;
} pair_t;

int str_append(str_t *str, const char *s, int length);
int str_append_str(str_t *str, str_t *b);
void str_init(str_t *str);
void str_clear(str_t *str);

#endif
