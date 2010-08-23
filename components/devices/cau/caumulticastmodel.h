/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/cau/caumulticastmodel.h
 * \brief ???
 */

#ifndef __components_devices_cau_caumulticastmodel_h__
#define __components_devices_cau_caumulticastmodel_h__

#include <pami.h>
#include "components/devices/MulticastModel.h"

namespace PAMI
{
  namespace Device
  {
    template <class T_Device, class T_Message>
    class CAUMulticastModel :
      public Interface::AMMulticastModel<CAUMulticastModel<T_Device, T_Message>,T_Device,sizeof(T_Message)>
    {
      public:
      static const size_t mcast_model_state_bytes = sizeof(T_Message);
      static const size_t sizeof_msg              = sizeof(T_Message);
      static const bool   is_active_message       = true;

      typedef void (*notifyFcn) (void          * cookie);

      struct msgHeader
      {
        uint32_t dispatch_id;
        uint32_t geometry_id;
        uint32_t header_len;
        uint32_t connection_id;
        size_t   root;
        uint64_t data_sz;
      };

      static void notifyRecv(void*)
        {
          PAMI_abort();
        }

      static void cau_mcast_send_done(lapi_handle_t *hndl, void * completion_param)
        {
          // Our multicast operation has completed.
          // This means that we can consume the bytes out of the PWQ,
          // and notify a consumer (if a fcn has been set up) on the PWQ.
          notifyFcn            fn;
          void                *cookie;
          CAUMcastSendMessage *m        = (CAUMcastSendMessage *) completion_param;
          PipeWorkQueue       *source   = m->_src_pwq;
          msgHeader           *hdr      = (msgHeader *)m->_xfer_msghdr;
          void                *ptr      = (void*)fn;

          source->getProducerUserInfo(&ptr,&cookie);
          source->consumeBytes(hdr[0].data_sz);
          if(fn)
            fn(cookie);

          // Now check to see if we have finished the send message
          // If we have no bytes left, the message is done and we can
          // complete the send
          size_t           bytesConsumed = m->_src_pwq->getBytesConsumed();
          size_t           bytesLeft     = m->_bytes - bytesLeft;
          if(bytesLeft == 0)
            {
              m->_fn((void*)m->_context, m->_cookie, PAMI_SUCCESS);
              m->freeHeader();
              return;
            }

          // Otherwise, we have more data to send
          // We issue a multicast from this completion handler
          // and fill out the message header with the updated information
          hdr[0].data_sz          = source->bytesAvailableToConsume();
          hdr[0].header_len       = sizeof(msgHeader);
          T_Device        *device = (T_Device *)m->_device;
          CAUGeometryInfo *gi     = (CAUGeometryInfo *)m->_devinfo;
          char            *buf    = source->bufferToConsume();
          CheckLapiRC(lapi_cau_multicast(device->getHdl(),                 // lapi handle
                                         gi->_cau_id,                      // group id
                                         device->getLapiId(m->_dispatch),  // dispatch id
                                         m->_xfer_msghdr,                  // header
                                         hdr[0].header_len,                // header len
                                         buf,                              // data
                                         hdr[0].data_sz,                   // data size
                                         cau_mcast_send_done,              // done cb
                                         m));                              // clientdata
        }

      static void cau_mcast_recv_done(lapi_handle_t *hndl, void * completion_param)
        {
          // We have received the next chunk in the pipeline
          // We now have to deliver the rest of the data
          // to the pipeline and receive notification
          // of each chunk
          CAUMcastRecvMessage *m = (CAUMcastRecvMessage*)completion_param;
          size_t bytesLeft       = m->_buflen - m->_bytesProduced;
          size_t bytesAvail      = m->_pwq->bytesAvailableToProduce();
          notifyFcn fn;
          void     *cookie;
          void     *ptr          = (void*)fn;

          size_t bytesToCopy     = MIN(bytesLeft, bytesAvail);
          if(bytesToCopy>0)
            {
              memcpy(m->_pwq->bufferToProduce(),
                     (void*)(((char*)m->_side_buf)+m->_bytesProduced),
                     bytesToCopy);
              m->_bytesProduced += bytesToCopy;
              m->_pwq->produceBytes(bytesToCopy);
              m->_pwq->getConsumerUserInfo(&ptr,&cookie);
              if(fn)
                fn(cookie);
            }
          // Check to see if we are done
          if(m->_bytesProduced >= m->_buflen)
            m->_cb_done.function(m->_context, m->_cb_done.clientdata, PAMI_SUCCESS);
          else
            {
              // Set up notification
              m->_pwq->setProducerUserInfo((void*)notifyRecv, m);
              if(fn)
                fn(cookie);
            }
          return;
        }

      static void cau_mcast_recv_done2(lapi_handle_t *hndl, void * completion_param)
        {
          // No pipeline fixup required here
          //
          CAUMcastRecvMessage *m = (CAUMcastRecvMessage*)completion_param;
          m->_cb_done.function(m->_context, m->_cb_done.clientdata, PAMI_SUCCESS);


        }

      static void * cau_mcast_recv_handler(lapi_handle_t  *hndl,
                                           void           *uhdr,
                                           uint           *uhdr_len,
                                           ulong          *retinfo,
                                           compl_hndlr_t **comp_h,
                                           void          **uinfo)
        {
          // Parse the message header
          // and pull out all the relevent information
          msgHeader           *msghdr  = (msgHeader*)uhdr;
          int                  did     = msghdr->dispatch_id;
          int                  gid     = msghdr->geometry_id;
          int                  cid     = msghdr->connection_id;
          size_t              root     = msghdr->root;
          CAUMulticastModel   *mc      = (CAUMulticastModel*) CAUDevice::getClientData(did);
          PAMI_GEOMETRY_CLASS *g       = (PAMI_GEOMETRY_CLASS*)mc->_device.mapidtogeometry(gid);
          CAUGeometryInfo     *gi      = (CAUGeometryInfo*) g->getKey(PAMI::Geometry::PAMI_GKEY_MCAST_CLASSROUTEID);
          void                *r       = NULL;
          lapi_return_info_t  *ri      = (lapi_return_info_t *) retinfo;

          // Deliver the user callback for the multicast
          pami_dispatch_multicast_fn  user_fn     = mc->_id_to_fn[did];
          void                       *user_cookie = mc->_id_to_async_arg[did];
          size_t                      rcvlen;
          PipeWorkQueue              *rcvpwq;
          pami_callback_t             cb_done;
          user_fn((pami_quad_t *)&msghdr[1],
                  msghdr[0].header_len<<4,
                  cid,
                  root,
                  msghdr[0].data_sz,
                  user_cookie,
                  &rcvlen,
                  (pami_pipeworkqueue_t**)&rcvpwq,
                  &cb_done);

          // Inspect the pipeline work queue for available data
          size_t                bytesToProduce = rcvpwq->bytesAvailableToProduce();
          void                 *bufToProduce   = rcvpwq->bufferToProduce();
          CAUMcastRecvMessage  *m              = NULL;
          m = (CAUMcastRecvMessage*)malloc(sizeof(CAUMcastRecvMessage));
          bool                  room;
          if(bytesToProduce < rcvlen)
            {
              new(m)CAUMcastRecvMessage(cb_done, bufToProduce,rcvlen,rcvpwq,mc->_device.getContext(),1);
              bufToProduce           = m->_side_buf;
              room = 1;
            }
          else
            {
              bytesToProduce = MIN(rcvlen, bytesToProduce);
              new(m)CAUMcastRecvMessage(cb_done, bufToProduce,rcvlen,rcvpwq,mc->_device.getContext());
              room = 0;
            }

          if (ri->udata_one_pkt_ptr)
            {
              notifyFcn fn;
              void     *cookie;
              void     *ptr      = (void*)fn;
              rcvpwq->getConsumerUserInfo(&ptr,&cookie);
              if(room)
                {
                  // This means the pwq didn't have enough room to
                  // produce the bytes into, so we have to copy as much
                  // as we can, and notify the consumer that we have produced
                  // the bytes.  We'll memcpy the entire buffer into the
                  // temp buffer and copy from that temp buffer as the pwq
                  // has room to produce.  We also have to set up to be notified
                  // when data has been consumed, as well as notify
                  // the consumer that data has been produced.
                  memcpy(m->_side_buf, (void *)ri->udata_one_pkt_ptr, m->_buflen);
                  m->_bytesProduced+=bytesToProduce;
                  rcvpwq->produceBytes(bytesToProduce);
                  rcvpwq->setProducerUserInfo((void*)notifyRecv, m);
                  if(fn)
                    fn(cookie);
                }
              else
                {
                  // There is enough in the PWQ to produce all the data
                  // So we just memory copy to the PWQ and call the done
                  // function
                  memcpy(bufToProduce,
                         (void *)ri->udata_one_pkt_ptr,
                         bytesToProduce);
                  rcvpwq->produceBytes(bytesToProduce);
                  if(fn)
                    fn(cookie);
                  cb_done.function(mc->_device.getContext(),
                                   cb_done.clientdata,
                                   PAMI_SUCCESS);
                }
              r             = NULL;
              *comp_h       = NULL;
              ri->ret_flags = LAPI_SEND_REPLY;
              ri->ctl_flags = LAPI_BURY_MSG;
            }
          else
            {
              if(room)
                {
                  // PWQ did not have enough room to hold the entire message
                  r       = m->_side_buf;
                  *comp_h = cau_mcast_recv_done;
                }
              else
                {
                  r       = bufToProduce;
                  *comp_h = cau_mcast_recv_done2;
                }
              *uinfo  = m;
              ri->ret_flags = LAPI_LOCAL_STATE;
            }
          return r;
        }


      CAUMulticastModel (T_Device & device, pami_result_t &status) :
        Interface::AMMulticastModel < CAUMulticastModel<T_Device, T_Message>, T_Device, sizeof(T_Message) > (device, status),
        _device(device)
          {};

        inline pami_result_t registerMcastRecvFunction_impl (int                         dispatch_id,
                                                             pami_dispatch_multicast_fn  recv_func,
                                                             void                       *async_arg)
          {
            _device.registerMcastDispatch(dispatch_id, cau_mcast_recv_handler, this);
            _id_to_fn[dispatch_id]        = recv_func;
            _id_to_async_arg[dispatch_id] = async_arg;
            return PAMI_SUCCESS;
          }


        inline pami_result_t postMulticast_impl (uint8_t (&state)[mcast_model_state_bytes],
                                                 pami_multicast_t *mcast,
                                                 void             *devinfo)
          {
            // Set up the send message for the multicast
            CAUGeometryInfo *gi     = (CAUGeometryInfo *)devinfo;
            CAUMcastSendMessage *m  = new(state)CAUMcastSendMessage(mcast->client,
                                                                    mcast->context,
                                                                    mcast->dispatch,
                                                                    mcast->cb_done.function,
                                                                    mcast->cb_done.clientdata,
                                                                    mcast->connection_id,
                                                                    mcast->roles,
                                                                    mcast->bytes,
                                                                    (PipeWorkQueue*)mcast->src,
                                                                    (Topology*)mcast->src_participants,
                                                                    (PipeWorkQueue*)mcast->dst,
                                                                    (Topology*)mcast->dst_participants,
                                                                    mcast->msginfo,
                                                                    mcast->msgcount,
                                                                    (void*) &_device,
                                                                    (void*)  gi);

            // Figure out our parameters for the cau multicast
            // Allocate storage for the user header and the system
            // header for the multicast
            size_t         uhdrsz = PAMIQuad_sizeof(mcast->msgcount);
            size_t         hdrsz  = uhdrsz+sizeof(msgHeader);
            msgHeader     *hdr    = (msgHeader*)m->allocateHeader(hdrsz);
            PipeWorkQueue *source = (PipeWorkQueue *)mcast->src;

            // Set up the fields of the header
            // and concatenate the user header into the
            // system header by memory copying
            hdr[0].dispatch_id    = _device.getLapiId(mcast->dispatch);
            hdr[0].geometry_id    = gi->_geometry_id;
            hdr[0].connection_id  = mcast->connection_id;
            hdr[0].root           = _device.taskid();
            hdr[0].header_len     = uhdrsz;
            hdr[0].data_sz        = source->bytesAvailableToConsume();
            char          *buf    = source->bufferToConsume();
            memcpy(&hdr[1],mcast->msginfo,uhdrsz);

            // Issue the multicast
            CheckLapiRC(lapi_cau_multicast(_device.getHdl(),                   // lapi handle
                                           gi->_cau_id,                        // group id
                                           _device.getLapiId(mcast->dispatch), // dispatch id
                                           m->_xfer_msghdr,                    // header
                                           hdrsz,                              // header len
                                           buf,                                // data
                                           hdr[0].data_sz,                     // data size
                                           cau_mcast_send_done,                // done cb
                                           m));                                // clientdata
            return PAMI_SUCCESS;
          }
        T_Device                                &_device;
        std::map<int,void*>                      _id_to_async_arg;
        std::map<int,pami_dispatch_multicast_fn> _id_to_fn;
    };
  };
};
#endif // __components_devices_cau_caumulticastmodel_h__
