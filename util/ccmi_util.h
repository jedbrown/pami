/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file util/ccmi_util.h
 * \brief ???
 */

#ifndef __util_ccmi_util_h__
#define __util_ccmi_util_h__

//#define throw() // Needed for xlC -noeh processing
//#include <config.h>

#include <new>
#include <stdio.h>
#include <assert.h>
#include <string.h>


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

#define CCMIQuad_sizeof(x)  ((sizeof(x)+15)>>4)

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
