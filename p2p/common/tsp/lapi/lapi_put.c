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
//#define TRACE(x) fprintf x
#define TRACE(x)

/* **************************************************************** */
/*  Put Header: data structure send with data in a PUT              */
/* **************************************************************** */

typedef struct 
{
  __pgasrt_svd_handle_t          dst;
  size_t                         offset;
  void                         * arg;
  unsigned                       srcNode;
  unsigned int                 * cmplt;
}
PutRequestInfo;


static void 
putSent (lapi_handle_t * handle, void * param, lapi_sh_info_t * info);


/* ******************************************************************/
/*        send a PUT                                                */
/* ******************************************************************/

#pragma weak __pgasrt_tsp_put = ___pgasrt_tsp_put
void * ___pgasrt_tsp_put (unsigned                      srcThread,
			  unsigned                      destNode,
			  __pgasrt_svd_handle_t         dst,
			  size_t                        offset,
			  __pgasrt_local_addr_t         src,
			  size_t                        len,
			  void                        * arg)
{
  /* --------------------- */
  /* set up the counter    */
  /* --------------------- */
  
  __pgasrt_lapi_request_t * r = __pgasrt_lapi_pool_allocate();
  r->cmplt = 0;
  
  /* --------------------- */
  /* create request header */
  /* ---------------------- */
#if defined(__cplusplus)
  PutRequestInfo hdr = {dst,offset,arg,
			__pgasrt_lapi_mynode,
			&r->cmplt};
#else
  PutRequestInfo hdr = { .dst     = dst,
			 .offset  = offset,
			 .arg     = arg,
			 .srcNode = __pgasrt_lapi_mynode,
			 .cmplt   = &r->cmplt };
#endif
  TRACE((stderr, "%d: PUT(%d) cmplt=%p\n",PGASRT_MYNODE,destNode,hdr.cmplt));
  
  lapi_xfer_t xfer_struct;
  if (sizeof(PutRequestInfo) + len < 128)
    {
      xfer_struct.Am.Xfer_type = LAPI_AM_LW_XFER;
      xfer_struct.Am.flags     = 0;
      xfer_struct.Am.tgt       = destNode;
      xfer_struct.Am.hdr_hdl   = (lapi_long_t)PGASRT_LAPI_PUTREQUESTHANDLER;
      xfer_struct.Am.uhdr      = (void *) &hdr;
      xfer_struct.Am.uhdr_len  = sizeof(hdr);
      xfer_struct.Am.udata     = src;
      xfer_struct.Am.udata_len = len;
      r->cmplt = 1;
    }
  else
    {
      PutRequestInfo * hdr2 = (PutRequestInfo *) malloc (sizeof(PutRequestInfo));
      if (hdr2==NULL) __pgasrt_fatalerror (-1, "Allocation error");
      memcpy (hdr2, &hdr, sizeof(PutRequestInfo));
      
      xfer_struct.Am.Xfer_type = LAPI_AM_XFER;
      xfer_struct.Am.flags     = 0;
      xfer_struct.Am.tgt       = destNode;
      xfer_struct.Am.hdr_hdl   = (lapi_long_t)PGASRT_LAPI_PUTREQUESTHANDLER;
      xfer_struct.Am.uhdr      = (void *)hdr2;
      xfer_struct.Am.uhdr_len  = sizeof(PutRequestInfo);
      xfer_struct.Am.udata     = src;
      xfer_struct.Am.udata_len = len;
      
      xfer_struct.Am.shdlr     = putSent;
      xfer_struct.Am.sinfo     = hdr2;
      xfer_struct.Am.org_cntr  = NULL;
      xfer_struct.Am.tgt_cntr  = NULL;
      xfer_struct.Am.cmpl_cntr = NULL;
    }

  CALL_AND_CHECK_RC((LAPI_Xfer(__pgasrt_lapi_handle, &xfer_struct)));
  return (void *) r;
}


/* **************************************************************** */
/* The request is sent.                                             */
/* **************************************************************** */

void putSent (lapi_handle_t * handle, void * param, lapi_sh_info_t * info)
{
  PutRequestInfo * hdr = (PutRequestInfo *) param;
  *(unsigned *)(hdr->cmplt) = 1;
  free (hdr);
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
static void reqCH (lapi_handle_t * h, void * param);

void * __pgasrt_lapi_putRequestHandler (lapi_handle_t   * hndl,
				       void            * uhdr,
				       uint            * uhdr_len,
				       ulong           * retinfo,
				       compl_hndlr_t  ** comp_h,
				       void           ** uinfo)
{
  lapi_return_info_t    * ri = (lapi_return_info_t *) retinfo;
  PutRequestInfo        * hi = (PutRequestInfo *) uhdr;
  unsigned char         * la = __pgasrt_svd_addrof (hi->dst) + hi->offset;
  *comp_h = reqCH;
  *uinfo  = uhdr;

  TRACE((stderr, "%d: PUT request from %d to %p\n",
	 __pgasrt_lapi_mynode, hi->srcNode, la));

  if (ri->udata_one_pkt_ptr)
    {
      if (la && ri->msg_len)
	memcpy(la,(void *)ri->udata_one_pkt_ptr,ri->msg_len);
      ri->ret_flags = LAPI_LOCAL_STATE;
      ri->ctl_flags = LAPI_BURY_MSG;
      return NULL;
    }
  else
    {
      ri->ret_flags = LAPI_LOCAL_STATE;
      if (!la) ri->ctl_flags = LAPI_BURY_MSG;
    }
  return (void *)la;
}

/**
 * **************************************************************************
 *                  Completion handler                                      *
 * **************************************************************************
 */

static void reqCH (lapi_handle_t * h, void * param)
{
  PutRequestInfo            * hi = (PutRequestInfo *)param;

  /* ------------------------------------- */
  /*     send an ack                       */
  /* ------------------------------------- */

  TRACE((stderr, "%d: PUT ack to %d cmplt=%p\n",
	 __pgasrt_lapi_mynode, hi->srcNode, hi->cmplt));

#if 0
  lapi_xfer_t xfer_struct;
  xfer_struct.Am.Xfer_type = LAPI_AM_LW_XFER;
  xfer_struct.Am.flags     = 0;
  xfer_struct.Am.tgt       = hi->srcNode;
  xfer_struct.Am.hdr_hdl   = (lapi_long_t)PGASRT_LAPI_PUTREPLYHANDLER;
  xfer_struct.Am.uhdr      = (void *) &hi->cmplt;
  xfer_struct.Am.uhdr_len  = sizeof(void *);
  xfer_struct.Am.udata     = NULL;
  xfer_struct.Am.udata_len = 0;
  CALL_AND_CHECK_RC((LAPI_Xfer(__pgasrt_lapi_handle, &xfer_struct)));
#endif
}

void * __pgasrt_lapi_putReplyHandler     (lapi_handle_t           * hndl,
					  void                    * uhdr,
					  uint                    * uhdr_len,
					  ulong                   * retinfo,
					  compl_hndlr_t          ** comp_h,
					  void                   ** uinfo)
{
  unsigned int * cmplt = * (unsigned int **)uhdr;
  TRACE((stderr, "%d:  PUT ACK received cmplt=%p \n",
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
