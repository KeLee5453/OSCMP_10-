//
// Created by lumin on 2020/11/16.
//

#ifndef LAB8_UTIL_H
#define LAB8_UTIL_H

#include <defs.h>

void set_bits_value(volatile uint32_t *bits, uint32_t mask, uint32_t value);
void set_bits_value_offset(volatile uint32_t *bits, uint32_t mask, uint32_t value, uint32_t offset);
void set_bit_p(volatile uint32_t *bits, uint32_t offset);
void clear_bit_p(volatile uint32_t *bits, uint32_t offset);

#define configASSERT(x)                                    \
    if ((x) == 0)                                          \
    {                                                      \
        cprintf("(%s:%d) %s\r\n", __FILE__, __LINE__, #x); \
        for (;;)                                           \
            ;                                              \
    }

#endif //LAB8_UTIL_H
