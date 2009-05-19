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
/*  GET Request header:                                             */
/* **************************************************************** */

typedef struct 
{
  __pgasrt_svd_handle_t     src;        /** where I want data from */
  size_t                    offset;     /** offset relative to SVD */
  __pgasrt_local_addr_t     destAddr;   /** where I want data deposited */
  unsigned int              destNode;   /** "me" */
  size_t                    len;        /** how much data I want */
  int                     * cmplt;      /** my completion flag */
}
GetRequestInfo;

/* **************************************************************** */
/*  GET Reply Header: data structure send with data                 */
/* **************************************************************** */

typedef struct
{
  __pgasrt_local_addr_t     destAddr;   /** requester's dest address */
  int                     * cmplt;      /** requester's completion flag */
}
GetReplyInfo;

/* *********************************************************************** */
/*                       Post a GET request                                */
/* *********************************************************************** */

#pragma weak __pgasrt_tsp_get = ___pgasrt_tsp_get
void * ___pgasrt_tsp_get (unsigned                  srcThread,
			  unsigned int              destNode,
			  __pgasrt_local_addr_t     dst,
			  __pgasrt_svd_handle_t     src,
			  size_t                    offset,
			  size_t                    len,
			  __pgasrt_LCompHandler_t   comp_h,
			  void                    * info)
{
  __pgasrt_lapi_request_t * r = __pgasrt_lapi_pool_allocate();

  /* --------------------- */
  /* create request header */
  /* --------------------- */

  GetRequestInfo reqInfo;
  reqInfo.src            = src;
  reqInfo.offset         = offset;
  reqInfo.destAddr       = dst;
  reqInfo.len            = len;
  reqInfo.destNode       = __pgasrt_lapi_mynode;
  reqInfo.cmplt          = (void *)&r->cmplt;

  /* ------------------------------------------ */
  /* send an active message with the GET header */
  /* ------------------------------------------ */

  lapi_xfer_t xfer_struct;
  xfer_struct.Am.Xfer_type = LAPI_AM_LW_XFER;
  xfer_struct.Am.flags     = 0;
  xfer_struct.Am.tgt       = destNode;
  xfer_struct.Am.hdr_hdl   = (lapi_long_t)PGASRT_LAPI_GETREQUESTHANDLER;
  xfer_struct.Am.uhdr      = (void *) &reqInfo;
  xfer_struct.Am.uhdr_len  = sizeof(reqInfo);
  xfer_struct.Am.udata     = NULL;
  xfer_struct.Am.udata_len = 0;
  CALL_AND_CHECK_RC((LAPI_Xfer(__pgasrt_lapi_handle, &xfer_struct)));

  /* ------------------------------------------ */
  /* announce sending of GET request            */
  /* ------------------------------------------ */

  TRACE((stderr, "%d: GETREQ [%d,%d] dstnode=%d off=0x%x req=%p\n",
	 (unsigned)__pgasrt_lapi_mynode, 
	 (unsigned)src._partition,
	 (unsigned)src._index,
	 (unsigned)destNode,
	 (unsigned)offset,
	 r));
  
  return (void *)r;
}

/* ************************************************************************* */
/*         GET request handler : incoming packet                             */
/* ************************************************************************* */

static void reqCH    (lapi_handle_t * hndl, void * p);
static void rplySent (lapi_handle_t *, void *, lapi_sh_info_t *);
static void rplyCH (lapi_handle_t * hndl, void * p);

void * __pgasrt_lapi_getRequestHandler (lapi_handle_t           * hndl,
					void                    * uhdr,
					uint                    * uhdr_len,
					ulong                   * retinfo,
					compl_hndlr_t          ** comp_h,
					void                   ** uinfo)
{
  lapi_return_info_t * ri = (lapi_return_info_t *) retinfo;
  *comp_h         = reqCH;
  *uinfo          = uhdr;
  ri->ret_flags   = LAPI_SEND_REPLY; /* we need a token to send a reply */
  ri->ctl_flags   = LAPI_BURY_MSG; /* there is no payload to deliver */
  ri->dgsp_handle = NULL;
  ri->bytes       = 0;
  return NULL;
}

/* ************************************************************************* */
/*        LAPI completion handler for incoming GET request handler.          */
/* This is where we start sending the reply.                                 */
/* ************************************************************************* */

