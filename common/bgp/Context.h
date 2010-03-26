///
/// \file common/bgp/Context.h
/// \brief PAMI BGP specific context implementation.
///
#ifndef __common_bgp_Context_h__
#define __common_bgp_Context_h__

#include <stdlib.h>
#include <string.h>

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

// BGP-specific devices...
#include "components/devices/bgp/global_interrupt/GIBarrierMsg.h"
#include "components/devices/bgp/collective_network/CNAllreduceMsg.h"
//#include "components/devices/bgp/collective_network/CNAllreduceShortMsg.h"
#include "components/devices/bgp/collective_network/CNAllreducePPMsg.h"
#include "components/devices/bgp/collective_network/CNAllreduceSum2PMsg.h"
#include "components/devices/bgp/collective_network/CNBroadcastMsg.h"

#include "components/devices/shmem/ShmemDevice.h"
#include "components/devices/shmem/ShmemPacketModel.h"
#include "util/fifo/FifoPacket.h"
#include "util/fifo/LinearFifo.h"

#include "components/atomic/bgp/BgpAtomic.h"
#include "components/atomic/bgp/LockBoxCounter.h"
#include "components/atomic/counter/CounterMutex.h"
#include "components/atomic/gcc/GccCounter.h"

#include "components/memory/MemoryAllocator.h"
#include "components/memory/MemoryManager.h"

#include "SysDep.h"
#include "Memregion.h"

#include "p2p/protocols/Send.h"
#include "p2p/protocols/send/eager/Eager.h"
//#include "p2p/protocols/send/adaptive/Adaptive.h"
//#include "p2p/protocols/send/datagram/Datagram.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif

namespace PAMI
{
  typedef PAMI::Mutex::CounterMutex<PAMI::Counter::GccProcCounter>  ContextLock;

  typedef Fifo::FifoPacket <16, 256> ShmemPacket;
  typedef Fifo::LinearFifo<Counter::BGP::LockBoxProcCounter, ShmemPacket, 128> ShmemFifo;
  //typedef Fifo::LinearFifo<Atomic::GccBuiltin, ShmemPacket, 128> ShmemFifo;
  typedef Device::ShmemDevice<ShmemFifo> ShmemDevice;
  typedef Device::Shmem::PacketModel<ShmemDevice> ShmemModel;

  //
  // >> Point-to-point protocol typedefs and dispatch registration.
  //typedef PAMI::Protocol::Send::Eager <ShmemModel, ShmemDevice> EagerShmem;
  // << Point-to-point protocol typedefs and dispatch registration.
  //

