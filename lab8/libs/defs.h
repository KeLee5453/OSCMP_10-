#ifndef __LIBS_DEFS_H__
#define __LIBS_DEFS_H__

#ifndef NULL
#define NULL ((void *)0)
#endif

#define __always_inline inline __attribute__((always_inline))
#define __noinline __attribute__((noinline))
#define __noreturn __attribute__((noreturn))

#define CHAR_BIT 8

#define false 0
#define true 1
/* Represents true-or-false values */
typedef long long bool;

/* Explicitly-sized versions of integer types */
typedef char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;
#if __riscv_xlen == 64
typedef uint64_t uint_t;
typedef int64_t sint_t;
#elif __riscv_xlen == 32
typedef uint32_t uint_t;
typedef int32_t sint_t;
#endif
// #define _TIMER_T_ unsigned long
// typedef _TIMER_T_ __timer_t;

// #if !defined(__timer_t_defined) && !defined(_TIMER_T_DECLARED)
// typedef __timer_t timer_t;
// #define __timer_t_defined
// #define _TIMER_T_DECLARED
// #endif

/* *
 * Pointers and addresses are 32 bits long.
 * We use pointer types to represent addresses,
 * uintptr_t to represent the numerical values of addresses.
 * */
typedef sint_t intptr_t;
typedef uint_t uintptr_t;

/* size_t is used for memory object sizes */
typedef uintptr_t size_t;
/* off_t is used for file offsets and lengths */
typedef intptr_t off_t;
/* used for page numbers */
typedef size_t ppn_t;

/* *
 * Rounding operations (efficient when n is a power of 2)
 * Round down to the nearest multiple of n
 * */
#define ROUNDDOWN(a, n) ({      \
  size_t __a = (size_t)(a);     \
  (typeof(a))(__a - __a % (n)); \
})

/* Round up to the nearest multiple of n */
#define ROUNDUP(a, n) ({                              \
  size_t __n = (size_t)(n);                           \
  (typeof(a))(ROUNDDOWN((size_t)(a) + __n - 1, __n)); \
})

/* Round up the result of dividing of n */
#define ROUNDUP_DIV(a, n) ({          \
  uint64_t __n = (uint64_t)(n);       \
  (typeof(a))(((a) + __n - 1) / __n); \
})

/* Return the offset of 'member' relative to the beginning of a struct type */
#define offsetof(type, member) \
  ((size_t)(&((type *)0)->member))

/* *
 * to_struct - get the struct from a ptr
 * @ptr:    a struct pointer of member
 * @type:   the type of the struct this is embedded in
 * @member: the name of the member within the struct
 * */
#define to_struct(ptr, type, member) \
  ((type *)((char *)(ptr)-offsetof(type, member)))

#ifndef __wint_t_defined
#define __wint_t_defined 1

/* Some versions of stddef.h provide wint_t, even though neither the
   C nor C++ standards, nor POSIX, specifies this.  We assume that
   stddef.h will define the macro _WINT_T if and only if it provides
   wint_t, and conversely, that it will avoid providing wint_t if
   _WINT_T is already defined.  */
#ifndef _WINT_T
#define _WINT_T 1

/* Integral type unchanged by default argument promotions that can
   hold any value corresponding to members of the extended character
   set, as well as at least one value that does not correspond to any
   member of the extended character set.  */
#ifndef __WINT_TYPE__
#define __WINT_TYPE__ unsigned int
#endif

typedef __WINT_TYPE__ wint_t;

#endif /* _WINT_T */
#endif /* bits/types/wint_t.h */

#endif /* !__LIBS_DEFS_H__ */
