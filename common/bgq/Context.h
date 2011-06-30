/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file common/bgq/Context.h
 * \brief PAMI Blue Gene\Q specific context implementation.
 */
#ifndef __common_bgq_Context_h__
#define __common_bgq_Context_h__


#include <stdlib.h>
#include <string.h>
#include <new>

#include <pami.h>
#include "common/ContextInterface.h"

#include "components/devices/generic/Device.h"
#include "components/devices/misc/ProgressFunctionMsg.h"
#include "components/devices/misc/AtomicBarrierMsg.h"
#include "components/devices/misc/AtomicMutexMsg.h"
#include "components/atomic/native/NativeCounter.h"
#include "components/atomic/bgq/L2Counter.h"
#include "components/atomic/counter/CounterMutex.h"
#include "components/atomic/indirect/IndirectCounter.h"
#include "components/atomic/indirect/IndirectMutex.h"

#include "api/extension/c/async_progress/ProgressExtension.h"

#include "components/memory/MemoryAllocator.h"

#include "components/memory/MemoryManager.h"
#include "Memregion.h"

#include "p2p/protocols/RGet.h"
#include "p2p/protocols/rget/GetRdma.h"
#include "p2p/protocols/rput/PutRdma.h"
#include "p2p/protocols/get/GetOverSend.h"
#include "p2p/protocols/put/PutOverSend.h"
#include "p2p/protocols/rmw/RmwOverSend.h"
#include "p2p/protocols/send/eager/Eager.h"
#include "p2p/protocols/send/composite/Composite.h"

#include "TypeDefs.h"
#include "algorithms/geometry/BGQMultiRegistration.h"

#include "algorithms/geometry/P2PCCMIRegistration.h"

#include "util/trace.h"

#define DO_TRACE_ENTEREXIT 0
#define DO_TRACE_DEBUG     0

#include "algorithms/geometry/GeometryOptimizer.h"

namespace PAMI
{

  typedef Mutex::Indirect<Mutex::Counter<Counter::BGQ::L2> >  ContextLock;

  typedef CollRegistration::P2P::CCMIRegistration < BGQGeometry,
  ShmemDevice,
  MUDevice,
  ProtocolAllocator,
  ShmemEager,
  ShmemDevice,
  ShmemNI_AM,
  ShmemNI_AS,
  MUEager,
  MUDevice,
  MUNI_AM,
  MUNI_AS,
  CompositeNI_AM,
  CompositeNI_AS > CCMIRegistration;

  typedef CollRegistration::BGQMultiRegistration < BGQGeometry, 
      ShmemDevice,
      AllSidedShmemNI, 
      MUDevice, 
      MUGlobalNI, 
      MUAxialDputNI, 
      MUShmemAxialDputNI > BGQRegistration;
  /**
   * \brief Class containing all devices used on this platform.
   *
   * This container object governs creation (allocation of device objects),
   * initialization of device objects, and advance of work. Note, typically
   * the devices advance routine is very short - or empty - since it only
   * is checking for received messages (if the device even has reception).
   *
   * The generic device is present in all platforms. This is how context_post
   * works as well as how many (most/all) devices enqueue work.
   */
  class PlatformDeviceList
  {
    public:
      PlatformDeviceList():
          _generics(NULL),
          _shmem(NULL),
          _progfunc(NULL),
          _atombarr(NULL),
#if 0
          _localallreduce(NULL),
          _localbcast(NULL),
          _localreduce(NULL),
#endif
          _mu(NULL),
          _atmmtx(NULL)
      { }

      /**
       * \brief initialize this platform device list
       *
       * This creates arrays (at least 1 element) for each device used in this platform.
       * Note, in some cases there may be only one device instance for the entire
       * process (all clients), but any handling of that (mutexing, etc) is hidden.
       *
       * Device arrays are semi-opaque (we don't know how many
       * elements each has).
       *
       * \param[in] clientid     Client ID (index)
       * \param[in] contextid    Context ID (index)
       */
      inline pami_result_t generate(size_t clientid, size_t num_ctx, Memory::MemoryManager &mm)
      {
        // these calls create (allocate and construct) each element.
        // We don't know how these relate to contexts, they are semi-opaque.
        TRACE_FN_ENTER();
        TRACE_FORMAT("<%p>",this);
        TRACE_STRING("device init: generic");
        _generics = PAMI::Device::Generic::Device::Factory::generate(clientid, num_ctx, mm, NULL);

        if (__global.useshmem())
          {
            TRACE_STRING("device init: shmem");
            _shmem = ShmemDevice::Factory::generate(clientid, num_ctx, mm, _generics);
#if 0
            TRACE_STRING("device init: local allreduce wq");
            _localallreduce = PAMI::Device::LocalAllreduceWQDevice::Factory::generate(clientid, num_ctx, mm, _generics);
            TRACE_STRING("device init: local bcast wq");
            _localbcast = PAMI::Device::LocalBcastWQDevice::Factory::generate(clientid, num_ctx, mm, _generics);
            TRACE_STRING("device init: local reduce wq");
            _localreduce = PAMI::Device::LocalReduceWQDevice::Factory::generate(clientid, num_ctx, mm, _generics);
#endif
          }

        TRACE_STRING("device init: progress function");
        _progfunc = PAMI::Device::ProgressFunctionDev::Factory::generate(clientid, num_ctx, mm, _generics);
        TRACE_STRING("device init: atomic barrier");
        _atombarr = PAMI::Device::AtomicBarrierDev::Factory::generate(clientid, num_ctx, mm, _generics);

        if (__global.useMU())
          {
            TRACE_STRING("device init: MU");
            _mu = Device::MU::Factory::generate(clientid, num_ctx, mm, _generics);
          }

        _atmmtx = PAMI::Device::AtomicMutexDev::Factory::generate(clientid, num_ctx, mm, _generics);
        PAMI_assertf(_atmmtx == _generics, "AtomicMutexDev must be a NillSubDevice");
        TRACE_FN_EXIT();
        return PAMI_SUCCESS;
      }

