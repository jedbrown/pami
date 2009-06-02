/* ************************************************************************* */
/*                            IBM Confidential                               */
/*                          OCO Source Materials                             */
/*                      IBM XL UPC Alpha Edition, V0.9                       */
/*                                                                           */
/*                      Copyright IBM Corp. 2005, 2007.                      */
/*                                                                           */
/* The source code for this program is not published or otherwise divested   */
/* of its trade secrets, irrespective of what has been deposited with the    */
/* U.S. Copyright Office.                                                    */
/* ************************************************************************* */


#include <assert.h>
#include <stdio.h>
#include "../include/pgasrt.h"

#if defined(__cplusplus)
extern "C" {
#endif


/* *********************************************************************** */
/*                   SMP fence                                             */
/* *********************************************************************** */

#pragma weak __pgasrt_smp_fence=___pgasrt_smp_fence
void ___pgasrt_smp_fence ()
{
#if defined(__xlC__)
  __sync();
#elif defined(__GNUC__)
#if defined(__i386__) || defined(__x86_64__)
  __asm__ __volatile__ ("lock; addl $0,0(%%esp)": : :"memory");
#else
  asm volatile ("sync" : : : "memory");
#endif /* __i386__ */
#endif /* __GNUC__ */
}

#if defined(__cplusplus)
}
#endif
