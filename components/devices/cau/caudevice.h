/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
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
      inline void          init(lapi_state_t  *lapi_state,
                                lapi_handle_t  lapi_handle,
                                pami_client_t  client,
                                size_t         client_id,
                                pami_context_t context,
                                size_t         context_id,
                                unsigned       cau_uniqifier,
                                int           *dispatch_id)
        {
          _lapi_state    = lapi_state;
          _lapi_handle   = lapi_handle;
          _client        = client;
          _client_id     = client_id;
          _context       = context;
          _context_id    = context_id;
          _cau_uniqifier = cau_uniqifier;
          _dispatch_id   = dispatch_id;
          lapi_qenv(_lapi_handle, TASK_ID, (int *)&_taskid);
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
          __global._id_to_device_table[my_dispatch_id] = clientdata;
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
          __global._id_to_device_table[dispatch_id]           =  clientdata;
          return dispatch_id;
        }

      inline pami_task_t          taskid()
        {
          return _taskid;
        }

      pami_context_t getContext()
        {
          return _context;
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


      static inline void         *getClientData(int id)
        {
          return __global._id_to_device_table[id];
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
      lapi_state_t                                                     *_lapi_state;
      lapi_handle_t                                                     _lapi_handle;
      pami_client_t                                                     _client;
      pami_context_t                                                    _context;
      size_t                                                            _client_id;
      size_t                                                            _context_id;
      int                                                              *_dispatch_id;
      pami_task_t                                                       _taskid;
      unsigned                                                          _cau_uniqifier;
      Generic::Device                                                  *_generics;
      PAMI::MemoryAllocator<sizeof(Generic::GenericThread), 16>         _work_alloc;
      PAMI::MemoryAllocator<sizeof(CAUMcastMessage),16>                 _bcast_msg_allocator;
      PAMI::MemoryAllocator<sizeof(CAUMcombineMessage),16>              _mcombine_msg_allocator;
      PAMI::MemoryAllocator<sizeof(CAUMsyncMessage),16>                 _msync_msg_allocator;
      PAMI::MemoryAllocator<sizeof(CAUMcastMessage::IncomingPacket),16> _pkt_allocator;
    };
  };
};
#endif // __components_devices_cau_caupacketdevice_h__