      /**
       * \brief initialize devices for specific context
       *
       * Called once per context, after context object is initialized.
       * Devices must handle having init() called multiple times, using
       * clientid and contextid to ensure initialization happens to the correct
       * instance and minimizing redundant initialization. When each is called,
       * the 'this' pointer actually points to the array - each device knows whether
       * that is truly an array and how many elements it contains.
       *
       * \param[in] clientid     Client ID (index)
       * \param[in] num_ctx      Number of contexts in this client
       * \param[in] ctx          Context opaque entity
       * \param[in] contextid    Context ID (index)
       */
      inline pami_result_t init(size_t clientid, size_t contextid, pami_client_t clt, pami_context_t ctx, PAMI::Memory::MemoryManager *mm)
      {
        PAMI::Device::Generic::Device::Factory::init(_generics, clientid, contextid, clt, ctx, mm, _generics);

        if (__global.useshmem())
          {
            ShmemDevice::Factory::init(_shmem, clientid, contextid, clt, ctx, mm, _generics);
#if 0
            PAMI::Device::LocalAllreduceWQDevice::Factory::init(_localallreduce, clientid, contextid, clt, ctx, mm, _generics);
            PAMI::Device::LocalBcastWQDevice::Factory::init(_localbcast, clientid, contextid, clt, ctx, mm, _generics);
            PAMI::Device::LocalReduceWQDevice::Factory::init(_localreduce, clientid, contextid, clt, ctx, mm, _generics);
#endif
          }

        PAMI::Device::ProgressFunctionDev::Factory::init(_progfunc, clientid, contextid, clt, ctx, mm , _generics);
        PAMI::Device::AtomicBarrierDev::Factory::init(_atombarr, clientid, contextid, clt, ctx, mm, _generics);

        if (__global.useMU())
          {
            Device::MU::Factory::init(_mu, clientid, contextid, clt, ctx, mm, _generics);
          }

        PAMI::Device::AtomicMutexDev::Factory::init(_atmmtx, clientid, contextid, clt, ctx, mm, _generics);
        return PAMI_SUCCESS;
      }

      /**
       * \brief advance all devices
       *
       * since device arrays are semi-opaque (we don't know how many
       * elements each has) we call a more-general interface here.
       *
       * \param[in] clientid     Client ID (index)
       * \param[in] contextid    Context ID (index)
       */
      inline size_t advance(size_t clientid, size_t contextid)
      {
        size_t events = 0;
        //unsigned long long t1 = PAMI_Wtimebase();
        events += PAMI::Device::Generic::Device::Factory::advance(_generics, clientid, contextid);

        if (__global.useshmem())
          {
            events += ShmemDevice::Factory::advance(_shmem, clientid, contextid);
#if 0
            events += PAMI::Device::LocalAllreduceWQDevice::Factory::advance(_localallreduce, clientid, contextid);
            events += PAMI::Device::LocalBcastWQDevice::Factory::advance(_localbcast, clientid, contextid);
            events += PAMI::Device::LocalReduceWQDevice::Factory::advance(_localreduce, clientid, contextid);
#endif
          }

        events += PAMI::Device::ProgressFunctionDev::Factory::advance(_progfunc, clientid, contextid);
        events += PAMI::Device::AtomicBarrierDev::Factory::advance(_atombarr, clientid, contextid);

        if (__global.useMU())
         events += Device::MU::Factory::advance(_mu, clientid, contextid);
        //unsigned long long t2 = PAMI_Wtimebase() -t1;
        //printf("overhead:%lld\n", t2);


        events += Device::AtomicMutexDev::Factory::advance(_atmmtx, clientid, contextid);
        return events;
      }

      PAMI::Device::Generic::Device *_generics; // need better name...
      ShmemDevice *_shmem; //compile-time always needs the devices since runtime is where the check is made to use them

      PAMI::Device::ProgressFunctionDev *_progfunc;
      PAMI::Device::AtomicBarrierDev *_atombarr;
#if 0
      PAMI::Device::LocalAllreduceWQDevice *_localallreduce;
      PAMI::Device::LocalBcastWQDevice *_localbcast;
      PAMI::Device::LocalReduceWQDevice *_localreduce;
#endif
      MUDevice *_mu;
      PAMI::Device::AtomicMutexDev *_atmmtx;
  }; // class PlatformDeviceList

