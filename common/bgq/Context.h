///
/// \file common/bgq/Context.h
/// \brief PAMI Blue Gene\Q specific context implementation.
///
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
#ifdef ENABLE_NEW_SHMEM
#include "components/devices/shmemcoll/ShmemCollDevice.h"
#endif
#include "components/atomic/native/NativeCounter.h"
#include "components/atomic/bgq/L2Counter.h"
#include "components/atomic/counter/CounterMutex.h"
#include "components/atomic/indirect/IndirectCounter.h"
#include "components/atomic/indirect/IndirectMutex.h"

#include "components/memory/MemoryAllocator.h"

#include "components/memory/MemoryManager.h"
#include "Memregion.h"

#include "p2p/protocols/RGet.h"
#include "p2p/protocols/rget/GetRdma.h"
#include "p2p/protocols/rput/PutRdma.h"
#include "p2p/protocols/send/eager/Eager.h"
#include "p2p/protocols/send/composite/Composite.h"

#include "TypeDefs.h"
#include "algorithms/geometry/BGQMultiRegistration.h"

#include "algorithms/geometry/P2PCCMIRegistration.h"

#undef TRACE_ERR
#define TRACE_ERR(x) //fprintf x

#ifdef _COLLSHM
// Collective shmem device
#include "components/devices/cshmem/CollSharedMemoryManager.h"
#include "components/devices/cshmem/CollShmDevice.h"
#include "algorithms/geometry/CCMICSMultiRegistration.h"
#include "algorithms/geometry/BGQ2DRegistration.h"
#include "NativeAtomics.h"
#include "components/atomic/native/NativeMutex.h"
#endif


namespace PAMI
{
#ifdef _COLLSHM   // New Collective Shmem Protocol Typedefs

  typedef Atomic::NativeAtomic                CSAtomic;
  typedef Counter::Native                     CSCounter;	// need to optimize for Q
  typedef Mutex::Native                       CSMutex;	// need to optimize for Q

  typedef Memory::CollSharedMemoryManager < CSAtomic, CSMutex, CSCounter, COLLSHM_SEGSZ,
      COLLSHM_PAGESZ, COLLSHM_WINGROUPSZ, COLLSHM_BUFSZ >          CSMemoryManager;
  typedef Device::CollShm::CollShmDevice < CSAtomic, CSMemoryManager,
      COLLSHM_DEVICE_NUMSYNCS, COLLSHM_DEVICE_SYNCCOUNT >  CSDevice;
  typedef Device::CollShm::CollShmModel<CSDevice, CSMemoryManager>         CollShmModel;
  typedef Device::CSNativeInterface<CollShmModel>                          CSNativeInterface;
  ;
  typedef CollRegistration::CCMICSMultiRegistration < BGQGeometry,
  CSNativeInterface, CSMemoryManager, CollShmModel >            CollShmCollreg;

  typedef CollRegistration::BGQ2D::BGQ2DRegistration < BGQGeometry,
  PAMI::Device::Generic::Device,
  MUDevice,
  CSNativeInterface,
  MUGlobalNI,
  CollShmModel, CSMemoryManager >                               BGQ2DCollreg;


#endif


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
#ifdef ENABLE_NEW_SHMEM
          _shmemcoll(NULL),
#endif
          _progfunc(NULL),
          _atombarr(NULL),
#ifndef ENABLE_NEW_SHMEM
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
        TRACE_ERR((stderr, "device init: generic\n"));
        _generics = PAMI::Device::Generic::Device::Factory::generate(clientid, num_ctx, mm, NULL);

