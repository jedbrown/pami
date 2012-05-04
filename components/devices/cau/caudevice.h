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
 * \file components/devices/cau/caudevice.h
 * \brief ???
 */

#ifndef __components_devices_cau_caudevice_h__
#define __components_devices_cau_caudevice_h__

#include "components/devices/BaseDevice.h"
#include "components/devices/cau/caumessage.h"
#include <map>

#ifdef TRACE
#undef TRACE
#define TRACE(x) //fprintf x
#else
#define TRACE(x) //fprintf x
#endif

extern pami_geometry_t mapidtogeometry (pami_context_t ctxt, int comm);

namespace PAMI
{
namespace Device
{
class CAUDevice: public Interface::BaseDevice<CAUDevice>
{
public:

  CAUDevice():
    _lapi_state(NULL),
    _lapi_handle(0)
  {}
  inline void          init(lapi_state_t   *lapi_state,
                            lapi_handle_t   lapi_handle,
                            pami_client_t   client,
                            size_t          client_id,
                            pami_context_t  context,
                            size_t          context_id,
                            unsigned        cau_uniqifier,
                            int            *dispatch_id)
  {
    pami_task_t task = __global.mapping.task();
    _lapi_state    = lapi_state;
    _lapi_handle   = lapi_handle;
    _client        = client;
    _client_id     = client_id;
    _context       = context;
    _context_id    = context_id;
    _my_endpoint   = PAMI_ENDPOINT_INIT(_client_id, task, _context_id);
    _cau_uniqifier = cau_uniqifier;
    _dispatch_id   = dispatch_id;
  }
  inline lapi_state_t  *getState() { return _lapi_state;}
  inline lapi_handle_t  getHdl() { return _lapi_handle;}
  inline unsigned       getUniqifier() { return _cau_uniqifier; }
  inline int            registerSyncDispatch(hdr_hndlr_t *hdr,
                                             void        *clientdata)
  {
    int my_dispatch_id = (*_dispatch_id)--;
    LapiImpl::Context *cp = (LapiImpl::Context *)_Lapi_port[_lapi_handle];
    internal_rc_t rc = (cp->*(cp->pDispatchSet))(my_dispatch_id,
                                                 (void*)hdr,
                                                 NULL,
                                                 null_dispatch_hint,
                                                 INTERFACE_LAPI);

    if(rc != SUCCESS) return -1;

    __global._id_to_device_table[_lapi_handle][my_dispatch_id] = clientdata;
    return my_dispatch_id;
  }

  inline int            registerMcastDispatch(int          dispatch_id,
                                              hdr_hndlr_t *hdr,
                                              void        *clientdata)
  {
    LapiImpl::Context *cp = (LapiImpl::Context *)_Lapi_port[_lapi_handle];
    internal_rc_t rc = (cp->*(cp->pDispatchSet))(dispatch_id,
                                                 (void*)hdr,
                                                 NULL,
                                                 null_dispatch_hint,
                                                 INTERFACE_LAPI);

    if(rc != SUCCESS) return -1;

    __global._id_to_device_table[_lapi_handle][dispatch_id]           =  clientdata;
    return dispatch_id;
  }

  inline pami_endpoint_t          endpoint()
  {
    return _my_endpoint;
  }
  pami_context_t getContext()
  {
    return _context;
  }
  size_t getContextId()
  {
    return _context_id;
  }
  pami_context_t getClient()
  {
    return _client;
  }

  void           setGenericDevices(Generic::Device *generics)
  {
    _generics = generics;
  }

  Generic::GenericThread * postWork( pami_work_function work_fn, void *cookie)
  {
    Generic::GenericThread *work =
      (Generic::GenericThread *) _work_alloc.allocateObject();
    work = new (work) Generic::GenericThread(work_fn, cookie);
    work->setStatus(Ready);
    _generics[_context_id].postThread(work);
    return work;
  }

  void freeWork(Generic::GenericThread *work)
  {
    _work_alloc.returnObject(work);
  }

  static inline void         *getClientData(lapi_handle_t *hndl, int id)
  {
    return __global._id_to_device_table[*hndl][id];
  }
  inline pami_geometry_t geometrymap (int comm)
  {
    return mapidtogeometry(_context, comm);
  }
  void allocMessage(CAUMcastMessage **msg)
  {
    *msg = (CAUMcastMessage*)_bcast_msg_allocator.allocateObject();
  }
  void freeMessage(CAUMcastMessage *msg)
  {
    _bcast_msg_allocator.returnObject(msg);
  }
  void allocMessage(CAUMcombineMessage **msg)
  {
    *msg = (CAUMcombineMessage*)_mcombine_msg_allocator.allocateObject();
  }
  void freeMessage(CAUMcombineMessage *msg)
  {
    _mcombine_msg_allocator.returnObject(msg);
  }
  void allocMessage(CAUMsyncMessage **msg)
  {
    *msg = (CAUMsyncMessage*)_msync_msg_allocator.allocateObject();
  }
  void freeMessage(CAUMsyncMessage *msg)
  {
    _msync_msg_allocator.returnObject(msg);
  }
public:
  lapi_state_t                                                       *_lapi_state;
  lapi_handle_t                                                       _lapi_handle;
  pami_client_t                                                       _client;
  pami_context_t                                                      _context;
  size_t                                                              _client_id;
  size_t                                                              _context_id;
  pami_endpoint_t                                                     _my_endpoint;
  int                                                                *_dispatch_id;
  unsigned                                                            _cau_uniqifier;
  Generic::Device                                                    *_generics;
  PAMI::MemoryAllocator<sizeof(Generic::GenericThread), 16>           _work_alloc;
  PAMI::MemoryAllocator<sizeof(CAUMcastMessage), 16>                  _bcast_msg_allocator;
  PAMI::MemoryAllocator<sizeof(CAUMcombineMessage), 16>               _mcombine_msg_allocator;
  PAMI::MemoryAllocator<sizeof(CAUMsyncMessage), 16>                  _msync_msg_allocator;
  PAMI::MemoryAllocator<sizeof(CAUMcastMessage::IncomingPacket), 16>  _pkt_allocator;
};
};
};
#endif // __components_devices_cau_caupacketdevice_h__