  class Context : public Interface::Context<PAMI::Context>
  {
    public:
      inline Context (pami_client_t client, size_t clientid, size_t id, size_t num,
                      PlatformDeviceList *devices,
                      PAMI::Memory::MemoryManager * pmm, size_t bytes,
                      BGQGeometry *world_geometry,
                      std::map<unsigned, pami_geometry_t> *geometry_map) :
          Interface::Context<PAMI::Context> (client, id),
          _client (client),
          _context ((pami_context_t)this),
          _clientid (clientid),
          _contextid (id),
          _dispatch_id(255),
          _geometry_map(geometry_map),
          _lock(),
          _multi_registration(NULL),
          _ccmi_registration(NULL),
          _world_geometry(world_geometry),
          _status(PAMI_SUCCESS),
          _shmemMcastModel(NULL),
          _shmemMsyncModel(NULL),
          _shmemMcombModel(NULL),
          _shmem_native_interface(NULL),
          _devices(devices),
          _pgas_mu_registration(NULL),
          _pgas_shmem_registration(NULL),
          _pgas_composite_registration(NULL),
          _dummy_disable(false),
          _dummy_disabled(false),
          _senderror(),
          _get(devices->_mu[_contextid]),
          _put(devices->_mu[_contextid]),
          _rmw(devices->_mu[_contextid])
//          _rmw(devices->_shmem[_contextid])
      {
        TRACE_FN_ENTER();
        _async_suspend = NULL;
        _async_resume = NULL;
	_async_cookie = NULL;
        char mmkey[PAMI::Memory::MMKEYSIZE];
        char *mms;
        mms = mmkey + sprintf(mmkey, "/pami-clt%zd-ctx%zd", clientid, id);

        TRACE_FORMAT( "<%p>", this);
        // ----------------------------------------------------------------
        // Compile-time assertions
        // ----------------------------------------------------------------

        // Make sure the memory allocator is large enough for all
        // protocol classes.
//        COMPILE_TIME_ASSERT(sizeof(MUGlobalNI) <= ProtocolAllocator::objsize);
        // ----------------------------------------------------------------
        // Compile-time assertions
        // ----------------------------------------------------------------

        strcpy(mms, "-mm");
        _mm.init(pmm, bytes, 16, 16, 0, mmkey);
        _self = PAMI_ENDPOINT_INIT(_clientid, __global.mapping.task(), _contextid);

        //strcpy(mms, "-lock");
        _lock.init(__global._wuRegion_mm, NULL);

        _devices->init(_clientid, _contextid, _client, _context, &_mm);

        pami_endpoint_t self = PAMI_ENDPOINT_INIT(_clientid, __global.mapping.task(), _contextid);
        _get.initialize (_dispatch_id--, self, _context);
        _put.initialize (_dispatch_id--, self, _context);
        _rmw.initialize (_dispatch_id--, self, _context);

        Protocol::Get::GetRdma <Device::MU::DmaModelMemoryFifoCompletion, MUDevice> * rget_mu = NULL;
        Protocol::Put::PutRdma <Device::MU::DmaModelMemoryFifoCompletion, MUDevice> * rput_mu = NULL;

        Protocol::Get::GetRdma <Device::Shmem::DmaModel<ShmemDevice, false>, ShmemDevice> * rget_shmem = NULL;
        Protocol::Put::PutRdma <Device::Shmem::DmaModel<ShmemDevice, false>, ShmemDevice> * rput_shmem = NULL;


        ///////////////////////////////////////////////////////////////
        // Setup rget/rput protocols
        ///////////////////////////////////////////////////////////////

        if (__global.useMU())
          {

            // Initialize mu rget and mu rput protocols.
            pami_result_t result = PAMI_ERROR;

            rget_mu = Protocol::Get::GetRdma <Device::MU::DmaModelMemoryFifoCompletion, MUDevice>::
                      generate (_devices->_mu[_contextid], _context, _request, result);

            if (result != PAMI_SUCCESS) rget_mu = NULL;

            rput_mu = Protocol::Put::PutRdma <Device::MU::DmaModelMemoryFifoCompletion, MUDevice>::
                      generate (_devices->_mu[_contextid], _context, _request, result);

            if (result != PAMI_SUCCESS) rput_mu = NULL;
          }

#if 0
        // ----------------------------------------------------------------
        // Initialize the memory region get protocol(s)
        // ----------------------------------------------------------------
        {
          pami_result_t result = PAMI_ERROR;

          if (__global.useMU() && __global.useshmem())
            {
              Protocol::Get::GetRdma <Device::MU::MUDmaModel<false>, MUDevice> * getmu =
                Protocol::Get::GetRdma <Device::MU::MUDmaModel<false>, MUDevice>::
                generate (_devices->_mu[_contextid], _request, result);
//        if (result != PAMI_SUCCESS)
//          getmu = Protocol::Get::NoRGet::generate (_context, _request);

              Protocol::Get::GetRdma <Device::Shmem::DmaModel<ShmemDevice, false>, ShmemDevice> * getshmem =
                Protocol::Get::GetRdma <Device::Shmem::DmaModel<ShmemDevice, false>, ShmemDevice>::
                generate (_devices->_shmem[_contextid], _request, result);
//        if (result != PAMI_SUCCESS)
//          getshmem = Protocol::Get::NoRGet::generate (_context, _request);

              //_rget = (Protocol::Get::RGet *) Protocol::Get::CompositeRGet::
              //_rget = Protocol::Get::CompositeRGet::
              //generate (_request, getshmem, getmu, result);
              _rget = (Protocol::Get::RGet *) Protocol::Get::Factory::
                      generate (getshmem, getmu, _request, result);

              Protocol::Put::PutRdma <Device::MU::MUDmaModel<false>, MUDevice> * putmu =
                Protocol::Put::PutRdma <Device::MU::MUDmaModel<false>, MUDevice>::
                generate (_devices->_mu[_contextid], _request, result);

              Protocol::Put::PutRdma <Device::Shmem::DmaModel<ShmemDevice, false>, ShmemDevice> * putshmem =
                Protocol::Put::PutRdma <Device::Shmem::DmaModel<ShmemDevice, false>, ShmemDevice>::
                generate (_devices->_shmem[_contextid], _request, result);

              _rput = (Protocol::Put::RPut *) Protocol::Put::Factory::
                      generate (putshmem, putmu, _request, result);

            }
          else if (__global.useshmem())
            {
              _rget = Protocol::Get::GetRdma <Device::Shmem::DmaModel<ShmemDevice, false>, ShmemDevice>::
                      generate (_devices->_shmem[_contextid], _request, result);
//        if (result != PAMI_SUCCESS)
//          _rget = Protocol::Get::NoGet::generate (_context, _request);

              _rput = Protocol::Put::PutRdma <Device::Shmem::DmaModel<ShmemDevice, false>, ShmemDevice>::
                      generate (_devices->_shmem[_contextid], _request, result);
//        if (result != PAMI_SUCCESS)
//          _rget = Protocol::Get::NoGet::generate (_context, _request);
            }
          else
            {
              _rget = Protocol::Get::GetRdma <Device::MU::MUDmaModel<false>, MUDevice>::
                      generate (_devices->_mu[_contextid], _request, result);

              _rput = Protocol::Put::PutRdma <Device::MU::MUDmaModel<false>, MUDevice>::
                      generate (_devices->_mu[_contextid], _request, result);
            }
        }
#endif
        if (__global.useshmem())
          {
            // Can't construct these models on single process nodes (no shmem)
            if (__global.topology_local.size() > 1)
              {
                // Initialize shmem rget and shmem rput protocols.
                pami_result_t result = PAMI_ERROR;

                rget_shmem = Protocol::Get::GetRdma <Device::Shmem::DmaModel<ShmemDevice, false>, ShmemDevice>::
                             generate (_devices->_shmem[_contextid], _context, _request, result);

                if (result != PAMI_SUCCESS) rget_shmem = NULL;

                rput_shmem = Protocol::Put::PutRdma <Device::Shmem::DmaModel<ShmemDevice, false>, ShmemDevice>::
                             generate (_devices->_shmem[_contextid], _context, _request, result);

                if (result != PAMI_SUCCESS) rput_shmem = NULL;
              }
            else TRACE_STRING("topology does not support shmem");
          }
        // Complete rget and rput protocol initialization
        if (((rget_mu != NULL) && (rget_shmem != NULL)) &&
            ((rput_mu != NULL) && (rput_shmem != NULL)))
          {
            // Create "composite" protocols
            pami_result_t result = PAMI_ERROR;
            _rget = (Protocol::Get::RGet *) Protocol::Get::Factory::
                    generate (rget_shmem, rget_mu, _request, result);
            _rput = (Protocol::Put::RPut *) Protocol::Put::Factory::
                    generate (rput_shmem, rput_mu, _request, result);
          }
        else if (((rget_mu != NULL) && (rget_shmem == NULL)) &&
                 ((rput_mu != NULL) && (rput_shmem == NULL)))
          {
            _rget = rget_mu;
            _rput = rput_mu;
          }
        else if (((rget_mu == NULL) && (rget_shmem != NULL)) &&
                 ((rput_mu == NULL) && (rput_shmem != NULL)))
          {
            _rget = rget_shmem;
            _rput = rput_shmem;
          }
        else
          {
            // No shmem or mu rget-rput protocols available
            _rget = Protocol::Get::NoRGet::generate (_request);
            _rput = Protocol::Put::NoRPut::generate (_request);
          }

        ///////////////////////////////////////////////////////////////
        // Register collectives
        ///////////////////////////////////////////////////////////////
        if (__global.useMU())
          {
            _pgas_mu_registration = new(_pgas_mu_registration_storage) MU_PGASCollreg(_client, (pami_context_t)this, _clientid, _contextid, _protocol, Device::MU::Factory::getDevice(_devices->_mu, _clientid, _contextid),ShmemDevice::Factory::getDevice(_devices->_shmem, _clientid, _contextid), &_dispatch_id, _geometry_map);
          }
        if (__global.useshmem())
          {
            // Can't construct these models on single process nodes (no shmem)
            if (__global.topology_local.size() > 1)
              {
                TRACE_FORMAT( "<%p> construct shmem native interface and models", this);
                _shmemMcastModel         = (ShmemMcstModel*)_shmemMcastModel_storage;
                _shmemMcombModel         = (ShmemMcombModel*)_shmemMcombModel_storage;
                _shmemMsyncModel         = (ShmemMsyncModel*)_shmemMsyncModel_storage;

                // if _shmemMsyncModel_storage is in heap...
                // PAMI_assert(Barrier_Model::checkCtorMm(__global.heap_mm);
                new (_shmemMsyncModel_storage)        ShmemMsyncModel(PAMI::Device::AtomicBarrierDev::Factory::getDevice(_devices->_atombarr, _clientid, _contextid), _status);

                //new (_shmemMcastModel_storage)        ShmemMcstModel(PAMI::Device::Generic::Device::Factory::getDevice(_devices->_generics,  _clientid, _contextid),_status);
                new (_shmemMcastModel_storage)        ShmemMcstModel(ShmemDevice::Factory::getDevice(_devices->_shmem, _clientid, _contextid),_status);
                new (_shmemMcombModel_storage)        ShmemMcombModel(ShmemDevice::Factory::getDevice(_devices->_shmem, _clientid, _contextid), _status);
#if 0
                new (_shmemMcastModel_storage)        ShmemMcstModel(PAMI::Device::LocalBcastWQDevice::Factory::getDevice(_devices->_localbcast, _clientid, _contextid), _status);
                new (_shmemMcombModel_storage)        ShmemMcombModel(PAMI::Device::LocalAllreduceWQDevice::Factory::getDevice(_devices->_localreduce, _clientid, _contextid), _status);
#endif

                _shmem_native_interface  = (AllSidedShmemNI*)_shmem_native_interface_storage;
                new (_shmem_native_interface_storage) AllSidedShmemNI(_shmemMcastModel, _shmemMsyncModel, _shmemMcombModel, client, (pami_context_t)this, id, clientid, &_dispatch_id);

                _pgas_shmem_registration = new(_pgas_shmem_registration_storage) Shmem_PGASCollreg(_client, (pami_context_t)this, _clientid, _contextid, _protocol, ShmemDevice::Factory::getDevice(_devices->_shmem, _clientid, _contextid), ShmemDevice::Factory::getDevice(_devices->_shmem, _clientid, _contextid), & _dispatch_id, _geometry_map);
              }
            else TRACE_STRING("topology does not support shmem");
          }
        if ((__global.useMU()) && (__global.useshmem()))
            _pgas_composite_registration = new(_pgas_composite_registration_storage) Composite_PGASCollreg(_client, (pami_context_t)this, _clientid, _contextid, _protocol, Device::MU::Factory::getDevice(_devices->_mu, _clientid, _contextid), _devices->_shmem[_contextid], &_dispatch_id, _geometry_map, true);

        TRACE_FORMAT( "<%p> Register collectives(%p,%p,%p,%zu,%zu", this, _shmem_native_interface, client, this, id, clientid);
        // The multi registration will use shmem/mu if they are ctor'd above.

#ifndef ENABLE_COLLECTIVE_MULTICONTEXT
        if (_contextid == 0) 
#endif
	  {
            _multi_registration       =  new ((BGQRegistration*) _multi_registration_storage)
	      BGQRegistration(_shmem_native_interface,
			      ShmemDevice::Factory::getDevice(_devices->_shmem, _clientid, _contextid),
			      PAMI::Device::MU::Factory::getDevice(_devices->_mu, _clientid, _contextid),
			      client,
			      (pami_context_t)this,
			      id,
			      clientid,
			      &_dispatch_id,
			      _geometry_map);
	    uint64_t inval = (uint64_t)-1;
	    _multi_registration->receive_global (_contextid, _world_geometry, &inval, 1);
	  }
	
#ifndef ENABLE_COLLECTIVE_MULTICONTEXT
        if (_contextid == 0)
#endif
            _ccmi_registration =  new((CCMIRegistration*)_ccmi_registration_storage) 
            CCMIRegistration(_client, _context, _contextid, _clientid, 
                             _devices->_shmem[_contextid], _devices->_mu[_contextid], 
                             _protocol, __global.useshmem(), __global.useMU(), __global.topology_global.size(), __global.topology_local.size(), 
                             &_dispatch_id, 
                             _geometry_map);

        // Can only use shmem pgas if the geometry is all local tasks, so check the topology
        if (_pgas_shmem_registration && ((PAMI::Topology*)_world_geometry->getTopology(PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX))->isLocal()) _pgas_shmem_registration->analyze(_contextid, _world_geometry, 0);

        // Can always use MU if it's available
        if (_pgas_mu_registration) _pgas_mu_registration->analyze(_contextid, _world_geometry, 0);

        // Can always use composite if it's available
        if (_pgas_composite_registration) _pgas_composite_registration->analyze(_contextid, _world_geometry, 0);

        if(_ccmi_registration) // && (((PAMI::Topology*)_world_geometry->getTopology(PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX))->size() != 1))
        {
            _world_geometry->resetUEBarrier(); // Reset so ccmi will select the UE barrier
            _ccmi_registration->analyze(_contextid, _world_geometry, 0);
         }

        if(_multi_registration) // && (((PAMI::Topology*)_world_geometry->getTopology(PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX))->size() != 1))
        {
           _multi_registration->analyze(_contextid, _world_geometry, 0);

           // for now, this is the only registration that has a phase 1...
           // We know that _world_geometry is always "optimized" at create time.
           _multi_registration->analyze(_contextid, _world_geometry, 1);
         }

        // dispatch_impl relies on the table being initialized to NULL's.
        size_t i, n = sizeof(_dispatch) / sizeof(Protocol::Send::Send *);
        for (i=0; i<n; i++)
          _dispatch[i] = (Protocol::Send::Send *) & _senderror;

        TRACE_FN_EXIT();
      }

