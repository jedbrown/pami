/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* This is an automatically generated copyright prolog.             */
/* After initializing,  DO NOT MODIFY OR MOVE                       */
/*  --------------------------------------------------------------- */
/* Licensed Materials - Property of IBM                             */
/* Blue Gene/Q 5765-PER 5765-PRP                                    */
/*                                                                  */
/* (C) Copyright IBM Corp. 2011, 2012 All Rights Reserved           */
/* US Government Users Restricted Rights -                          */
/* Use, duplication, or disclosure restricted                       */
/* by GSA ADP Schedule Contract with IBM Corp.                      */
/*                                                                  */
/*  --------------------------------------------------------------- */
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
#define TRACE(x) // fprintf x
#else
#define TRACE(x) // fprintf x
#endif

namespace PAMI
{
namespace Device
{
template <class T_Device, class T_Message>
class CAUMultisyncModel : public Interface::MultisyncModel<CAUMultisyncModel<T_Device, T_Message>, T_Device, sizeof(T_Message)>
{

  static void cleanup_fn(void  *msync, void *message, pami_result_t res)
  {
    T_Message         *m   = (T_Message  *)message;
    CAUMultisyncModel *ms  = (CAUMultisyncModel *)msync;

    if(ms)
      {
        ms->_device.freeMessage(m);
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
    CAUMultisyncModel   *ms      = (CAUMultisyncModel*) CAUDevice::getClientData(hndl, did);
    PAMI_GEOMETRY_CLASS *g       = (PAMI_GEOMETRY_CLASS*)ms->_device.geometrymap(gid);
    // Next, search the posted queue for a message with the incoming seq number
    // A message must always be found because the reduction is synchronizing!
    CAUGeometryInfo    *gi  = (CAUGeometryInfo*)
                              g->getKey(ms->_device.getContextId(),
                                        PAMI::Geometry::CKEY_MSYNC_CLASSROUTEID);
    TRACE((stderr, "cau_mcast_handler: gi=%p geometry=%p ms=%p contextId=%lu\n",
           gi, g, ms, ms->_device.getContextId()));
    T_Message          *m   = (T_Message *)gi->_postedBar.findAndDelete(seqno);
    lapi_return_info_t *ri  = (lapi_return_info_t *) retinfo;

    if(m == NULL)
      {
        fprintf(stderr, "CAUMultisyncModel: cau_mcast_handler:"
                " Message was not found in the postedBar queue\n");
        fflush(stderr);
        PAMI_abort();
      }

    // Message was found, we can free it and dequeue
    // Deliver the user's callback
    TRACE((stderr, "cau_mcast_handler: found and delete seqno=%d\n", seqno));

    if (ri->udata_one_pkt_ptr)
      {
        m->_user_done_fn(m->_context,
                         m->_user_cookie,
                         PAMI_SUCCESS);
        *comp_h       = NULL;
        ri->ret_flags = LAPI_SEND_REPLY;
        ri->ctl_flags = LAPI_BURY_MSG;
      }
    else
      {
        fprintf(stderr, "CAUMultisyncModel: cau_mcast_handler:"
                "ri->udata_one_pkt_ptr is null\n");
        fflush(stderr);
        PAMI_abort();
      }

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
    CAUMultisyncModel   *ms      = (CAUMultisyncModel*) CAUDevice::getClientData(hndl,did);
    PAMI_GEOMETRY_CLASS *g       = (PAMI_GEOMETRY_CLASS*)ms->_device.geometrymap(gid);
    // Next, search the posted queue for a message with the incoming seq number
    TRACE((stderr, "cau_red_handler:  Geometry=%p:  ms=%p contextId=%d\n",
           g, ms, ms->_device.getContextId()));
    CAUGeometryInfo    *gi  = (CAUGeometryInfo*)
                              g->getKey(ms->_device.getContextId(),
                                        PAMI::Geometry::CKEY_MSYNC_CLASSROUTEID);
    T_Message          *m   = (T_Message *)gi->_postedBar.findAndDelete(seqno);
    lapi_return_info_t *ri  = (lapi_return_info_t *) retinfo;
    // If no message was found, return, because we aren't ready to broadcast
    // Push a new message to the unexpected queue
    TRACE((stderr, "cau_red_handler: found and delete seqno=%d\n", seqno));

    if(m == NULL)
      {
        TRACE((stderr, "cau_red_handler: NOT FOUND %d\n", seqno));
        ms->_device.allocMessage(&m);
        cau_reduce_op_t  red;
        red.operand_type     = CAU_DOUBLE;
        red.operation        = CAU_SUM;
        new(m) T_Message(0.0, red,
                         ms->_device.getContext(),
                         NULL,
                         NULL,
                         cleanup_fn, //I need a cleanup function in this case since I allocated this message.
                         gi,
                         ms->_dispatch_mcast_id,
                         ms->_dispatch_red_id,
                         ms->_device.getHdl(),
                         ms);
        gi->_ueBar.pushTail((MatchQueueElem*)m);
      }
    else
      {
        // Message was found, we can free it and dequeue
        // We can now issue the multicast and complete the
        // msync.  Message will be freed when send done.
        TRACE((stderr, "cau_red_handler: FOUND %d\n", seqno));

        if (ri->udata_one_pkt_ptr)
          {
            TRACE((stderr, "cau_red_handler: Advance Root cau_multicast: on context=%d, seqno=%d\n",
                   ms->_device.getContextId(), seqno)); 
            m->advanceRoot();
          }
        else
          PAMI_abort();
      }

    *comp_h       = NULL;
    ri->ret_flags = LAPI_SEND_REPLY;
    ri->ctl_flags = LAPI_BURY_MSG;
    return NULL;
  }

public:
  static const size_t msync_model_state_bytes = sizeof(T_Message);
  static const size_t sizeof_msg              = sizeof(T_Message);
  CAUMultisyncModel (T_Device      &device, pami_result_t &status):
    Interface::MultisyncModel<CAUMultisyncModel<T_Device, T_Message>, T_Device, sizeof(T_Message)>(device, status),
    _device(device)
  {
    status                            = PAMI_SUCCESS;
    _dispatch_red_id                  = _device.registerSyncDispatch(cau_red_handler, this);

    if(_dispatch_red_id == -1) status   = PAMI_ERROR;

    _dispatch_mcast_id                = _device.registerSyncDispatch(cau_mcast_handler, this);

    TRACE((stderr, "CAU:  Registering Dispatch Handler:  %p %p(%d %d) ctxt=%p ctxt_id=%lu, device=%p\n",
           cau_red_handler, cau_mcast_handler,
           _dispatch_red_id, _dispatch_mcast_id,
           _device.getContext(), _device.getContextId(),
           &_device));
    
    if(_dispatch_mcast_id == -1) status = PAMI_ERROR;
  }

  pami_result_t postMultisync (uint8_t         (&state)[msync_model_state_bytes],
                               size_t           client,
                               size_t           context,
                               pami_multisync_t *msync,
                               void             *devinfo)
  {
    TRACE((stderr, "CAU:  PostMultisync\n"));
    PAMI::Topology               *topology = (PAMI::Topology*) msync->participants;

    if(topology->size() == 1)
      {
        msync->cb_done.function(_device.getContext(), msync->cb_done.clientdata, PAMI_SUCCESS);
        return PAMI_SUCCESS;
      }

    cau_reduce_op_t  red;
    red.operand_type     = CAU_DOUBLE;
    red.operation        = CAU_SUM;
    // This assert ensures that the topology that the user created
    // at geometry create time is the same as the geometry input here
    CAUGeometryInfo *gi = (CAUGeometryInfo *)devinfo;
    // Search the per-geometry UE queue for an "unexpected message"
    // Since the reduction has already been handled, we can begin our
    // broadcast of the data.  Only the root of the reduction can receive
    // an unexpected message (because the hardware handles the reductions and
    // contributers all have to make the "call-in" to participate.
    // Likewise, only the root performs the multicast, all other nodes
    // will receive an async composite.  Only the root node, which we choose
    // to be the first rank in the topology, can receive the ue message.
    T_Message       *m      = NULL;
    bool             amRoot = (topology->index2Endpoint(0) == _device.endpoint()) ? true : false;

    if(amRoot)
      {
        TRACE((stderr, "postMultiSync:  root task\n"));
        m = (T_Message*)gi->_ueBar.findAndDelete(gi->_seqno);

        if (m != NULL)
          {
            TRACE((stderr, "CAU:  Multicast\n"));
            new(m) T_Message(0.0, red,
                             _device.getContext(),
                             msync->cb_done.function,
                             msync->cb_done.clientdata,
                             cleanup_fn, //I need a cleanup function here since this is an allocated message.
                             gi,
                             _dispatch_mcast_id,
                             _dispatch_red_id,
                             _device.getHdl(),
                             this);
            m->_xfer_data[0]     = _dispatch_red_id;
            m->_xfer_data[1]     = gi->_geometry_id;
            m->_xfer_data[2]     = gi->_seqno;
            gi->_seqno++;
            TRACE((stderr, "Advance Root cau_multicast: on context=%d\n", _device.getContextId()));
            m->advanceRoot();
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
    m  = new(state) T_Message(0.0, red,
                              _device.getContext(),
                              msync->cb_done.function,
                              msync->cb_done.clientdata,
                              NULL, //no need for a cleanup here. The message is allocated in the user buffer
                              gi,
                              _dispatch_mcast_id,
                              _dispatch_red_id,
                              _device.getHdl(),
                              NULL);
    m->_xfer_data[0]     = _dispatch_red_id;
    m->_xfer_data[1]     = gi->_geometry_id;
    m->_xfer_data[2]     = gi->_seqno;
    gi->_seqno++;
    // First task in the topology is the root
    TRACE((stderr, "CAU:  endpointid = %d, pushing tail\n", _device.endpoint()));
    gi->_postedBar.pushTail((MatchQueueElem*)m);

    if(!amRoot)
      {
        // We have tp push this first because the reduce call can
        // call our callback
        TRACE((stderr, "CAU:  calling advanceNonRoot on context=%d\n", _device.getContextId()));
        m->advanceNonRoot();
      }

    return PAMI_SUCCESS;
  }
  T_Device                     &_device;
  int                           _dispatch_red_id;
  int                           _dispatch_mcast_id;
};

};
};
#endif // __components_devices_mpi_mpimultisyncmodel_h__
