#include "jv-utils.h"
#include "jv_unicode.h"

#include <assert.h>

/*
jq is copyright (C) 2012 Stephen Dolan

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

int unhex(char* hex, int cnt) {
  int r = 0;
  for (int i=0; i<cnt; i++) {
    char c = *hex++;
    int n;
    if ('0' <= c && c <= '9') n = c - '0';
    else if ('a' <= c && c <= 'f') n = c - 'a' + 10;
    else if ('A' <= c && c <= 'F') n = c - 'A' + 10;
    else {
        return -1;
    }
    r <<= 4;
    r |= n;
  }
  return r;
}

int inplace_unescape(char* in, char *end) {
  char *orig_in = in;
  char* out = in;
  
  while (in < end) {
    char c = *in++;
    if (c == '\\') {
      if (in >= end)
        return -1; //"Expected escape character at end of string";
      c = *in++;
      switch (c) {
      case '\\':
      case '"':
      case '/': *out++ = c;    break;
      case 'b': *out++ = '\b'; break;
      case 'f': *out++ = '\f'; break;
      case 't': *out++ = '\t'; break;
      case 'n': *out++ = '\n'; break;
      case 'r': *out++ = '\r'; break;

      case 'x':
        if (in + 2 > end) {
          return -1; //"Invalid \\xXX escape";
        }
        int hexvalue2 = unhex(in, 2);
        in += 2;
        if( hexvalue2 >= 0 ) {
          *out++ = hexvalue2;
        }
        break;

      case 'u':
        /* ahh, the complicated case */
        if (in + 4 > end)
          return -1; //"Invalid \\uXXXX escape";
        int hexvalue = unhex(in, 4);
        if (hexvalue < 0)
          return -1; //"Invalid characters in \\uXXXX escape";
        unsigned long codepoint = (unsigned long)hexvalue;
        in += 4;
        if (0xD800 <= codepoint && codepoint <= 0xDBFF) {
          /* who thought UTF-16 surrogate pairs were a good idea? */
          if (in + 6 > end || in[0] != '\\' || in[1] != 'u')
            return -1; //"Invalid \\uXXXX\\uXXXX surrogate pair escape";
          unsigned long surrogate = unhex(in+2, 4);
          if (!(0xDC00 <= surrogate && surrogate <= 0xDFFF))
            return -1; //"Invalid \\uXXXX\\uXXXX surrogate pair escape";
          in += 6;
          codepoint = 0x10000 + (((codepoint - 0xD800) << 10)
                                 |(surrogate - 0xDC00));
        }
        // FIXME assert valid codepoint
        out += jvp_utf8_encode(codepoint, out);
        break;

      default:
        return -1; //"Invalid escape";
      }
    } else {
      *out++ = c;
    }
  }
  *out = 0;
  return out - orig_in;
}
