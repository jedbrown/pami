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


static __pgasrt_AMHeaderHandler_t _regtable [PGASRT_TSP_AMSEND_REG_MAX];

/* **************************************************************** */
/*  AMSend                                                          */
/* **************************************************************** */
static void amSent (lapi_handle_t *, void *, lapi_sh_info_t *);


#pragma weak __pgasrt_tsp_amsend = ___pgasrt_tsp_amsend
void *
___pgasrt_tsp_amsend (unsigned                      destNode,
		      __pgasrt_AMHeader_t         * header,
		      const __pgasrt_local_addr_t   data,
		      unsigned                      datalen,
		      void (*comp_h) (void *),
		      void * info)
{

  /* -------------------------------------------- */
  /* setup the put transfer using active messages */
  /* -------------------------------------------- */

  if (header->headerlen < sizeof(__pgasrt_AMHeader_t))
    __pgasrt_fatalerror(-1, "AMSend: header length too small\n");
  if (header->headerlen > 128)
    __pgasrt_fatalerror(-1, "AmSend: header too large "
		       "(>128B not implemented)\n");
  if (header->handler == NULL)
    __pgasrt_fatalerror(-1, "AmSend: NULL handler\n");

  /* ------------------------------------- */
  /* ------------------------------------- */
  if (header->headerlen + datalen < 128)
    {
      lapi_xfer_t xfer_struct;
      xfer_struct.Am.Xfer_type = LAPI_AM_LW_XFER;
      xfer_struct.Am.flags     = 0;
      xfer_struct.Am.tgt       = destNode;
      xfer_struct.Am.hdr_hdl   = (lapi_long_t)PGASRT_LAPI_AMSENDREQUESTHANDLER;
      xfer_struct.Am.uhdr      = (void *) header;
      xfer_struct.Am.uhdr_len  = header->headerlen;
      xfer_struct.Am.udata     = (void *) data;
      xfer_struct.Am.udata_len = datalen;
      CALL_AND_CHECK_RC((LAPI_Xfer(__pgasrt_lapi_handle, &xfer_struct)));
      if (comp_h) (*comp_h)(info);
      return NULL;
    }
  else
    {
      __pgasrt_lapi_request_t * r;
      CHECK_NULL(r, (__pgasrt_lapi_request_t *)
		 malloc(sizeof(__pgasrt_lapi_request_t)));
      assert (r != NULL);
      r->cmplt = 0;
      r->comp_h = comp_h;
      r->arg = info;
      lapi_xfer_t xfer_struct;
      xfer_struct.Am.Xfer_type = LAPI_AM_XFER;
      xfer_struct.Am.flags     = 0;
      xfer_struct.Am.tgt       = destNode;
      xfer_struct.Am.hdr_hdl   = (lapi_long_t)PGASRT_LAPI_AMSENDREQUESTHANDLER;
      xfer_struct.Am.uhdr      = (void *) header;
      xfer_struct.Am.uhdr_len  = header->headerlen;
      xfer_struct.Am.udata     = (void *) data;
      xfer_struct.Am.udata_len = datalen;
      xfer_struct.Am.shdlr     = (scompl_hndlr_t*) amSent;
      xfer_struct.Am.sinfo     = (void *) r;
      xfer_struct.Am.org_cntr  = NULL;
      xfer_struct.Am.cmpl_cntr = NULL;
      xfer_struct.Am.tgt_cntr  = NULL;
      CALL_AND_CHECK_RC((LAPI_Xfer(__pgasrt_lapi_handle, &xfer_struct)));
     
      /* ------------------------------------------------------------------- */
      /* if the user specified a non-NULL completion handler we don't return */
      /* the request object to him: he cannot __pgasrt_tsp_wait() on the     */
      /* request; he has to take action in the completion handler.           */
      /* ------------------------------------------------------------------- */

      return comp_h ? NULL : r;
    }
}

/* **************************************************************** */
/*            register an active message handler                    */
/* **************************************************************** */

#pragma weak __pgasrt_tsp_amsend_reg = ___pgasrt_tsp_amsend_reg
void ___pgasrt_tsp_amsend_reg (int                          id,
			       __pgasrt_AMHeaderHandler_t   hndlr)
{
  if (id >= PGASRT_TSP_AMSEND_REG_MAX)
    __pgasrt_fatalerror (-1, "Cannot register id=%d for AmSend", id);
  _regtable[id] = hndlr;
}

/* **************************************************************** */
/* **************************************************************** */

#pragma weak __pgasrt_tsp_amsend_lookup = ___pgasrt_tsp_amsend_lookup
__pgasrt_AMHeaderHandler_t
___pgasrt_tsp_amsend_lookup (__pgasrt_AMHeaderHandler_t id)
{
  if ((size_t)id >= PGASRT_TSP_AMSEND_REG_MAX) return id;
  else return _regtable[(size_t) id];
}

/* **************************************************************** */
/* The request is sent. If the user specified a completion handler  */
/* we are now free to release the request buffer.                   */
/* **************************************************************** */

void amSent (lapi_handle_t * handle, void * param, lapi_sh_info_t * info)
{
  __pgasrt_lapi_request_t * r = (__pgasrt_lapi_request_t *) param;
  r->cmplt = 1;
  if (r->comp_h) { r->comp_h (r->arg); free (r); }
}

/* **************************************************************** */
/* **************************************************************** */

void * __pgasrt_lapi_amSendRequestHandler (lapi_handle_t   * hndl,
					   void            * uhdr,
					   uint            * uhdr_len,
					   ulong           * retinfo,
					   compl_hndlr_t  ** comp_h,
					   void           ** uinfo)
{
  lapi_return_info_t        * ri = (lapi_return_info_t *) retinfo;
  const __pgasrt_AMHeader_t * hi = (const __pgasrt_AMHeader_t *) uhdr;
  void                      * r = NULL;

  __pgasrt_AMHeaderHandler_t hh =  hi->handler;
  if ((size_t)hh < PGASRT_TSP_AMSEND_REG_MAX) hh = _regtable [ (size_t) hh];
  if (!hh) __pgasrt_fatalerror (-1, "amsend: NULL header handler");
  r = hh (hi, (void (**)(void *, void *))comp_h, uinfo);

  if (ri->udata_one_pkt_ptr)
    {
      if (r && ri->msg_len) 
	memcpy(r,(void *)ri->udata_one_pkt_ptr,ri->msg_len);
      ri->ret_flags = LAPI_SEND_REPLY;
      ri->ctl_flags = LAPI_BURY_MSG;
      return NULL;
    }
  else
    {
      ri->ret_flags = LAPI_SEND_REPLY;
      if (!r) ri->ctl_flags = LAPI_BURY_MSG;
    }
  return r;
}

#if defined(__cplusplus)
}
#endif