        if (__global.useshmem())
          {
            TRACE_ERR((stderr, "device init: shmem\n"));
            _shmem = ShmemDevice::Factory::generate(clientid, num_ctx, mm, _generics);
#ifdef ENABLE_NEW_SHMEM
            TRACE_ERR((stderr, "device init: shmem coll\n"));
            _shmemcoll = ShmemCollDevice::Factory::generate(clientid, num_ctx, mm, _generics);
#else
            TRACE_ERR((stderr, "device init: local allreduce wq\n"));
            _localallreduce = PAMI::Device::LocalAllreduceWQDevice::Factory::generate(clientid, num_ctx, mm, _generics);
            TRACE_ERR((stderr, "device init: local bcast wq\n"));
            _localbcast = PAMI::Device::LocalBcastWQDevice::Factory::generate(clientid, num_ctx, mm, _generics);
            TRACE_ERR((stderr, "device init: local reduce wq\n"));
            _localreduce = PAMI::Device::LocalReduceWQDevice::Factory::generate(clientid, num_ctx, mm, _generics);
#endif
          }

        TRACE_ERR((stderr, "device init: progress function\n"));
        _progfunc = PAMI::Device::ProgressFunctionDev::Factory::generate(clientid, num_ctx, mm, _generics);
        TRACE_ERR((stderr, "device init: atomic barrier\n"));
        _atombarr = PAMI::Device::AtomicBarrierDev::Factory::generate(clientid, num_ctx, mm, _generics);

        if (__global.useMU())
          {
            TRACE_ERR((stderr, "device init: MU\n"));
            _mu = Device::MU::Factory::generate(clientid, num_ctx, mm, _generics);
          }

        _atmmtx = PAMI::Device::AtomicMutexDev::Factory::generate(clientid, num_ctx, mm, _generics);
        PAMI_assertf(_atmmtx == _generics, "AtomicMutexDev must be a NillSubDevice");
        TRACE_ERR((stderr, "device init: done!\n"));
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
#ifdef ENABLE_NEW_SHMEM
            ShmemCollDevice::Factory::init(_shmemcoll, clientid, contextid, clt, ctx, mm, _generics);
#else
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
#ifdef ENABLE_NEW_SHMEM
            events += ShmemCollDevice::Factory::advance(_shmemcoll, clientid, contextid);
#else
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

        return events;
        events += Device::AtomicMutexDev::Factory::advance(_atmmtx, clientid, contextid);
      }

      PAMI::Device::Generic::Device *_generics; // need better name...
      ShmemDevice *_shmem; //compile-time always needs the devices since runtime is where the check is made to use them
#ifdef ENABLE_NEW_SHMEM
      ShmemCollDevice *_shmemcoll;
#endif
      PAMI::Device::ProgressFunctionDev *_progfunc;
      PAMI::Device::AtomicBarrierDev *_atombarr;
#ifndef ENABLE_NEW_SHMEM
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
#ifdef _COLLSHM
          _coll_shm_registration(NULL),
          _bgq2d_registration(NULL),
