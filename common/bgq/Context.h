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
//#include "components/devices/workqueue/WQRingReduceMsg.h"
//#include "components/devices/workqueue/WQRingBcastMsg.h"

#include "components/atomic/gcc/GccBuiltin.h"
#include "components/atomic/bgq/L2Counter.h"
//#include "components/atomic/pthread/Pthread.h"

#include "components/memory/MemoryAllocator.h"

#include "SysDep.h"
#include "Memregion.h"

#include "p2p/protocols/RGet.h"
#include "p2p/protocols/rget/GetRdma.h"
#include "p2p/protocols/rput/PutRdma.h"
#include "p2p/protocols/send/eager/Eager.h"
#include "p2p/protocols/send/composite/Composite.h"

#include "TypeDefs.h"
#include "algorithms/geometry/BGQMultiRegistration.h"

#ifdef TRACE_ERR
#undef TRACE_ERR
#endif
#define TRACE_ERR(x) //fprintf x


namespace PAMI
{

  typedef Mutex::CounterMutex<Counter::BGQ::L2NodeCounter>  ContextLock;

  typedef MemoryAllocator<2048, 16> ProtocolAllocator;

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
//    _wqringreduce(NULL),
//    _wqringbcast(NULL),
      _localallreduce(NULL),
      _localbcast(NULL),
      _localreduce(NULL),
      _mu(NULL)
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
        if(__global.useshmem())
        {
          TRACE_ERR((stderr,"device init: shmem\n"));
          _shmem = ShmemDevice::Factory::generate(clientid, num_ctx, mm, _generics);
          TRACE_ERR((stderr,"device init: local allreduce wq\n"));
          _localallreduce = PAMI::Device::LocalAllreduceWQDevice::Factory::generate(clientid, num_ctx, mm, _generics);
          TRACE_ERR((stderr,"device init: local bcast wq\n"));
          _localbcast = PAMI::Device::LocalBcastWQDevice::Factory::generate(clientid, num_ctx, mm, _generics);
          TRACE_ERR((stderr,"device init: local reduce wq\n"));
          _localreduce = PAMI::Device::LocalReduceWQDevice::Factory::generate(clientid, num_ctx, mm, _generics);
        }
        TRACE_ERR((stderr,"device init: progress function\n"));
        _progfunc = PAMI::Device::ProgressFunctionDev::Factory::generate(clientid, num_ctx, mm, _generics);
        TRACE_ERR((stderr,"device init: atomic barrier\n"));
        _atombarr = PAMI::Device::AtomicBarrierDev::Factory::generate(clientid, num_ctx, mm, _generics);
//      TRACE_ERR((stderr,"device init: wqring reduce\n"));
//      _wqringreduce = PAMI::Device::WQRingReduceDev::Factory::generate(clientid, num_ctx, mm, _generics);
//      TRACE_ERR((stderr,"device init: wqring bcast\n"));
//      _wqringbcast = PAMI::Device::WQRingBcastDev::Factory::generate(clientid, num_ctx, mm, _generics);
        if(__global.useMU())
        {
         TRACE_ERR((stderr,"device init: MU\n"));
        _mu = MUDevice::Factory::generate(clientid, num_ctx, mm, _generics);
        }
         TRACE_ERR((stderr,"device init: done!\n"));
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
       * \param[in] sd           SysDep object
       * \param[in] clientid     Client ID (index)
       * \param[in] num_ctx      Number of contexts in this client
       * \param[in] ctx          Context opaque entity
       * \param[in] contextid    Context ID (index)
       */
      inline pami_result_t init(size_t clientid, size_t contextid, pami_client_t clt, pami_context_t ctx, PAMI::Memory::MemoryManager *mm)
      {
        PAMI::Device::Generic::Device::Factory::init(_generics, clientid, contextid, clt, ctx, mm, _generics);
         if(__global.useshmem())
         {
           ShmemDevice::Factory::init(_shmem, clientid, contextid, clt, ctx, mm, _generics);
           PAMI::Device::LocalAllreduceWQDevice::Factory::init(_localallreduce, clientid, contextid, clt, ctx, mm, _generics);
           PAMI::Device::LocalBcastWQDevice::Factory::init(_localbcast, clientid, contextid, clt, ctx, mm, _generics);
           PAMI::Device::LocalReduceWQDevice::Factory::init(_localreduce, clientid, contextid, clt, ctx, mm, _generics);
         }
        PAMI::Device::ProgressFunctionDev::Factory::init(_progfunc, clientid, contextid, clt, ctx, mm , _generics);
        PAMI::Device::AtomicBarrierDev::Factory::init(_atombarr, clientid, contextid, clt, ctx, mm, _generics);
//      PAMI::Device::WQRingReduceDev::Factory::init(_wqringreduce, clientid, contextid, clt, ctx, mm , _generics);
//      PAMI::Device::WQRingBcastDev::Factory::init(_wqringbcast, clientid, contextid, clt, ctx, mm, _generics);
        if(__global.useMU())
        {
         MUDevice::Factory::init(_mu, clientid, contextid, clt, ctx, mm, _generics);
        }
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
        events += PAMI::Device::Generic::Device::Factory::advance(_generics, clientid, contextid);
        if(__global.useshmem())
        {
            events += ShmemDevice::Factory::advance(_shmem, clientid, contextid);
            events += PAMI::Device::LocalAllreduceWQDevice::Factory::advance(_localallreduce, clientid, contextid);
            events += PAMI::Device::LocalBcastWQDevice::Factory::advance(_localbcast, clientid, contextid);
            events += PAMI::Device::LocalReduceWQDevice::Factory::advance(_localreduce, clientid, contextid);
        }
        events += PAMI::Device::ProgressFunctionDev::Factory::advance(_progfunc, clientid, contextid);
        events += PAMI::Device::AtomicBarrierDev::Factory::advance(_atombarr, clientid, contextid);
//      events += PAMI::Device::WQRingReduceDev::Factory::advance(_wqringreduce, clientid, contextid);
//      events += PAMI::Device::WQRingBcastDev::Factory::advance(_wqringbcast, clientid, contextid);
        if(__global.useMU())
           events += MUDevice::Factory::advance(_mu, clientid, contextid);
        return events;
      }

