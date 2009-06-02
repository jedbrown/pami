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
/*  Put Header: data structure send with data in a PUT              */
/* **************************************************************** */

typedef struct 
{
  __pgasrt_svd_handle_t        dst;
  size_t                       offset;
  size_t                       elemSize;    /* bytes in a chunk */
  size_t                       stride;      /* distance btw. chunks */
  size_t                       numElems;    /* number of chunks */
  unsigned                     srcNode;
  unsigned int               * cmplt;
  __pgasrt_lapi_request_t             * r;
}
PutvRequestInfo;

/* **************************************************************** */
/*        send a PUT                                                */
/* **************************************************************** */

#pragma weak __pgasrt_tsp_putv=___pgasrt_tsp_putv
void * ___pgasrt_tsp_putv (unsigned                     localThread,
			   unsigned                     destNode,
			   __pgasrt_svd_handle_t        dst,
			   size_t                       offset,
			   __pgasrt_local_addr_t        src,
			   size_t                       elemSize,
			   size_t                       stride,
			   size_t                       numElems)
{
  /* ------------------ */
  /*  create request    */
  /* ------------------ */

  // __pgasrt_fatalerror(-1, "___pgasrt_tsp_putv Not implemented");
  
  __pgasrt_lapi_request_t * r = __pgasrt_lapi_pool_allocate();
  r->cmplt = 0;
  
  r->vec.vec_type = LAPI_GEN_STRIDED_XFER;
  r->vec.len      = NULL;
  r->vec.num_vecs = numElems;
  r->vec.info     = (void **)&r->info;
  r->vec.info[0]  = (void *)src;
  r->vec.info[1]  = (void *)elemSize;
  r->vec.info[2]  = (void *)elemSize;
  
  /* --------------------- */
  /* create request header */
  /* --------------------- */
#if defined(__cplusplus)
  PutvRequestInfo hdr = {dst,offset,numElems,
			 elemSize,stride,
			 __pgasrt_lapi_mynode,
			 &r->cmplt, NULL};
#else  
  PutvRequestInfo hdr = { 
    .dst          = dst,
    .offset       = offset,
    .numElems     = numElems, 
    .elemSize     = elemSize,
    .stride       = stride,
    .srcNode      = __pgasrt_lapi_mynode,
    .cmplt        = &r->cmplt,
    .r            = NULL
  };
#endif
  lapi_xfer_t xfer_struct;

  xfer_struct.Amv.Xfer_type = LAPI_AMV_XFER;
  xfer_struct.Amv.flags     = 0;
  xfer_struct.Amv.tgt       = destNode;
  xfer_struct.Amv.hdr_hdl   = (lapi_long_t)PGASRT_LAPI_PUTVREQUESTHANDLER;
  xfer_struct.Amv.uhdr_len  = sizeof(hdr);
  xfer_struct.Amv.uhdr      = (void *) &hdr;

  xfer_struct.Amv.org_vec   = &r->vec;

  xfer_struct.Amv.shdlr     = NULL;
  xfer_struct.Amv.sinfo     =  0;
  xfer_struct.Amv.org_cntr  = NULL;
  xfer_struct.Amv.tgt_cntr  = NULL;
  xfer_struct.Amv.cmpl_cntr = NULL;

  CALL_AND_CHECK_RC((LAPI_Xfer(__pgasrt_lapi_handle, &xfer_struct)));
  return (void *) r;
}

/**
 * **************************************************************************
 * \brief put header handler. Invoked on the receiver side when first       *
 * packet of the data has arrived.                                          *
 *                                                                          *
 *  \param hndl      lapi handle                                            *
 *  \param uhdr      pointer to user header (contains the svd entry of      *
 *                   the destination variable)                              *
 *  \param uhdr_len  the length of the user header (the svd entry)          *
 *  \param msg_len   message length or lapi_return_info_t pointer           *
 *  \param comp_h    OUT parameter for a completion handler                 *
 *  \param uinfo     OUT parameter for completion handler params.           *
 * **************************************************************************
 */
  static void reqCH (lapi_handle_t *, void *);

lapi_vec_t * __pgasrt_lapi_putvRequestHandler (lapi_handle_t   * hndl,
					       void            * uhdr,
					       uint              uhdr_len,
					       uint            * vec_len[],
					       compl_hndlr_t  ** comp_h,
					       void           ** uinfo)
{
  PutvRequestInfo *hi = (PutvRequestInfo *) uhdr;  

  __pgasrt_lapi_request_t * r = __pgasrt_lapi_pool_allocate();

  r->vec.vec_type = LAPI_GEN_STRIDED_XFER;
  r->vec.len      = NULL;
  r->vec.num_vecs = hi->numElems;
  r->vec.info     = (void **)&r->info;
  r->vec.info[0]  = (void *) (__pgasrt_svd_addrof (hi->dst) + hi->offset);
  r->vec.info[1]  = (void *) hi->elemSize;
  r->vec.info[2]  = (void *) hi->stride;
  hi->r           = r;

  * comp_h        = reqCH;
  * uinfo         = uhdr;
  return &r->vec;
}

/* ************************************************************************ */
/* completion handler on target: free temp. buffers, deliver ack to sender  */
/* ************************************************************************ */

static void reqCH (lapi_handle_t *handle, void *param)
{
  PutvRequestInfo *hi = (PutvRequestInfo *) param;
  __pgasrt_lapi_pool_free (hi->r);

  /* ------------------------------------- */
  /*     send an ack                       */
  /* ------------------------------------- */

  TRACE((stderr, "%d: PUT ack to %d cmplt=%p\n",
         __pgasrt_lapi_mynode, hi->srcNode, hi->cmplt));

  lapi_xfer_t xfer_struct;
  xfer_struct.Am.Xfer_type = LAPI_AM_LW_XFER;
  xfer_struct.Am.flags     = 0;
  xfer_struct.Am.tgt       = hi->srcNode;
  xfer_struct.Am.hdr_hdl   = (lapi_long_t)PGASRT_LAPI_PUTREPLYHANDLER;
  xfer_struct.Am.uhdr      = (void *)& hi->cmplt;
  xfer_struct.Am.uhdr_len  = sizeof(void *);
  xfer_struct.Am.udata     = NULL;
  xfer_struct.Am.udata_len = 0;
  CALL_AND_CHECK_RC((LAPI_Xfer(__pgasrt_lapi_handle, &xfer_struct)));
}

/* ************************************************************************ */
/*          deliver "send complete" acknowledgement to sender               */
/* ************************************************************************ */

void * __pgasrt_lapi_putvReplyHandler    (lapi_handle_t           * hndl,
                                          void                    * uhdr,
                                          uint                    * uhdr_len,
                                          ulong                   * retinfo,
                                          compl_hndlr_t          ** comp_h,
                                          void                   ** uinfo)
{
  unsigned int * cmplt = * (unsigned int **)uhdr;
  TRACE((stderr, "%d:  PUTV ACK received cmplt=%p \n",
         __pgasrt_lapi_mynode, cmplt));
  * cmplt = 1;

  lapi_return_info_t    * ri = (lapi_return_info_t *) retinfo;
  ri->ret_flags   = LAPI_LOCAL_STATE; /* no ack */
  ri->ctl_flags   = LAPI_BURY_MSG; /* there is no payload to deliver */

  return NULL;
}

#if defined(__cplusplus)
}
#endif
