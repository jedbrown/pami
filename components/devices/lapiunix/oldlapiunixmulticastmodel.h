/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/lapi/oldlapimulticastmodel.h
 * \brief ???
 */

#ifndef __components_devices_lapi_lapioldmulticastmodel_h__
#define __components_devices_lapi_lapioldmulticastmodel_h__

#include "sys/xmi.h"
#include "components/devices/OldMulticastModel.h"
#include "components/devices/lapiunix/lapiunixmessage.h"
#include <lapi.h>
#include "util/common.h"
namespace XMI
{
  namespace Device
  {

    template <class T_Device, class T_Message>
    class LAPIOldmulticastModel : public Interface::OldmulticastModel<LAPIOldmulticastModel<T_Device, T_Message>, T_Device, T_Message>
    {
    public:
      LAPIOldmulticastModel (T_Device & device) :
        Interface::OldmulticastModel<LAPIOldmulticastModel<T_Device, T_Message>, T_Device, T_Message> (device),
        _device(device)
        {
          assert(0);
        };

      inline void setCallback (xmi_olddispatch_multicast_fn cb_recv, void *arg)
        {
          assert(0);
        }

      static void amSent (lapi_handle_t * handle, void * param, lapi_sh_info_t * info)
        {
          LAPIMcastSendReq * r = (LAPIMcastSendReq *) param;
          if (r->user_done_callback.function)
            r->user_done_callback.function (NULL, r->user_done_callback.clientdata, XMI_SUCCESS);
        }


      inline unsigned  send   (XMI_Request_t             * request,
                               const xmi_callback_t      * cb_done,
                               xmi_consistency_t           consistency,
                               const xmi_quad_t          * info,
                               unsigned                    info_count,
                               unsigned                    connection_id,
                               const char                * buf,
                               unsigned                    size,
                               unsigned                  * hints,
                               unsigned                  * ranks,
                               unsigned                    nranks,
                               xmi_op                      op    = XMI_UNDEFINED_OP,
                               xmi_dt                      dtype = XMI_UNDEFINED_DT )
        {
          LAPIMcastMessage msg;
          void *r           = NULL;
          int   rc          = -1;
          msg._info_count   = info_count;
          msg._size         = size;
          msg._peer         = 0;
          msg._conn         = connection_id;
          msg._dispatch_id  = _dispatch_id;
          if ( info )
              {
                if(info_count > 2)
                    {
                      fprintf(stderr, "FIX:  The lapiunix adaptor only supports up to 2 quads\n");
                      assert(0);
                    }
                memcpy (&msg._info[0],& info[0], info_count *sizeof (xmi_quad_t));
              }

          LAPISendInfo *si              = (LAPISendInfo*)request;
          si->_totalsends               = nranks;
          si->_numsends                 = 0;
          si->_user_cb_done.function    = cb_done->function;
          si->_user_cb_done.clientdata  = cb_done->clientdata;

          if (sizeof(LAPIMcastMessage) + size < 128)
              {
                for (unsigned count = 0; count < nranks; count ++)
                    {
                      assert (hints[count] == XMI_PT_TO_PT_SUBTASK);
                      lapi_xfer_t xfer_struct;
                      xfer_struct.Am.Xfer_type = LAPI_AM_LW_XFER;
                      xfer_struct.Am.flags     = 0;
                      xfer_struct.Am.tgt       = ranks[count];
                      xfer_struct.Am.hdr_hdl   = (lapi_long_t)_dispatch_id;
                      xfer_struct.Am.uhdr      = (void *) &msg;
                      xfer_struct.Am.uhdr_len  = sizeof(LAPIMcastMessage);
                      xfer_struct.Am.udata     = (void *) buf;
                      xfer_struct.Am.udata_len = size;
                      CALL_AND_CHECK_RC((LAPI_Xfer(_device._lapi_handle, &xfer_struct)));
                    }
                if (si->_user_cb_done.function)
                  si->_user_cb_done.function(NULL, si->_user_cb_done.clientdata, XMI_SUCCESS);
              }
          else
              {
                LAPIMcastSendReq *reqs;
                CHECK_NULL(reqs, (LAPIMcastSendReq*)malloc(sizeof(LAPIMcastSendReq)*nranks));
                for (unsigned count = 0; count < nranks; count ++)
                    {
                      assert (hints[count] == XMI_PT_TO_PT_SUBTASK);
                      lapi_xfer_t xfer_struct;
                      LAPIMcastSendReq *r      = &reqs[count];
                      r->user_done_callback    = *cb_done;
                      xfer_struct.Am.Xfer_type = LAPI_AM_XFER;
                      xfer_struct.Am.flags     = 0;
                      xfer_struct.Am.tgt       = ranks[count];
                      xfer_struct.Am.hdr_hdl   = (lapi_long_t)_dispatch_id;
                      xfer_struct.Am.uhdr      = (void *) &msg;
                      xfer_struct.Am.uhdr_len  = sizeof(LAPIMcastMessage);
                      xfer_struct.Am.udata     = (void *) buf;
                      xfer_struct.Am.udata_len = size;
                      xfer_struct.Am.shdlr     = (scompl_hndlr_t*) amSent;
                      xfer_struct.Am.sinfo     = (void *) r;
                      xfer_struct.Am.org_cntr  = NULL;
                      xfer_struct.Am.cmpl_cntr = NULL;
                      xfer_struct.Am.tgt_cntr  = NULL;
                      CALL_AND_CHECK_RC((LAPI_Xfer(_device._lapi_handle, &xfer_struct)));
                    }
              }
          return XMI_SUCCESS;
        }

      inline unsigned send (xmi_oldmulticast_t *mcastinfo)
        {
          return this->send((XMI_Request_t*)mcastinfo->request,
                            &mcastinfo->cb_done,
                            XMI_MATCH_CONSISTENCY,
                            mcastinfo->msginfo,
                            mcastinfo->count,
                            mcastinfo->connection_id,
                            mcastinfo->src,
                            mcastinfo->bytes,
                            (unsigned *)mcastinfo->opcodes,
                            mcastinfo->ranks,
                            mcastinfo->nranks);
        }


      inline unsigned postRecv (XMI_Request_t          * request,
                                const xmi_callback_t   * cb_done,
                                unsigned                 conn_id,
                                char                   * buf,
                                unsigned                 size,
                                unsigned                 pwidth,
                                unsigned                 hint   = XMI_UNDEFINED_SUBTASK,
                                xmi_op                   op     = XMI_UNDEFINED_OP,
                                xmi_dt                   dtype  = XMI_UNDEFINED_DT)
        {
          assert(0);
	  return 0;
        }

      inline unsigned postRecv (xmi_oldmulticast_recv_t  *mrecv)
        {
          postRecv((XMI_Request_t*)mrecv->request,
                   &mrecv->cb_done,
                   mrecv->connection_id,
                   mrecv->rcvbuf,
                   mrecv->bytes,
                   mrecv->pipelineWidth,
                   mrecv->opcode,
                   mrecv->op,
                   mrecv->dt);
	  return 0;
        }
      T_Device                     &_device;
      size_t                        _dispatch_id;
      xmi_olddispatch_multicast_fn  _cb_async_head;
      void                         *_async_arg;
    };
  };
};
#endif // __components_devices_lapi_lapioldmulticastmodel_h__
