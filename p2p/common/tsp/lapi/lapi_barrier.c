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


#include "../../include/pgasrt.h"
#include "pgasrt_lapi.h"
#if defined(__cplusplus)
extern "C" {
#endif


/* ************************************************************************* */
/*                         simple barrier                                    */
/* ************************************************************************* */

#pragma weak __pgasrt_tsp_barrier=___pgasrt_tsp_barrier
void ___pgasrt_tsp_barrier (void) 
{
  LAPI_Gfence (__pgasrt_lapi_handle);
}

/* ************************************************************************* */
/*                        simple fence                                       */
/* ************************************************************************* */

#pragma weak __pgasrt_tsp_fence=___pgasrt_tsp_fence
void ___pgasrt_tsp_fence (unsigned localThread)
{
  LAPI_Fence (__pgasrt_lapi_handle);
}

#if defined(__cplusplus)
}
#endif
