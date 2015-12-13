#include "str.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

int str_isempty(const str_t *str) {
    return ! str || ! str->ptr || str->len < 1;
}

void str_init(str_t *str) {
    str->ptr = NULL;
    str->len = 0;
}

void str_clear(str_t *str) {
    if( str ) {
        if( str->ptr ) {
            free(str->ptr);
        }
        str_init(str);
    }
}

int str_len(const str_t *str) {
    if( ! str_isempty(str) ) {
        return str->len;
    }
    return 0;
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

int str_append_str(str_t *str, const str_t *b) {
    return str_append(str, (const char *)b->ptr, b->len);
}

void str_ptime(const str_t *str, const char *format, struct tm *output) {
    if( ! str_isempty(str) ) {
   struct tm local_timestamp;
        strptime((char*)str->ptr, format, &local_timestamp);
        long int gmtoff = local_timestamp.tm_gmtoff;
        time_t actual_time = timegm(&local_timestamp) - gmtoff;
        gmtime_r(&actual_time, output);
    }
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
    str_append_str(&newstr, input);
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

pair_t *pair_add_give(pair_t *pair, str_t *key, str_t *val) {
    pair_t *newpair = malloc(sizeof(pair_t));
    newpair->key = *key;
    newpair->val = *val;
    newpair->next = (struct pair_t *)pair;    
    return newpair;
}

pair_t *pair_add(pair_t *pair, str_t *key, str_t *val) {
    str_t new_key = str_clone(key);
    str_t new_val = str_clone(val);
    return pair_add_give(pair, &new_key, &new_val);
}

str_t *pair_bykey(pair_t *pair, str_t *key) {
    while( pair ) {
        if( str_eq(&pair->key, key) )  {
            return &pair->val;
        }
        pair = (pair_t*)pair->next;
    }
    return NULL;
}

str_t *pair_byval(pair_t *pair, str_t *val) {
    while( pair ) {
        if( str_eq(&pair->val, val) )  {
            return &pair->key;
        }
        pair = (pair_t*)pair->next;
    }
    return NULL;
}