      inline ~Context() { }

      inline pami_client_t getClient_impl ()
      {
        return _client;
      }

      inline size_t getId_impl ()
      {
        return _contextid;
      }

      inline pami_result_t destroy_impl ()
      {
        //return PAMI_UNIMPL;
        return PAMI_SUCCESS;
      }

      inline pami_result_t post_impl (pami_work_t *state, pami_work_function work_fn, void * cookie)
      {
        PAMI::Device::Generic::GenericThread *work;
        COMPILE_TIME_ASSERT(sizeof(*state) >= sizeof(*work));
        work = new (state) PAMI::Device::Generic::GenericThread(work_fn, cookie);
        _devices->_generics[_contextid].postThread(work);
        return PAMI_SUCCESS;
      }

      inline size_t advance_impl (size_t maximum, pami_result_t & result)
      {
//          result = PAMI_EAGAIN;
        result = PAMI_SUCCESS;
        size_t events = 0;
        unsigned i;

        for (i = 0; i < maximum && events == 0; i++)
          {
            events += _devices->advance(_clientid, _contextid);
          }

        //if (events > 0) result = PAMI_SUCCESS;

        return events;
      }

      inline pami_result_t lock_impl ()
      {
        _lock.acquire ();
        return PAMI_SUCCESS;
      }