#endif
          _multi_registration((CollRegistration::BGQMultiRegistration < BGQGeometry, AllSidedShmemNI, MUDevice, MUGlobalNI, MUAxialNI, MUAxialDputNI, MUShmemAxialDputNI >*) _multi_registration_storage),
          _ccmi_registration((CCMIRegistration*)_ccmi_registration_storage),
          _world_geometry(world_geometry),
          _status(PAMI_SUCCESS),
          _shmemMcastModel(NULL),
          _shmemMsyncModel(NULL),
          _shmemMcombModel(NULL),
          _shmem_native_interface(NULL),
          _devices(devices),
          _pgas_mu_registration(NULL),
          _pgas_shmem_registration(NULL),
          _dummy_disable(false),
          _dummy_disabled(false)
      {
        char mmkey[PAMI::Memory::MMKEYSIZE];
        char *mms;
        mms = mmkey + sprintf(mmkey, "/pami-clt%zd-ctx%zd", clientid, id);

        TRACE_ERR((stderr,  "<%p>Context::Context() enter\n", this));
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


        Protocol::Get::GetRdma <Device::MU::DmaModelMemoryFifoCompletion, MUDevice> * rget_mu = NULL;
        Protocol::Put::PutRdma <Device::MU::DmaModelMemoryFifoCompletion, MUDevice> * rput_mu = NULL;

        Protocol::Get::GetRdma <Device::Shmem::DmaModel<ShmemDevice, false>, ShmemDevice> * rget_shmem = NULL;
        Protocol::Put::PutRdma <Device::Shmem::DmaModel<ShmemDevice, false>, ShmemDevice> * rput_shmem = NULL;

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

            _pgas_mu_registration = new(_pgas_mu_registration_storage) MU_PGASCollreg(_client, (pami_context_t)this, _clientid, _contextid, _protocol, Device::MU::Factory::getDevice(_devices->_mu, _clientid, _contextid), _devices->_shmem[_contextid], &_dispatch_id, _geometry_map);

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
                TRACE_ERR((stderr,  "<%p>Context::Context() construct shmem native interface and models\n", this));
                _shmemMcastModel         = (ShmemMcstModel*)_shmemMcastModel_storage;
                _shmemMcombModel         = (ShmemMcombModel*)_shmemMcombModel_storage;
                _shmemMsyncModel         = (ShmemMsyncModel*)_shmemMsyncModel_storage;

                // if _shmemMsyncModel_storage is in heap...
                // PAMI_assert(Barrier_Model::checkCtorMm(__global.heap_mm);
                new (_shmemMsyncModel_storage)        ShmemMsyncModel(PAMI::Device::AtomicBarrierDev::Factory::getDevice(_devices->_atombarr, _clientid, _contextid), _status);

#ifdef ENABLE_NEW_SHMEM
                //new (_shmemMcastModel_storage)        ShmemMcstModel(PAMI::Device::Generic::Device::Factory::getDevice(_devices->_generics,  _clientid, _contextid),_status);
                new (_shmemMcastModel_storage)        ShmemMcstModel(ShmemCollDevice::Factory::getDevice(_devices->_shmemcoll, _clientid, _contextid),_status);
                new (_shmemMcombModel_storage)        ShmemMcombModel(ShmemCollDevice::Factory::getDevice(_devices->_shmemcoll, _clientid, _contextid), _status);
#else
                new (_shmemMcastModel_storage)        ShmemMcstModel(PAMI::Device::LocalBcastWQDevice::Factory::getDevice(_devices->_localbcast, _clientid, _contextid), _status);
                new (_shmemMcombModel_storage)        ShmemMcombModel(PAMI::Device::LocalAllreduceWQDevice::Factory::getDevice(_devices->_localreduce, _clientid, _contextid), _status);
#endif

                _shmem_native_interface  = (AllSidedShmemNI*)_shmem_native_interface_storage;
                new (_shmem_native_interface_storage) AllSidedShmemNI(_shmemMcastModel, _shmemMsyncModel, _shmemMcombModel, client, (pami_context_t)this, id, clientid, &_dispatch_id);

                // Initialize shmem rget and shmem rput protocols.
                pami_result_t result = PAMI_ERROR;

                rget_shmem = Protocol::Get::GetRdma <Device::Shmem::DmaModel<ShmemDevice, false>, ShmemDevice>::
                             generate (_devices->_shmem[_contextid], _context, _request, result);

                if (result != PAMI_SUCCESS) rget_shmem = NULL;

                rput_shmem = Protocol::Put::PutRdma <Device::Shmem::DmaModel<ShmemDevice, false>, ShmemDevice>::
                             generate (_devices->_shmem[_contextid], _context, _request, result);

                if (result != PAMI_SUCCESS) rput_shmem = NULL;

                _pgas_shmem_registration = new(_pgas_shmem_registration_storage) Shmem_PGASCollreg(_client, (pami_context_t)this, _clientid, _contextid, _protocol, ShmemDevice::Factory::getDevice(_devices->_shmem, _clientid, _contextid), _devices->_shmem[_contextid],& _dispatch_id, _geometry_map);
              }
            else TRACE_ERR((stderr, "topology does not support shmem\n"));
          }

        TRACE_ERR((stderr,  "<%p>Context::Context() Register collectives(%p,%p,%p,%zu,%zu\n", this, _shmem_native_interface, client, this, id, clientid));
        // The multi registration will use shmem/mu if they are ctor'd above.