void reqCH (lapi_handle_t * hndl, void * p)
{
  GetRequestInfo            * hi = (GetRequestInfo *)p;
  __pgasrt_local_addr_t  la = __pgasrt_svd_addrof (hi->src);
  
  TRACE((stderr, "%d: GETREQ_CH [%d,%d] LAddr=%p Loff=%d bytes=%d dest=%d\n",
	 __pgasrt_lapi_mynode, 
	 hi->src._partition,
	 hi->src._index,
	 la,
	 hi->offset,
	 hi->len,
	 hi->destNode));

  assert (la != NULL);

  /* ------------------------------------- */
  /* Send the Reply Message                */
  /* ------------------------------------- */
  if (sizeof(GetReplyInfo) + hi->len < 128)
    {
      GetReplyInfo hdr;
      hdr.destAddr             = hi->destAddr;
      hdr.cmplt                = hi->cmplt;

      lapi_xfer_t xfer_struct;
      xfer_struct.Am.Xfer_type = LAPI_AM_LW_XFER;
      xfer_struct.Am.flags     = 0;
      xfer_struct.Am.tgt       = hi->destNode;
      xfer_struct.Am.hdr_hdl   = (lapi_long_t)PGASRT_LAPI_GETREPLYHANDLER;
      xfer_struct.Am.uhdr      = (void *) &hdr;
      xfer_struct.Am.uhdr_len  = sizeof(hdr);
      xfer_struct.Am.udata     = (void *)(la + hi->offset);
      xfer_struct.Am.udata_len = hi->len;
      CALL_AND_CHECK_RC((LAPI_Xfer(__pgasrt_lapi_handle, &xfer_struct)));

    }
  else
    {
      GetReplyInfo       * hdr;
      CHECK_NULL (hdr,(GetReplyInfo *) malloc(sizeof(GetReplyInfo)));
      hdr->destAddr            = hi->destAddr;
      hdr->cmplt               = hi->cmplt;
  
      lapi_xfer_t xfer_struct;
      xfer_struct.Am.Xfer_type = LAPI_AM_XFER;
      xfer_struct.Am.flags     = 0;
      xfer_struct.Am.tgt       = hi->destNode;
      xfer_struct.Am.hdr_hdl   = (lapi_long_t) PGASRT_LAPI_GETREPLYHANDLER;
      xfer_struct.Am.uhdr      = (void *) hdr;
      xfer_struct.Am.uhdr_len  = sizeof(GetReplyInfo);
      xfer_struct.Am.udata     = (void *) (la + hi->offset);
      xfer_struct.Am.udata_len = hi->len;
      xfer_struct.Am.shdlr     = (scompl_hndlr_t*) rplySent;
      xfer_struct.Am.sinfo     = (void *) hdr;
      xfer_struct.Am.org_cntr  = NULL;
      xfer_struct.Am.cmpl_cntr = NULL;
      xfer_struct.Am.tgt_cntr  = NULL;
      CALL_AND_CHECK_RC((LAPI_Xfer(*hndl, &xfer_struct)));
    }
}

/* ************************************************************************* */
/*              send reply is now sent; free memory                          */
/* ************************************************************************* */

void rplySent (lapi_handle_t   * handle,
	       void            * param,
	       lapi_sh_info_t  * info)
{
  TRACE((stderr, "%d: %s(hdr=%p)\n",  
	 __pgasrt_lapi_mynode, __FUNCTION__, param));
  GetReplyInfo *hdr = (GetReplyInfo *) param;
  free(hdr);
}

/* **************************************************************************/
/* GET reply header handler                                                 */
/* **************************************************************************/

void * __pgasrt_lapi_getReplyHandler (lapi_handle_t   * hndl,
				      void            * uhdr,
				      uint            * uhdr_len,
				      ulong           * retinfo,
				      compl_hndlr_t  ** comp_h,
				      void           ** uinfo)
{
  lapi_return_info_t    * ri = (lapi_return_info_t *) retinfo;
  GetReplyInfo          * hi = (GetReplyInfo *) uhdr;
  unsigned char         * la = hi->destAddr;
  
  TRACE((stderr, "%d: GETRPLY_CH req=%p destaddr=%p\n",
	 __pgasrt_lapi_mynode, 
	 hi->cmplt,
	 hi->destAddr));

  if (ri->udata_one_pkt_ptr)
    {
      memcpy(la,(void *)ri->udata_one_pkt_ptr,ri->msg_len);
      ri->ret_flags = LAPI_LOCAL_STATE;
      ri->ctl_flags = LAPI_BURY_MSG;
      * hi->cmplt     = 1; /* notify receiver */
      return NULL;
    }
  else
    {
      ri->ret_flags = LAPI_LOCAL_STATE;
      *comp_h = rplyCH;
      *uinfo = hi->cmplt;
      ri->dgsp_handle = NULL;
      ri->bytes = 0;
      return (void *)la;
    }
}


void rplyCH (lapi_handle_t * hndl, void * p)
{
  * ((int *) p) = 1;
}

#if defined(__cplusplus)
}
#endif