      inline pami_result_t trylock_impl ()
      {
        if (_lock.tryAcquire ())
          {
            return PAMI_SUCCESS;
          }

        return PAMI_EAGAIN;
      }

      inline pami_result_t unlock_impl ()
      {
        _lock.release ();
        return PAMI_SUCCESS;
      }

      inline pami_result_t send_impl (pami_send_t * parameters)
      {
        TRACE_FN_ENTER();
        size_t id = (size_t) (parameters->send.dispatch);
        TRACE_FORMAT("_dispatch[%zu] = %p", id, _dispatch[id]);

        pami_result_t rc = _dispatch[id]->simple (parameters);

        TRACE_FORMAT("rc = %d", rc);
        TRACE_FN_EXIT();
        return rc;
      }

      inline pami_result_t send_impl (pami_send_immediate_t * parameters)
      {
        TRACE_FN_ENTER();
        size_t id = (size_t) (parameters->dispatch);
        TRACE_FORMAT("_dispatch[%zu] = %p", id, _dispatch[id]);

        pami_result_t rc = _dispatch[id]->immediate (parameters);

        TRACE_FORMAT("rc = %d", rc);
        TRACE_FN_EXIT();
        return rc;
      }

      inline pami_result_t send_impl (pami_send_typed_t * parameters)
      {
        return PAMI_UNIMPL;
      }

