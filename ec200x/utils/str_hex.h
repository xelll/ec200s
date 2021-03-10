#ifndef __STR_HEX_H
#define __STR_HEX_H

#include <stdint.h>

void hex_str(const uint8_t *inchar, const uint16_t len, uint8_t *outtxt);

uint16_t str_hex(uint8_t *str, uint8_t *hex);

#endif /* __STR_HEX_H */
