#ifndef CORE_H_
#define CORE_H_

#include <sys/types.h>
#include <stdint.h>
#include <time.h>

typedef struct {
    unsigned char* ptr;
    size_t len;
} str_t;

struct pair_t;
typedef struct {
    str_t key;
    str_t val;
    struct pair_t *next;
} pair_t;

str_t str_clone(const str_t *input);
int str_append(str_t *str, const char *s, uint32_t length);
int str_append_str(str_t *str, const str_t *b);
void str_init(str_t *str);
void str_clear(str_t *str);
void str_ptime(const str_t *str, const char *format, struct tm *output);
int str_len(const str_t *str);
int str_eq(const str_t *a, const str_t *b);

int pair_count(const pair_t *pair);
pair_t *pair_clear(pair_t *pair);
pair_t *pair_add_give(pair_t *pair, str_t *key, str_t *val);
pair_t *pair_add(pair_t *pair, str_t *key, str_t *val);
str_t *pair_bykey(pair_t *pair, str_t *key);
str_t *pair_byval(pair_t *pair, str_t *val);

#endif
