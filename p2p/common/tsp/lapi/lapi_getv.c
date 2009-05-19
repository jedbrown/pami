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


//#undef TRACE
//#define TRACE(x) fprintf x

/* **************************************************************** */
/*  GETV Request header:                                            */
/* **************************************************************** */

typedef struct 
{
  __pgasrt_svd_handle_t         src;            /* SVD of data source */
  size_t                        offset;         /* offset relative to SVD */
  __pgasrt_local_addr_t         destAddr;       /* address at destination */
  unsigned int                  destNode;       /* destination node */
  unsigned                      elemSize;       /* # bytes in each stride */
  unsigned                      stride;         /* stride size */
  unsigned                      numElems;       /* # of elements */
  lapi_vec_t                  * vec;            /* stride info */
  unsigned int                * cmplt;          /* target counter */
}
GetvRequestInfo;

/* **************************************************************** */
/*  GETV Reply Header: data structure send with data                */
/* **************************************************************** */

typedef struct
{
  lapi_vec_t                  * vec;
  lapi_vec_t                    rplyvec;
  unsigned long long            rplyvecinfo[3];
  unsigned int                * cmplt;
}
GetvReplyInfo;

/* ************************************************************************* */
/*                       Post a GETV request                                 */
/* ************************************************************************* */

#pragma weak __pgasrt_tsp_getv = ___pgasrt_tsp_getv
void * ___pgasrt_tsp_getv (unsigned                       localThread,
			   unsigned                       srcNode,
			   __pgasrt_local_addr_t          dst,
			   __pgasrt_svd_handle_t          src,
			   size_t                         offset,
			   size_t                         elemSize,
			   size_t                         stride,
			   size_t                         numElems)
{

  /* --------------------- */
  /* create request        */
  /* --------------------- */

  __pgasrt_lapi_request_t * r = __pgasrt_lapi_pool_allocate();
  r->vec.vec_type = LAPI_GEN_STRIDED_XFER;
  r->vec.len      = NULL;
  r->vec.num_vecs = numElems;
  r->vec.info     = (void **)&r->info;
  r->vec.info[0]  = (void *)dst;
  r->vec.info[1]  = (void *)elemSize;
  r->vec.info[2]  = (void *)elemSize;

  /* --------------------- */
  /* create request header */
  /* --------------------- */
#if defined(__cplusplus)
  GetvRequestInfo reqInfo = {src,offset,dst,
			     __pgasrt_lapi_mynode,elemSize,
			     stride,numElems,&r->vec, &r->cmplt};
#else
  GetvRequestInfo reqInfo = {
    .src            = src,
    .offset         = offset,
    .destAddr       = dst,
    .destNode       = __pgasrt_lapi_mynode,    
    .elemSize       = elemSize,
    .stride         = stride,
    .numElems       = numElems,
    .vec            = &r->vec,
    .cmplt          = &r->cmplt
  };
#endif
  /* ------------------------------------------ */
  /* send an active message with the GET header */
  /* ------------------------------------------ */

  lapi_xfer_t xfer_struct;
  xfer_struct.Am.Xfer_type = LAPI_AM_LW_XFER;
  xfer_struct.Am.flags     = 0;
  xfer_struct.Am.tgt       = srcNode;
  xfer_struct.Am.hdr_hdl   = (lapi_long_t)PGASRT_LAPI_GETVREQUESTHANDLER;
  xfer_struct.Am.uhdr      = (void *) &reqInfo;
  xfer_struct.Am.uhdr_len  = sizeof(reqInfo);
  xfer_struct.Am.udata     = NULL;
  xfer_struct.Am.udata_len = 0;
  CALL_AND_CHECK_RC((LAPI_Xfer(__pgasrt_lapi_handle, &xfer_struct)));

  /* ------------------------------------------ */
  /*  announce GETV request                     */
  /* ------------------------------------------ */

  TRACE((stderr, "%d: GETREQV [%d,%d] off=%d req=%p\n",
	 (unsigned)__pgasrt_lapi_mynode, 
	 (unsigned)src._partition,
	 (unsigned)src._index,
	 (unsigned)offset,
	 r));

  return (void *)r;
}

/* ************************************************************************* */
/*         GETV request handler : incoming packet                            */
/* ************************************************************************* */

static void reqCH    (lapi_handle_t * hndl, void * p);
static void rplySent (lapi_handle_t *, void *, lapi_sh_info_t *);
static void rplyCH   (lapi_handle_t * hndl, void * p);

