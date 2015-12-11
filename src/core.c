
#include "core.h"

#include "md5.h"

void str_init(str_t *str) {
    str->ptr = NULL;
    str->len = 0;
}

void str_free(str_t *str) {
    if( str ) {
        if( str->ptr ) {
            free(str->ptr);
        }
        str_init(str);
    }
}

int str_append(str_t *str, const char *s, int length) {
    str->ptr = realloc(str->ptr, str->len + length + 1);
    if( ! str->ptr ) {
        return 0;
    }
    strncpy((char*)(str->ptr + str->len), s, length);
    return str->len += length;    
}

int str_append_str(str_t *str, str_t *b) {
    return str_append(str, b->ptr, b->len);
}
