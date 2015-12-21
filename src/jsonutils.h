#ifndef UTILS_H_
#define UTILS_H_

#include "log.h"
#include "json.h"

int json_print_key(json_printer *printer, const char *key);
void print_keystr(json_printer *jp, char *key, str_t *str);
void print_keystr2(json_printer *jp, char *key, char* str);
void print_keyraw(json_printer *jp, char *key, str_t *str);
void print_strraw(json_printer *jp, str_t *str);
void logline_print_id(logline_t *line, json_printer *jp, const char* key);
void json_print_splitpath(json_printer *jp, char *path, size_t len);
void print_optkeystr2(json_printer *jp, char *key, char* str);
void print_optkeystr(json_printer *jp, char *key, str_t *str);
void print_strraw_or_null(json_printer *jp, str_t *str);

#endif
