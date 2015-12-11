#include "str.h"

#include <stdlib.h>
#include <string.h>

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

int str_append(str_t *str, const char *data, int length) {
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

int str_append_str(str_t *str, str_t *b) {
    return str_append(str, b->ptr, b->len);
}
