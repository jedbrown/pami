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
/*                       Post a PUT request                                */
/* *********************************************************************** */
#pragma weak __pgasrt_tsp_dput =___pgasrt_tsp_dput
static void dputCH    (lapi_handle_t * hndl, void * p, lapi_sh_info_t *);

void * ___pgasrt_tsp_dput (unsigned                 localThread,
			   unsigned                 destNode,
			   __pgasrt_addrdesc_t      dst,
			   size_t                   offset,
			   __pgasrt_local_addr_t    src,
			   size_t                   len)
{
  __pgasrt_lapi_request_t * r = __pgasrt_lapi_pool_allocate();
  
  lapi_xfer_t xfer_struct;
  xfer_struct.Put.Xfer_type = LAPI_PUT_XFER;
  xfer_struct.Put.flags     = 0;
  xfer_struct.Put.tgt       = destNode;
  xfer_struct.Put.tgt_addr  = (lapi_long_t) (dst + offset);
  xfer_struct.Put.org_addr  = (void *) src;
  xfer_struct.Put.len       = (ulong) len;
  xfer_struct.Put.shdlr     = dputCH;
  xfer_struct.Put.sinfo     = (void *) r;
  xfer_struct.Put.org_cntr  = NULL;
  xfer_struct.Put.tgt_cntr  = NULL;
  xfer_struct.Put.cmpl_cntr = NULL;
  CALL_AND_CHECK_RC((LAPI_Xfer(__pgasrt_lapi_handle, &xfer_struct)));

  return (void *) r;
}


static void dputCH (lapi_handle_t * hndl, void * p, lapi_sh_info_t * info)
{
  ((unsigned *)p)[0] = 1;
}

#if defined(__cplusplus)
}
#endif
