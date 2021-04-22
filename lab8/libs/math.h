/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Fast approximate math functions.
 *
 */
#ifndef __FMATH_H
#define __FMATH_H
#include <defs.h>
// #include <math.h>

#define fmin(a, b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b; })
#define fminf(a, b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b; })
#define fmax(a, b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b; })
#define fmaxf(a, b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b; })

#define sqrt(x) fast_sqrtf(x)
#define sqrtf(x) fast_sqrtf(x)
#define floor(x) fast_floorf(x)
#define floorf(x) fast_floorf(x)
#define ceil(x) fast_ceilf(x)
#define ceilf(x) fast_ceilf(x)
#define round(x) fast_roundf(x)
#define roundf(x) fast_roundf(x)
#define atan(x) fast_atanf(x)
#define atanf(x) fast_atanf(x)
#define atan2(y, x) fast_atan2f((y), (x))
#define atan2f(y, x) fast_atan2f((y), (x))
#define exp(x) fast_expf(x)
#define expf(x) fast_expf(x)
#define cbrt(x) fast_cbrtf(x)
#define cbrtf(x) fast_cbrtf(x)
#define fabs(x) fast_fabsf(x)
#define fabsf(x) fast_fabsf(x)
#define log(x) fast_log(x)
#define logf(x) fast_log(x)
#undef log2
#define log2(x) fast_log2(x)
#undef log2f
#define log2f(x) fast_log2(x)

static inline float fast_sqrtf(float x)
{
  //return sqrtf(x);
  asm("fsqrt.s %0, %1"
      : "=f"(x)
      : "f"(x));
  return x;
}
static inline int fast_floorf(float x)
{
  return (int)(x);
}

static inline int fast_ceilf(float x)
{
  return (int)(x + 0.9999f);
}

static inline int fast_roundf(float x)
{
  return (int)(x);
}

static inline float fast_fabsf(float d)
{
  return fabsf(d);
}

typedef union
{
  uint32_t l;
  struct
  {
    uint32_t m : 20;
    uint32_t e : 11;
    uint32_t s : 1;
  };
} exp_t;

inline float fast_expf(float x)
{
  exp_t e;
  e.l = (uint32_t)(1512775 * x + 1072632447);
  // IEEE binary32 format
  e.e = (e.e - 1023 + 127) & 0xFF; // rebase

  uint32_t packed = (e.s << 31) | (e.e << 23) | e.m << 3;
  return *((float *)&packed);
}
#pragma GCC diagnostic pop

inline float fast_cbrtf(float x)
{
  union
  {
    int ix;
    float x;
  } v;
  v.x = x;                     // x can be viewed as int.
  v.ix = v.ix / 4 + v.ix / 16; // Approximate divide by 3.
  v.ix = v.ix + v.ix / 16;
  v.ix = v.ix + v.ix / 256;
  v.ix = 0x2a511cd0 + v.ix; // Initial guess.
  return v.x;
}

inline float fast_log2(float x)
{
  union
  {
    float f;
    uint32_t i;
  } vx = {x};
  union
  {
    uint32_t i;
    float f;
  } mx = {(vx.i & 0x007FFFFF) | 0x3f000000};
  float y = vx.i;
  y *= 1.1920928955078125e-7f;

  return y - 124.22551499f - 1.498030302f * mx.f - 1.72587999f / (0.3520887068f + mx.f);
}

inline float fast_log(float x)
{
  return 0.69314718f * fast_log2(x);
}

inline float fast_powf(float a, float b)
{
  union
  {
    float d;
    int x;
  } u = {a};
  u.x = (int)((b * (u.x - 1064866805)) + 1064866805);
  return u.d;
}

/*#define fast_sqrtf(x) (sqrtf(x))
#define fast_floorf(x) ((int)floorf(x))
#define fast_ceilf(x) ((int)ceilf(x))
#define fast_roundf(x) ((int)roundf(x))
#define fast_atanf(x) (atanf(x))
#define fast_atan2f(x,y) (atan2f((x),(y)))
#define fast_expf(x) (expf(x))
#define fast_cbrtf(x) (cbrtf(x))
#define fast_fabsf(x) (fabsf(x))
#define fast_log(x) (log(x))
#define fast_log2(x) (log2(x))
#define fast_powf(x,y) (powf((x),(y)))
*/

extern const float cos_table[360];
extern const float sin_table[360];
#endif // __FMATH_H