      inline pami_result_t put_impl (pami_put_simple_t * parameters)
      {
        return _put.simple (parameters);
      }

      inline pami_result_t put_typed (pami_put_typed_t * parameters)
      {
        return PAMI_UNIMPL;
      }

      inline pami_result_t get_impl (pami_get_simple_t * parameters)
      {

        return _get.get (parameters);
      }

      inline pami_result_t get_typed (pami_get_typed_t * parameters)
      {
        return PAMI_UNIMPL;
      }

      inline pami_result_t rmw (pami_rmw_t * parameters)
      {
        return _rmw.start (parameters);
      }

      inline pami_result_t memregion_create_impl (void             * address,
                                                  size_t             bytes_in,
                                                  size_t           * bytes_out,
                                                  pami_memregion_t * memregion)
      {
        new ((void *) memregion) Memregion ();
        Memregion * mr = (Memregion *) memregion;
        return mr->createMemregion (bytes_out, bytes_in, address, 0);
      }

      inline pami_result_t memregion_destroy_impl (pami_memregion_t * memregion)
      {
        // Memory regions do not need to be deregistered on BG/Q so this
        // interface is implemented as a noop.
        return PAMI_SUCCESS;
      }

      inline pami_result_t rput_impl (pami_rput_simple_t * parameters)
      {
        TRACE_FN_ENTER();

        pami_result_t rc = _rput->simple (parameters);

        TRACE_FORMAT("rc = %d", rc);
        TRACE_FN_EXIT();
        return rc;
      }

      inline pami_result_t rput_typed (pami_rput_typed_t * parameters)
      {
        return PAMI_UNIMPL;
      }

      inline pami_result_t rget_impl (pami_rget_simple_t * parameters)
      {
        TRACE_FN_ENTER();

        pami_result_t rc = _rget->simple (parameters);

        TRACE_FORMAT("rc = %d", rc);
        TRACE_FN_EXIT();
        return rc;
      }

      inline pami_result_t rget_typed (pami_rget_typed_t * parameters)
      {
        return PAMI_UNIMPL;
      }

      inline pami_result_t purge_totask (pami_endpoint_t *dest, size_t count)
      {
        return PAMI_SUCCESS;
      }

      inline pami_result_t resume_totask (pami_endpoint_t *dest, size_t count)
      {
        return PAMI_SUCCESS;
      }

      inline pami_result_t fence_begin ()
      {
        return PAMI_UNIMPL;
      }

      inline pami_result_t fence_end ()
      {
        return PAMI_UNIMPL;
      }

      inline pami_result_t fence_all (pami_event_function   done_fn,
                                      void               * cookie)
      {
        return PAMI_UNIMPL;
      }

      inline pami_result_t fence_endpoint (pami_event_function   done_fn,
                                           void                * cookie,
                                           pami_endpoint_t       endpoint)
      {
        return PAMI_UNIMPL;
      }


      inline pami_result_t collective_impl (pami_xfer_t * parameters)
      {
        TRACE_FN_ENTER();
        Geometry::Algorithm<BGQGeometry> *algo = (Geometry::Algorithm<BGQGeometry> *)parameters->algorithm;
        TRACE_FORMAT("algorithm %p, context %p",algo,this);
        algo->setContext((pami_context_t) this);
        TRACE_FN_EXIT();
        return algo->generate(parameters);
      }

      inline pami_result_t amcollective_dispatch_impl (pami_algorithm_t            algorithm,
                                                       size_t                     dispatch,
                                                       pami_dispatch_callback_function fn,
                                                       void                     * cookie,
                                                       pami_collective_hint_t      options)
      {
        Geometry::Algorithm<BGQGeometry> *algo = (Geometry::Algorithm<BGQGeometry> *)algorithm;
        return algo->dispatch_set(dispatch, fn, cookie, options);
      }

      inline pami_result_t dispatch_impl (size_t                          id,
                                          pami_dispatch_callback_function fn,
                                          void                          * cookie,
                                          pami_dispatch_hint_t            options)
      {
        TRACE_FN_ENTER();
        pami_result_t result = PAMI_ERROR;
        TRACE_FORMAT(" _dispatch[%zu] = %p, options = %#X", id, _dispatch[id], *(unsigned*)&options);

        // Return an error for invalid / unimplemented 'hard' hints.
        if (
            options.use_rdma              == PAMI_HINT_ENABLE  ||
            (options.use_shmem            == PAMI_HINT_ENABLE  && !__global.useshmem()) ||
            (options.use_shmem            == PAMI_HINT_DISABLE && !__global.useMU())    ||
            false)
          {
            TRACE_FN_EXIT();
            return PAMI_ERROR;
          }

        pami_endpoint_t self = PAMI_ENDPOINT_INIT(_clientid, __global.mapping.task(), _contextid);

        using namespace Protocol::Send;


        if (_dispatch[id] == (Send *) & _senderror)
          {
            if ((options.use_shmem == PAMI_HINT_DISABLE) || (__global.useMU() && !__global.useshmem()))
              {
                _dispatch[id] =
                  Eager <Device::MU::PacketModel>::generate (id, fn.p2p, cookie,
                                                             _devices->_mu[_contextid],
                                                             self, _context, options,
                                                             _protocol, result);
              }
            else if ((options.use_shmem == PAMI_HINT_ENABLE) || (!__global.useMU() && __global.useshmem()))
              {
                _dispatch[id] =
                  Eager <ShmemPacketModel>::generate (id, fn.p2p, cookie,
                                                             _devices->_shmem[_contextid],
                                                             self, _context, options,
                                                             _protocol, result);
              }
            else
              {
                _dispatch[id] =
                  Eager <ShmemPacketModel, Device::MU::PacketModel>::generate (id, fn.p2p, cookie,
                                                             _devices->_shmem[_contextid],
                                                             _devices->_mu[_contextid],
                                                             self, _context, options,
                                                             _protocol, result);
              }
          } // end dispatch[id]==null

        TRACE_FORMAT("result = %d", result);
        TRACE_FN_EXIT();
        return result;
      }

