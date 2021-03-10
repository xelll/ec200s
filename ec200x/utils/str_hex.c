#include "str_hex.h"

void hex_str(const uint8_t *inchar, const uint16_t len, uint8_t *outtxt)
{
    uint16_t i;
    uint8_t hbit, lbit;

    for(i = 0; i < len; i++)
    {
        hbit = (*(inchar + i) & 0xf0) >> 4;
        lbit = *(inchar + i) & 0x0f;
        if(hbit > 9)
            outtxt[2 * i] = 'A' + hbit - 10;
        else
            outtxt[2 * i] = '0' + hbit;
        if(lbit > 9)
            outtxt[2 * i + 1] = 'A' + lbit - 10;
        else
            outtxt[2 * i + 1] = '0' + lbit;
    }
    outtxt[2 * i] = 0;
    return;
}

uint16_t str_hex(uint8_t *str, uint8_t *hex)
{
    uint8_t ctmp, ctmp1, half;
    uint16_t num = 0;
    do
    {
        do
        {
            half = 0;
            ctmp = *str;
            if(!ctmp)
                break;
            str++;
        } while((ctmp == 0x20) || (ctmp == 0x2c) || (ctmp == '\t'));
        if(!ctmp)
            break;
        if(ctmp >= 'a')
            ctmp = ctmp - 'a' + 10;
        else if(ctmp >= 'A')
            ctmp = ctmp - 'A' + 10;
        else
            ctmp = ctmp - '0';
        ctmp = ctmp << 4;
        half = 1;
        ctmp1 = *str;
        if(!ctmp1)
            break;
        str++;
        if((ctmp1 == 0x20) || (ctmp1 == 0x2c) || (ctmp1 == '\t'))
        {
            ctmp = ctmp >> 4;
            ctmp1 = 0;
        } else if(ctmp1 >= 'a')
            ctmp1 = ctmp1 - 'a' + 10;
        else if(ctmp1 >= 'A')
            ctmp1 = ctmp1 - 'A' + 10;
        else
            ctmp1 = ctmp1 - '0';
        ctmp += ctmp1;
        *hex = ctmp;
        hex++;
        num++;
    } while(1);
    if(half)
    {
        ctmp = ctmp >> 4;
        *hex = ctmp;
        num++;
    }
    return (num);
}