  typedef MemoryAllocator<1024, 16> ProtocolAllocator;

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
  class PlatformDeviceList {
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
     * \param[in] clientid	Client ID (index)
     * \param[in] num_ctx	Number of contexts being created
     * \param[in] mm		MemeoryManager for use in generating devices
     */
    inline pami_result_t generate(size_t clientid, size_t num_ctx, Memory::MemoryManager &mm) {
        // these calls create (allocate and construct) each element.
        // We don't know how these relate to contexts, they are semi-opaque.
        _generics = PAMI::Device::Generic::Device::Factory::generate(clientid, num_ctx, mm);
        _shmem = ShmemDevice::Factory::generate(clientid, num_ctx, mm);
        _progfunc = PAMI::Device::ProgressFunctionDev::Factory::generate(clientid, num_ctx, mm);
        _atombarr = PAMI::Device::AtomicBarrierDev::Factory::generate(clientid, num_ctx, mm);
        _wqringreduce = PAMI::Device::WQRingReduceDev::Factory::generate(clientid, num_ctx, mm);
        _wqringbcast = PAMI::Device::WQRingBcastDev::Factory::generate(clientid, num_ctx, mm);
        _localallreduce = PAMI::Device::LocalAllreduceWQDevice::Factory::generate(clientid, num_ctx, mm);
        _localbcast = PAMI::Device::LocalBcastWQDevice::Factory::generate(clientid, num_ctx, mm);
        _localreduce = PAMI::Device::LocalReduceWQDevice::Factory::generate(clientid, num_ctx, mm);
        // BGP-specific devices...
        _gibarr = PAMI::Device::BGP::giDevice::Factory::generate(clientid, num_ctx, mm);
        _cnallred = PAMI::Device::BGP::CNAllreduceDevice::Factory::generate(clientid, num_ctx, mm);
        _cnppallred = PAMI::Device::BGP::CNAllreducePPDevice::Factory::generate(clientid, num_ctx, mm);
        _cn2pallred = PAMI::Device::BGP::CNAllreduce2PDevice::Factory::generate(clientid, num_ctx, mm);
        _cnbcast = PAMI::Device::BGP::CNBroadcastDevice::Factory::generate(clientid, num_ctx, mm);
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
     * \param[in] clientid	Client ID (index)
     * \param[in] contextid	Context ID (index)
     * \param[in] clt		Client opaque entity
     * \param[in] ctx		Context opaque entity
     * \param[in] sd		SysDep object
     */
    inline pami_result_t init(size_t clientid, size_t contextid, pami_client_t clt, pami_context_t ctx, PAMI::Memory::MemoryManager *mm) {
        PAMI::Device::Generic::Device::Factory::init(_generics, clientid, contextid, clt, ctx, mm, _generics);
        ShmemDevice::Factory::init(_shmem, clientid, contextid, clt, ctx, mm, _generics);
        PAMI::Device::ProgressFunctionDev::Factory::init(_progfunc, clientid, contextid, clt, ctx, mm, _generics);
        PAMI::Device::AtomicBarrierDev::Factory::init(_atombarr, clientid, contextid, clt, ctx, mm, _generics);
        PAMI::Device::WQRingReduceDev::Factory::init(_wqringreduce, clientid, contextid, clt, ctx, mm, _generics);
        PAMI::Device::WQRingBcastDev::Factory::init(_wqringbcast, clientid, contextid, clt, ctx, mm, _generics);
        PAMI::Device::LocalAllreduceWQDevice::Factory::init(_localallreduce, clientid, contextid, clt, ctx, mm, _generics);
        PAMI::Device::LocalBcastWQDevice::Factory::init(_localbcast, clientid, contextid, clt, ctx, mm, _generics);
        PAMI::Device::LocalReduceWQDevice::Factory::init(_localreduce, clientid, contextid, clt, ctx, mm, _generics);
        PAMI::Device::BGP::giDevice::Factory::init(_gibarr, clientid, contextid, clt, ctx, mm, _generics);
        PAMI::Device::BGP::CNAllreduceDevice::Factory::init(_cnallred, clientid, contextid, clt, ctx, mm, _generics);
        PAMI::Device::BGP::CNAllreducePPDevice::Factory::init(_cnppallred, clientid, contextid, clt, ctx, mm, _generics);
        PAMI::Device::BGP::CNAllreduce2PDevice::Factory::init(_cn2pallred, clientid, contextid, clt, ctx, mm, _generics);
        PAMI::Device::BGP::CNBroadcastDevice::Factory::init(_cnbcast, clientid, contextid, clt, ctx, mm, _generics);
        return PAMI_SUCCESS;
    }

    /**
     * \brief advance all devices
     *
     * since device arrays are semi-opaque (we don't know how many
     * elements each has) we call a more-general interface here.
     *
     * \param[in] clientid	Client ID (index)
     * \param[in] contextid	Context ID (index)
     */
    inline size_t advance(size_t clientid, size_t contextid) {
        size_t events = 0;
        events += PAMI::Device::Generic::Device::Factory::advance(_generics, clientid, contextid);
        events += ShmemDevice::Factory::advance(_shmem, clientid, contextid);
        events += PAMI::Device::ProgressFunctionDev::Factory::advance(_progfunc, clientid, contextid);
        events += PAMI::Device::AtomicBarrierDev::Factory::advance(_atombarr, clientid, contextid);
        events += PAMI::Device::WQRingReduceDev::Factory::advance(_wqringreduce, clientid, contextid);
        events += PAMI::Device::WQRingBcastDev::Factory::advance(_wqringbcast, clientid, contextid);
        events += PAMI::Device::LocalAllreduceWQDevice::Factory::advance(_localallreduce, clientid, contextid);
        events += PAMI::Device::LocalBcastWQDevice::Factory::advance(_localbcast, clientid, contextid);
        events += PAMI::Device::LocalReduceWQDevice::Factory::advance(_localreduce, clientid, contextid);
        // BGP-specific devices...
        events += PAMI::Device::BGP::giDevice::Factory::advance(_gibarr, clientid, contextid);
        events += PAMI::Device::BGP::CNAllreduceDevice::Factory::advance(_cnallred, clientid, contextid);
        events += PAMI::Device::BGP::CNAllreducePPDevice::Factory::advance(_cnppallred, clientid, contextid);
        events += PAMI::Device::BGP::CNAllreduce2PDevice::Factory::advance(_cn2pallred, clientid, contextid);
        events += PAMI::Device::BGP::CNBroadcastDevice::Factory::advance(_cnbcast, clientid, contextid);
        return events;
    }

    PAMI::Device::Generic::Device *_generics; // need better name...
    ShmemDevice *_shmem;
    PAMI::Device::ProgressFunctionDev *_progfunc;
    PAMI::Device::AtomicBarrierDev *_atombarr;
    PAMI::Device::WQRingReduceDev *_wqringreduce;
    PAMI::Device::WQRingBcastDev *_wqringbcast;;
    PAMI::Device::LocalAllreduceWQDevice *_localallreduce;
    PAMI::Device::LocalBcastWQDevice *_localbcast;
    PAMI::Device::LocalReduceWQDevice *_localreduce;
    // BGP-specific devices...
    PAMI::Device::BGP::giDevice *_gibarr;
    PAMI::Device::BGP::CNAllreduceDevice *_cnallred;
    PAMI::Device::BGP::CNAllreducePPDevice *_cnppallred;
    PAMI::Device::BGP::CNAllreduce2PDevice *_cn2pallred;
    PAMI::Device::BGP::CNBroadcastDevice *_cnbcast;
  }; // class PlatformDeviceList


