///
/// \file common/bgq/Context.h
/// \brief PAMI Blue Gene\Q specific context implementation.
///
#ifndef __common_bgq_Context_h__
#define __common_bgq_Context_h__


#include <stdlib.h>
#include <string.h>
#include <new>

#include "sys/pami.h"
#include "common/ContextInterface.h"

#include "components/devices/generic/Device.h"
#include "components/devices/misc/ProgressFunctionMsg.h"
#include "components/devices/misc/AtomicBarrierMsg.h"
#include "components/devices/workqueue/WQRingReduceMsg.h"
#include "components/devices/workqueue/WQRingBcastMsg.h"
#include "components/devices/workqueue/LocalAllreduceWQMessage.h"
#include "components/devices/workqueue/LocalReduceWQMessage.h"
#include "components/devices/workqueue/LocalBcastWQMessage.h"

#include "components/atomic/gcc/GccBuiltin.h"
#include "components/atomic/bgq/L2Counter.h"
//#include "components/atomic/pthread/Pthread.h"

#include "components/memory/MemoryAllocator.h"

#include "SysDep.h"
#include "Memregion.h"

#include "p2p/protocols/send/eager/Eager.h"
#include "p2p/protocols/send/composite/Composite.h"

#include "p2p/protocols/get/Get.h"

#include "TypeDefs.h"
#include "algorithms/geometry/CCMIMultiRegistration.h"

#ifdef TRACE_ERR
#undef TRACE_ERR
#endif
#define TRACE_ERR(x) //fprintf x



namespace PAMI
{
  typedef CollRegistration::CCMIMultiRegistration < BGQGeometry, AllSidedNI > MultiCollectiveRegistration;

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
      PlatformDeviceList() { }

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
#ifdef ENABLE_SHMEM_DEVICE
         TRACE_ERR((stderr,"device init: shmem\n"));
        _shmem = ShmemDevice::Factory::generate(clientid, num_ctx, mm, _generics);
        TRACE_ERR((stderr,"device init shmem done, progress func\n"));
#endif
         TRACE_ERR((stderr,"device init: progress function\n"));
        _progfunc = PAMI::Device::ProgressFunctionDev::Factory::generate(clientid, num_ctx, mm, _generics);
        TRACE_ERR((stderr,"device init: atomic barrier\n"));
        _atombarr = PAMI::Device::AtomicBarrierDev::Factory::generate(clientid, num_ctx, mm, _generics);
        TRACE_ERR((stderr,"device init: wqring reduce\n"));
        _wqringreduce = PAMI::Device::WQRingReduceDev::Factory::generate(clientid, num_ctx, mm, _generics);
        TRACE_ERR((stderr,"device init: wqring bcast\n"));
        _wqringbcast = PAMI::Device::WQRingBcastDev::Factory::generate(clientid, num_ctx, mm, _generics);
        TRACE_ERR((stderr,"device init: local allreduce wq\n"));
        _localallreduce = PAMI::Device::LocalAllreduceWQDevice::Factory::generate(clientid, num_ctx, mm, _generics);
        TRACE_ERR((stderr,"device init: local bcast wq\n"));
        _localbcast = PAMI::Device::LocalBcastWQDevice::Factory::generate(clientid, num_ctx, mm, _generics);
        TRACE_ERR((stderr,"device init: local reduce wq\n"));
        _localreduce = PAMI::Device::LocalReduceWQDevice::Factory::generate(clientid, num_ctx, mm, _generics);
#ifdef ENABLE_MU_DEVICE
         TRACE_ERR((stderr,"device init: MU\n"));
        _mu = MUDevice::Factory::generate(clientid, num_ctx, mm, _generics);
#endif
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
#ifdef ENABLE_SHMEM_DEVICE
        ShmemDevice::Factory::init(_shmem, clientid, contextid, clt, ctx, mm, _generics);
#endif
        PAMI::Device::ProgressFunctionDev::Factory::init(_progfunc, clientid, contextid, clt, ctx, mm , _generics);
        PAMI::Device::AtomicBarrierDev::Factory::init(_atombarr, clientid, contextid, clt, ctx, mm, _generics);
        PAMI::Device::WQRingReduceDev::Factory::init(_wqringreduce, clientid, contextid, clt, ctx, mm , _generics);
        PAMI::Device::WQRingBcastDev::Factory::init(_wqringbcast, clientid, contextid, clt, ctx, mm, _generics);
        PAMI::Device::LocalAllreduceWQDevice::Factory::init(_localallreduce, clientid, contextid, clt, ctx, mm, _generics);
        PAMI::Device::LocalBcastWQDevice::Factory::init(_localbcast, clientid, contextid, clt, ctx, mm, _generics);
        PAMI::Device::LocalReduceWQDevice::Factory::init(_localreduce, clientid, contextid, clt, ctx, mm, _generics);
#ifdef ENABLE_MU_DEVICE
        MUDevice::Factory::init(_mu, clientid, contextid, clt, ctx, mm, _generics);
#endif
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
#ifdef ENABLE_SHMEM_DEVICE
        events += ShmemDevice::Factory::advance(_shmem, clientid, contextid);
#endif
        events += PAMI::Device::ProgressFunctionDev::Factory::advance(_progfunc, clientid, contextid);
        events += PAMI::Device::AtomicBarrierDev::Factory::advance(_atombarr, clientid, contextid);
        events += PAMI::Device::WQRingReduceDev::Factory::advance(_wqringreduce, clientid, contextid);
        events += PAMI::Device::WQRingBcastDev::Factory::advance(_wqringbcast, clientid, contextid);
        events += PAMI::Device::LocalAllreduceWQDevice::Factory::advance(_localallreduce, clientid, contextid);
        events += PAMI::Device::LocalBcastWQDevice::Factory::advance(_localbcast, clientid, contextid);
        events += PAMI::Device::LocalReduceWQDevice::Factory::advance(_localreduce, clientid, contextid);
#ifdef ENABLE_MU_DEVICE
        events += MUDevice::Factory::advance(_mu, clientid, contextid);
#endif
        return events;
      }

      PAMI::Device::Generic::Device *_generics; // need better name...