#ifdef _COLLSHM   // New Collective Shmem Registration
        // only enable collshm for context 0
        if (_contextid == 0)
          {

            PAMI::Topology *local_master_topo = (PAMI::Topology *) _world_geometry->getTopology(PAMI::Geometry::MASTER_TOPOLOGY_INDEX);
            uint64_t *invec;
            pami_result_t rc;
            rc = __global.heap_mm->memalign((void **) & invec, 0,
                                            local_master_topo->size() * sizeof(uint64_t));
            PAMI_assertf(rc == PAMI_SUCCESS, "out of memory for invec");

            for (size_t i = 0; i < local_master_topo->size(); ++i)  invec[i] = 0ULL;

            if (__global.useshmem())
              {
                _coll_shm_registration = new((CollShmCollreg *) _coll_shm_registration_storage)
                CollShmCollreg(_client, _clientid, _context, _contextid, PAMI::Device::Generic::Device::Factory::getDevice(_devices->_generics, _clientid, _contextid));
              }
            else  _coll_shm_registration = NULL;

            _bgq2d_registration = new((BGQ2DCollreg *) _bgq2d_registration_storage)
            BGQ2DCollreg(_client, _context, _contextid, _clientid,
                         PAMI::Device::Generic::Device::Factory::getDevice(_devices->_generics, _clientid, _contextid),
                         PAMI::Device::MU::Factory::getDevice(_devices->_mu, _clientid, _contextid),
                         __global.mapping,
                         &_dispatch_id);

            if (((PAMI::Topology*)_world_geometry->getTopology(PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX))->isLocal() && _coll_shm_registration)
              _coll_shm_registration->analyze_global(_contextid, _world_geometry, invec);
            else _bgq2d_registration->analyze_global(_contextid, _world_geometry, invec);

            free(invec);
          }
        else
          {
            _coll_shm_registration = NULL;
            _bgq2d_registration = NULL;
          }