  class Context : public Interface::Context<PAMI::Context>
  {
    public:
      inline Context (pami_client_t client, size_t clientid, size_t id, size_t num,
                                      PlatformDeviceList *devices,
                                void * addr, size_t bytes) :
          Interface::Context<PAMI::Context> (client, id),
          _client (client),
          _context ((pami_context_t)this),
          _clientid (clientid),
          _contextid (id),
          _mm (addr, bytes),
          _sysdep (_mm),
          _lock (),
          _devices(devices)
      {
        // ----------------------------------------------------------------
        // Compile-time assertions
        // ----------------------------------------------------------------

        // Make sure the memory allocator is large enough for all
        // protocol classes.
        //COMPILE_TIME_ASSERT(sizeof(EagerShmem) <= ProtocolAllocator::objsize);

        // ----------------------------------------------------------------
        // Compile-time assertions
        // ----------------------------------------------------------------

        _lock.init(&_mm);
        _devices->init(_clientid, _contextid, _client, _context, &_mm);
        _local_generic_device = & PAMI::Device::Generic::Device::Factory::getDevice(_devices->_generics, clientid, id);


        // dispatch_impl relies on the table being initialized to NULL's.
        memset(_dispatch, 0x00, sizeof(_dispatch));
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
        return PAMI_SUCCESS;
      }

