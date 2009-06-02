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
/*                hybrid barrier                                           */
/* *********************************************************************** */

#pragma weak __pgasrt_hbrd_barrier=___pgasrt_hbrd_barrier
void ___pgasrt_hbrd_barrier (void)
{
  __pgasrt_smp_barrier ();
  if (PGASRT_NODES<=1) return;
  if (PGASRT_MYSMPTHREAD==0) __pgasrt_tsp_barrier ();
  __pgasrt_smp_barrier ();
}

/* *********************************************************************** */
/*                    hybrid fence                                         */
/* *********************************************************************** */

#pragma weak __pgasrt_hbrd_fence=___pgasrt_hbrd_fence
void ___pgasrt_hbrd_fence()
{
  __pgasrt_smp_fence();
  if (PGASRT_NODES>1) __pgasrt_tsp_fence(PGASRT_MYSMPTHREAD);
}

#if defined(__cplusplus)
}
#endif
