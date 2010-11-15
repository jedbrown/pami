/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
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

#if defined(__bgl__) || defined(__bgx__) || defined(__bgxl__)
  #define MAX_NUM_CORES	2
#elif defined(__bgp__)
  #define MAX_NUM_CORES	4
// ...and so on, for BGQ, ...
#endif /* __bgl__ || __bgx__ ... */

// \brief do some debug code
#undef DO_DEBUG
#define DO_DEBUG(x) x

/// \brief Trace the template name: eg.
/// DO_DEBUG((templateName<T_Mcast>()));
///   ...results in something like this traced:
/// templateName() [with T_Model = PAMI::Device::LocalBcastWQModel]
template <class T>
inline void templateName()
{
  fprintf(stderr, "%s\n", __PRETTY_FUNCTION__);
}

#endif