#endif

        _multi_registration       =  new (_multi_registration)
        CollRegistration::BGQMultiRegistration < BGQGeometry, AllSidedShmemNI, MUDevice, MUGlobalNI, MUAxialNI, MUAxialDputNI, MUShmemAxialDputNI >(_shmem_native_interface,
            PAMI::Device::MU::Factory::getDevice(_devices->_mu, _clientid, _contextid),
            client,
            (pami_context_t)this,
            id,
            clientid,
            &_dispatch_id,
            _geometry_map);

        _ccmi_registration =  new(_ccmi_registration) CCMIRegistration(_client, _context, _contextid, _clientid, _devices->_shmem[_contextid], _devices->_mu[_contextid], _protocol, __global.useshmem(), __global.useMU(), __global.topology_global.size(), __global.topology_local.size(), &_dispatch_id, _geometry_map);

        // Can only use shmem pgas if the geometry is all local tasks, so check the topology
        if (_pgas_shmem_registration && ((PAMI::Topology*)_world_geometry->getTopology(PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX))->isLocal()) _pgas_shmem_registration->analyze(_contextid, _world_geometry, 0);

        // Can always use MU if it's available
        if (_pgas_mu_registration) _pgas_mu_registration->analyze(_contextid, _world_geometry, 0);

        _world_geometry->resetUEBarrier(); // Reset so ccmi will select the UE barrier
        _ccmi_registration->analyze(_contextid, _world_geometry, 0);

        _multi_registration->analyze(_contextid, _world_geometry, 0);

        // for now, this is the only registration that has a phase 1...
        // We know that _world_geometry is always "optimized" at create time.
        _multi_registration->analyze(_contextid, _world_geometry, 1);

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

        // dispatch_impl relies on the table being initialized to NULL's.
        memset(_dispatch, 0x00, sizeof(_dispatch));

        TRACE_ERR((stderr,  "<%p>Context:: exit\n", this));
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
        work->setStatus(PAMI::Device::OneShot);
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
        size_t id = (size_t)(parameters->send.dispatch);
        TRACE_ERR((stderr, "Context::send_impl('simple'), _dispatch[%zu] = %p\n", id, _dispatch[id]));
        PAMI_assert_debug (_dispatch[id] != NULL);

        PAMI::Protocol::Send::Send * send =
          (PAMI::Protocol::Send::Send *) _dispatch[id];
        pami_result_t rc = send->simple (parameters);

        TRACE_ERR((stderr, "Context::send_impl('simple') rc = %d\n", rc));
        return rc;
      }

      inline pami_result_t send_impl (pami_send_immediate_t * parameters)
      {
        size_t id = (size_t)(parameters->dispatch);
        TRACE_ERR((stderr, "Context::send_impl('immediate'), _dispatch[%zu] = %p\n", id, _dispatch[id]));
        PAMI_assert_debug (_dispatch[id] != NULL);

        PAMI::Protocol::Send::Send * send =
          (PAMI::Protocol::Send::Send *) _dispatch[id];
        pami_result_t rc = send->immediate (parameters);

        TRACE_ERR((stderr, "Context::send_impl('immediate') rc = %d\n", rc));
        return rc;
      }

      inline pami_result_t send_impl (pami_send_typed_t * parameters)
      {
        return PAMI_UNIMPL;
      }

      inline pami_result_t put_impl (pami_put_simple_t * parameters)
      {
        return PAMI_UNIMPL;
      }

      inline pami_result_t put_typed (pami_put_typed_t * parameters)
      {
        return PAMI_UNIMPL;
      }

      inline pami_result_t get_impl (pami_get_simple_t * parameters)
      {

        return PAMI_UNIMPL;
      }

      inline pami_result_t get_typed (pami_get_typed_t * parameters)
      {
        return PAMI_UNIMPL;
      }

      inline pami_result_t rmw (pami_rmw_t * parameters)
      {
        return PAMI_UNIMPL;
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
        TRACE_ERR((stderr, ">> rput_impl('simple')\n"));

        pami_result_t rc = _rput->simple (parameters);

        TRACE_ERR((stderr, "<< rput_impl('simple') rc = %d\n", rc));
        return rc;
      }

      inline pami_result_t rput_typed (pami_rput_typed_t * parameters)
      {
        return PAMI_UNIMPL;
      }

      inline pami_result_t rget_impl (pami_rget_simple_t * parameters)
      {
        TRACE_ERR((stderr, ">> rget_impl('simple')\n"));

        pami_result_t rc = _rget->simple (parameters);

        TRACE_ERR((stderr, "<< rget_impl('simple') rc = %d\n", rc));
        return rc;
      }

      inline pami_result_t rget_typed (pami_rget_typed_t * parameters)
      {
        return PAMI_UNIMPL;
      }

      inline pami_result_t purge_totask (pami_endpoint_t *dest, size_t count)
      {
        return PAMI_UNIMPL;
      }

      inline pami_result_t resume_totask (pami_endpoint_t *dest, size_t count)
      {
        return PAMI_UNIMPL;
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

      inline pami_result_t geometry_algorithms_num_impl (pami_geometry_t geometry,
                                                         pami_xfer_type_t colltype,
                                                         size_t *lists_lengths)
      {
        BGQGeometry *_geometry = (BGQGeometry*) geometry;
        return _geometry->algorithms_num(colltype, lists_lengths, _contextid);
      }

      inline pami_result_t geometry_algorithms_info_impl (pami_geometry_t geometry,
                                                          pami_xfer_type_t colltype,
                                                          pami_algorithm_t  *algs0,
                                                          pami_metadata_t   *mdata0,
                                                          size_t               num0,
                                                          pami_algorithm_t  *algs1,
                                                          pami_metadata_t   *mdata1,
                                                          size_t               num1)
      {
        BGQGeometry *_geometry = (BGQGeometry*) geometry;
        return _geometry->algorithms_info(colltype,
                                          algs0,
                                          mdata0,
                                          num0,
                                          algs1,
                                          mdata1,
                                          num1,
                                          _contextid);
      }

      inline pami_result_t collective_impl (pami_xfer_t * parameters)
      {
        Geometry::Algorithm<BGQGeometry> *algo = (Geometry::Algorithm<BGQGeometry> *)parameters->algorithm;
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

      inline pami_result_t dispatch_impl (size_t                     id,
                                          pami_dispatch_callback_function fn,
                                          void                      * cookie,
                                          pami_send_hint_t            options)
      {
        pami_result_t result = PAMI_ERROR;
        TRACE_ERR((stderr, "Context::dispatch_impl .. _dispatch[%zu] = %p, options = %#X\n", id, _dispatch[id], *(unsigned*)&options));

        pami_endpoint_t self = PAMI_ENDPOINT_INIT(_clientid, __global.mapping.task(), _contextid);

        if (_dispatch[id] == NULL)
          {
            // either runtime options OR user-specified device only so we have to look at both
            bool no_shmem  = (options.use_shmem == PAMI_HINT3_FORCE_OFF) || (!__global.useshmem() && __global.useMU());
            bool use_shmem = (options.use_shmem == PAMI_HINT3_FORCE_ON ) || (!__global.useMU() && __global.useshmem());

            TRACE_ERR((stderr, "global.useshmem: %d, global.useMU: %d\n",
                       (int)__global.useshmem(), (int)__global.useMU()));
            TRACE_ERR((stderr, "options.use_shmem: %d, no_shmem: %d, use_shmem: %d\n",
                       (int)options.use_shmem, (int)no_shmem, (int)use_shmem));

            if (no_shmem == 1)
              {
                if (__global.useMU())
                  {
                    TRACE_ERR((stderr, "Only registering MU pt2pt protocols\n"));

                    if (options.no_long_header == 1)
                      {
                        _dispatch[id] = (Protocol::Send::Send *)
                                        Protocol::Send::Eager <Device::MU::PacketModel, MUDevice, false>::
                                        generate (id, fn.p2p, cookie, _devices->_mu[_contextid], self, _context, _protocol, result);
                      }
                    else
                      {
                        _dispatch[id] = (Protocol::Send::Send *)
                                        Protocol::Send::Eager <Device::MU::PacketModel, MUDevice, true>::
                                        generate (id, fn.p2p, cookie, _devices->_mu[_contextid], self, _context, _protocol, result);
                      }
                  }
                else
                  {
                    PAMI_abortf("No non-shmem protocols available.");
                  }
              }
            else if (use_shmem == 1)
              {
                // Register only the "shmem" eager send protocol
                if (__global.useshmem())
                  {
                    TRACE_ERR((stderr, "Only register shmem pt2pt protocols\n"));

                    if (options.no_long_header == 1)
                      {
                        _dispatch[id] = (Protocol::Send::Send *)
                                        Protocol::Send::Eager <ShmemPacketModel, ShmemDevice, false>::
                                        generate (id, fn.p2p, cookie, _devices->_shmem[_contextid], self, _context, _protocol, result);
                      }
                    else
                      {
                        _dispatch[id] = (Protocol::Send::Send *)
                                        Protocol::Send::Eager <ShmemPacketModel, ShmemDevice, true>::
                                        generate (id, fn.p2p, cookie, _devices->_shmem[_contextid], self, _context, _protocol, result);
                      }
                  }
                else
                  {
                    PAMI_abortf("No shmem protocols available.");
                  }
              }
            else
              {
                // Register both the "mu" and "shmem" eager send protocols
                if (__global.useshmem() && __global.useMU())
                  {
                    if (options.no_long_header == 1)
                      {
                        Protocol::Send::Eager <Device::MU::PacketModel, MUDevice, false> * eagermu =
                          Protocol::Send::Eager <Device::MU::PacketModel, MUDevice, false>::
                          generate (id, fn.p2p, cookie, _devices->_mu[_contextid], self, _context, _protocol, result);

                        Protocol::Send::Eager <ShmemPacketModel, ShmemDevice, false> * eagershmem =
                          Protocol::Send::Eager <ShmemPacketModel, ShmemDevice, false>::
                          generate (id, fn.p2p, cookie, _devices->_shmem[_contextid], self, _context, _protocol, result);

                        _dispatch[id] = (Protocol::Send::Send *) Protocol::Send::Factory::
                                        generate (eagershmem, eagermu, _protocol, result);
                      }
                    else
                      {
                        Protocol::Send::Eager <Device::MU::PacketModel, MUDevice, true> * eagermu =
                          Protocol::Send::Eager <Device::MU::PacketModel, MUDevice, true>::
                          generate (id, fn.p2p, cookie, _devices->_mu[_contextid], self, _context, _protocol, result);

                        Protocol::Send::Eager <ShmemPacketModel, ShmemDevice, true> * eagershmem =
                          Protocol::Send::Eager <ShmemPacketModel, ShmemDevice, true>::
                          generate (id, fn.p2p, cookie, _devices->_shmem[_contextid], self, _context, _protocol, result);

                        _dispatch[id] = (Protocol::Send::Send *) Protocol::Send::Factory::
                                        generate (eagershmem, eagermu, _protocol, result);
                      }
                  }
              }
          } // end dispatch[id]==null

        TRACE_ERR((stderr, "Context::Context::dispatch_impl .. result = %d\n", result));
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
#if 1
        bool affinity = (coreAffinity() == Kernel_ProcessorCoreID());
        TRACE_ERR((stderr, "acquire=%d, affinity=%d, coreAffinity=%u, Kernel_ProcessorCoreID=%u, ContextID=%zu\n", acquire, affinity, coreAffinity(), Kernel_ProcessorCoreID(), _contextid));
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
      }

      inline pami_result_t analyze_local (size_t         context_id,
                                          BGQGeometry    *geometry,
                                          uint64_t       *reduce_result)
      {
        TRACE_ERR((stderr, "Context::analyze_local context id %zu, geometry %p\n", context_id, geometry));
#ifdef _COLLSHM

        if (_coll_shm_registration && ((PAMI::Topology*)geometry->getTopology(PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX))->isLocal())
          _coll_shm_registration->analyze_local(context_id, geometry, reduce_result);
        else if (_bgq2d_registration) _bgq2d_registration->analyze_global(context_id, geometry, reduce_result);

#endif // _COLLSHM
        return PAMI_SUCCESS;
      }

      inline pami_result_t analyze_global(size_t         context_id,
                                          BGQGeometry    *geometry,
                                          uint64_t       *reduce_result)
      {
        TRACE_ERR((stderr, "Context::analyze_global context id %zu, geometry %p\n", context_id, geometry));
#ifdef _COLLSHM

        if (_coll_shm_registration && ((PAMI::Topology*)geometry->getTopology(PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX))->isLocal())
          _coll_shm_registration->analyze_global(context_id, geometry, reduce_result);
        else if (_bgq2d_registration) _bgq2d_registration->analyze_global(context_id, geometry, reduce_result);

#endif // _COLLSHM
        return PAMI_SUCCESS;
      }

      inline pami_result_t analyze(size_t         context_id,
                                   BGQGeometry    *geometry,
                                   int phase = 0)
      {
        TRACE_ERR((stderr, "Context::analyze context id %zu, registration %p, phase %d\n", context_id, geometry, phase));

// Only analyzed in analyze_local and/or analyze_global
//#ifdef _COLLSHM
//        if(_coll_shm_registration && ((PAMI::Topology*)geometry->getTopology(PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX))->isLocal())
//            _coll_shm_registration->analyze(context_id, geometry, phase);
//#endif
        // Can only use shmem pgas if the geometry is all local tasks, so check the topology
        if (_pgas_shmem_registration && ((PAMI::Topology*)geometry->getTopology(PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX))->isLocal())
          _pgas_shmem_registration->analyze(_contextid, geometry, phase);

        // Can always use MU if it's available
        if (phase == 0 && _pgas_mu_registration) _pgas_mu_registration->analyze(_contextid, geometry, phase);

        geometry->resetUEBarrier(); // Reset so ccmi will select the UE barrier
        _ccmi_registration->analyze(context_id, geometry, phase);

        _multi_registration->analyze(context_id, geometry, phase);

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
    private:

      pami_client_t                _client;
      pami_context_t               _context;
      size_t                       _clientid;
      size_t                       _contextid;
      int                          _dispatch_id;
      std::map<unsigned, pami_geometry_t> *_geometry_map;
      pami_endpoint_t              _self;

      PAMI::Memory::GenMemoryManager  _mm;

      void *                       _dispatch[1024];
      Protocol::Get::RGet         *_rget;
      Protocol::Put::RPut         *_rput;
      MemoryAllocator<1024,64,16> _request;
      ContextLock                  _lock;
#ifdef _COLLSHM
      CollShmCollreg              *_coll_shm_registration;
      BGQ2DCollreg                *_bgq2d_registration;
#endif
      CollRegistration::BGQMultiRegistration < BGQGeometry, AllSidedShmemNI, MUDevice, MUGlobalNI, MUAxialNI, MUAxialDputNI, MUShmemAxialDputNI >    *_multi_registration;
      CCMIRegistration            *_ccmi_registration;
      BGQGeometry                 *_world_geometry;
      pami_result_t                _status;
      ShmemMcstModel              *_shmemMcastModel;
      ShmemMsyncModel             *_shmemMsyncModel;
      ShmemMcombModel             *_shmemMcombModel;
      AllSidedShmemNI             *_shmem_native_interface;
      uint8_t                      _ccmi_registration_storage[sizeof(CCMIRegistration)];
#ifdef _COLLSHM
      uint8_t                      _coll_shm_registration_storage[sizeof(CollShmCollreg)];
      uint8_t                      _bgq2d_registration_storage[sizeof(BGQ2DCollreg)];
#endif
      uint8_t                      _multi_registration_storage[sizeof(CollRegistration::BGQMultiRegistration < BGQGeometry, AllSidedShmemNI, MUDevice, MUGlobalNI, MUAxialNI, MUAxialDputNI, MUShmemAxialDputNI >)];
      uint8_t                      _shmemMcastModel_storage[sizeof(ShmemMcstModel)];
      //uint8_t                      _shmemMcastModel_storage[sizeof(ShmemMcstModel)];
      uint8_t                      _shmemMsyncModel_storage[sizeof(ShmemMsyncModel)];
      uint8_t                      _shmemMcombModel_storage[sizeof(ShmemMcombModel)];
      uint8_t                      _shmem_native_interface_storage[sizeof(AllSidedShmemNI)];
      ProtocolAllocator            _protocol;
      PlatformDeviceList          *_devices;
      MU_PGASCollreg              *_pgas_mu_registration;
      uint8_t                      _pgas_mu_registration_storage[sizeof(MU_PGASCollreg)];
      Shmem_PGASCollreg           *_pgas_shmem_registration;
      uint8_t                      _pgas_shmem_registration_storage[sizeof(Shmem_PGASCollreg)];

      bool _dummy_disable;
      bool _dummy_disabled;
      PAMI::Device::Generic::GenericThread _dummy_work;
  }; // end PAMI::Context
}; // end namespace PAMI

#endif // __components_context_bgq_bgqcontext_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
