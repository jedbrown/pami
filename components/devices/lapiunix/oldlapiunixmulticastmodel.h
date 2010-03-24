/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/lapiunix/oldlapiunixmulticastmodel.h
 * \brief ???
 */

#ifndef __components_devices_lapiunix_oldlapiunixmulticastmodel_h__
#define __components_devices_lapiunix_oldlapiunixmulticastmodel_h__

#include "sys/pami.h"
#include "components/devices/OldMulticastModel.h"
#include "components/devices/lapiunix/lapiunixmessage.h"
#include "common/lapiunix/lapifunc.h"
#include "util/common.h"
namespace PAMI
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
          _dispatch_id = _device.initMcast();
        };

      inline void setCallback (pami_olddispatch_multicast_fn cb_recv, void *arg)
        {
          _device.registerMcastRecvFunction (_dispatch_id, cb_recv, arg);
        }

      static void __pami_lapi_mcast_senddone_fn (lapi_handle_t * handle, void * param, lapi_sh_info_t * info)
        {
          OldLAPIMcastSendReq * sreq = (OldLAPIMcastSendReq *) param;
          sreq->_count++;
          if (sreq->_user_done.function && sreq->_count == sreq->_total)
            sreq->_user_done.function (NULL, sreq->_user_done.clientdata, PAMI_SUCCESS);
        }


      inline unsigned  send   (PAMI_Request_t             * request,
                               const pami_callback_t      * cb_done,
                               pami_consistency_t           consistency,
                               const pami_quad_t          * info,
                               unsigned                    info_count,
                               unsigned                    connection_id,
                               const char                * buf,
                               unsigned                    size,
                               unsigned                  * hints,
                               unsigned                  * ranks,
                               unsigned                    nranks,
                               pami_op                      op    = PAMI_UNDEFINED_OP,
                               pami_dt                      dtype = PAMI_UNDEFINED_DT )
        {
          _device.lock();
          OldLAPIMcastMessage msg;
          msg._info_count   = info_count;
          msg._size         = size;
          msg._peer         = __global.mapping.task();
          msg._conn         = connection_id;
          msg._dispatch_id  = _dispatch_id;
          if ( info )
              {
                if(info_count > 2)
                    {
                      fprintf(stderr, "FIX:  The lapiunix adaptor only supports up to 2 quads\n");
                      assert(0);
                    }
                memcpy (&msg._info[0],& info[0], info_count *sizeof (pami_quad_t));
              }

          OldLAPIMcastSendReq *sreq    = (OldLAPIMcastSendReq*)request;
          sreq->_count              = 0;
          sreq->_total              = nranks;
          if(cb_done)
              {
                sreq->_user_done.function    = cb_done->function;
                sreq->_user_done.clientdata  = cb_done->clientdata;
              }
          else
              {
                sreq->_user_done.function    = NULL;
                sreq->_user_done.clientdata  = NULL;
              }
          if (sizeof(OldLAPIMcastMessage) + size < 128)
              {
                for (unsigned count = 0; count < nranks; count ++)
                    {
#if 0
                      fprintf(stderr, "Sending Short Lapi Message to %d hsize=%d buf=%p sz=%d\n",
                              ranks[count],
                              sizeof(OldLAPIMcastMessage),
                              buf,
                              size);
#endif
                      assert (hints[count] == PAMI_PT_TO_PT_SUBTASK);
                      lapi_xfer_t xfer_struct;
                      xfer_struct.Am.Xfer_type = LAPI_AM_LW_XFER;
                      xfer_struct.Am.flags     = 0;
                      xfer_struct.Am.tgt       = ranks[count];
                      xfer_struct.Am.hdr_hdl   = (lapi_long_t)1L;
                      xfer_struct.Am.uhdr      = (void *) &msg;
                      xfer_struct.Am.uhdr_len  = sizeof(OldLAPIMcastMessage);
                      xfer_struct.Am.udata     = (void *) buf;
                      xfer_struct.Am.udata_len = size;
                      CheckLapiRC(lapi_xfer(_device._lapi_handle, &xfer_struct));
                    }
                if (sreq->_user_done.function)
                  sreq->_user_done.function(NULL,sreq->_user_done.clientdata, PAMI_SUCCESS);
              }
          else
              {
                for (unsigned count = 0; count < nranks; count ++)
                    {
#if 0
                      fprintf(stderr, "Sending Long Lapi Message to %d hsize=%d buf=%p sz=%d\n",
                              ranks[count],
                              sizeof(OldLAPIMcastMessage),
                              buf,
                              size);
#endif
                      assert (hints[count] == PAMI_PT_TO_PT_SUBTASK);
                      lapi_xfer_t xfer_struct;
                      xfer_struct.Am.Xfer_type = LAPI_AM_XFER;
                      xfer_struct.Am.flags     = 0;
                      xfer_struct.Am.tgt       = ranks[count];
                      xfer_struct.Am.hdr_hdl   = (lapi_long_t)1L;
                      xfer_struct.Am.uhdr      = (void *) &msg;
                      xfer_struct.Am.uhdr_len  = sizeof(OldLAPIMcastMessage);
                      xfer_struct.Am.udata     = (void *) buf;
                      xfer_struct.Am.udata_len = size;
                      xfer_struct.Am.shdlr     = (scompl_hndlr_t*) __pami_lapi_mcast_senddone_fn;
                      xfer_struct.Am.sinfo     = (void *) sreq;
                      xfer_struct.Am.org_cntr  = NULL;
                      xfer_struct.Am.cmpl_cntr = NULL;
                      xfer_struct.Am.tgt_cntr  = NULL;
                      CheckLapiRC(lapi_xfer(_device._lapi_handle, &xfer_struct));
                    }
              }
          _device.unlock();
          return PAMI_SUCCESS;
        }

      inline unsigned send (pami_oldmulticast_t *mcastinfo)
        {
          return this->send((PAMI_Request_t*)mcastinfo->request,
                            &mcastinfo->cb_done,
                            PAMI_MATCH_CONSISTENCY,
                            mcastinfo->msginfo,
                            mcastinfo->count,
                            mcastinfo->connection_id,
                            mcastinfo->src,
                            mcastinfo->bytes,
                            (unsigned *)mcastinfo->opcodes,
                            mcastinfo->tasks,
                            mcastinfo->ntasks);
        }


      inline unsigned postRecv (PAMI_Request_t          * request,
                                const pami_callback_t   * cb_done,
                                unsigned                 conn_id,
                                char                   * buf,
                                unsigned                 size,
                                unsigned                 pwidth,
                                unsigned                 hint   = PAMI_UNDEFINED_SUBTASK,
                                pami_op                   op     = PAMI_UNDEFINED_OP,
                                pami_dt                   dtype  = PAMI_UNDEFINED_DT)
        {
          _device.lock();
          OldLAPIMcastRecvMessage *msg = (OldLAPIMcastRecvMessage*)malloc(sizeof(*msg));
          msg->_dispatch_id = _dispatch_id;
          msg->_conn     = conn_id;
          msg->_done_fn  = cb_done->function;
          msg->_cookie   = cb_done->clientdata;
          msg->_buf      = buf;
          msg->_size     = size;
          msg->_pwidth   = pwidth;
          msg->_counter  = 0;
          //          msg->_hint     = hint;
          //          msg->_op       = op;
          //          msg->_dtype    = dtype;
          _device.enqueue(msg);
          _device.unlock();
          return 0;
        }

      inline unsigned postRecv (pami_oldmulticast_recv_t  *mrecv)
        {
          postRecv((PAMI_Request_t*)mrecv->request,
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
      pami_olddispatch_multicast_fn  _cb_async_head;
      void                         *_async_arg;
    };
  };
};
#endif // __components_devices_lapi_lapioldmulticastmodel_h__
