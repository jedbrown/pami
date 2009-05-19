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


/* *********************************************************************** */
/*                       Post a GET request                                */
/* *********************************************************************** */
#pragma weak __pgasrt_tsp_dget=___pgasrt_tsp_dget

static void dgetCH    (lapi_handle_t * hndl, void * p);

void * ___pgasrt_tsp_dget (unsigned                      localThread,
			   unsigned int                  srcNode,
			   __pgasrt_local_addr_t         dst,
			   __pgasrt_addrdesc_t           remote_src,
			   size_t                        offset,
			   size_t                        len)
{
  TRACE(("%d: DGET(%d) lbuf=%p rmt=%0x off=%d len=%d\n",
	 (unsigned)__pgasrt_lapi_mynode, 
	 (unsigned)srcNode,
	 (void *)dst, remote_src, len));

  __pgasrt_lapi_request_t * r = __pgasrt_lapi_pool_allocate();
  
  lapi_xfer_t xfer_struct;
  xfer_struct.Get.Xfer_type = LAPI_GET_XFER;
  xfer_struct.Get.flags     = 0;
  xfer_struct.Get.tgt       = srcNode;
  xfer_struct.Get.tgt_addr  = (lapi_long_t) (remote_src + offset);
  xfer_struct.Get.org_addr  = (void *) dst;
  xfer_struct.Get.len       = (ulong) len;
  xfer_struct.Get.chndlr    = dgetCH;
  xfer_struct.Get.cinfo     = (void *) r;
  xfer_struct.Get.org_cntr  = NULL;
  xfer_struct.Put.cmpl_cntr = NULL;
  xfer_struct.Get.tgt_cntr  = NULL;
  CALL_AND_CHECK_RC((LAPI_Xfer(__pgasrt_lapi_handle, &xfer_struct)));

  return (void *) r;
}

static void dgetCH (lapi_handle_t * hndl, void * p)
{
  ((unsigned *)p)[0] = 1;
}

#if defined(__cplusplus)
}
#endif