#ifdef ENABLE_SHMEM_DEVICE
      ShmemDevice *_shmem;
#endif
      PAMI::Device::ProgressFunctionDev *_progfunc;
      PAMI::Device::AtomicBarrierDev *_atombarr;
      PAMI::Device::WQRingReduceDev *_wqringreduce;
      PAMI::Device::WQRingBcastDev *_wqringbcast;;
      PAMI::Device::LocalAllreduceWQDevice *_localallreduce;
      PAMI::Device::LocalBcastWQDevice *_localbcast;
      PAMI::Device::LocalReduceWQDevice *_localreduce;
#ifdef ENABLE_MU_DEVICE
      MUDevice *_mu;
#endif
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
          _multi_registration(NULL),
          _world_geometry(world_geometry),
          _status(PAMI_SUCCESS),
          _mcastModel(NULL),
          _msyncModel(NULL),
          _mcombModel(NULL),
          _native_interface(NULL),
          _devices(devices)
      {
        TRACE_ERR((stderr,  "Context::Context() enter\n"));
        // ----------------------------------------------------------------
        // Compile-time assertions
        // ----------------------------------------------------------------

        // Make sure the memory allocator is large enough for all
        // protocol classes.
#ifdef ENABLE_SHMEM_DEVICE
        COMPILE_TIME_ASSERT(sizeof(EagerShmem) <= ProtocolAllocator::objsize);
        COMPILE_TIME_ASSERT(sizeof(GetShmem) <= ProtocolAllocator::objsize);
#endif
#ifdef ENABLE_MU_DEVICE
        COMPILE_TIME_ASSERT(sizeof(MUGlobalNI) <= ProtocolAllocator::objsize);
#endif
        // ----------------------------------------------------------------
        // Compile-time assertions
        // ----------------------------------------------------------------

        _devices->init(_clientid, _contextid, _client, _context, &_mm);
#ifdef ENABLE_MU_DEVICE
        // Can't construct NI until device is init()'d.  Ctor into member storage.
        _global_mu_ni = new (_global_mu_ni_storage) MUGlobalNI(MUDevice::Factory::getDevice(_devices->_mu, _clientid, _contextid), _client, _context, _contextid, _clientid);

#endif

        _mcastModel         = (Device::LocalBcastWQModel*)_mcastModel_storage;
        _msyncModel         = (Barrier_Model*)_msyncModel_storage;
        _mcombModel         = (Device::LocalReduceWQModel*)_mcombModel_storage;
        _native_interface   = (AllSidedNI*)_native_interface_storage;
        _multi_registration = (MultiCollectiveRegistration*) _multi_registration_storage;

	if (__global.topology_local.size() > 1) {
        	new (_mcastModel_storage)       Device::LocalBcastWQModel(PAMI::Device::LocalBcastWQDevice::Factory::getDevice(_devices->_localbcast, _clientid, _contextid),_status);
        	new (_msyncModel_storage)       Barrier_Model(PAMI::Device::AtomicBarrierDev::Factory::getDevice(_devices->_atombarr, _clientid, _contextid),_status);
        	new (_mcombModel_storage)       Device::LocalReduceWQModel(PAMI::Device::LocalReduceWQDevice::Factory::getDevice(_devices->_localreduce, _clientid, _contextid),_status);
        	new (_native_interface_storage) AllSidedNI(_mcastModel, _msyncModel, _mcombModel, client, (pami_context_t)this, id, clientid);
        	new (_multi_registration)       MultiCollectiveRegistration(*_native_interface, client, (pami_context_t)this, id, clientid);

        	_multi_registration->analyze(_contextid, _world_geometry);
	}

        /** \todo #warning This should not be here? */
#if 0 // not working yet? not fully implemented?
        pami_result_t result ;
        _get = (void *) _request.allocateObject ();
        new ((void *)_get) GetShmem(ShmemDevice::Factory::getDevice(_devices->_shmem, _clientid, _contextid), result);

        // dispatch_impl relies on the table being initialized to NULL's.
        memset(_dispatch, 0x00, sizeof(_dispatch));
#endif

        TRACE_ERR((stderr,  "Context:: exit\n"));
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

      inline pami_result_t put (pami_put_simple_t * parameters)
      {
        return PAMI_UNIMPL;
      }

      inline pami_result_t put_typed (pami_put_typed_t * parameters)
      {
        return PAMI_UNIMPL;
      }

      inline pami_result_t get (pami_get_simple_t * parameters)
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

      inline pami_result_t memregion_register_impl (void            * address,
                                                    size_t            bytes,
                                                    pami_memregion_t * memregion)
      {
        return PAMI_UNIMPL;
      }

      inline pami_result_t memregion_deregister_impl (pami_memregion_t memregion)
      {
        // Memory regions do not need to be deregistered on BG/Q so this
        // interface is implemented as a noop.
        return PAMI_SUCCESS;
      }

      inline pami_result_t memregion_query (pami_memregion_t    memregion,
                                           void            ** address,
                                           size_t           * bytes,
                                           size_t           * task)
      {
        return PAMI_UNIMPL;
      }

      inline pami_result_t rput (pami_rput_simple_t * parameters)
      {
        return PAMI_UNIMPL;
      }

      inline pami_result_t rput_typed (pami_rput_typed_t * parameters)
      {
        return PAMI_UNIMPL;
      }

      inline pami_result_t rget (pami_rget_simple_t * parameters)
      {
#if 0 // not implemented yet???
        ((GetShmem*)_get)->getimpl (	parameters->rma.done_fn,
                                     parameters->rma.cookie,
                                     parameters->rma.dest,
                                     parameters->rget.bytes,
                                     (Memregion*)parameters->rget.local_mr,
                                     (Memregion*)parameters->rget.remote_mr,
                                     parameters->rget.local_offset,
                                     parameters->rget.remote_offset);
#endif
        return PAMI_SUCCESS;
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
            bool no_shmem  = options.no_shmem;
#ifndef ENABLE_SHMEM_DEVICE
            no_shmem = true;
#endif

            bool use_shmem = options.use_shmem;
#ifndef ENABLE_MU_DEVICE
            use_shmem = true;
#endif

            if (no_shmem == 1)
            {
              // Register only the "mu" eager send protocol
#ifdef ENABLE_MU_DEVICE

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

#else
              PAMI_abortf("No non-shmem protocols available.");
#endif
            }
            else if (use_shmem == 1)
            {
              // Register only the "shmem" eager send protocol
#ifdef ENABLE_SHMEM_DEVICE

              if (options.no_long_header == 1)
                {
                  _dispatch[id] = (Protocol::Send::Send *)
                    Protocol::Send::Eager <ShmemModel, ShmemDevice, false>::
                      generate (id, fn, cookie, _devices->_shmem[_contextid], _protocol, result);
                }
              else
                {
                  _dispatch[id] = (Protocol::Send::Send *)
                    Protocol::Send::Eager <ShmemModel, ShmemDevice, true>::
                      generate (id, fn, cookie, _devices->_shmem[_contextid], _protocol, result);
                }

#else
              PAMI_abortf("No shmem protocols available.");
#endif
            }

#if defined(ENABLE_SHMEM_DEVICE) && defined(ENABLE_MU_DEVICE)
            else
            {
              // Register both the "mu" and "shmem" eager send protocols
              if (options.no_long_header == 1)
                {
                  Protocol::Send::Eager <Device::MU::MUPacketModel, MUDevice, false> * eagermu =
                    Protocol::Send::Eager <Device::MU::MUPacketModel, MUDevice, false>::
                      generate (id, fn, cookie, _devices->_mu[_contextid], _protocol, result);

                  Protocol::Send::Eager <ShmemModel, ShmemDevice, false> * eagershmem =
                    Protocol::Send::Eager <ShmemModel, ShmemDevice, false>::
                      generate (id, fn, cookie, _devices->_shmem[_contextid], _protocol, result);

                  _dispatch[id] = (Protocol::Send::Send *) Protocol::Send::Factory::
                      generate (eagershmem, eagermu, _protocol, result);
                }
              else
                {
                  Protocol::Send::Eager <Device::MU::MUPacketModel, MUDevice, true> * eagermu =
                    Protocol::Send::Eager <Device::MU::MUPacketModel, MUDevice, true>::
                      generate (id, fn, cookie, _devices->_mu[_contextid], _protocol, result);

                  Protocol::Send::Eager <ShmemModel, ShmemDevice, true> * eagershmem =
                    Protocol::Send::Eager <ShmemModel, ShmemDevice, true>::
                      generate (id, fn, cookie, _devices->_shmem[_contextid], _protocol, result);

                  _dispatch[id] = (Protocol::Send::Send *) Protocol::Send::Factory::
                      generate (eagershmem, eagermu, _protocol, result);
                }
            }

#endif
          }

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

#ifdef ENABLE_MU_DEVICE
        TRACE_ERR((stderr, "Context::dispatch_new_impl multicast %zu\n", id));

        if (_global_mu_ni == NULL) // lazy ctor
          {
            MUGlobalNI* temp = (MUGlobalNI*) _protocol.allocateObject ();
            TRACE_ERR((stderr, "new MUGlobalNI(%p, %zu, %p, %zu) = %p, size %zu\n",
                       &_devices->_mu, _clientid, _context, _contextid, temp, sizeof(MUGlobalNI)));
            _global_mu_ni = new (temp) MUGlobalNI(MUDevice::Factory::getDevice(_devices->_mu, _clientid, _contextid), _client, _context, _contextid, _clientid);
          }

        if (_dispatch[id] == NULL)
          {
            _dispatch[id] = (void *)_global_mu_ni; // Only have one multicast right now
            return _global_mu_ni->setDispatch(fn, cookie);
            TRACE_ERR((stderr, "Context::dispatch_new_impl multicast %zu\n", id));
            PAMI_assertf(_protocol.objsize >= sizeof(PAMI::Device::MU::MUMulticastModel), "%zu >= %zu\n", _protocol.objsize, sizeof(PAMI::Device::MU::MUMulticastModel));
            // Allocate memory for the protocol object.
            _dispatch[id] = (void *) _protocol.allocateObject ();

            PAMI::Device::MU::MUMulticastModel * model = new ((void*)_dispatch[id]) PAMI::Device::MU::MUMulticastModel(MUDevice::Factory::getDevice(_devices->_mu, _clientid, _contextid), result);
            model->registerMcastRecvFunction(id, fn.multicast, cookie);

          }

#endif
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
#if defined(ENABLE_MU_DEVICE)
        TRACE_ERR((stderr, "Context::multicast_impl multicast %zu, %p\n", mcastinfo->dispatch, mcastinfo));
        CCMI::Interfaces::NativeInterface * ni = (CCMI::Interfaces::NativeInterface *) _dispatch[mcastinfo->dispatch];
        return ni->multicast(mcastinfo); // this version of ni allocates/frees our request storage for us.
#else
        return PAMI_UNIMPL;
#endif
      };


      inline pami_result_t manytomany_impl(pami_manytomany_t *m2minfo)
      {
        return PAMI_UNIMPL;
      };


      inline pami_result_t multisync_impl(pami_multisync_t *msyncinfo)
      {
#if defined(ENABLE_MU_DEVICE)

        if (_global_mu_ni == NULL) // lazy ctor
          {
            MUGlobalNI* temp = (MUGlobalNI*) _protocol.allocateObject ();
            TRACE_ERR((stderr, "new MUGlobalNI(%p, %zu, %p, %zu) = %p, size %zu\n",
                       &_devices->_mu, _clientid, _context, _contextid, temp, sizeof(MUGlobalNI)));
            _global_mu_ni = new (temp) MUGlobalNI(MUDevice::Factory::getDevice(_devices->_mu, _clientid, _contextid), _client, _context, _contextid, _clientid);
          }

        TRACE_ERR((stderr, "Context::multisync_impl multisync %p\n", msyncinfo));
        return _global_mu_ni->multisync(msyncinfo); // Only have one multisync right now
#else
        return PAMI_UNIMPL;
#endif
      };


      inline pami_result_t multicombine_impl(pami_multicombine_t *mcombineinfo)
      {
#if defined(ENABLE_MU_DEVICE)

        if (_global_mu_ni == NULL) // lazy ctor
          {
            MUGlobalNI* temp = (MUGlobalNI*) _protocol.allocateObject ();
            TRACE_ERR((stderr, "new MUGlobalNI(%p, %zu, %p, %zu) = %p, size %zu\n",
                       &_devices->_mu, _clientid, _context, _contextid, temp, sizeof(MUGlobalNI)));
            _global_mu_ni = new (temp) MUGlobalNI(MUDevice::Factory::getDevice(_devices->_mu, _clientid, _contextid), _client, _context, _contextid, _clientid);
          }

        TRACE_ERR((stderr, "Context::multicombine_impl multicombine %p\n", mcombineinfo));
        return _global_mu_ni->multicombine(mcombineinfo);// Only have one multicombine right now
#else
        return PAMI_UNIMPL;
#endif
      };

      inline pami_result_t analyze(size_t         context_id,
                                  BGQGeometry    *geometry)
      {
        return _multi_registration->analyze(context_id,geometry);
      }


    private:

      pami_client_t                _client;
      pami_context_t               _context;
      size_t                       _clientid;
      size_t                       _contextid;

      PAMI::Memory::MemoryManager  _mm;
      SysDep                       _sysdep;

      void *                       _dispatch[1024];
      void* _get; //use for now..remove later
      MemoryAllocator<1024, 16>    _request;
      ContextLock                  _lock;
    public:
      MultiCollectiveRegistration *_multi_registration;
      BGQGeometry                 *_world_geometry;
    private:
      pami_result_t                _status;
      Device::LocalBcastWQModel   *_mcastModel;
      Barrier_Model               *_msyncModel;
      Device::LocalReduceWQModel  *_mcombModel;
      AllSidedNI                  *_native_interface;
      uint8_t                      _multi_registration_storage[sizeof(MultiCollectiveRegistration)];
      uint8_t                      _mcastModel_storage[sizeof(Device::LocalBcastWQModel)];
      uint8_t                      _msyncModel_storage[sizeof(Barrier_Model)];
      uint8_t                      _mcombModel_storage[sizeof(Device::LocalReduceWQModel)];
      uint8_t                      _native_interface_storage[sizeof(AllSidedNI)];
      ProtocolAllocator            _protocol;
      PlatformDeviceList          *_devices;
#if defined(ENABLE_MU_DEVICE)
      MUGlobalNI                  *_global_mu_ni;
      uint8_t                      _global_mu_ni_storage[sizeof(MUGlobalNI)];
#endif
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
