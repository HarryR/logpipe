#include "config.h"
#include "str.h"
// memrchr isn't on all platforms...

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

int str_isempty(const str_t *str) {
    return ! str || ! str->ptr || str->len < 1;
}

void str_init(str_t *str) {
    str->ptr = NULL;
    str->len = 0;
}

char *str_ptr(str_t *str) {
    if( str ) return (char*)str->ptr;
    return NULL;
}

char str_char(str_t *str, size_t offset) {
    if( str_isempty(str) || offset >= str_len(str) ) {
        return 0;
    }
    return str->ptr[offset];
}

char *str_rpos(str_t *str, const char c) {
    assert( str );
    if( ! str->len || ! str->ptr ) {
        return NULL;
    }
    return memrchr(str->ptr, c, str->len);
}

void str_clear(str_t *str) {
    if( str ) {
        if( str->ptr ) {
            free(str->ptr);
        }
        str_init(str);
    }
}

size_t str_len(const str_t *str) {
    if( str_isempty(str) ) {
        return 0;
    }
    return str->len;
}

int str_append(str_t *str, const char *data, uint32_t length) {
    assert( str != NULL );
    str->ptr = realloc(str->ptr, str->len + length + 1);
    if( ! str->ptr ) {
        str->len = 0;
        return -1;
    }
    memcpy((char*)(str->ptr + str->len), data, length);
    
    int old_len = str->len;
    str->len += length;
    str->ptr[str->len] = 0;
    return old_len;
}

int str_append_cstr(str_t *str, const char *cstr) {
    uint32_t len = strlen(cstr);
    return str_append(str, cstr, len);
}

str_t str_init_cstr(const char *cstr) {
    str_t str;
    str_init(&str);
    uint32_t len = strlen(cstr);
    str_append(&str, cstr, len);
    return str;
}

int str_append_str(str_t *str, const str_t *b) {
    assert( b );
    return str_append(str, (const char *)b->ptr, b->len);
}

int str_ptime(const str_t *str, const char *format, struct tm *output) {
    int fail = str_isempty(str);
    if( ! fail ) {
        struct tm local_timestamp;
        fail = strptime((char*)str->ptr, format, &local_timestamp) == NULL;
        if( ! fail ) {
            long int gmtoff = local_timestamp.tm_gmtoff;
            time_t actual_time = timegm(&local_timestamp) - gmtoff;
            gmtime_r(&actual_time, output);
        }
    }
    // Upon failure wipe the output struct
    if( fail ) {
        memset(output, 0, sizeof(*output));
    }
    return ! fail;
}

int str_ptime_rfc1123(const str_t *str, struct tm *output) {
    return str_ptime(str, "%d/%b/%Y:%H:%M:%S %z", output);
}

int str_ptime_epoch_secs(const str_t *str, struct tm *output) {
    return str_ptime(str, "%s", output);
}

int str_eq(const str_t *a, const str_t *b) {
    if( ! a && ! b ) return 0;
    if( ! a && b ) return -1;
    if( a && ! b ) return 1;
    assert( a && b );
    return memcmp(a->ptr, b->ptr, (a->len < b->len ? a->len : b->len));
}


str_t str_clone(const str_t *input) {
    str_t newstr;
    str_init(&newstr);
    if( input ) {
        str_append_str(&newstr, input);
    }
    return newstr;
}


// -------

int pair_count( const pair_t *pair ) {
    int i = 0;
    while( pair ) {
        i += 1;
        pair = (const pair_t*)pair->next;
    }
    return i;
}

pair_t *pair_clear( pair_t *pair ) {
    pair_t *old;
    int i = 0;
    while( pair ) {
        i += 1;
        old = pair;
        pair = (pair_t*)pair->next;
        str_clear(&old->key);
        str_clear(&old->val);
        old->next = NULL;
        free(old);
    }
    return NULL;
}

pair_t *strpair_add_give(pair_t *pair, str_t *key, str_t *val) {
    pair_t *newpair = malloc(sizeof(pair_t));
    newpair->key = *key;
    newpair->val = *val;
    newpair->next = (struct pair_t *)pair;    
    return newpair;
}

pair_t *strpair_add(pair_t *pair, str_t *key, str_t *val) {
    str_t new_key = str_clone(key);
    str_t new_val = str_clone(val);
    return strpair_add_give(pair, &new_key, &new_val);
}

pair_t *strpair_bykey(pair_t *pair, str_t *key) {
    while( pair ) {
        if( str_eq(&pair->key, key) )  {
            return pair;
        }
        pair = (pair_t*)pair->next;
    }
    return NULL;
}

pair_t *strpair_byval(pair_t *pair, str_t *val) {
    while( pair ) {
        if( str_eq(&pair->val, val) )  {
            return pair;
        }
        pair = (pair_t*)pair->next;
    }
    return NULL;
}