      /// \brief Dummy work function to keep context advance returning "busy"
      ///
      /// \see pami_work_function
      ///
      static pami_result_t _dummy_work_fn(pami_context_t ctx, void *cookie)
      {
        Context *thus = (Context *)cookie;

        if (!thus->_dummy_disable)
          {
            return PAMI_EAGAIN;
          }
        else
          {
            thus->_dummy_disabled = true;
            // Note: dequeue happens sometime after return...
            // So all participants must lock the context to
            // ensure an atomic view of the above variables.
            return PAMI_SUCCESS;
          }
      }

      /// \brief BGQ-only method to return the BGQ core to which a context has best affinity
      ///
      /// \return	BGQ Core ID that is best for context
      ///
      inline uint32_t coreAffinity()
      {
        // might be other affinities to consider, in the future.
        // could be cached in context, instead of calling MU RM (etc) every time
        // When the MU is active, go with its affinity.
        // Otherwise, compute an affinity.
        if ( __global.useMU() )
          {
            return Device::MU::Factory::getDevice(_devices->_mu, _clientid, _contextid).affinity();
          }
        else
          {
            return (NUM_CORES - 1) - (_contextid % (Kernel_ProcessorCount() / NUM_SMT));
          }
      }

      /// \brief BGQ-only method to fix any problems with hardware affinity to core/thread
      ///
      /// This method is (initially) to address a problem with MU interrupts having
      /// hardware affinity to a core. This means that if a commthread is advancing
      /// a context from a "foriegn" core it cannot depend on the WakeUp Unit to
      /// wake up a waitimpl instruction and so we have to effectively disable the
      /// wait. This is done by posting a dummy (persistent) function on the generic
      /// device which will cause the return value of advance to always be "1", which
      /// will prevent the commthread from going into wait.
      ///
      /// Caller holds the context lock.
      ///
      /// \param[in] acquire	True if acquiring the context,
      ///			false means the context is being released.
      ///
      inline void cleanupAffinity(bool acquire)
      {
        TRACE_FN_ENTER();
#if 1
        bool affinity = (coreAffinity() == Kernel_ProcessorCoreID());
        TRACE_FORMAT("acquire=%d, affinity=%d, coreAffinity=%u, Kernel_ProcessorCoreID=%u, ContextID=%zu", acquire, affinity, coreAffinity(), Kernel_ProcessorCoreID(), _contextid);
#else
        bool affinity = !__global.useMU(); // if no MU, affinity anywhere
#endif
        bool enqueue = (acquire && !affinity);
        bool cancel = (enqueue && _dummy_disable && !_dummy_disabled);
        bool dequeue = (!acquire);

        // If we are using the MU, tell the MU device to set up interrupts properly
        // depending on whether we are acquiring the context or releasing it.
        if ( __global.useMU() )
          {
            Device::MU::Factory::getDevice(_devices->_mu, _clientid, _contextid).setInterrupts(acquire);
          }

        if (cancel)
          {
            _dummy_disable = _dummy_disabled = false;
          }
        else if (enqueue)
          {
            _dummy_disable = _dummy_disabled = false;
            PAMI::Device::Generic::GenericThread *work = new (&_dummy_work)
            PAMI::Device::Generic::GenericThread(_dummy_work_fn, this);
            _devices->_generics[_contextid].postThread(work);
          }
        else if (dequeue)
          {
            _dummy_disable = true;
          }
        TRACE_FN_EXIT();
      }

      inline pami_result_t registerWithOptimizer (Geometry::GeometryOptimizer<BGQGeometry>  *go)
      {
        TRACE_FN_ENTER();
#ifndef ENABLE_COLLECTIVE_MULTICONTEXT
        if (_contextid == 0) 
#endif
            {
            TRACE_FORMAT("id %zu, geometry %p", _contextid, go->geometry());
            uint64_t  reduce_result[16];
            int n_multi = 0;
            _multi_registration->register_local (_contextid, go->geometry(), reduce_result, n_multi);	
            go->registerWithOptimizer (_contextid, reduce_result, n_multi, receive_global, this );

        }
        TRACE_FN_EXIT();
        return PAMI_SUCCESS;
      }

      
      static void receive_global( size_t           context_id,
				  void           * cookie,
				  uint64_t       * reduce_result,
				  size_t           n,
				  BGQGeometry    * geometry,
				  pami_result_t    result )
      {
        TRACE_FN_ENTER();
        TRACE_FORMAT("context id %zu, geometry %p", context_id, geometry);
        Context *context = (Context *) cookie;
        context->_multi_registration->receive_global(context_id, geometry, reduce_result, n);
        TRACE_FN_EXIT();
      }

      inline pami_result_t analyze(size_t         context_id,
                                   BGQGeometry    *geometry,
                                   int phase = 0)
      {
        TRACE_FN_ENTER();
        TRACE_FORMAT("id %zu, registration %p, phase %d", context_id, geometry, phase);
	
        // Can only use shmem pgas if the geometry is all local tasks, so check the topology
        if (_pgas_shmem_registration && ((PAMI::Topology*)geometry->getTopology(PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX))->isLocal())
          _pgas_shmem_registration->analyze(_contextid, geometry, phase);

        // Can always use MU if it's available
        if (phase == 0 && _pgas_mu_registration) _pgas_mu_registration->analyze(_contextid, geometry, phase);

        // Can always use composite if it's available
        if (_pgas_composite_registration) _pgas_composite_registration->analyze(_contextid, geometry, phase);

        if(_ccmi_registration)// && (((PAMI::Topology*)geometry->getTopology(PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX))->size() != 1))
        {   
          geometry->resetUEBarrier(); // Reset so ccmi will select the UE barrier
          _ccmi_registration->analyze(context_id, geometry, phase);
        }

        if(_multi_registration)// && (((PAMI::Topology*)geometry->getTopology(PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX))->size() != 1))
            _multi_registration->analyze(context_id, geometry, phase);

        TRACE_FN_EXIT();
        return PAMI_SUCCESS;
      }

