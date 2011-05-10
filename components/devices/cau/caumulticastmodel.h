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
#include "components/devices/cau/caumessage.h"

#ifdef TRACE
#undef TRACE
#define TRACE(x)// fprintf x
#else
#define TRACE(x)// fprintf x
#endif

namespace PAMI
{
  namespace Device
  {
    template <class T_Device, class T_Message>
    class CAUMulticastModel :
      public Interface::AMMulticastModel<CAUMulticastModel<T_Device, T_Message>,T_Device,sizeof(T_Message)>
    {
      public:

      // Required metadata for the m-* protocols
      // These are defined by the interface
      static const size_t mcast_model_state_bytes = sizeof(T_Message);
      static const size_t sizeof_msg              = sizeof(T_Message);
      static const bool   is_active_message       = true;

      // The maximum size of a CAU header is 12 bytes
      // This msgHeader structure is sent along with the
      // CAU data and used to fully multiplex the broadcast.
      // Since the header is only 12 bytes, for now, we don't support
      // any user header on CAU with this multicast, since it is all used
      // for multi-* protocol data
      struct msgHeader
      {
        unsigned dispatch_id:12;
        unsigned geometry_id:12;
        unsigned connection_id:12;
        unsigned root:20;
        unsigned data_sz:8;
        unsigned msg_sz:32;
      } __attribute__((__packed__));

      // This is a structure used to track pipelines
      // for incomplete receives.  CAU only allows for 64 byte packets
      // so we have to track pipeline information internally
      struct rcvInfo
      {
        size_t                      rcvlen;
        PipeWorkQueue              *rcvpwq;
        pami_callback_t             cb_done;
        int                         gid;
        CAUMulticastModel          *mc;
        CAUMcastRecvMessage        *msg;
      };


      // This is the "internal" send done function
      // It uses a lapi callback signature since we sit on top of the lapi
      // cau routines.  This routine handles pipelining of sends into larger
      // sets of sends.  If we have a message larger than 64 bytes, this routine
      // will post the next 64 byte message to pipeline the data.
      static void cau_mcast_send_done(lapi_handle_t *hndl, void * completion_param)
        {
          // Our multicast operation has completed.
          // This means that we can consume the bytes out of the PWQ,
          // and notify a consumer (if a fcn has been set up) on the PWQ.
          CAUMcastSendMessage *m        = (CAUMcastSendMessage *) completion_param;
          PipeWorkQueue       *source   = m->_src_pwq;
          msgHeader           *hdr      = (msgHeader *)m->_xfer_msghdr;

          TRACE((stderr, "CAUMulticastModel:  cau_mcast_send_done, consuming %d bytes\n", hdr[0].data_sz));
          source->consumeBytes(hdr[0].data_sz);
          // Now check to see if we have finished the send message
          // If we have no bytes left, the message is done and we can
          // complete the send
          size_t           bytesConsumed = source->getBytesConsumed();
          size_t           bytesLeft     = m->_bytes - bytesConsumed;

          TRACE((stderr, "CAUMulticastModel:  cau_mcast_send_done:  consumed=%ld, bytesLeft=%ld bytesConsumed=%ld\n",
                 hdr[0].data_sz,
                 bytesLeft,
                 bytesConsumed));

          if(bytesLeft == 0)
            {
              TRACE((stderr, "CAUMulticastModel:  cau_mcast_send_done: delivering send done callback\n"));
              m->_fn((void*)m->_context, m->_cookie, PAMI_SUCCESS);
              m->freeHeader();
              return;
            }

          // Otherwise, we have more data to send
          // We issue a multicast from this completion handler
          // and fill out the message header with the updated information
          unsigned minsize        = MIN(source->bytesAvailableToConsume(), 64);
          hdr[0].data_sz          = minsize;
          T_Device        *device = (T_Device *)m->_device;
          CAUGeometryInfo *gi     = (CAUGeometryInfo *)m->_devinfo;
          char            *buf    = source->bufferToConsume();
          TRACE((stderr, "CAUMulticastModel:  cau_mcast_send_done: issuing pipelined multicast of %u bytes\n", hdr[0].data_sz));
          CheckLapiRC(lapi_cau_multicast(device->getHdl(),                 // lapi handle
                                         gi->_cau_id,                      // group id
                                         m->_dispatch,                     // dispatch id
                                         m->_xfer_msghdr,                  // header
                                         sizeof(msgHeader),                // message header size
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
          TRACE((stderr, "CAUMulticastModel:  cau_mcast_recv_done\n"));
          CAUMcastRecvMessage *m = (CAUMcastRecvMessage*)completion_param;
          size_t bytesLeft       = m->_buflen - m->_bytesProduced;
          size_t bytesAvail      = m->_pwq->bytesAvailableToProduce();
          size_t bytesToCopy     = MIN(bytesLeft, bytesAvail);
          if(bytesToCopy>0)
            {
              memcpy(m->_pwq->bufferToProduce(),
                     (void*)(((char*)m->_side_buf)+m->_bytesProduced),
                     bytesToCopy);
              m->_bytesProduced += bytesToCopy;
              m->_pwq->produceBytes(bytesToCopy);
            }
          // Check to see if we are done
          if(m->_bytesProduced >= m->_buflen)
            m->_cb_done.function(m->_context, m->_cb_done.clientdata, PAMI_SUCCESS);
          else
            {
            }
          return;
        }

      static void cau_mcast_recv_done2(lapi_handle_t *hndl, void * completion_param)
        {
          // No pipeline fixup required here
          //
          TRACE((stderr, "CAUMulticastModel:  cau_mcast_recv_done2\n"));
          CAUMcastRecvMessage *m = (CAUMcastRecvMessage*)completion_param;
          m->_cb_done.function(m->_context, m->_cb_done.clientdata, PAMI_SUCCESS);
        }


      static pami_result_t UeWorkFn(pami_context_t ctxt, void* cookie)
        {
          rcvInfo *rcv = (rcvInfo*)cookie;
          // How much data is in the ue buffer, ready to copy?
          unsigned              incomingBytes  = rcv->msg->_bytesProduced - rcv->msg->_bytesCopied;

          // How much room in the PWQ?
          size_t                pwqAvail       = rcv->rcvpwq->bytesAvailableToProduce();
          size_t                bytesToProduce = MIN(pwqAvail, incomingBytes);
          void                 *bufToProduce   = rcv->rcvpwq->bufferToProduce();

          TRACE((stderr, "CAUMulticastModel:  UE Generic Device handler, incomingBytes=%d"
                 " msg->bp=%ld msg->bc=%ld pwqAvail=%ld, bytesToProduce=%ld bufToProduce=%p\n",
                 incomingBytes,
                 rcv->msg->_bytesProduced,
                 rcv->msg->_bytesCopied,
                 pwqAvail,
                 bytesToProduce,
                 bufToProduce));

          if(bytesToProduce)
            {
              TRACE((stderr, "CAUMulticastModel:  UE Generic Device handler, COPYING ctxt=%p cookie=%p\n",
                     ctxt, cookie));
              memcpy(bufToProduce,
                     ((char*)rcv->msg->_side_buf) + rcv->msg->_bytesCopied,
                     bytesToProduce);
              rcv->msg->_bytesCopied +=bytesToProduce;
              rcv->rcvpwq->produceBytes(bytesToProduce);
            }

          if(rcv->rcvpwq->getBytesProduced() >= rcv->rcvlen)
            {
              TRACE((stderr, "CAUMulticastModel:  UE Generic Device handler, MESSAGE DONE ctxt=%p cookie=%p\n",
                     ctxt, cookie));

              rcv->cb_done.function(rcv->mc->_device.getContext(),
                                    rcv->cb_done.clientdata,
                                    PAMI_SUCCESS);
              rcv->mc->_device.freeWork(rcv->msg->_thread);
              rcv->mc->_rcvmsg_alloc.returnObject(rcv->msg);
              rcv->mc->_rcvinfo_alloc.returnObject(rcv);
              rcv->mc->_gid_to_rcvinfo[rcv->gid] = NULL;
              return PAMI_SUCCESS;
            }
          else
            return PAMI_EAGAIN;
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
          PAMI_GEOMETRY_CLASS *g       = (PAMI_GEOMETRY_CLASS*)mc->_device.geometrymap(gid);
          CAUGeometryInfo     *gi      = (CAUGeometryInfo*) g->getKey(PAMI::Geometry::GKEY_MCAST_CLASSROUTEID);
          void                *r       = NULL;
          lapi_return_info_t  *ri      = (lapi_return_info_t *) retinfo;

          // Deliver the user callback for the multicast
          // if the incoming callback has not been delivered already
          pami_dispatch_multicast_function user_fn     = mc->_id_to_fn[did];
          void                       *user_cookie = mc->_id_to_async_arg[did];
          rcvInfo                    *rcv         = mc->_gid_to_rcvinfo[gid];

          TRACE((stderr, "CAUMulticastModel:  cau_mcast_recv_handler did=%d gid=%d cid=%d root=%d user_fn=%p cookie=%p rcv=%p\n",
                 did, gid,cid, root, user_fn, user_cookie, rcv));
          if(!rcv)
            {
              TRACE((stderr, "CAUMulticastModel:  cau_mcast_recv_handler First Packet!\n"));
              rcv = (rcvInfo*)mc->_rcvinfo_alloc.allocateObject();
              user_fn(mc->_device.getContext(),
                      NULL,
                      0,
                      cid,
                      root,
                      msghdr->msg_sz,
                      user_cookie,
                      &rcv->rcvlen,
                      (pami_pipeworkqueue_t**)&rcv->rcvpwq,
                      &rcv->cb_done);
              mc->_gid_to_rcvinfo[gid] = rcv;
              rcv->mc                  = mc;
              rcv->gid                 = gid;
              rcv->msg                 = NULL;
              PAMI_assert(rcv->rcvlen >= msghdr->msg_sz);
            }
          else
            {
              TRACE((stderr, "CAUMulticastModel:  cau_mcast_recv_handler Non First Packet!\n"));
            }
          // Inspect the pipeline work queue for available data
          unsigned              incomingBytes  = msghdr->data_sz;
          size_t                pwqAvail       = rcv->rcvpwq->bytesAvailableToProduce();
          size_t                bytesToProduce = MIN(pwqAvail, incomingBytes);
          void                 *bufToProduce   = rcv->rcvpwq->bufferToProduce();
          bool                  noroom;
          TRACE((stderr, "CAUMulticastModel:  cau_mcast_recv_handler:  incomingBytes=%u, pwqAvail=%ld"
                 " bytesToProduce=%ld, bufToProduce=%p rcvlen=%ld\n",
                 incomingBytes,
                 pwqAvail,
                 bytesToProduce,
                 bufToProduce,
                 rcv->rcvlen));

          // We have "incomingBytes", and we have "bytesToProduce" available space
          // If incomingBytes is greater than the bytesToProduce, we need to store the
          // message into a temporary buffer and fill the buffer later OR
          // we can abort and push this up to the protocol to ensure that he provides us
          // with a pipework queue with enough storage.  For now, we will copy this incoming
          // packet into a temporary buffer
          noroom = 1;
          if(rcv->msg == NULL)
            {
              if(bytesToProduce < incomingBytes)
                {
                  // Not enough space in the pwq for this incoming packet
                  // create a message and allocate a new ue buffer for this
                  TRACE((stderr, "CAUMulticastModel:  cau_mcast_recv_handler:  user buffer smaller than incoming bytes\n"));
                  rcv->msg = (CAUMcastRecvMessage*)mc->_rcvmsg_alloc.allocateObject();
                  new(rcv->msg)CAUMcastRecvMessage(rcv->cb_done,
                                                   bufToProduce,
                                                   rcv->rcvlen,
                                                   rcv->rcvpwq,
                                                   mc->_device.getContext(),
                                                   1);
                  bufToProduce           = rcv->msg->_side_buf;
                  noroom                 = 1;
                }
              else
                {
                  TRACE((stderr, "CAUMulticastModel:  cau_mcast_recv_handler:  enough space in PWQ \n"));
                  rcv->msg = NULL;
                  noroom                 = 0;
                }
            }
          TRACE((stderr, "CAUMulticastModel:  cau_mcast_recv_handler after callback:  bytesToProduce=%ld, bufToProduce=%p rcv->rcvlen=%ld rcv->msg=%p\n",
                 bytesToProduce, bufToProduce, rcv->rcvlen, rcv->msg));

          if (ri->udata_one_pkt_ptr)
            {
              TRACE((stderr, "CAUMulticastModel:  cau_mcast_recv_handler, 1pkt noroom=%d\n", noroom));
              if(noroom)
                {
                  // This means the pwq didn't have enough room to produce the bytes
                  // We'll copy the incoming data into the side buffer
                  // and post the receive message to the generic device to check for
                  // user buffer availability
                  memcpy(((char*)rcv->msg->_side_buf)+rcv->msg->_bytesProduced,
                         (void *)ri->udata_one_pkt_ptr,
                         incomingBytes);
                  rcv->msg->_bytesProduced+=incomingBytes;
                  // Post the message to the generic device to handle the out of band
                  // copying of data
                  if(!rcv->msg->_thread)
                    rcv->msg->_thread = mc->_device.postWork(UeWorkFn, rcv);
                }
              else
                {
                  if(!rcv->msg)
                    {
                      // There is enough in the PWQ to produce all the data
                      // So we just memory copy to the PWQ and call the done
                      // function.  Also, the previous receive message wasn't posted
                      // so the copying is not handled by a posted PWQ work function
                      TRACE((stderr, "CAUMulticastModel:  cau_mcast_recv_handler, :: copying %ld to rpwq=%p, bufToProd=%p\n",
                             bytesToProduce, rcv->rcvpwq, bufToProduce));
                      memcpy(bufToProduce,
                             (void *)ri->udata_one_pkt_ptr,
                             bytesToProduce);
                      rcv->rcvpwq->produceBytes(bytesToProduce);

                      TRACE((stderr, "CAUMulticastModel: cau_mcast_recv_handler, SIMPLE::  bytesProduced=%ld, rcvlen=%ld\n",
                             rcv->rcvpwq->getBytesProduced(), rcv->rcvlen));
                      if(rcv->rcvpwq->getBytesProduced() >= rcv->rcvlen)
                        {
                          rcv->cb_done.function(mc->_device.getContext(),
                                                rcv->cb_done.clientdata,
                                                PAMI_SUCCESS);
                          mc->_rcvinfo_alloc.returnObject(rcv);
                          mc->_gid_to_rcvinfo[gid] = NULL;
                        }
                    }
                  else
                    {
                      TRACE((stderr, "CAUMulticastModel:  cau_mcast_recv_handler, UE:: copying %ld to rpwq=%p, bufToProd=%p\n",
                             bytesToProduce, rcv->rcvpwq, bufToProduce));
                      // This chunk of the pipeline is into a side buffer
                      // We will continue the copying into the side buffer
                      // and let the pwq thread finish the copying into the final
                      // PWQ buffer and to finish the message completion
                      memcpy(((char*)rcv->msg->_side_buf)+rcv->msg->_bytesProduced,
                             (void *)ri->udata_one_pkt_ptr,
                             incomingBytes);
                      rcv->msg->_bytesProduced+=incomingBytes;
                    }
                }
              r             = NULL;
              *comp_h       = NULL;
              ri->ret_flags = LAPI_SEND_REPLY;
              ri->ctl_flags = LAPI_BURY_MSG;
            }
          else
            {
              // Not handling the multi-packet case for cau
              PAMI_abort();
              TRACE((stderr, "CAUMulticastModel:  cau_mcast_recv_handler, multi-pkt noroom=%d", noroom));
              if(noroom)
                {
                  // PWQ did not have enough noroom to hold the entire message
                  r       = rcv->msg->_side_buf;
                  *comp_h = cau_mcast_recv_done;
                }
              else
                {
                  r       = bufToProduce;
                  *comp_h = cau_mcast_recv_done2;
                }
              *uinfo  = rcv->msg;
              ri->ret_flags = LAPI_LOCAL_STATE;
            }
          return r;
        }


      CAUMulticastModel (T_Device & device, pami_result_t &status) :
        Interface::AMMulticastModel < CAUMulticastModel<T_Device, T_Message>, T_Device, sizeof(T_Message) > (device, status),
        _device(device)
          {};

        inline pami_result_t registerMcastRecvFunction_impl (int                         dispatch_id,
                                                             pami_dispatch_multicast_function recv_func,
                                                             void                       *async_arg)
          {
            TRACE((stderr, "CAUMulticastModel:  registerMcastRecvFunction:  dispatch_id=%d fcn=%p cookie=%p"
                   " user_fcn=%p user_cookie=%p\n",
                   dispatch_id, cau_mcast_recv_handler, this, recv_func, async_arg));
            
            _device.registerMcastDispatch(dispatch_id, cau_mcast_recv_handler, this);
            _id_to_fn[dispatch_id]        = recv_func;
            _id_to_async_arg[dispatch_id] = async_arg;
            return PAMI_SUCCESS;
          }


        inline pami_result_t postMulticast_impl (uint8_t (&state)[mcast_model_state_bytes],
						 size_t            client,
						 size_t            context,
                                                 pami_multicast_t *mcast,
                                                 void             *devinfo)
          {
            // Set up the send message for the multicast
            CAUGeometryInfo *gi     = (CAUGeometryInfo *)devinfo;
            CAUMcastSendMessage *m  = new(state)CAUMcastSendMessage(client,
                                                                    context,
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
            PAMI_assert(mcast->msgcount == 0);
            COMPILE_TIME_ASSERT(sizeof(msgHeader) <= 12);
            size_t         hdrsz  = sizeof(msgHeader);
            msgHeader     *hdr    = (msgHeader*)m->allocateHeader(hdrsz);
            PipeWorkQueue *source = (PipeWorkQueue *)mcast->src;
            unsigned       minsize= MIN(source->bytesAvailableToConsume(), 64);

            // Set up the fields of the header
            // and concatenate the user header into the
            // system header by memory copying
            hdr[0].dispatch_id    = mcast->dispatch;
            hdr[0].geometry_id    = gi->_geometry_id;
            hdr[0].connection_id  = mcast->connection_id;
            hdr[0].root           = _device.taskid();
            hdr[0].data_sz        = minsize;
            hdr[0].msg_sz         = mcast->bytes;
            char          *buf    = source->bufferToConsume();

            TRACE((stderr, "CAUMulticastModel:  postMulticast0: bytesAvailable=%ld min=%ld, msize=%ld, hsize=%u\n",
                   source->bytesAvailableToConsume(),
                   MIN(source->bytesAvailableToConsume(), 64),
                   mcast->bytes,
                   hdr[0].data_sz));

            TRACE((stderr, "CAUMulticastModel:  postMulticast: cau_id=%d did=%d hdrsz=%d buf=%p data_sz=%d\n",
                   gi->_cau_id, hdr[0].dispatch_id, hdrsz, buf, hdr[0].data_sz));

            // Issue the multicast
            CheckLapiRC(lapi_cau_multicast(_device.getHdl(),                   // lapi handle
                                           gi->_cau_id,                        // group id
                                           hdr[0].dispatch_id,                 // dispatch id
                                           m->_xfer_msghdr,                    // header
                                           hdrsz,                              // header len
                                           buf,                                // data
                                           hdr[0].data_sz,                     // data size
                                           cau_mcast_send_done,                // done cb
                                           m));                                // clientdata
            return PAMI_SUCCESS;
          }
        T_Device                                             &_device;
        std::map<int,void*>                                   _id_to_async_arg;
        std::map<int,pami_dispatch_multicast_function>              _id_to_fn;
        std::map<int,rcvInfo*>                                _gid_to_rcvinfo;
        PAMI::MemoryAllocator<sizeof(rcvInfo), 16>            _rcvinfo_alloc;
        PAMI::MemoryAllocator<sizeof(CAUMcastRecvMessage),16> _rcvmsg_alloc;
    };
  };
};
#endif // __components_devices_cau_caumulticastmodel_h__