      PAMI::Device::Generic::Device *_generics; // need better name...
      ShmemDevice *_shmem; //compile-time always needs the devices since runtime is where the check is made to use them
      PAMI::Device::ProgressFunctionDev *_progfunc;
      PAMI::Device::AtomicBarrierDev *_atombarr;
//    PAMI::Device::WQRingReduceDev *_wqringreduce;
//    PAMI::Device::WQRingBcastDev *_wqringbcast;;
      PAMI::Device::LocalAllreduceWQDevice *_localallreduce;
      PAMI::Device::LocalBcastWQDevice *_localbcast;
      PAMI::Device::LocalReduceWQDevice *_localreduce;
      MUDevice *_mu;
  }; // class PlatformDeviceList

  class Context : public Interface::Context<PAMI::Context>
  {
    public:
      inline Context (pami_client_t client, size_t clientid, size_t id, size_t num,
                      PlatformDeviceList *devices,
                      void * addr, size_t bytes,
                      BGQGeometry *world_geometry) :
          Interface::Context<PAMI::Context> (client, id),
          _client (client),
          _context ((pami_context_t)this),
          _clientid (clientid),
          _contextid (id),
          _mm (addr, bytes),
          _sysdep (_mm),
          _lock(),
          _multi_registration((CollRegistration::BGQMultiRegistration < BGQGeometry, AllSidedShmemNI, MUGlobalNI >*) _multi_registration_storage),
          _world_geometry(world_geometry),
          _status(PAMI_SUCCESS),
          _shmemMcastModel(NULL),
          _shmemMsyncModel(NULL),
          _shmemMcombModel(NULL),
          _shmem_native_interface(NULL),
          _devices(devices),
          _global_mu_ni(NULL)
      {
        TRACE_ERR((stderr,  "<%p>Context::Context() enter\n",this));
        // ----------------------------------------------------------------
        // Compile-time assertions
        // ----------------------------------------------------------------

        // Make sure the memory allocator is large enough for all
        // protocol classes.
        COMPILE_TIME_ASSERT(sizeof(MUGlobalNI) <= ProtocolAllocator::objsize);
        // ----------------------------------------------------------------
        // Compile-time assertions
        // ----------------------------------------------------------------

        _lock.init(&_mm);
        _devices->init(_clientid, _contextid, _client, _context, &_mm);


        Protocol::Get::GetRdma <Device::MU::MUDmaModel<false>, MUDevice> * rget_mu = NULL;
        Protocol::Put::PutRdma <Device::MU::MUDmaModel<false>, MUDevice> * rput_mu = NULL;

        Protocol::Get::GetRdma <Device::Shmem::DmaModel<ShmemDevice,false>, ShmemDevice> * rget_shmem = NULL;
        Protocol::Put::PutRdma <Device::Shmem::DmaModel<ShmemDevice,false>, ShmemDevice> * rput_shmem = NULL;

        if(__global.useMU())
        {
          TRACE_ERR((stderr,  "<%p>Context::Context() construct mu native interface\n",this));
          // Can't construct NI until device is init()'d.  Ctor into member storage.
          _global_mu_ni = new (_global_mu_ni_storage) MUGlobalNI(MUDevice::Factory::getDevice(_devices->_mu, _clientid, _contextid), _client, _context, _contextid, _clientid);

          // Initialize mu rget and mu rput protocols.
          pami_result_t result = PAMI_ERROR;

          rget_mu = Protocol::Get::GetRdma <Device::MU::MUDmaModel<false>, MUDevice>::
            generate (_devices->_mu[_contextid], _request, result);
          if (result != PAMI_SUCCESS) rget_mu = NULL;

          rput_mu = Protocol::Put::PutRdma <Device::MU::MUDmaModel<false>, MUDevice>::
            generate (_devices->_mu[_contextid], _request, result);
          if (result != PAMI_SUCCESS) rput_mu = NULL;
        }

#if 0
        // ----------------------------------------------------------------
        // Initialize the memory region get protocol(s)
        // ----------------------------------------------------------------
       {
         pami_result_t result = PAMI_ERROR;
         if(__global.useMU() && __global.useshmem())
         {
        Protocol::Get::GetRdma <Device::MU::MUDmaModel<false>, MUDevice> * getmu =
          Protocol::Get::GetRdma <Device::MU::MUDmaModel<false>, MUDevice>::
            generate (_devices->_mu[_contextid], _request, result);
//        if (result != PAMI_SUCCESS)
//          getmu = Protocol::Get::NoRGet::generate (_context, _request);

        Protocol::Get::GetRdma <Device::Shmem::DmaModel<ShmemDevice,false>, ShmemDevice> * getshmem =
          Protocol::Get::GetRdma <Device::Shmem::DmaModel<ShmemDevice,false>, ShmemDevice>::
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

        Protocol::Put::PutRdma <Device::Shmem::DmaModel<ShmemDevice,false>, ShmemDevice> * putshmem =
          Protocol::Put::PutRdma <Device::Shmem::DmaModel<ShmemDevice,false>, ShmemDevice>::
            generate (_devices->_shmem[_contextid], _request, result);

        _rput = (Protocol::Put::RPut *) Protocol::Put::Factory::
                      generate (putshmem, putmu, _request, result);

         }
         else if (__global.useshmem())
         {
        _rget = Protocol::Get::GetRdma <Device::Shmem::DmaModel<ShmemDevice,false>, ShmemDevice>::
          generate (_devices->_shmem[_contextid], _request, result);
//        if (result != PAMI_SUCCESS)
//          _rget = Protocol::Get::NoGet::generate (_context, _request);

        _rput = Protocol::Put::PutRdma <Device::Shmem::DmaModel<ShmemDevice,false>, ShmemDevice>::
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
        if(__global.useshmem())
        {
          // Can't construct these models on single process nodes (no shmem)
          if (__global.topology_local.size() > 1)
          {
            TRACE_ERR((stderr,  "<%p>Context::Context() construct shmem native interface and models\n",this));
            _shmemMcastModel         = (Device::LocalBcastWQModel*)_shmemMcastModel_storage;
            _shmemMcombModel         = (Device::LocalAllreduceWQModel*)_shmemMcombModel_storage;
            _shmemMsyncModel         = (Barrier_Model*)_shmemMsyncModel_storage;

            new (_shmemMsyncModel_storage)        Barrier_Model(PAMI::Device::AtomicBarrierDev::Factory::getDevice(_devices->_atombarr, _clientid, _contextid),_status);
            new (_shmemMcastModel_storage)        Device::LocalBcastWQModel(PAMI::Device::LocalBcastWQDevice::Factory::getDevice(_devices->_localbcast, _clientid, _contextid),_status);
            new (_shmemMcombModel_storage)        Device::LocalAllreduceWQModel(PAMI::Device::LocalAllreduceWQDevice::Factory::getDevice(_devices->_localreduce, _clientid, _contextid),_status);

            _shmem_native_interface  = (AllSidedShmemNI*)_shmem_native_interface_storage;
            new (_shmem_native_interface_storage) AllSidedShmemNI(_shmemMcastModel, _shmemMsyncModel, _shmemMcombModel, client, (pami_context_t)this, id, clientid);

            // Initialize shmem rget and shmem rput protocols.
            pami_result_t result = PAMI_ERROR;

            rget_shmem = Protocol::Get::GetRdma <Device::Shmem::DmaModel<ShmemDevice,false>, ShmemDevice>::
              generate (_devices->_shmem[_contextid], _request, result);
            if (result != PAMI_SUCCESS) rget_shmem = NULL;

            rput_shmem = Protocol::Put::PutRdma <Device::Shmem::DmaModel<ShmemDevice,false>, ShmemDevice>::
              generate (_devices->_shmem[_contextid], _request, result);
            if (result != PAMI_SUCCESS) rput_shmem = NULL;
          }
          else TRACE_ERR((stderr, "topology does not support shmem\n"));
        }

        TRACE_ERR((stderr,  "<%p>Context::Context() Register collectives(%p,%p,%p,%p,%zu,%zu\n",this, _shmem_native_interface, _global_mu_ni, client, this, id, clientid));
        // The multi registration will use shmem/mu if they are ctor'd above.
        _multi_registration       =  new (_multi_registration) 
        CollRegistration::BGQMultiRegistration < BGQGeometry, AllSidedShmemNI, MUGlobalNI >(_shmem_native_interface, _global_mu_ni, client, (pami_context_t)this, id, clientid);

        _multi_registration->analyze(_contextid, _world_geometry);

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

        TRACE_ERR((stderr,  "<%p>Context:: exit\n",this));
      }
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

        TRACE_ERR((stderr, "Context::send_impl('simple') rc = %d\n",rc));
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

        TRACE_ERR((stderr, "Context::send_impl('immediate') rc = %d\n",rc));
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

        TRACE_ERR((stderr, "<< rput_impl('simple') rc = %d\n",rc));
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

        TRACE_ERR((stderr, "<< rget_impl('simple') rc = %d\n",rc));
        return rc;
      }

      inline pami_result_t rget_typed (pami_rget_typed_t * parameters)
      {
        return PAMI_UNIMPL;
      }

      inline pami_result_t purge_totask (size_t *dest, size_t count)
      {
        return PAMI_UNIMPL;
      }

      inline pami_result_t resume_totask (size_t *dest, size_t count)
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

      inline pami_result_t fence_task (pami_event_function   done_fn,
                                      void               * cookie,
                                      size_t               task)
      {
        return PAMI_UNIMPL;
      }

      inline pami_result_t geometry_algorithms_num_impl (pami_geometry_t geometry,
                                                        pami_xfer_type_t colltype,
                                                        int *lists_lengths)
      {
        BGQGeometry *_geometry = (BGQGeometry*) geometry;
        return _geometry->algorithms_num(colltype, lists_lengths, _contextid);
      }

      inline pami_result_t geometry_algorithms_info_impl (pami_geometry_t geometry,
                                                         pami_xfer_type_t colltype,
                                                         pami_algorithm_t  *algs0,
                                                         pami_metadata_t   *mdata0,
                                                         int               num0,
                                                         pami_algorithm_t  *algs1,
                                                         pami_metadata_t   *mdata1,
                                                         int               num1)
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
                                                      pami_dispatch_callback_fn   fn,
                                                      void                     * cookie,
                                                      pami_collective_hint_t      options)
      {
        Geometry::Algorithm<BGQGeometry> *algo = (Geometry::Algorithm<BGQGeometry> *)algorithm;
        return algo->dispatch_set(dispatch, fn, cookie, options);
      }

      inline pami_result_t dispatch_impl (size_t                     id,
                                         pami_dispatch_callback_fn   fn,
                                         void                     * cookie,
                                         pami_send_hint_t            options)
      {
        pami_result_t result = PAMI_ERROR;
        TRACE_ERR((stderr, "Context::dispatch_impl .. _dispatch[%zu] = %p, options = %#X\n", id, _dispatch[id], *(unsigned*)&options));

         if (_dispatch[id] == NULL)
         {
            // either runtime options OR user-specified device only so we have to look at both
            bool no_shmem = options.no_shmem || (!__global.useshmem() && __global.useMU());
            bool use_shmem = options.use_shmem || (!__global.useMU() && __global.useshmem());

            TRACE_ERR((stderr, "global.useshmem: %d, global.useMU: %d\n",
                        (int)__global.useshmem(), (int)__global.useMU()));
            TRACE_ERR((stderr, "optons.no_shmem: %d, options. use_shmem: %d, no_shmem: %d, use_shmem: %d\n",
                        (int)options.no_shmem, (int)options.use_shmem, (int)no_shmem, (int)use_shmem));
            if (no_shmem == 1)
            {
               if(__global.useMU())
               {
                  TRACE_ERR((stderr,"Only registering MU pt2pt protocols\n"));
                  if (options.no_long_header == 1)
                  {
                     _dispatch[id] = (Protocol::Send::Send *)
                           Protocol::Send::Eager <Device::MU::MUPacketModel, MUDevice, false>::
                           generate (id, fn, cookie, _devices->_mu[_contextid], _protocol, result);
                  }
                  else
                  {
                     _dispatch[id] = (Protocol::Send::Send *)
                        Protocol::Send::Eager <Device::MU::MUPacketModel, MUDevice, true>::
                        generate (id, fn, cookie, _devices->_mu[_contextid], _protocol, result);
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
               if(__global.useshmem())
               {
                  TRACE_ERR((stderr,"Only register shmem pt2pt protocols\n"));
                  if (options.no_long_header == 1)
                  {
                     _dispatch[id] = (Protocol::Send::Send *)
                        Protocol::Send::Eager <ShmemPacketModel, ShmemDevice, false>::
                        generate (id, fn, cookie, _devices->_shmem[_contextid], _protocol, result);
                  }
                  else
                  {
                     _dispatch[id] = (Protocol::Send::Send *)
                        Protocol::Send::Eager <ShmemPacketModel, ShmemDevice, true>::
                        generate (id, fn, cookie, _devices->_shmem[_contextid], _protocol, result);
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
               if(__global.useshmem() && __global.useMU())
               {
                  if (options.no_long_header == 1)
                  {
                     Protocol::Send::Eager <Device::MU::MUPacketModel, MUDevice, false> * eagermu =
                        Protocol::Send::Eager <Device::MU::MUPacketModel, MUDevice, false>::
                        generate (id, fn, cookie, _devices->_mu[_contextid], _protocol, result);

                     Protocol::Send::Eager <ShmemPacketModel, ShmemDevice, false> * eagershmem =
                        Protocol::Send::Eager <ShmemPacketModel, ShmemDevice, false>::
                        generate (id, fn, cookie, _devices->_shmem[_contextid], _protocol, result);

                     _dispatch[id] = (Protocol::Send::Send *) Protocol::Send::Factory::
                        generate (eagershmem, eagermu, _protocol, result);
                  }
                  else
                  {
                     Protocol::Send::Eager <Device::MU::MUPacketModel, MUDevice, true> * eagermu =
                        Protocol::Send::Eager <Device::MU::MUPacketModel, MUDevice, true>::
                        generate (id, fn, cookie, _devices->_mu[_contextid], _protocol, result);

                     Protocol::Send::Eager <ShmemPacketModel, ShmemDevice, true> * eagershmem =
                        Protocol::Send::Eager <ShmemPacketModel, ShmemDevice, true>::
                        generate (id, fn, cookie, _devices->_shmem[_contextid], _protocol, result);

                     _dispatch[id] = (Protocol::Send::Send *) Protocol::Send::Factory::
                        generate (eagershmem, eagermu, _protocol, result);
                  }
               }
            }
         } // end dispatch[id]==null

        TRACE_ERR((stderr, "Context::Context::dispatch_impl .. result = %d\n", result));
        return result;
      }

      inline pami_result_t dispatch_new_impl (size_t                     id,
                                             pami_dispatch_callback_fn   fn,
                                             void                     * cookie,
                                             pami_dispatch_hint_t        options)
      {
         pami_result_t result        = PAMI_ERROR;

         if (options.type == PAMI_P2P_SEND)
         {
            return dispatch_impl (id,
                                  fn,
                                  cookie,
                                  options.hint.send);
         }
         if(__global.useshmem())
         {
           return PAMI_UNIMPL; // no shmem active message yet
         }
         else if(__global.useMU())
         {

            TRACE_ERR((stderr, "Context::dispatch_new_impl multicast %zu\n", id));
            if (_dispatch[id] == NULL)
            {
            _dispatch[id] = (void *)_global_mu_ni; // Only have one multicast right now
            return _global_mu_ni->setDispatch(fn, cookie);
            }

         }
         else
            return PAMI_UNIMPL;
         return result;
      }

      inline pami_result_t multisend_getroles_impl(size_t          dispatch,
                                             int            *numRoles,
                                             int            *replRole)
      {
        return PAMI_UNIMPL;
      };

      inline pami_result_t multicast_impl(pami_multicast_t *mcastinfo)
      {
        if(__global.useshmem())
        {
          TRACE_ERR((stderr, "Context::multicast_impl shmem multicast %p\n", mcastinfo));
          return _shmem_native_interface->multicast(mcastinfo); // Only have one multicast right now

        }
        else if(__global.useMU())
         {
        TRACE_ERR((stderr, "Context::multicast_impl mu multicast %zu, %p\n", mcastinfo->dispatch, mcastinfo));
        CCMI::Interfaces::NativeInterface * ni = (CCMI::Interfaces::NativeInterface *) _dispatch[mcastinfo->dispatch];
        return ni->multicast(mcastinfo); // this version of ni allocates/frees our request storage for us.
        }
        else
          PAMI_abortf("%s<%u>\n", __PRETTY_FUNCTION__, __LINE__);
        return PAMI_UNIMPL;
      };


      inline pami_result_t manytomany_impl(pami_manytomany_t *m2minfo)
      {
        PAMI_abortf("%s<%u>\n", __PRETTY_FUNCTION__, __LINE__);
        return PAMI_UNIMPL;
      };


      inline pami_result_t multisync_impl(pami_multisync_t *msyncinfo)
      {
        if(__global.useshmem())
        {
          TRACE_ERR((stderr, "Context::multisync_impl shmem multisync %p\n", msyncinfo));
          return _shmem_native_interface->multisync(msyncinfo); // Only have one multisync right now
        }
        else if(__global.useMU())
         {

        TRACE_ERR((stderr, "Context::multisync_impl mu multisync %p\n", msyncinfo));
        return _global_mu_ni->multisync(msyncinfo); // Only have one multisync right now
        }
        else
          PAMI_abortf("%s<%u>\n", __PRETTY_FUNCTION__, __LINE__);

        return PAMI_UNIMPL;
      };


      inline pami_result_t multicombine_impl(pami_multicombine_t *mcombineinfo)
      {
        if(__global.useshmem())
        {
          TRACE_ERR((stderr, "Context::multicombine_impl shmem multicombine %p\n", mcombineinfo));
          return _shmem_native_interface->multicombine(mcombineinfo); // Only have one multicombine right now
        }
         else if(__global.useMU())
         {

        TRACE_ERR((stderr, "Context::multicombine_impl multicombine %p\n", mcombineinfo));
        return _global_mu_ni->multicombine(mcombineinfo);// Only have one multicombine right now
        }
        else
          PAMI_abortf("%s<%u>\n", __PRETTY_FUNCTION__, __LINE__);
        return PAMI_UNIMPL;
      };

      inline pami_result_t analyze(size_t         context_id,
                                  BGQGeometry    *geometry)
      {
        pami_result_t result = PAMI_NERROR;
        result = _multi_registration->analyze(context_id,geometry);
        return result;
      }


    private:

      pami_client_t                _client;
      pami_context_t               _context;
      size_t                       _clientid;
      size_t                       _contextid;

      PAMI::Memory::MemoryManager  _mm;
      SysDep                       _sysdep;

      void *                       _dispatch[1024];
      Protocol::Get::RGet         *_rget;
      Protocol::Put::RPut         *_rput;
      MemoryAllocator<1024, 16>    _request;
      ContextLock                  _lock;
    public:
      CollRegistration::BGQMultiRegistration < BGQGeometry, AllSidedShmemNI, MUGlobalNI >    *_multi_registration;
      BGQGeometry                 *_world_geometry;
    private:
      pami_result_t                _status;
      Device::LocalBcastWQModel   *_shmemMcastModel;
      Barrier_Model               *_shmemMsyncModel;
      Device::LocalAllreduceWQModel  *_shmemMcombModel;
      AllSidedShmemNI             *_shmem_native_interface;
      uint8_t                      _multi_registration_storage[sizeof(CollRegistration::BGQMultiRegistration < BGQGeometry, AllSidedShmemNI, MUGlobalNI >)];
      uint8_t                      _shmemMcastModel_storage[sizeof(Device::LocalBcastWQModel)];
      uint8_t                      _shmemMsyncModel_storage[sizeof(Barrier_Model)];
      uint8_t                      _shmemMcombModel_storage[sizeof(Device::LocalAllreduceWQModel)];
      uint8_t                      _shmem_native_interface_storage[sizeof(AllSidedShmemNI)];
      ProtocolAllocator            _protocol;
      PlatformDeviceList          *_devices;
      MUGlobalNI                  *_global_mu_ni;
      uint8_t                      _global_mu_ni_storage[sizeof(MUGlobalNI)];
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