      inline pami_result_t post_impl (pami_work_t *state, pami_work_function work_fn, void * cookie)
      {
        TRACE_ERR((stderr, ">> Context::post_impl(%p, %p, %p)\n", state, work_fn, cookie));
        PAMI::Device::Generic::GenericThread *work;
        COMPILE_TIME_ASSERT(sizeof(*state) >= sizeof(*work));
        TRACE_ERR((stderr, "   Context::post_impl(%p, %p, %p) .. 0\n", state, work_fn, cookie));
        work = new (state) PAMI::Device::Generic::GenericThread(work_fn, cookie);
        TRACE_ERR((stderr, "   Context::post_impl(%p, %p, %p) .. 1\n", state, work_fn, cookie));
        work->setStatus(PAMI::Device::OneShot);
        TRACE_ERR((stderr, "   Context::post_impl(%p, %p, %p) .. 2\n", state, work_fn, cookie));
        //_devices->_generics[_contextid].postThread(work);
        _local_generic_device->postThread(work);
        TRACE_ERR((stderr, "<< Context::post_impl(%p, %p, %p)\n", state, work_fn, cookie));
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
        TRACE_ERR((stderr, ">> lock_impl()\n"));
        _lock.acquire ();
        TRACE_ERR((stderr, "<< lock_impl()\n"));
        return PAMI_SUCCESS;
      }

      inline pami_result_t trylock_impl ()
      {
        TRACE_ERR((stderr, ">> trylock_impl()\n"));

        if (_lock.tryAcquire ())
          {
            TRACE_ERR((stderr, "<< trylock_impl(), PAMI_SUCCESS\n"));
            return PAMI_SUCCESS;
          }

        TRACE_ERR((stderr, "<< trylock_impl(), PAMI_EAGAIN\n"));
        return PAMI_EAGAIN;
      }

      inline pami_result_t unlock_impl ()
      {
        TRACE_ERR((stderr, ">> release_impl()\n"));
        _lock.release ();
        TRACE_ERR((stderr, "<< release_impl()\n"));
        return PAMI_SUCCESS;
      }

      inline pami_result_t send_impl (pami_send_t * parameters)
      {
        size_t id = (size_t)(parameters->send.dispatch);
        TRACE_ERR((stderr, ">> send_impl('simple'), _dispatch[%zu] = %p\n", id, _dispatch[id]));
        PAMI_assert_debug (_dispatch[id] != NULL);

        PAMI::Protocol::Send::Send * send =
          (PAMI::Protocol::Send::Send *) _dispatch[id];
        pami_result_t result = send->simple (parameters);

        TRACE_ERR((stderr, "<< send_impl('simple')\n"));
        return result;
      }

