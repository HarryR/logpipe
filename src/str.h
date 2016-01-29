#ifndef CORE_H_
#define CORE_H_

#include <sys/types.h>
#include <stdint.h>
#include <time.h>

typedef struct {
    unsigned char* ptr;
    size_t len;
} str_t;

int str_isempty(const str_t *str);
str_t str_clone(const str_t *input);
size_t str_append(str_t *str, const char *s, size_t length);
size_t str_append_str(str_t *str, const str_t *b);
size_t str_append_cstr(str_t *str, const char *cstr);
void str_init(str_t *str);
void str_clear(str_t *str);
int str_ptime(const str_t *str, const char *format, struct tm *output);
int str_ptime_rfc1123(const str_t *str, struct tm *output);
int str_ptime_epoch_secs(const str_t *str, struct tm *output);
size_t str_len(const str_t *str);
unsigned char str_char(str_t *str, size_t offset);
int str_cmp(const str_t *a, const str_t *b);
int str_eq(const str_t *a, const str_t *b);
int str_casecmp(const str_t *a, const str_t *b);
int str_caseeq(const str_t *a, const str_t *b);
int str_caseeq_cstr(const str_t *a, const char *b_str);
unsigned char *str_rpos(str_t *str, const char c);
unsigned char *str_pos(str_t *str, const char c);
unsigned char *str_ptr(str_t *str);
str_t str_init_cstr(const char *cstr);


struct pair_t;
typedef struct {
    str_t key;
    str_t val;
    struct pair_t *next;
} pair_t;

/**
 * Split a string, deliminated by whitespace
 * into a sequence of pairs containing only values
 */
pair_t *strpair_split(const str_t *str);

int strpair_count(const pair_t *pair);
pair_t *strpair_clear(pair_t *pair);
pair_t *strpair_next(pair_t *pair);
pair_t *strpair_add_give(pair_t *pair, str_t *key, str_t *val);
pair_t *strpair_add(pair_t *pair, const str_t *key, const str_t *val);
pair_t *strpair_bykey(pair_t *pair, const str_t *key);
pair_t *strpair_bykey_cstr(pair_t *pair, const char *key);
pair_t *strpair_byval(pair_t *pair, const str_t *val);
pair_t *strpair_byval_cstr(pair_t *pair, const char *val);

#endif
