#ifndef __LIBS_STDLIB_H__
#define __LIBS_STDLIB_H__

#include <defs.h>

/* the largest number rand will return */
#define RAND_MAX    2147483647UL
// #define	__malloc_like	__attribute__((__malloc__))
// #define	__result_use_check	__attribute__((__warn_unused_result__))
// #define	__alloc_size(x)	__attribute__((__alloc_size__(x)))
#define _NOTHROW
/* libs/rand.c */
int rand(void);
void srand(unsigned int seed);

/* libs/hash.c */
uint32_t hash32(uint32_t val, unsigned int bits);
//void *malloc(size_t) __malloc_like __result_use_check __alloc_size(1) _NOTHROW;
#endif /* !__LIBS_RAND_H__ */

