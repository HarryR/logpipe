#include "config.h"
#include "str.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#ifdef _WIN32
inline struct tm * gmtime_r(const time_t *sec, struct tm *result) {
    gmtime_s(result, sec);
    return result;
}
#endif

int str_isempty(const str_t *str) {
    return ! str || ! str->ptr || str->len < 1;
}

void str_init(str_t *str) {
    str->ptr = NULL;
    str->len = 0;
}

unsigned char *str_ptr(str_t *str) {
    if( str ) return str->ptr;
    return NULL;
}

unsigned char str_char(str_t *str, size_t offset) {
    if( str_isempty(str) || offset >= str_len(str) ) {
        return 0;
    }
    return str->ptr[offset];
}

unsigned char *str_rpos(str_t *str, const char c) {
    assert( str );
    if( ! str->len || ! str->ptr ) {
        return NULL;
    }
    return memrchr(str->ptr, c, str->len);
}

unsigned char *str_pos(str_t *str, const char c) {
    assert( str );
    if( ! str->len || ! str->ptr ) {
        return NULL;
    }
    return memchr(str->ptr, c, str->len);
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

size_t str_append(str_t *str, const char *data, size_t length) {
    assert( str != NULL );
    str->ptr = realloc(str->ptr, str->len + length + 1);
    if( ! str->ptr ) {
        str->len = 0;
        return 0;
    }
    memcpy((char*)(str->ptr + str->len), data, length);
    
    size_t old_len = str->len;
    str->len += length;
    str->ptr[str->len] = 0;
    return old_len;
}

size_t str_append_cstr(str_t *str, const char *cstr) {
    size_t len = strlen(cstr);
    return str_append(str, cstr, len);
}

size_t str_append_str(str_t *str, const str_t *b) {
    assert( b );
    return str_append(str, (const char *)b->ptr, b->len);
}

str_t str_init_cstr(const char *cstr) {
    str_t str;
    str_init(&str);
    size_t len = strlen(cstr);
    str_append(&str, cstr, len);
    return str;
}


int str_ptime(const str_t *str, const char *format, struct tm *output) {
    int fail = str_isempty(str);
    if( ! fail ) {
        struct tm local_timestamp;
        fail = strptime((char*)str->ptr, format, &local_timestamp) == NULL;
        if( ! fail ) {
            // XXX: Windows and some other platforms don't have tm_gmtoff
            //      How do we adjust to UTC time from local time in that case?
            #ifdef HAVE_TM_GMTOFF
            long int gmtoff = local_timestamp.tm_gmtoff;
            time_t actual_time = timegm(&local_timestamp) - gmtoff;
            #else
            time_t actual_time = timegm(&local_timestamp);
            #endif
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

int str_cmp(const str_t *a, const str_t *b) {
    if( ! a && ! b ) return 0;
    if( ! a && b ) return -1;
    if( a && ! b ) return 1;
    if( a->len != b->len ) {
        return (int)(b->len - a->len);
    }
    return memcmp(a->ptr, b->ptr, a->len);
}

int str_casecmp(const str_t *a, const str_t *b) {
    if( ! a && ! b ) return 0;
    if( ! a && b ) return -1;
    if( a && ! b ) return 1;
    if( a->len != b->len ) {
        return (int)(b->len - a->len);
    }
    return strncasecmp((char*)a->ptr, (char*)b->ptr, a->len);
}

int str_eq(const str_t *a, const str_t *b) {
    return str_cmp(a, b) == 0;
}

int str_caseeq(const str_t *a, const str_t *b) {
    return str_casecmp(a, b) == 0;
}

int str_caseeq_cstr(const str_t *a, const char *b_str) {
    const str_t b = {(unsigned char*)b_str, strlen(b_str)};
    return str_casecmp(a, &b) == 0;
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

pair_t *strpair_next(pair_t *pair) {
    if( pair ) {
        return (pair_t*)pair->next;
    }
    return NULL;
}

int strpair_count( const pair_t *pair ) {
    int i = 0;
    while( pair ) {
        i += 1;
        pair = (const pair_t*)pair->next;
    }
    return i;
}

pair_t *strpair_clear( pair_t *pair ) {
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
    if( key ) {
        newpair->key = *key;
    }
    else {
        str_init(&newpair->key);
    }
    if( val ) {
        newpair->val = *val;
    }
    else {
        str_init(&newpair->val);
    }
    newpair->next = (struct pair_t *)pair;    
    return newpair;
}

pair_t *strpair_add(pair_t *pair, const str_t *key, const str_t *val) {
    str_t new_key = str_clone(key);
    str_t new_val = str_clone(val);
    return strpair_add_give(pair, &new_key, &new_val);
}

pair_t *strpair_bykey(pair_t *pair, const str_t *key) {
    while( pair ) {
        if( str_eq(&pair->key, key) )  {
            return pair;
        }
        pair = (pair_t*)pair->next;
    }
    return NULL;
}

pair_t *strpair_bykey_cstr(pair_t *pair, const char *key) {
    const str_t key_str = {(unsigned char *)key, strlen(key)};
    return strpair_bykey(pair, &key_str);
}

pair_t *strpair_byval(pair_t *pair, const str_t *val) {
    while( pair ) {
        if( str_eq(&pair->val, val) )  {
            return pair;
        }
        pair = (pair_t*)pair->next;
    }
    return NULL;
}

pair_t *strpair_byval_cstr(pair_t *pair, const char *val) {
    const str_t val_str = {(unsigned char *)val, strlen(val)};
    return strpair_byval(pair, &val_str);
}
