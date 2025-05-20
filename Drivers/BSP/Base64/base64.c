#include "./BSP/Base64/base64.h"

static const char base64_table[] = 
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64_encode(const unsigned char *src, 
                  uint32_t len, 
                  unsigned char *dst)
{
    uint32_t i = 0, j = 0;
    uint8_t byte3[3], byte4[4];
    
    while (len--) {
        byte3[i++] = *(src++);
        if (i == 3) {
            byte4[0] = (byte3[0] & 0xfc) >> 2;
            byte4[1] = ((byte3[0] & 0x03) << 4) | ((byte3[1] & 0xf0) >> 4);
            byte4[2] = ((byte3[1] & 0x0f) << 2) | ((byte3[2] & 0xc0) >> 6);
            byte4[3] = byte3[2] & 0x3f;

            for (i = 0; i < 4; i++) {
                dst[j++] = base64_table[byte4[i]];
            }
            i = 0;
        }
    }

    if (i) {
        for (uint32_t k = i; k < 3; k++) {
            byte3[k] = '\0';
        }

        byte4[0] = (byte3[0] & 0xfc) >> 2;
        byte4[1] = ((byte3[0] & 0x03) << 4) | ((byte3[1] & 0xf0) >> 4);
        byte4[2] = ((byte3[1] & 0x0f) << 2) | ((byte3[2] & 0xc0) >> 6);

        for (uint32_t k = 0; k < i + 1; k++) {
            dst[j++] = base64_table[byte4[k]];
        }

        while (i++ < 3) {
            dst[j++] = '=';
        }
    }

    dst[j] = '\0';
}
