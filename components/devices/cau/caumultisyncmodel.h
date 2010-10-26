/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/cau/caumultisyncmodel.h
 * \brief ???
 */

#ifndef __components_devices_cau_caumultisyncmodel_h__
#define __components_devices_cau_caumultisyncmodel_h__

#include <pami.h>
#include "components/devices/MultisyncModel.h"
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
    class CAUMultisyncModel :
      public Interface::MultisyncModel<CAUMultisyncModel<T_Device, T_Message>,T_Device,sizeof(T_Message)>
    {
      static void cau_red_send_done(lapi_handle_t *hndl, void * completion_param)
        {
          TRACE((stderr, "cau_red_send_done\n"));
        }
      static void cau_mcast_send_done(lapi_handle_t *hndl, void * completion_param)
        {
          TRACE((stderr, "cau_mcast_send_done\n"));
          // This means that the synchronization is complete on the root node
          CAUMsyncMessage    *m   = (CAUMsyncMessage *)completion_param;
          m->_user_done_fn(m->_context,
                           m->_user_cookie,
                           PAMI_SUCCESS);
          if(m->_toFree)
            {
              CAUMultisyncModel *ms = (CAUMultisyncModel *)m->_toFree;
              ms->_msg_allocator.returnObject(m);
            }
        }

      static void * cau_mcast_handler(lapi_handle_t  *hndl,
                                      void           *uhdr,
                                      uint           *uhdr_len,
                                      ulong          *retinfo,
                                      compl_hndlr_t **comp_h,
                                      void          **uinfo)
        {
          TRACE((stderr, "cau_mcast_handler\n"));
          // First, get the geometry
          int                 *didPtr  = (int *) uhdr;
          int                  did     = didPtr[0];
          int                  gid     = didPtr[1];
          int                  seqno   = didPtr[2];
          CAUMultisyncModel   *ms      = (CAUMultisyncModel*) CAUDevice::getClientData(did);
          PAMI_GEOMETRY_CLASS *g       = (PAMI_GEOMETRY_CLASS*)ms->_device.mapidtogeometry(gid);

          // Next, search the posted queue for a message with the incoming seq number
          // A message must always be found because the reduction is synchronizing!
          CAUGeometryInfo    *gi  = (CAUGeometryInfo*) g->getKey(PAMI::Geometry::PAMI_GKEY_MSYNC_CLASSROUTEID);
          CAUMsyncMessage    *m   = (CAUMsyncMessage *)gi->_posted.findAndDelete(seqno);
          void               *r   = NULL;
          lapi_return_info_t *ri  = (lapi_return_info_t *) retinfo;

          if(m == NULL)
            PAMI_abort();

          // Message was found, we can free it and dequeue
          // Deliver the user's callback
          TRACE((stderr, "cau_mcast_handler: found and delete seqno=%d\n", seqno));
          if (ri->udata_one_pkt_ptr)
            {
              m->_user_done_fn(m->_context,
                               m->_user_cookie,
                               PAMI_SUCCESS);
              r             = NULL;
              *comp_h       = NULL;
              ri->ret_flags = LAPI_SEND_REPLY;
              ri->ctl_flags = LAPI_BURY_MSG;
              return NULL;
            }
          else
            PAMI_abort();

          // Should not be reached
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
          // The reduction handler can only be called on the "Root" node
          // This message can be unexpected, meaning that the root node has not
          // explicitly entered the reduction.

          // First, get the geometry
          int                 *didPtr  = (int *) uhdr;
          int                  did     = didPtr[0];
          int                  gid     = didPtr[1];
          int                  seqno   = didPtr[2];
          CAUMultisyncModel   *ms      = (CAUMultisyncModel*) CAUDevice::getClientData(did);
          PAMI_GEOMETRY_CLASS *g       = (PAMI_GEOMETRY_CLASS*)ms->_device.mapidtogeometry(gid);

          // Next, search the posted queue for a message with the incoming seq number
          CAUGeometryInfo    *gi  = (CAUGeometryInfo*) g->getKey(PAMI::Geometry::PAMI_GKEY_MSYNC_CLASSROUTEID);
          CAUMsyncMessage    *m   = (CAUMsyncMessage *)gi->_posted.findAndDelete(seqno);
          void               *r   = NULL;
          lapi_return_info_t *ri  = (lapi_return_info_t *) retinfo;

          // If no message was found, return, because we aren't ready to broadcast
          // Push a new message to the unexpected queue
          TRACE((stderr, "cau_red_handler: found and delete seqno=%d\n", seqno));
          if(m == NULL)
            {
              TRACE((stderr, "cau_red_handler: NOT FOUND %d\n", seqno));
              m = (CAUMsyncMessage *)ms->_msg_allocator.allocateObject();
              new(m) CAUMsyncMessage(0.0,
                                     ms->_device.getContext(),
                                     NULL,
                                     NULL,
                                     gi->_seqno++,
                                     ms);
              gi->_ue.pushTail((MatchQueueElem*)m);
              r             = NULL;
              *comp_h       = NULL;
              ri->ret_flags = LAPI_SEND_REPLY;
              ri->ctl_flags = LAPI_BURY_MSG;
              return r;
            }

          // Message was found, we can free it and dequeue
          // We can now issue the multicast and complete the
          // msync.  We can free the message as well
          TRACE((stderr, "cau_red_handler: NOT FOUND %d\n", seqno));
          if (ri->udata_one_pkt_ptr)
            {
              TRACE((stderr, "cau_red_handler: POSTING MULTICAST %d\n", seqno));

              CheckLapiRC(lapi_cau_multicast(ms->_device.getHdl(),            // lapi handle
                                             gi->_cau_id,                     // group id
                                             ms->_dispatch_mcast_id,          // dispatch id
                                             &m->_xfer_data[0],               // header
                                             sizeof(m->_xfer_data),           // header len
                                             &m->_reduce_val,                 // data
                                             sizeof(m->_reduce_val),          // data size
                                             cau_mcast_send_done,             // done cb
                                             m));                              // clientdata
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

      public:
      static const size_t msync_model_state_bytes = sizeof(T_Message);
      static const size_t sizeof_msg              = sizeof(T_Message);
      CAUMultisyncModel (T_Device      &device,pami_result_t &status):
        Interface::MultisyncModel<CAUMultisyncModel<T_Device, T_Message>,T_Device,sizeof(T_Message)>(device,status),
        _device(device)
          {
            TRACE((stderr, "CAU:  Registering Dispatch Handler:  %p %p\n", cau_red_handler, cau_mcast_handler));
            status                            = PAMI_SUCCESS;
            _dispatch_red_id                  = _device.registerSyncDispatch(cau_red_handler, this);
            if(_dispatch_red_id==-1) status   = PAMI_ERROR;
            _dispatch_mcast_id                = _device.registerSyncDispatch(cau_mcast_handler, this);
            if(_dispatch_mcast_id==-1) status = PAMI_ERROR;
          }

        pami_result_t postMultisync (uint8_t         (&state)[msync_model_state_bytes],
                                     pami_multisync_t *msync,
                                     void             *devinfo)
        {
          PAMI::Topology               *topology = (PAMI::Topology*) msync->participants;
          if(topology->size() == 1)
            {
              msync->cb_done.function(_device.getContext(), msync->cb_done.clientdata, PAMI_SUCCESS);
              return PAMI_SUCCESS;
            }


          TRACE((stderr, "CAU:  PostMultisync\n"));
          // This assert ensures that the topology that the user created
          // at geometry create time is the same as the geometry input here
          CAUGeometryInfo *gi = (CAUGeometryInfo *)devinfo;
          PAMI_assert(topology == gi->_topo);

          // Search the per-geometry UE queue for an "unexpected message"
          // Since the reduction has already been handled, we can begin our
          // broadcast of the data.  Only the root of the reduction can receive
          // an unexpected message (because the hardware handles the reductions and
          // contributers all have to make the "call-in" to participate.
          // Likewise, only the root performs the multicast, all other nodes
          // will receive an async composite.  Only the root node, which we choose
          // to be the first rank in the topology, can receive the ue message.
          CAUMsyncMessage *m   = NULL;
          pami_task_t     *tl  = NULL;
          topology->rankList(&tl);

          if(tl[0] == _device.taskid())
            {
              TRACE((stderr, "CAU:  FindAndDelete\n"));
              m = (CAUMsyncMessage*)gi->_ue.findAndDelete(gi->_seqno);
              if (m != NULL)
                {
                  TRACE((stderr, "CAU:  Multicast\n"));
                  CheckLapiRC(lapi_cau_multicast(_device.getHdl(),            // lapi handle
                                                 gi->_cau_id,                 // group id
                                                 _dispatch_mcast_id,          // dispatch id
                                                 &m->_xfer_data[0],           // header
                                                 sizeof(m->_xfer_data),       // header len
                                                 &m->_reduce_val,             // data
                                                 sizeof(m->_reduce_val),      // data size
                                                 cau_mcast_send_done,         // done cb
                                                 m));                          // clientdata
                  return PAMI_SUCCESS;
                }
            }
          // The message is not found on the root node OR we are the root node
          // We need to enter the allreduce on non-root nodes, so we create an allreduce message
          // to capture the state.  Remember, only the non-root nodes explicitly
          // participate.  On the root node, (the only node that can receive the UE message),
          // we will post the message to the "posted" queue.
          // The data transferred in the header will be the same on all nodes
          TRACE((stderr, "CAU:  Create Msync Message %d\n", gi->_seqno));
          m  = new(state) CAUMsyncMessage(0.0,
                                          _device.getContext(),
                                          msync->cb_done.function,
                                          msync->cb_done.clientdata,
                                          gi->_seqno++,
                                          NULL);
          int              rc;
          cau_reduce_op_t  red;
          red.operand_type     = CAU_DOUBLE;
          red.operation        = CAU_SUM;
          m->_xfer_data[0]     = _dispatch_red_id;
          m->_xfer_data[1]     = gi->_geometry_id;
          m->_xfer_data[2]     = gi->_seqno-1;
          // First task in the topology is the root
          TRACE((stderr, "CAU:  tl[0]=%d taskid=%d\n", tl[0], _device.taskid()));
          if(tl[0] != _device.taskid())
            {
              // We have tp push this first because the reduce call can
              // call our callback
              TRACE((stderr, "CAU:  Reduce\n"));
              gi->_posted.pushTail((MatchQueueElem*)m);
              CheckLapiRC(lapi_cau_reduce(_device.getHdl(),         // lapi handle
                                          gi->_cau_id,              // group id
                                          _dispatch_red_id,         // dispatch id
                                          &m->_xfer_data[0],        // header
                                          sizeof(m->_xfer_data),    // header_len
                                          &m->_reduce_val,          // data
                                          sizeof(m->_reduce_val),   // data size
                                          red,                      // reduction op
                                          cau_red_send_done,        // send completion handler
                                          m));                       // cookie
              return PAMI_SUCCESS;
            }
          TRACE((stderr, "CAU:  Pushing Tail\n"));
          gi->_posted.pushTail((MatchQueueElem*)m);
          return PAMI_SUCCESS;
        }
        T_Device                     &_device;
        int                           _dispatch_red_id;
        int                           _dispatch_mcast_id;
        PAMI::MemoryAllocator<sizeof(CAUMsyncMessage),16> _msg_allocator;
    };
  };
};
#endif // __components_devices_mpi_mpimultisyncmodel_h__