      inline pami_result_t dispatch_query_impl(size_t                dispatch,
                                               pami_configuration_t  configuration[],
                                               size_t                num_configs)
      {
        PAMI::Protocol::Send::Send * send =
          (PAMI::Protocol::Send::Send *) _dispatch[dispatch];
        return send->getAttributes (configuration, num_configs);
      }

      inline pami_result_t dispatch_update_impl(size_t                dispatch,
                                                pami_configuration_t  configuration[],
                                                size_t                num_configs)
      {
        return PAMI_INVAL;
      }
      inline pami_result_t query_impl(pami_configuration_t  configuration[],
                                      size_t                num_configs)
      {
        pami_result_t result = PAMI_SUCCESS;
        size_t i;

        for (i = 0; i < num_configs; i++)
          {
            switch (configuration[i].name)
              {
                case PAMI_CONTEXT_DISPATCH_ID_MAX:
                default:
                  result = PAMI_INVAL;
              }
          }

        return result;
      }

      inline pami_result_t update_impl(pami_configuration_t  configuration[],
                                       size_t                num_configs)
      {
        return PAMI_INVAL;
      }

      inline size_t getClientId ()
      {
	return _clientid;
      }

      inline pami_result_t registerAsync(
		PAMI::ProgressExtension::pamix_async_function progress,
		PAMI::ProgressExtension::pamix_async_function suspend,
		PAMI::ProgressExtension::pamix_async_function resume,
		void *cookie)
      {
        _async_suspend = suspend;
        _async_resume = resume;
	_async_cookie = cookie;
	return PAMI_SUCCESS;
      }

      inline pami_result_t getAsyncRegs(
		PAMI::ProgressExtension::pamix_async_function *progress,
		PAMI::ProgressExtension::pamix_async_function *suspend,
		PAMI::ProgressExtension::pamix_async_function *resume,
		void **cookie)
      {
        *progress = NULL;
	*suspend = _async_suspend;
        *resume = _async_resume;
	*cookie = _async_cookie;
	return PAMI_SUCCESS;
      }

    private:

      pami_client_t                _client;
      pami_context_t               _context;
      size_t                       _clientid;
      size_t                       _contextid;
      PAMI::ProgressExtension::pamix_async_function _async_suspend;
      PAMI::ProgressExtension::pamix_async_function _async_resume;
      void                        *_async_cookie;
      int                          _dispatch_id;
      std::map<unsigned, pami_geometry_t> *_geometry_map;
      pami_endpoint_t              _self;

      PAMI::Memory::GenMemoryManager  _mm;

      PAMI::Protocol::Send::Send  *_dispatch[1024];
      Protocol::Get::RGet         *_rget;
      Protocol::Put::RPut         *_rput;
      MemoryAllocator<1024,64,16> _request;
      ContextLock                  _lock;
      BGQRegistration             *_multi_registration;
      CCMIRegistration            *_ccmi_registration;
      BGQGeometry                 *_world_geometry;
      pami_result_t                _status;
      ShmemMcstModel              *_shmemMcastModel;
      ShmemMsyncModel             *_shmemMsyncModel;
      ShmemMcombModel             *_shmemMcombModel;
      AllSidedShmemNI             *_shmem_native_interface;
      uint8_t                      _ccmi_registration_storage[sizeof(CCMIRegistration)];
      uint8_t                      _multi_registration_storage[sizeof(BGQRegistration)];
      uint8_t                      _shmemMcastModel_storage[sizeof(ShmemMcstModel)];
      uint8_t                      _shmemMsyncModel_storage[sizeof(ShmemMsyncModel)];
      uint8_t                      _shmemMcombModel_storage[sizeof(ShmemMcombModel)];
      uint8_t                      _shmem_native_interface_storage[sizeof(AllSidedShmemNI)];
      ProtocolAllocator            _protocol;
      PlatformDeviceList          *_devices;
      MU_PGASCollreg              *_pgas_mu_registration;
      uint8_t                      _pgas_mu_registration_storage[sizeof(MU_PGASCollreg)];
      Shmem_PGASCollreg           *_pgas_shmem_registration;
      uint8_t                      _pgas_shmem_registration_storage[sizeof(Shmem_PGASCollreg)];
      Composite_PGASCollreg       *_pgas_composite_registration;
      uint8_t                      _pgas_composite_registration_storage[sizeof(Composite_PGASCollreg)];

      bool _dummy_disable;
      bool _dummy_disabled;
      PAMI::Device::Generic::GenericThread _dummy_work;
      PAMI::Protocol::Send::Error  _senderror;
      
      Protocol::Get::GetOverSend<Device::MU::PacketModel> _get;
      Protocol::Put::PutOverSend<Device::MU::PacketModel> _put;
      Protocol::Rmw::RmwOverSend<Device::MU::PacketModel> _rmw;
      //Protocol::Rmw::RmwOverSend<ShmemPacketModel> _rmw;
      
      
  }; // end PAMI::Context
}; // end namespace PAMI

#undef DO_TRACE_ENTEREXIT 
#undef DO_TRACE_DEBUG     

#endif // __components_context_bgq_bgqcontext_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