void * __pgasrt_lapi_getvRequestHandler (lapi_handle_t           * hndl,
					 void                    * uhdr,
					 uint                    * uhdr_len,
					 ulong                   * retinfo,
					 compl_hndlr_t          ** comp_h,
					 void                   ** uinfo)
{
  lapi_return_info_t * ri = (lapi_return_info_t *) retinfo;
  *comp_h                 = reqCH;
  *uinfo                  = uhdr;
  ri->ret_flags           = LAPI_SEND_REPLY;
  ri->ctl_flags           = LAPI_BURY_MSG; /* there is no payload to deliver */
  ri->dgsp_handle         = NULL;
  ri->bytes               = 0;
  return NULL;
}

/* ************************************************************************* */
/*           GETV request completion handler: start sending reply            */
/* ************************************************************************* */

void reqCH (lapi_handle_t * hndl, void * p)
{
  GetvRequestInfo * hi = (GetvRequestInfo *) p;

  __pgasrt_local_addr_t la = __pgasrt_svd_addrof(hi->src);

  /* ----------------------------------------- */
  /* create a reply header and request object  */
  /* ----------------------------------------- */
  
  GetvReplyInfo       * hdr;
  CHECK_NULL (hdr,(GetvReplyInfo *)malloc(sizeof(GetvReplyInfo)));

  /* ---------------------------------------- */
  /* set strided vector part of reply header  */
  /* ---------------------------------------- */

  hdr->vec               = hi->vec;
  hdr->rplyvec.vec_type  = LAPI_GEN_STRIDED_XFER;
  hdr->rplyvec.info      = (void **)&hdr->rplyvecinfo;
  hdr->rplyvec.len       = NULL;
  hdr->rplyvec.num_vecs  = hi->numElems;
  hdr->rplyvec.info[0]   = (void *) (la + hi->offset);
  hdr->rplyvec.info[1]   = (void *) hi->elemSize;
  hdr->rplyvec.info[2]   = (void *) hi->stride;
  hdr->cmplt             = hi->cmplt;
  
  /* ------------------------------------- */
  /* Send the Reply Message                */
  /* ------------------------------------- */

  lapi_xfer_t xfer_struct;
  xfer_struct.Amv.Xfer_type = LAPI_AMV_XFER;
  xfer_struct.Amv.flags     = 0;
  xfer_struct.Amv.tgt       = hi->destNode;
  xfer_struct.Amv.hdr_hdl   = (lapi_long_t) PGASRT_LAPI_GETVREPLYHANDLER;
  xfer_struct.Amv.uhdr      = (void *) hdr;
  xfer_struct.Amv.uhdr_len  = sizeof(GetvReplyInfo);
  xfer_struct.Amv.shdlr     = (scompl_hndlr_t *) rplySent;
  xfer_struct.Amv.sinfo     = (void *) hdr;
  xfer_struct.Amv.org_vec   = &hdr->rplyvec;
  xfer_struct.Amv.org_cntr  = NULL;
  xfer_struct.Amv.cmpl_cntr = NULL;
  xfer_struct.Amv.tgt_cntr  = NULL;
  CALL_AND_CHECK_RC((LAPI_Xfer(*hndl, &xfer_struct)));
}

/* ************************************************************************* */
/*                 free reply header once reply is sent                      */
/* ************************************************************************* */

void rplySent (lapi_handle_t   * handle,
	       void            * param,
	       lapi_sh_info_t  * info)
{
  TRACE((stderr, 
	 "%d: %s(hdr=%p)\n", __pgasrt_lapi_mynode, __FUNCTION__, param));
  GetvReplyInfo *hdr = (GetvReplyInfo *) param;
  free(hdr);
}

/* ************************************************************************* */
/*                 handle incoming GETV reply                                */
/* ************************************************************************* */

lapi_vec_t * __pgasrt_lapi_getvReplyHandler (lapi_handle_t         * handle, 
					     void                  * uhdr,
					     uint                    uhdr_len,
					     uint                  * vec_len[],
					     compl_hndlr_t        ** comp_h,
					     void                 ** uinfo)
{
  GetvReplyInfo        * hi = (GetvReplyInfo *) uhdr;
  *comp_h = rplyCH;
  *uinfo = hi->cmplt;
  return hi->vec;
}

void rplyCH (lapi_handle_t * hndl, void * p)
{
  * ((int *) p) = 1;
}


#if defined(__cplusplus)
}
#endif
