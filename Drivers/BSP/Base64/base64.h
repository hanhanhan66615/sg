#ifndef BASE64_H
#define BASE64_H

#include <stdint.h>

#define BASE64_ENCODE_OUT_SIZE(s) ((uint32_t)((((s) + 2) / 3) * 4 + 1))

void base64_encode(const unsigned char *src, 
                  uint32_t len, 
                  unsigned char *dst);

#endif
