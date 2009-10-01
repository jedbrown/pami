/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file util/common.h
 * \brief Basic header file to define simple and common items
 */
#ifndef __util_common_h__
#define __util_common_h__

#include <new>
#include <stdint.h>
#include <stdio.h>

#include "sys/xmi.h"
#ifndef MIN
#define MIN(a,b)  (((a)<(b))?(a):(b))
#endif

#ifndef MAX
#define MAX(a,b)  (((a)>(b))?(a):(b))
#endif

#ifndef CEIL
#define CEIL(x,y) (((x)+(y)-1)/(y))
#endif

#define XMIQuad_sizeof(x)  ((sizeof(x)+15)>>4)

/// abort macros defined for all assertion levels
#define XMI_abort()                       abort()
#define XMI_abortf(fmt...)                { fprintf(stderr, __FILE__ ":%d: ", __LINE__); fprintf(stderr, fmt); abort(); }

#if ASSERT_LEVEL==0    // All asserts are disabled

  #define XMI_assert(expr)
  #define XMI_assertf(expr, fmt...)
  #define XMI_assert_debug(expr)
  #define XMI_assert_debugf(expr, fmt...)

#elif ASSERT_LEVEL==1  // Only "normal" asserts, not debug, are enabled

  #define XMI_assert(expr)                assert(expr)
  #define XMI_assertf(expr, fmt...)       { if (!(expr)) XMI_abortf(fmt); }
  #define XMI_assert_debug(expr)
  #define XMI_assert_debugf(expr, fmt...)

#else // ASSERT_LEVEL==2 ... All asserts are enabled

  #define XMI_assert(expr)                assert(expr)
  #define XMI_assertf(expr, fmt...)       { if (!(expr)) XMI_abortf(fmt); }
  #define XMI_assert_debug(expr)          assert(expr)
  #define XMI_assert_debugf(expr, fmt...) XMI_assertf(expr, fmt)

#endif // ASSERT_LEVEL


static inline int64_t min_nb64(int64_t x,int64_t y)
{
  return x+(((y-x)>>(63))&(y-x));
}

static inline int32_t min_nb32(int32_t x,int32_t y)
{
  return x+(((y-x)>>(31))&(y-x));
}

static inline int64_t max_nb64(int64_t x,int64_t y)
{
  return x-(((x-y)>>(63))&(x-y));
}

static inline int32_t max_nb32(int32_t x,int32_t y)
{
  return x-(((x-y)>>(31))&(x-y));
}
#if 0
inline void* operator new(size_t obj_size, void* pointer)
{
/*   printf("%s: From %p for %u\n", __PRETTY_FUNCTION__, pointer, obj_size); */
//  CCMI_assert_debug(pointer != NULL);
  return pointer;
}
#endif

/**
 * \brief Creates a compile error if the condition is false.
 *
 * This macro must be used within a function for the compiler to process it.
 * It is suggested that C++ classes and C files create an inline function
 * similar to the following example. The inline function is never used at
 * runtime and should be optimized out by the compiler. It exists for the sole
 * purpose of moving runtime \c assert calls to compile-time errors.
 *
 * \code
 * static inline void compile_time_assert ()
 * {
 *   // This compile time assert will succeed.
 *   COMPILE_TIME_ASSERT(sizeof(char) <= sizeof(double));
 *
 *   // This compile time assert will fail.
 *   COMPILE_TIME_ASSERT(sizeof(double) <= sizeof(char));
 * }
 * \endcode
 *
 * Compile time assert errors will look similar to the following:
 *
 * \code
 * foo.h: In function compile_time_assert:
 * foo.h:43: error: duplicate case value
 * foo.h:43: error: previously used here
 * \endcode
 */
#define COMPILE_TIME_ASSERT(expr) switch(0){case 0:case expr:;}

typedef xmi_geometry_t (*xmi_mapidtogeometry_fn) (int comm);


#endif // __util_common_h__
