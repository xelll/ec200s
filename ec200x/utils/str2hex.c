#include <stdio.h>
#include <stdint.h>
#include <string.h>

int str2hex(const char *str, char *hex)
{
    const char *cHex = "0123456789ABCDEF";

    size_t j, i = 0;
    size_t str_len = strlen(str);

    for(j = 0; j < str_len; j++) {
        uint8_t a = (uint8_t)str[j];

        hex[i++] = cHex[(a & 0xf0) >> 4];
        hex[i++] = cHex[(a & 0x0f)];
    }
    hex[i] = '\0';

    return (int)i;
}

static inline uint8_t hex2int(char c)
{
    if(c >= '0' && c <= '9') {
        return (uint8_t)(c - '0');
    } else if(c >= 'A' && c <= 'F') {
        return (uint8_t)(0x0a + c - 'A');
    } else if(c >= 'a' && c <= 'f') {
        return (uint8_t)(0x0a + c - 'a');
    } else {
        return 0;
    }
}

void hex2str(const char *hex, char *str)
{
    uint8_t a, b;
    int i = 0;
    for(int j = 0; j < strlen(hex) - 1;) {
        a = hex2int(hex[j++]);
        b = hex2int(hex[j++]);
        str[i++] = ((a << 4) & 0xF0) | (b & 0x0F);
    }
    str[i] = '\0';
}

int hexstr2_hex(const char *hex_str, char *out, int len)
{
    uint8_t a, b;
    int i = 0;
    for(int j = 0; j < len - 1;) {
        a = hex2int(hex_str[j++]);
        b = hex2int(hex_str[j++]);
        out[i++] = ((a << 4) & 0xF0) | (b & 0x0F);
    }

    return i;
}
