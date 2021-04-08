#ifndef __MATH_H__
#define __MATH_H__

#  define __LEAF
#  define __THROW	__attribute__ ((__nothrow__ __LEAF))
# define __USING_NAMESPACE_STD(name)
# define __BEGIN_NAMESPACE_C99
# define __END_NAMESPACE_C99
# define __USING_NAMESPACE_C99(name)
#define __CONCAT(x,y)	x ## y
#define _Mdouble_		double
# define _Mdouble_BEGIN_NAMESPACE __BEGIN_NAMESPACE_C99
# define _Mdouble_END_NAMESPACE   __END_NAMESPACE_C99

#define __MATH_PRECNAME(name,r)	__CONCAT(name,r)

#define __MATHDECL_1(type, function,suffix, args) \
  extern type __MATH_PRECNAME(function,suffix) args __THROW

#define __MATHDECL(type, function,suffix, args) \
  __MATHDECL_1(type, function,suffix, args); \
  __MATHDECL_1(type, __CONCAT(__,function),suffix, args)
#define __MATHCALL(function,suffix, args)	\
  __MATHDECL (_Mdouble_,function,suffix, args)
#define __MATHCALLX(function,suffix, args, attrib)	\
  __MATHDECLX (_Mdouble_,function,suffix, args, attrib)
#define __MATHDECLX(type, function,suffix, args, attrib) \
  __MATHDECL_1(type, function,suffix, args) __attribute__ (attrib); \
  __MATHDECL_1(type, __CONCAT(__,function),suffix, args) __attribute__ (attrib)


/* Smallest integral value not less than X.  */
_Mdouble_BEGIN_NAMESPACE
__MATHCALLX (ceil,, (_Mdouble_ __x), (__const__));

/* Absolute value of X.  */
__MATHCALLX (fabs,, (_Mdouble_ __x), (__const__));

/* Largest integer not greater than X.  */
__MATHCALLX (floor,, (_Mdouble_ __x), (__const__));

/* Floating-point modulo remainder of X/Y.  */
__MATHCALL (fmod,, (_Mdouble_ __x, _Mdouble_ __y));
_Mdouble_END_NAMESPACE
#endif