      inline pami_result_t send_impl (pami_send_immediate_t * parameters)
      {
        size_t id = (size_t)(parameters->dispatch);
        TRACE_ERR((stderr, ">> send_impl('immediate'), _dispatch[%zu] = %p\n", id, _dispatch[id]));
        PAMI_assert_debug (_dispatch[id] != NULL);

        PAMI::Protocol::Send::Send * send =
          (PAMI::Protocol::Send::Send *) _dispatch[id];
        pami_result_t result = send->immediate (parameters);

        TRACE_ERR((stderr, "<< send_impl('immediate')\n"));
        return result;
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

      inline pami_result_t memregion_register (void            * address,
                                              size_t            bytes,
                                              pami_memregion_t * memregion)
      {
        return PAMI_UNIMPL;
      }

      inline pami_result_t memregion_deregister (pami_memregion_t * memregion)
      {
        return PAMI_UNIMPL;
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
        return PAMI_UNIMPL;
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

      inline pami_result_t geometry_initialize (pami_geometry_t       * geometry,
                                               unsigned               id,
                                               pami_geometry_range_t * rank_slices,
                                               size_t                 slice_count)
      {
        return PAMI_UNIMPL;
      }

      inline pami_result_t geometry_world (pami_geometry_t * world_geometry)
      {
        return PAMI_UNIMPL;
      }

      inline pami_result_t geometry_finalize (pami_geometry_t geometry)
      {
        return PAMI_UNIMPL;
      }

      inline pami_result_t geometry_algorithms_num_impl (pami_geometry_t geometry,
                                                        pami_xfer_type_t ctype,
                                                        int *lists_lengths)
      {
        return PAMI_UNIMPL;
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
        PAMI_abort();
        return PAMI_SUCCESS;
      }

      inline pami_result_t collective (pami_xfer_t * parameters)
      {
        return PAMI_UNIMPL;
      }

    inline pami_result_t amcollective_dispatch_impl (pami_algorithm_t            algorithm,
                                                    size_t                     dispatch,
                                                    pami_dispatch_callback_fn   fn,
                                                    void                     * cookie,
                                                    pami_collective_hint_t      options)
      {
        PAMI_abort();
        return PAMI_SUCCESS;
      }


      inline pami_result_t dispatch_impl (size_t                     id,
                                         pami_dispatch_callback_fn   fn,
                                         void                     * cookie,
                                         pami_send_hint_t            options)
      {
        pami_result_t result = PAMI_ERROR;
        size_t index = (size_t) id;
        TRACE_ERR((stderr, ">> dispatch_impl(), _dispatch[%zu] = %p\n", index, _dispatch[index]));

        if (_dispatch[index] == NULL)
          {
            TRACE_ERR((stderr, "   dispatch_impl(), before protocol init\n"));

            if (options.no_long_header == 1)
              {
                _dispatch[id] = _protocol.allocateObject ();
                new (_dispatch[id])
//                Protocol::Send::Datagram <ShmemModel, ShmemDevice, false>
//                Protocol::Send::Adaptive <ShmemModel, ShmemDevice, false>
                Protocol::Send::Eager <ShmemModel, ShmemDevice, false>
                (id, fn, cookie, ShmemDevice::Factory::getDevice(_devices->_shmem, _clientid, _contextid), result);
              }
            else
              {
                _dispatch[id] = _protocol.allocateObject ();
                new (_dispatch[id])
                Protocol::Send::Eager <ShmemModel, ShmemDevice, true>
//                Protocol::Send::Adaptive <ShmemModel, ShmemDevice, true>
//                Protocol::Send::Datagram <ShmemModel, ShmemDevice, true>
                (id, fn, cookie, ShmemDevice::Factory::getDevice(_devices->_shmem, _clientid, _contextid), result);
              }

            TRACE_ERR((stderr, "   dispatch_impl(),  after protocol init, result = %zu\n", result));

            if (result != PAMI_SUCCESS)
              {
                _protocol.returnObject (_dispatch[id]);
                _dispatch[id] = NULL;
              }
          }

        TRACE_ERR((stderr, "<< dispatch_impl(), result = %zu, _dispatch[%zu] = %p\n", result, index, _dispatch[index]));
        return result;
      }

      inline pami_result_t dispatch_new_impl (size_t                     id,
                                             pami_dispatch_callback_fn   fn,
                                             void                     * cookie,
                                             pami_dispatch_hint_t        options)
      {
        pami_result_t result        = PAMI_ERROR;
        if(options.type == PAMI_P2P_SEND)
        {
          return dispatch_impl (id,
                                fn,
                                cookie,
                                options.hint.send);
        }
          //TRACE_ERR((stderr, "<< dispatch_new_impl(), result = %zu, _dispatch[%zu] = %p\n", result, index, _dispatch[index]));
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
        return PAMI_UNIMPL;
      };


      inline pami_result_t manytomany_impl(pami_manytomany_t *m2minfo)
      {
        return PAMI_UNIMPL;
      };


      inline pami_result_t multisync_impl(pami_multisync_t *msyncinfo)
      {
        return PAMI_UNIMPL;
      };


      inline pami_result_t multicombine_impl(pami_multicombine_t *mcombineinfo)
      {
        return PAMI_UNIMPL;
      };



    private:

      pami_client_t  _client;
      pami_context_t _context;
      size_t        _clientid;
      size_t        _contextid;

      PAMI::Memory::MemoryManager _mm;
      SysDep _sysdep;

      // devices...
      ContextLock _lock;

      void * _dispatch[1024];
      ProtocolAllocator _protocol;
      PlatformDeviceList *_devices;

      PAMI::Device::Generic::Device * _local_generic_device;
  }; // end PAMI::Context
}; // end namespace PAMI

#undef TRACE_ERR

#endif // __components_context_bgp_bgpcontext_h__
