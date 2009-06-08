/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file adaptor/ccmi_util.h
 * \brief ???
 */

#ifndef   __basic_ccmi_util_types_h__
#define   __basic_ccmi_util_types_h__

//#define throw() // Needed for xlC -noeh processing
//#include <config.h>

#include <stdio.h>
#include <assert.h>
#include <string.h>

#define TRACE_ERR(x)     //fprintf x
#define MEMCPY           memcpy
#define MEMSET           memset

#if ASSERT_LEVEL==0
  #define CCMI_abort()         assert(0)
  #define CCMI_assert(x)
  #define CCMI_assert_debug(x)
#elif ASSERT_LEVEL==1
  #define CCMI_abort()         assert(0)
  #define CCMI_assert(x)       assert(x)
  #define CCMI_assert_debug(x)
#else /* ASSERT_LEVEL==2 */
  #define CCMI_abort()         assert(0)
  #define CCMI_assert(x)       assert(x)
  #define CCMI_assert_debug(x) assert(x)
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

#define __dcmf_ccmi_new_defined__
#ifndef  __dcmf_ccmi_new_defined__
  #define  __dcmf_ccmi_new_defined__
  #ifdef __cplusplus
inline void* operator new(size_t obj_size, void* pointer, size_t avail_size)
{
/*   printf("%s: From %p for %u out of %u\n", __PRETTY_FUNCTION__, pointer, obj_size, avail_size); */
  CCMI_assert_debug(pointer != NULL);
  CCMI_assert_debug(obj_size <= avail_size);
  return pointer;
}
// Just to keep BEAM from complaining
inline void operator delete(void* pointer0, void* pointer, size_t avail_size)
{
/*   printf("%s: From %p for %u out of %u\n", __PRETTY_FUNCTION__, pointer, obj_size, avail_size); */
  CCMI_assert_debug(0);
  return;
}

inline void* operator new(size_t obj_size, void* pointer)
{
/*   printf("%s: From %p for %u\n", __PRETTY_FUNCTION__, pointer, obj_size); */
  CCMI_assert_debug(pointer != NULL);
  return pointer;
}
// Just to keep BEAM from complaining
inline void operator delete(void* pointer0, void* pointer)
{
/*   printf("%s: From %p for %u\n", __PRETTY_FUNCTION__, pointer, obj_size); */
  CCMI_assert_debug(0);
  return;
}
  #endif
#endif

#if defined(__bgl__) || defined(__bgx__) || defined(__bgxl__)
  #define MAX_NUM_CORES	2
#elif defined(__bgp__)
  #define MAX_NUM_CORES	4
// ...and so on, for BGQ, ...
#endif /* __bgl__ || __bgx__ ... */

#endif
