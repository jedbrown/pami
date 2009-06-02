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


/* **************************************************************** */
/*  Accumulate header: data struct used in an accumulate request    */
/* **************************************************************** */

typedef struct
{
  __pgasrt_svd_handle_t         dst;
  size_t                        offset;
  __pgasrt_dtypes_t             optype;
  __pgasrt_ops_t                op;
  unsigned long long            data;
}
AccRequestInfo;


/* ********************************************************************** */
/*           allocate and post an ACCUMULATE request                      */
/* ********************************************************************** */

/* !!! broken implementation - we don't wait for any acknowledgements. */

#pragma weak __pgasrt_tsp_update = ___pgasrt_tsp_update
void * ___pgasrt_tsp_update (unsigned localThread,
			  unsigned destNode,
			  __pgasrt_svd_handle_t dst,
			  size_t offset,
			  __pgasrt_local_addr_t src,
			  __pgasrt_dtypes_t optype,
			  __pgasrt_ops_t op)
{
  /* ------------------------------- */
  /* create header and push out data */
  /* ------------------------------- */

  // __pgasrt_lapi_request_t * r = __pgasrt_lapi_pool_allocate();

  AccRequestInfo hdr;
  hdr.dst     = dst;
  hdr.offset  = offset;
  hdr.optype  = optype;
  hdr.op      = op;

  switch (optype)
    {
    case PGASRT_DT_byte: memcpy (&hdr.data, src, sizeof(char));      break;
    case PGASRT_DT_chr:  memcpy (&hdr.data, src, sizeof(char));      break;
    case PGASRT_DT_srt:  memcpy (&hdr.data, src, sizeof(short));     break;
    case PGASRT_DT_hwrd: memcpy (&hdr.data, src, sizeof(short));     break;
    case PGASRT_DT_int:  memcpy (&hdr.data, src, sizeof(int));       break;
    case PGASRT_DT_word: memcpy (&hdr.data, src, sizeof(int));       break;
    case PGASRT_DT_llg:  memcpy (&hdr.data, src, sizeof(long long)); break;
    case PGASRT_DT_dwrd: memcpy (&hdr.data, src, sizeof(long long)); break;
    case PGASRT_DT_dbl:  memcpy (&hdr.data, src, sizeof(double));    break;
    case PGASRT_DT_flt:  memcpy (&hdr.data, src, sizeof(float));     break;
    }

  lapi_xfer_t xfer_struct;
  xfer_struct.Am.Xfer_type = LAPI_AM_LW_XFER;
  xfer_struct.Am.flags     = 0;
  xfer_struct.Am.tgt       = destNode;
  xfer_struct.Am.hdr_hdl   = (lapi_long_t)PGASRT_LAPI_ACCREQUESTHANDLER;
  xfer_struct.Am.uhdr      = (void *) &hdr;
  xfer_struct.Am.uhdr_len  = sizeof(hdr);
  xfer_struct.Am.udata     = NULL;
  xfer_struct.Am.udata_len = 0;
  CALL_AND_CHECK_RC((LAPI_Xfer(__pgasrt_lapi_handle, &xfer_struct)));

  // return (void *) r;
  return NULL;
}

/* ************************************************************************* */
/*                     request handler for accumulate                        */
/* ************************************************************************* */

void * __pgasrt_lapi_accRequestHandler     (lapi_handle_t           * hndl,
					    void                    * uhdr,
					    uint                    * uhdr_len,
					    ulong                   * retinfo,
					    compl_hndlr_t          ** comp_h,
					    void                   ** uinfo)
{
  lapi_return_info_t    * ri = (lapi_return_info_t *) retinfo;
  AccRequestInfo        * hi = (AccRequestInfo *) uhdr;

  void * la      = __pgasrt_svd_addrof (hi->dst) + hi->offset;
  void * src     = (void *) & hi->data;
  __pgasrt_update ((unsigned char*)la, (unsigned char *)src, hi->optype, hi->op);

  ri->ret_flags = LAPI_LOCAL_STATE;
  ri->ctl_flags = LAPI_BURY_MSG;
  return NULL;
}

#if defined(__cplusplus)
}
#endif
