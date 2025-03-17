#pragma once

#define bit_set(reg, bit)    ((reg) |= (1 << (bit)))
#define bit_clear(reg, bit)  ((reg) &= ~(1 << (bit)))
#define bit_get(reg, bit)    (((reg) >> (bit)) & 0x01)
#define bit_toggle(reg, bit) ((reg) ^= (1 << (bit)))
