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


#include "pgasrt.h"
#include "pgasrt_lapi.h"
#if defined(__cplusplus)
extern "C" {
#endif


#undef TRACE
#define TRACE(x)
//#define TRACE(x) printf x


/* ************************************************************************ */
/*          explicitly pin and unpin memory for RDMA operations             */
/* ------------------------------------------------------------------------ */
/* Memory registration fails silently.                                      */
/* ************************************************************************ */

#pragma weak __pgasrt_tsp_register_memory=___pgasrt_tsp_register_memory
__xlupc_addrdesc_t
___pgasrt_tsp_register_memory   (void                * ptr, 
				  unsigned int          len)
{
#if 0
  lapi_get_pvo_t util_pvo;
  util_pvo.Util_type = LAPI_XLATE_ADDRESS;
  util_pvo.length    = len;
  util_pvo.usr_pvo   = 0;
  util_pvo.address   = ptr;
  util_pvo.operation = LAPI_RDMA_ACQUIRE;
  int rc=LAPI_Util(__pgasrt_lapi_handle, (lapi_util_t *) &util_pvo);
  TRACE(("%d: registered memory: %p, len=%d rc=%d pvo=%lld\n",
	 __pgasrt_lapi_mynode,
	 ptr, 
	 len, 
	 rc,
	 util_pvo.usr_pvo));
  return util_pvo.usr_pvo;
#else
  return 0;
#endif
}

/* ************************************************************************ */
/*               free memory                                                */
/* ************************************************************************ */

#pragma weak __pgasrt_tsp_deregister_memory=___pgasrt_tsp_deregister_memory
void
___pgasrt_tsp_deregister_memory (void                * ptr, 
				  __xlupc_addrdesc_t    desc,
				  unsigned int          len)
{
#if 0
  lapi_get_pvo_t util_pvo;
  util_pvo.Util_type = LAPI_XLATE_ADDRESS;
  util_pvo.usr_pvo   = desc;
  util_pvo.operation = LAPI_RDMA_RELEASE;
  LAPI_Util(__pgasrt_lapi_handle, (lapi_util_t *) &util_pvo);
#endif
}

#if defined(__cplusplus)
}
#endif
