//
// Created by lumin on 2020/11/16.
//

#include <util.h>

void set_bits_value(volatile uint32_t *bits, uint32_t mask, uint32_t value)
{
    uint32_t masked_origin_value = (*bits) & ~mask;
    *bits = masked_origin_value | (value & mask);
}

void set_bits_value_offset(volatile uint32_t *bits, uint32_t mask, uint32_t value, uint32_t offset)
{
    set_bits_value(bits, mask << offset, value << offset);
}

void set_bit(volatile uint32_t *bits, uint32_t offset)
{
    set_bits_value(bits, 1 << offset, 1 << offset);
}

 void clear_bit(volatile uint32_t *bits, uint32_t offset)
{
    set_bits_value(bits, 1 << offset, 0);
}
uint32_t is_memory_cache(uintptr_t address)
{
    #define MEM_CACHE_LEN (6 * 1024 * 1024)//k210内存空间为0x80000000~0x80600000？

    return ((address >= 0x80000000) && (address < 0x80000000 + MEM_CACHE_LEN));
}