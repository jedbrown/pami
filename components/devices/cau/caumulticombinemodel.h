/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/cau/caumulticombinemodel.h
 * \brief ???
 */

#ifndef __components_devices_cau_caumulticombinemodel_h__
#define __components_devices_cau_caumulticombinemodel_h__

#include <pami.h>
#include "components/devices/MulticombineModel.h"
#include "components/devices/cau/caumessage.h"

#ifdef TRACE
#undef TRACE
#define TRACE(x)  // fprintf x
#else
#define TRACE(x) // fprintf x
#endif

namespace PAMI
{
  namespace Device
  {
    template <class T_Device, class T_Message>
    class CAUMulticombineModel :
      public Interface::MulticombineModel<CAUMulticombineModel<T_Device, T_Message>,T_Device,sizeof(T_Message)>
    {
      public:
      // These are the interface specified constants
      // These define the semantics of the multicombine
      static const size_t multicombine_model_state_bytes = sizeof(T_Message);
      static const size_t sizeof_msg                     = sizeof(T_Message);

      // The done function for the reduction operation
      // As part of the multicombine operation
      // This reduction is the data movement function that indicates
      // a portion of the reduction has completed
      static void cau_red_send_done(lapi_handle_t *hndl, void * completion_param)
        {
          // Todo:  Support send pipelining with generic device if no room in pwq
          TRACE((stderr, "CAUMultiombineModel: NonRoot:  entering: cau_red_send_done\n"));
          CAUMcombineMessage    *m          = (CAUMcombineMessage*)completion_param;
          size_t                 sentBytes  = m->_currentBytes;
          m->_bytesReduced                 += sentBytes;
          m->_sndpwq->consumeBytes(sentBytes);
          T_Device        *device  = (T_Device *)m->_device;

          TRACE((stderr, "CAUMulticombineModel: NonRoot:  cau_red_send_done bytesReduced=%lu, bytesToReduce=%lu\n",
                 m->_bytesReduced, m->_bytesToReduce));
          if(m->_bytesReduced == m->_bytesToReduce)
            {
              if(m->_reduceOnly)
              {
                TRACE((stderr, "CAUMulticombineModel: NonRoot:  reduce only, no multicast required, completing\n"));
                m->_user_done_fn(m->_context, m->_user_cookie, PAMI_SUCCESS);
                if(m->_workfcn)
                  device->freeWork(m->_workfcn);
              }
              else
              {
                TRACE((stderr, "CAUMulticombineModel: NonRoot:  data reduce done, waiting for multicast\n"));
              }
              return;
            }
          else if(m->_reduceOnly)
          {
            char            *redbuf  = m->_sndpwq->bufferToConsume();
            unsigned         minsize = MIN(m->_sndpwq->bytesAvailableToConsume(), 64);
            TRACE((stderr, "CAUMulticombineModel: NonRoot(cau_red_send_done): More data to reduce: redbuf=%p toReduce=%lu dispatch_id=%d\n",
                   redbuf, minsize, m->_xfer_header.dispatch_id));
            CAUGeometryInfo        *gi       = (CAUGeometryInfo *)m->_devinfo;
            CheckLapiRC(lapi_cau_reduce(device->getHdl(),             // lapi handle
                                        gi->_cau_id,                  // group id
                                        m->_xfer_header.dispatch_id,  // dispatch id
                                        &m->_xfer_header,             // header
                                        sizeof(m->_xfer_header),      // header_len
                                        redbuf,                       // data
                                        minsize,                      // data size
                                        m->_red,                      // reduction op
                                        cau_red_send_done,            // send completion handler
                                        m));
          }
        }

      static void cau_mcast_send_done(lapi_handle_t *hndl, void * completion_param)
        {
          TRACE((stderr, "CAUMulticombineModel: cau_mcast_send_done\n"));
          CAUMcombineMessage *m        = (CAUMcombineMessage*)completion_param;

          TRACE((stderr, "CAUMulticombineModel: cau_mcast_send_done:  bytesBroadcasted=%lu bytesToBroadcast=%lu\n",
                 m->_bytesBroadcasted, m->_bytesToBroadcast));
          if(m->_bytesBroadcasted == m->_bytesToBroadcast)
            {
              TRACE((stderr, "CAUMulticombineModel: delivering user callback\n"));
              T_Device                          *device = (T_Device *)m->_device;
              PAMI_GEOMETRY_CLASS               *g      = (PAMI_GEOMETRY_CLASS*)device->geometrymap(m->_xfer_header.geometry_id);
              CAUGeometryInfo                   *gi     = (CAUGeometryInfo*)g->getKey(PAMI::Geometry::GKEY_MCOMB_CLASSROUTEID);
              gi->_postedRed.deleteElem(m);              
              m->_user_done_fn(m->_context, m->_user_cookie, PAMI_SUCCESS);
              return;
            }
        }

      static void * cau_mcast_handler(lapi_handle_t  *hndl,
                                      void           *uhdr,
                                      uint           *uhdr_len,
                                      ulong          *retinfo,
                                      compl_hndlr_t **comp_h,
                                      void          **uinfo)
        {
          TRACE((stderr, "CAUMulticombineModel: entering cau_mcast_handler\n"));
          CAUMcombineMessage::xfer_header_b *hdr   = (CAUMcombineMessage::xfer_header_b*)uhdr;
          int                                did   = hdr->dispatch_id;
          int                                gid   = hdr->geometry_id;
          int                                seqno = hdr->seqno;
          CAUMulticombineModel              *mc    = (CAUMulticombineModel*) CAUDevice::getClientData(did);
          PAMI_GEOMETRY_CLASS               *g     = (PAMI_GEOMETRY_CLASS*)mc->_device.geometrymap(gid);
          CAUGeometryInfo                   *gi    = (CAUGeometryInfo*) g->getKey(PAMI::Geometry::GKEY_MCOMB_CLASSROUTEID);
          CAUMcombineMessage                *m     = (CAUMcombineMessage *)gi->_postedBcast.find(seqno);
          void                              *r     = NULL;
          lapi_return_info_t                *ri    = (lapi_return_info_t *) retinfo;
          TRACE((stderr, "CAUMulticombineModel: cau_mcast_handler: seqno=%lu m=%p\n",
                 seqno, m));
          PAMI_assert(m!=NULL);
          if (ri->udata_one_pkt_ptr)
            {
              size_t bytesLeft       = m->_bytesToBroadcast - m->_bytesBroadcasted;
              size_t bytesAvail      = m->_rcvpwq->bytesAvailableToProduce();
              size_t bytesToCopyOK   = MIN(bytesLeft, bytesAvail);
              size_t bytesToCopy     = hdr->pktsize;
              void * buf             = m->_rcvpwq->bufferToProduce();
              TRACE((stderr, "CAUMulticombineModel: cau_mcast_handler: copying data buf=%p bytesToB=%lu, bytesB=%lu bytesToC=%lu bytesToCOK=%lu\n",
                     buf, m->_bytesToBroadcast, m->_bytesBroadcasted,bytesToCopy, bytesToCopyOK));
              PAMI_assert(bytesToCopyOK >= bytesToCopy);
              if(bytesToCopy>0)
                {
                  memcpy(buf,ri->udata_one_pkt_ptr,bytesToCopy);
                  m->_bytesBroadcasted += bytesToCopy;
                  m->_rcvpwq->produceBytes(bytesToCopy);
                }
              if(m->_bytesBroadcasted == m->_bytesToBroadcast)
                {
                  TRACE((stderr, "CAUMulticombineModel: delivering user callback\n"));
                  gi->_postedBcast.deleteElem(m);
                  m->_user_done_fn(m->_context, m->_user_cookie, PAMI_SUCCESS);
                  if(m->_workfcn)
                    mc->_device.freeWork(m->_workfcn);
                }
              r             = NULL;
              *comp_h       = NULL;
              ri->ret_flags = LAPI_SEND_REPLY;
              ri->ctl_flags = LAPI_BURY_MSG;

              if(m->_bytesReduced < m->_bytesToReduce)
                {
                  T_Device        *device  = (T_Device *)m->_device;
                  char            *redbuf  = m->_sndpwq->bufferToConsume();
                  unsigned         minsize = MIN(m->_sndpwq->bytesAvailableToConsume(), 64);
                  TRACE((stderr, "CAUMulticombineModel: NonRoot(cau_mcast_handler): More data to reduce: redbuf=%p toReduce=%lu did=%d\n",
                         redbuf, minsize, m->_xfer_header.dispatch_id));
                  CheckLapiRC(lapi_cau_reduce(device->getHdl(),             // lapi handle
                                              gi->_cau_id,                  // group id
                                              m->_xfer_header.dispatch_id,  // dispatch id
                                              &m->_xfer_header,             // header
                                              sizeof(m->_xfer_header),      // header_len
                                              redbuf,                       // data
                                              minsize,                      // data size
                                              m->_red,                      // reduction op
                                              cau_red_send_done,            // send completion handler
                                              m));                          // cookie
                }
              return NULL;
            }
          else
            PAMI_abort();
          // Not reached
          PAMI_abort();
          return NULL;
        }



      static void * cau_red_handler(lapi_handle_t  *hndl,
                                    void           *uhdr,
                                    uint           *uhdr_len,
                                    ulong          *retinfo,
                                    compl_hndlr_t **comp_h,
                                    void          **uinfo)
        {
          TRACE((stderr, "CAUMulticombineModel: Root:  entering: cau_red_handler\n"));
          // The reduction handler can only be called on the "Root" node
          // This message can be unexpected, meaning that the root node has not
          // explicitly entered the reduction.
          CAUMcombineMessage::xfer_header *hdr   = (CAUMcombineMessage::xfer_header*)uhdr;
          int                              did   = hdr->dispatch_id;
          int                              gid   = hdr->geometry_id;
          int                              seqno = hdr->seqno;
          CAUMulticombineModel            *mc    = (CAUMulticombineModel*) CAUDevice::getClientData(did);
          PAMI_GEOMETRY_CLASS             *g     = (PAMI_GEOMETRY_CLASS*)mc->_device.geometrymap(gid);
          CAUGeometryInfo                 *gi    = (CAUGeometryInfo*) g->getKey(PAMI::Geometry::GKEY_MCOMB_CLASSROUTEID);

          // Next, search the posted queue for a message with the incoming seq number
          CAUMcombineMessage              *m     = (CAUMcombineMessage *)gi->_postedRed.find(seqno);
          void                            *r     = NULL;
          lapi_return_info_t              *ri    = (lapi_return_info_t *) retinfo;

          // No message found
          // Post unexpected message
          TRACE((stderr, "CAUMulticombineModel: cau_red_handler: posted:  found and delete seqno=%d\n", seqno));
          if(m == NULL)
            {
              // In the case of an unposted message, we will increment the current sequence number
              // and allocate a buffer for the packet size.  Since only one packet will be outstanding
              // in this implementation, we don't need to allocate for the entire message
              // We cannot do the math yet on this packet because the user hasn't posted their
              // math operation.
              TRACE((stderr, "CAUMulticombineModel: cau_red_handler: NOT FOUND %d\n", seqno));
              cau_reduce_op_t red = {CAU_SIGNED_INT, CAU_NOP};              
              m = (CAUMcombineMessage *)mc->_msg_allocator.allocateObject();
              new(m) CAUMcombineMessage(0,                        // bytesToReduce
                                        0,                        // type size
                                        NULL,                     // rcvPWQ
                                        NULL,                     // sndPWQ
                                        red,                      // CAU operation
                                        mc->_device.getContext(), // Context
                                        NULL,                     // done function
                                        NULL,                     // user cookie
                                        gi->_seqnoRed,            // ue search key
                                        mc);
              m->_ue_buf        =  malloc(hdr->pktsize);
              m->_ue_bytes      =  hdr->pktsize;
              m->_ue_hdr        =  malloc(*uhdr_len);
              m->_ue_hdr_bytes  = *uhdr_len;
              memcpy(m->_ue_buf, ri->udata_one_pkt_ptr, hdr->pktsize);
              memcpy(m->_ue_hdr, uhdr, *uhdr_len);

              gi->_ueRed.pushTail((MatchQueueElem*)m);
              r             = NULL;
              *comp_h       = NULL;
              ri->ret_flags = LAPI_SEND_REPLY;
              ri->ctl_flags = LAPI_BURY_MSG;
              return r;
            }

          // We found the message in the posted queue
          // The message is being filled in currently, so
          // we can start to multicast to the destinations
          TRACE((stderr, "CAUMulticombineModel: cau_red_handler: FOUND %d\n", seqno));
          if (ri->udata_one_pkt_ptr)
            {
              TRACE((stderr, "CAUMulticombineModel: cau_red_handler: seqno=%d key=%d\n",
                     seqno, m->key()));
              size_t bytesLeft            = m->_bytesToReduce - m->_bytesReduced;
              size_t bytesAvailToProduce  = m->_rcvpwq->bytesAvailableToProduce();
              size_t bytesAvailToConsume  = m->_sndpwq->bytesAvailableToConsume();
//              size_t bytesToCopy          = MIN(bytesAvailToConsume,MIN(MIN(bytesLeft, bytesAvailToProduce),64));
              size_t bytesToCopy          = MIN(MIN(bytesLeft, bytesAvailToProduce),64);
              void * buf                  = m->_rcvpwq->bufferToProduce();

              TRACE((stderr, "CAUMulticombineModel:  bytesLeft=%ld, bytesAvailToProduce=%ld, bytesAvailToConsume=%ld, bytesToCopy=%ld, buf=%p rcvpwq=%p, sndpwq=%p\n",
                     bytesLeft, bytesAvailToProduce, bytesAvailToConsume, bytesToCopy, buf, m->_rcvpwq, m->_sndpwq));
              if(bytesToCopy>0)
                {
                  char *sbuf      =  m->_sndpwq->bufferToConsume();
                  void * inputs[] = {ri->udata_one_pkt_ptr,sbuf};
                  m->_math_func(buf, inputs, 2, bytesToCopy/m->_sizeoftype);
                  m->_bytesReduced += bytesToCopy;
                  m->_rcvpwq->produceBytes(bytesToCopy);
                  m->_sndpwq->consumeBytes(bytesToCopy);
                }

              m->_xfer_header_b.dispatch_id = mc->_dispatch_mcast_id;
              m->_xfer_header_b.geometry_id = gid;
              m->_xfer_header_b.seqno       = seqno;
              m->_xfer_header_b.pktsize     = bytesToCopy;
              m->_xfer_header_b.msgsize     = m->_bytesToReduce;
              m->_bytesBroadcasted         += bytesToCopy;

              TRACE((stderr, "CAUMulticombineModel: cau_red_handler: buf=%p bytes=%ld\n",
                     buf, bytesToCopy));

              lapi_handle_t hdl = mc->_device.getHdl();
              // If this is a reduction only we do not need to multicast the results
              if(m->_reduceOnly)
              {
                TRACE((stderr, "CAUMulticombineModel: cau_red_handler: No need to MULTICAST(reduction) seqno=%d key=%d\n",
                       seqno, m->key()));
                                
                cau_mcast_send_done(&hdl, m);
              }
              else
              {
                TRACE((stderr, "CAUMulticombineModel: cau_red_handler: MULTICAST seqno=%d key=%d\n",
                       seqno, m->key()));

                CheckLapiRC(lapi_cau_multicast(hdl,                      // lapi handle
                                               gi->_cau_id,              // group id
                                               mc->_dispatch_mcast_id,   // dispatch id
                                               &m->_xfer_header_b,       // header
                                               sizeof(m->_xfer_header_b),// header len
                                               buf,                      // data
                                               bytesToCopy,              // data size
                                               cau_mcast_send_done,      // done cb
                                               m));                      // clientdata
              }
                
              r             = NULL;
              *comp_h       = NULL;
              ri->ret_flags = LAPI_SEND_REPLY;
              ri->ctl_flags = LAPI_BURY_MSG;
              return NULL;
            }
          else
            PAMI_abort();

          // Not reached
          PAMI_abort();
          return NULL;
        }

      static pami_result_t defer_reduce (pami_context_t context, void *cookie)
        {
          CAUMcombineMessage *m       = (CAUMcombineMessage*) cookie;
          unsigned            minsize =  MIN(m->_sndpwq->bytesAvailableToConsume(), 64);
          char               *buf     =  m->_sndpwq->bufferToConsume();
          T_Device           *device  = (T_Device *)m->_device;
          lapi_handle_t       hdl     = device->getHdl();
          CAUGeometryInfo    *gi      = (CAUGeometryInfo *)m->_devinfo;
          m->_currentBytes         +=  minsize;
          
          if(minsize)
          {
            CheckLapiRC(lapi_cau_reduce(hdl,                         // lapi handle
                                        gi->_cau_id,                 // group id
                                        m->_xfer_header.dispatch_id, // dispatch id
                                        &m->_xfer_header,            // header
                                        sizeof(m->_xfer_header),     // header_len
                                        buf,                         // data
                                        minsize,                     // data size
                                        m->_red,                     // reduction op
                                        cau_red_send_done,           // send completion handler
                                        m));                         // cookie
            return PAMI_SUCCESS;
          }
          return PAMI_EAGAIN;
          
        }


      
      CAUMulticombineModel (T_Device & device, pami_result_t &status) :
        Interface::MulticombineModel < CAUMulticombineModel<T_Device, T_Message>, T_Device, sizeof(T_Message) > (device, status),
        _device(device)
          {
            TRACE((stderr, "CAUMulticombineModel:  Registering Dispatch Handler:  %p %p\n", cau_red_handler, cau_mcast_handler));
            _dispatch_red_id   = _device.registerSyncDispatch(cau_red_handler, this);
            _dispatch_mcast_id = _device.registerSyncDispatch(cau_mcast_handler, this);
            TRACE((stderr, "CAUMulticombineModel:  reduction dispatch id/bcast dispatch id:  %d %d\n", _dispatch_red_id, _dispatch_mcast_id));
            status             = PAMI_SUCCESS;
          };
        inline pami_result_t postMulticombine_impl (uint8_t (&state)[multicombine_model_state_bytes],
						    size_t               client,
						    size_t               context,
                                                    pami_multicombine_t *mcombine,
                                                    void                *devinfo)
          {
            int              rc;
            cau_reduce_op_t  red;
            red.operand_type                 = _device.pami_to_lapi_dt(mcombine->dtype);
            red.operation                    = _device.pami_to_lapi_op(mcombine->optor);
            pami_task_t            *tl       = NULL;
            pami_task_t            *tl_root  = NULL;
            PipeWorkQueue          *source   = (PipeWorkQueue*)mcombine->data;
            PipeWorkQueue          *dest     = (PipeWorkQueue*)mcombine->results;
            CAUMcombineMessage     *m        = NULL;
            Topology               *topo_src = (Topology*)mcombine->data_participants;
            Topology               *topo_dst = (Topology*)mcombine->results_participants;
            CAUGeometryInfo        *gi       = (CAUGeometryInfo *)devinfo;
            topo_src->rankList(&tl);
            topo_dst->rankList(&tl_root);
            bool                    handleUE     = false;
            void                   *ueBuf        = NULL;
            size_t                  ueBytes      = 0;
            void                   *ueHdrBuf     = NULL;
            size_t                  ueHdrBytes   = 0;
            bool                    isReduceOnly = false;

            if(topo_dst->size()==1)
              isReduceOnly=true;

            TRACE((stderr, "CAUMulticombineModel: ReduceOnly=%d, tl_root=%ld, taskid=%ld dst_topo_size=%d src_topo_size=%d\n", 
                   isReduceOnly, tl_root[0], _device.taskid(), topo_dst->size(), topo_src->size()));
            
            if(tl_root[0] == _device.taskid())
            {
              // This is the root of the multicombine.  This node needs
              // to perform the math operations and start broadcasting
              // the data to the destination nodes.  The math operation
              // is the local data and the incoming reduction stream
              // If the message is not found, just let the async
              // handler handle this
              TRACE((stderr, "CAUMulticombineModel: Root: postMulticombine ue queue FindAndDelete: %lu\n",
                     gi->_seqnoRed));
              m = (CAUMcombineMessage*)gi->_ueRed.findAndDelete(gi->_seqnoRed);
              if (m != NULL)
                {
                  // We found an unexpected message
                  // We can construct this message as if it is a new lapi message
                  // and call the reduction header handler handle this ue message.                  
                  TRACE((stderr, "CAUMulticombineModel: Root: postMulticombine ue message Found\n"));
                  handleUE    = true;
                  ueBuf       = m->_ue_buf;
                  ueBytes     = m->_ue_bytes;
                  ueHdrBuf    = m->_ue_hdr;
                  ueHdrBytes  = m->_ue_hdr_bytes;
                  _msg_allocator.returnObject(m);
                  m        = NULL;
                }
              TRACE((stderr, "CAUMulticombineModel: Root: postMulticombine ue message Not Found\n"));
            }

            if(!m)
              {
                unsigned        sizeOfType;
                coremath        mathfunc;
                CCMI::Adaptor::Allreduce::getReduceFunction(mcombine->dtype,
                                                            mcombine->optor,
                                                            sizeOfType,
                                                            mathfunc);
                m  = new(state) CAUMcombineMessage(mcombine->count,               // count
                                                   sizeOfType,                    // size of the datatype
                                                   dest,                          // Receive PWQ
                                                   source,                        // Send PWQ
                                                   red,                           // reduce operation
                                                   _device.getContext(),          // Context
                                                   mcombine->cb_done.function,    // User done fcn
                                                   mcombine->cb_done.clientdata,  // Done fcn
                                                   gi->_seqnoRed++,               // Reduction sequence number
                                                   NULL);                          // toFree?

                m->_xfer_header.dispatch_id        =  _dispatch_red_id;
                m->_xfer_header.geometry_id        =  gi->_geometry_id;
                m->_xfer_header.seqno              =  gi->_seqnoRed-1;
                m->_xfer_header.pktsize            =  64;
                m->_xfer_header.msgsize            =  mcombine->count;
                m->_math_func                      =  mathfunc;
                m->_device                         = &_device;
                m->_devinfo = devinfo;
                TRACE((stderr, "CAUMulticombineModel: Created New Message=%p (%lu)to post to expected queue\n",
                       m, m->_xfer_header.seqno));
              }
            unsigned        minsize =  MIN(source->bytesAvailableToConsume(), 64);
            char           *buf     =  source->bufferToConsume();
            m->_currentBytes        =  minsize;
            m->_reduceOnly          =  isReduceOnly;

            // This node is not the root, start the reduction
            lapi_handle_t hdl = _device.getHdl();
            if(tl_root[0] != _device.taskid())
              {
                TRACE((stderr, "CAUMulticombineModel: Nonroot Calling Reduce and posting to posted broadcast queue seqno=%lu buf=%p dispatch_red_id=%d\n",
                       m->_xfer_header.seqno, buf, _dispatch_red_id));
                gi->_postedBcast.pushTail((MatchQueueElem*)m);

                if(minsize>0)
                {
                  CheckLapiRC(lapi_cau_reduce(hdl,                      // lapi handle
                                              gi->_cau_id,              // group id
                                              _dispatch_red_id,         // dispatch id
                                              &m->_xfer_header,         // header
                                              sizeof(m->_xfer_header),  // header_len
                                              buf,                      // data
                                              minsize,                  // data size
                                              red,                      // reduction op
                                              cau_red_send_done,        // send completion handler
                                              m));                      // cookie
                }
                else
                {
                  m->_workfcn = _device.postWork(defer_reduce, m);
                }
                
                return PAMI_SUCCESS;
              }
            TRACE((stderr, "CAUMulticombineModel: Root: postMulticombine: posting to posted queue\n"));
            gi->_postedRed.pushTail((MatchQueueElem*)m);
            if(handleUE==true)
              {
                TRACE((stderr, "CAUMulticombineModel: Root: postMulticombine: handling UE message\n"));
                lapi_return_info_t      ri;
                ri.udata_one_pkt_ptr =  ueBuf;
                compl_hndlr_t          *comp_h;
                cau_red_handler(&hdl,
                                ueHdrBuf,
                                (unsigned int*)&ueHdrBytes,
                                (ulong*)&ri,
                                &comp_h,
                                NULL);
                free(ueHdrBuf);
                free(ueBuf);
              }
            return PAMI_SUCCESS;
          }
      public:
        T_Device                                             &_device;
        int                                                   _dispatch_red_id;
        int                                                   _dispatch_mcast_id;
        PAMI::MemoryAllocator<sizeof(CAUMcombineMessage),16>  _msg_allocator;
    };
  };
};
#endif // __components_devices_cau_caumulticombinemodel_h__
