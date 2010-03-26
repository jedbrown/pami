///
/// \file common/socklinux/Context.h
/// \brief PAMI Sockets on Linux specific context implementation.
///
#ifndef __common_socklinux_Context_h__
#define __common_socklinux_Context_h__

#define ENABLE_SHMEM_DEVICE
#define ENABLE_UDP_DEVICE

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

#ifdef ENABLE_UDP_DEVICE
#include "components/devices/udp/UdpDevice.h"
#include "components/devices/udp/UdpModel.h"
#include "components/devices/udp/UdpMessage.h"
#endif

#ifdef ENABLE_SHMEM_DEVICE
#include "components/devices/shmem/ShmemDevice.h"
#include "components/devices/shmem/ShmemPacketModel.h"
#include "util/fifo/FifoPacket.h"
#include "util/fifo/LinearFifo.h"
#endif

#include "components/atomic/gcc/GccBuiltin.h"
//#include "components/atomic/pthread/Pthread.h"

#include "components/memory/MemoryAllocator.h"

#include "SysDep.h"
//#include "Memregion.h"

#include "p2p/protocols/send/eager/Eager.h"
#include "p2p/protocols/send/datagram/Datagram.h"
#include "p2p/protocols/send/composite/Composite.h"

#include "p2p/protocols/get/Get.h"
#ifndef TRACE_ERR
#define TRACE_ERR(x) // fprintf x
#endif

namespace PAMI
{
  //typedef PAMI::Mutex::CounterMutex<PAMI::Counter::GccProcCounter>  ContextLock;
#ifdef ENABLE_UDP_DEVICE
  typedef Device::UDP::UdpDevice<SysDep> UdpDevice;
  typedef Device::UDP::UdpModel<UdpDevice,Device::UDP::UdpSendMessage> UdpModel;
  typedef PAMI::Protocol::Send::Datagram < UdpModel, UdpDevice > DatagramUdp;
#endif

#ifdef ENABLE_SHMEM_DEVICE
  typedef Fifo::FifoPacket <16, 240> ShmemPacket;
  typedef Fifo::LinearFifo<Atomic::GccBuiltin, ShmemPacket, 128> ShmemFifo;
  typedef Device::ShmemDevice<ShmemFifo> ShmemDevice;
  typedef Device::Shmem::PacketModel<ShmemDevice> ShmemModel;
#endif

  typedef MemoryAllocator<1152, 16> ProtocolAllocator;

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
     * \param[in] clientid     Client ID (index)
     * \param[in] contextid    Context ID (index)
     */
    inline pami_result_t generate(size_t clientid, size_t num_ctx, Memory::MemoryManager &mm) {
       // these calls create (allocate and construct) each element.
       // We don't know how these relate to contexts, they are semi-opaque.
        _generics = PAMI::Device::Generic::Device::Factory::generate(clientid, num_ctx, mm);
#ifdef ENABLE_SHMEM_DEVICE
        _shmem = ShmemDevice::Factory::generate(clientid, num_ctx, mm);
#endif
#ifdef ENABLE_UDP_DEVICE
        _udp = UdpDevice::Factory::generate(clientid, num_ctx, mm);
#endif
        _progfunc = PAMI::Device::ProgressFunctionDev::Factory::generate(clientid, num_ctx, mm);
        _atombarr = PAMI::Device::AtomicBarrierDev::Factory::generate(clientid, num_ctx, mm);
        _wqringreduce = PAMI::Device::WQRingReduceDev::Factory::generate(clientid, num_ctx, mm);
        _wqringbcast = PAMI::Device::WQRingBcastDev::Factory::generate(clientid, num_ctx, mm);
        _localallreduce = PAMI::Device::LocalAllreduceWQDevice::Factory::generate(clientid, num_ctx, mm);
        _localbcast = PAMI::Device::LocalBcastWQDevice::Factory::generate(clientid, num_ctx, mm);
        _localreduce = PAMI::Device::LocalReduceWQDevice::Factory::generate(clientid, num_ctx, mm);
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
    inline pami_result_t init(size_t clientid, size_t contextid, pami_client_t clt, pami_context_t ctx, PAMI::Memory::MemoryManager *mm) {
        PAMI::Device::Generic::Device::Factory::init(_generics, clientid, contextid, clt, ctx, mm, _generics);
#ifdef ENABLE_SHMEM_DEVICE
        ShmemDevice::Factory::init(_shmem, clientid, contextid, clt, ctx, mm, _generics);
#endif
#ifdef ENABLE_UDP_DEVICE
        UdpDevice::Factory::init(_udp, clientid, contextid, clt, ctx, mm, _generics);
#endif
        PAMI::Device::ProgressFunctionDev::Factory::init(_progfunc, clientid, contextid, clt, ctx, mm, _generics);
        PAMI::Device::AtomicBarrierDev::Factory::init(_atombarr, clientid, contextid, clt, ctx, mm, _generics);
        PAMI::Device::WQRingReduceDev::Factory::init(_wqringreduce, clientid, contextid, clt, ctx, mm, _generics);
        PAMI::Device::WQRingBcastDev::Factory::init(_wqringbcast, clientid, contextid, clt, ctx, mm, _generics);
        PAMI::Device::LocalAllreduceWQDevice::Factory::init(_localallreduce, clientid, contextid, clt, ctx, mm, _generics);
        PAMI::Device::LocalBcastWQDevice::Factory::init(_localbcast, clientid, contextid, clt, ctx, mm, _generics);
        PAMI::Device::LocalReduceWQDevice::Factory::init(_localreduce, clientid, contextid, clt, ctx, mm, _generics);
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
    inline size_t advance(size_t clientid, size_t contextid) {
        size_t events = 0;
        events += PAMI::Device::Generic::Device::Factory::advance(_generics, clientid, contextid);
#ifdef ENABLE_SHMEM_DEVICE
        events += ShmemDevice::Factory::advance(_shmem, clientid, contextid);
#endif
#ifdef ENABLE_UDP_DEVICE
        events += UdpDevice::Factory::advance(_udp, clientid, contextid);
#endif
        events += PAMI::Device::ProgressFunctionDev::Factory::advance(_progfunc, clientid, contextid);
        events += PAMI::Device::AtomicBarrierDev::Factory::advance(_atombarr, clientid, contextid);
        events += PAMI::Device::WQRingReduceDev::Factory::advance(_wqringreduce, clientid, contextid);
        events += PAMI::Device::WQRingBcastDev::Factory::advance(_wqringbcast, clientid, contextid);
        events += PAMI::Device::LocalAllreduceWQDevice::Factory::advance(_localallreduce, clientid, contextid);
        events += PAMI::Device::LocalBcastWQDevice::Factory::advance(_localbcast, clientid, contextid);
        events += PAMI::Device::LocalReduceWQDevice::Factory::advance(_localreduce, clientid, contextid);
        return events;
    }

    PAMI::Device::Generic::Device *_generics; // need better name...
#ifdef ENABLE_SHMEM_DEVICE
    ShmemDevice *_shmem;
#endif
#ifdef ENABLE_UDP_DEVICE
    UdpDevice *_udp;
#endif
    PAMI::Device::ProgressFunctionDev *_progfunc;
    PAMI::Device::AtomicBarrierDev *_atombarr;
    PAMI::Device::WQRingReduceDev *_wqringreduce;
    PAMI::Device::WQRingBcastDev *_wqringbcast;;
    PAMI::Device::LocalAllreduceWQDevice *_localallreduce;
    PAMI::Device::LocalBcastWQDevice *_localbcast;
    PAMI::Device::LocalReduceWQDevice *_localreduce;
  }; // class PlatformDeviceList

  class Context : public Interface::Context<PAMI::Context>
  {
    public:
      inline Context (pami_client_t client, size_t clientid, size_t contextid, size_t num,
                      PlatformDeviceList *devices, void * addr, size_t bytes) :
          Interface::Context<PAMI::Context> (client, contextid),
          _client (client),
          _context ((pami_context_t)this),
          _clientid (clientid),
          _contextid (contextid),
          _mm (addr, bytes),
          _sysdep (_mm),
          _devices(devices)
      {
        TRACE_ERR((stderr, ">> Context::Context()\n"));
        // ----------------------------------------------------------------
        // Compile-time assertions
        // ----------------------------------------------------------------

        // ----------------------------------------------------------------
        // Compile-time assertions
        // ----------------------------------------------------------------
        _devices->init(_clientid, _contextid, _client, _context, &_mm);
        TRACE_ERR((stderr, "<< Context::Context()\n"));
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

        //std::cout << "<" << __global.mapping.task() << ">: advance  max= " << maximum << std::endl;
        for (i = 0; i < maximum && events == 0; i++)
          {
                events += _devices->advance(_clientid, _contextid);
          }
        //std::cout << "<" << __global.mapping.task() << ">: advance  events= " << events << std::endl;

        if (events > 0) result = PAMI_SUCCESS;

        return events;
      }

      inline pami_result_t lock_impl ()
      {
        //_lock.acquire ();
        return PAMI_SUCCESS;
      }

      inline pami_result_t trylock_impl ()
      {
        //if (_lock.tryAcquire ()) {
                return PAMI_SUCCESS;
        //}
        //return PAMI_EAGAIN;
      }

      inline pami_result_t unlock_impl ()
      {
        //_lock.release ();
        return PAMI_SUCCESS;
      }

      inline pami_result_t send_impl (pami_send_t * parameters)
      {
        size_t id = (size_t)(parameters->send.dispatch);
        TRACE_ERR((stderr, ">> send_impl('simple'), _dispatch[%zu] = %p\n", id, _dispatch[id]));
        PAMI_assert_debug (_dispatch[id] != NULL);

        PAMI::Protocol::Send::Send * send =
          (PAMI::Protocol::Send::Send *) _dispatch[id];
        send->simple (parameters);

        TRACE_ERR((stderr, "<< send_impl('simple')\n"));
        return PAMI_SUCCESS;
      }

      inline pami_result_t send_impl (pami_send_immediate_t * parameters)
      {
        size_t id = (size_t)(parameters->dispatch);
        TRACE_ERR((stderr, ">> send_impl('immediate'), _dispatch[%zu] = %p\n", id, _dispatch[id]));
        PAMI_assert_debug (_dispatch[id] != NULL);

        PAMI::Protocol::Send::Send * send =
          (PAMI::Protocol::Send::Send *) _dispatch[id];
        send->immediate (parameters);

        TRACE_ERR((stderr, "<< send_impl('immediate')\n"));
        return PAMI_SUCCESS;
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

      inline pami_result_t dispatch_impl (size_t                     id,
                                         pami_dispatch_callback_fn   fn,
                                         void                     * cookie,
                                         pami_send_hint_t            options)
      {
        pami_result_t result = PAMI_ERROR;
        TRACE_ERR((stderr, ">> socklinux::dispatch_impl .. _dispatch[%zu] = %p, result = %d\n", id, _dispatch[id], result));

        if (_dispatch[id] == NULL)
          {
            bool no_shmem  = options.no_shmem;
#ifndef ENABLE_SHMEM_DEVICE
            no_shmem = true;
#endif

            bool use_shmem = options.use_shmem;
#ifdef ENABLE_UDP_DEVICE
            use_shmem = true;
#endif

            if (no_shmem == 1)
            {
              // Register only the "udp" datagram protocol
              //
              // This udp datagram protocol code should be changed to respect the
              // "long header" option
              //
#ifdef ENABLE_UDP_DEVICE
              _dispatch[id] = (Protocol::Send::Send *)
                DatagramUdp::generate (id, fn, cookie, _devices->_udp[_contextid], _protocol, result);
#else
              PAMI_abortf("No non-shmem protocols available.");
#endif
            }
            else if (options.use_shmem == 1)
            {
              // Register only the "shmem" eager protocol
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
#if defined(ENABLE_SHMEM_DEVICE) && defined(ENABLE_UDP_DEVICE)
            else
            {
              // Register both the "udp" datagram and the "shmem" eager protocols
              //
              // This udp datagram protocol code should be changed to respect the
              // "long header" option
              //
              DatagramUdp * datagram =
                DatagramUdp::generate (id, fn, cookie, _devices->_udp[_contextid], _protocol, result);

              if (options.no_long_header == 1)
                {
                  Protocol::Send::Eager <ShmemModel, ShmemDevice, false> * eager =
                    Protocol::Send::Eager <ShmemModel, ShmemDevice, false>::
                      generate (id, fn, cookie, _devices->_shmem[_contextid], _protocol, result);

                  _dispatch[id] = (Protocol::Send::Send *) Protocol::Send::Factory::
                      generate (eager, datagram, _protocol, result);
                }
              else
                {
                  Protocol::Send::Eager <ShmemModel, ShmemDevice, true> * eager =
                    Protocol::Send::Eager <ShmemModel, ShmemDevice, true>::
                      generate (id, fn, cookie, _devices->_shmem[_contextid], _protocol, result);

                  _dispatch[id] = (Protocol::Send::Send *) Protocol::Send::Factory::
                      generate (eager, datagram, _protocol, result);
                }
            }
#endif
          }

        TRACE_ERR((stderr, "<< socklinux::dispatch_impl .. result = %d\n", result));
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
        return result;
    }

      inline pami_result_t multisend_getroles(size_t          dispatch,
                                             int            *numRoles,
                                             int            *replRole)
      {
        return PAMI_UNIMPL;
      };

      inline pami_result_t multicast(pami_multicast_t *mcastinfo)
      {
        return PAMI_UNIMPL;
      };


      inline pami_result_t manytomany(pami_manytomany_t *m2minfo)
      {
        return PAMI_UNIMPL;
      };


      inline pami_result_t multisync(pami_multisync_t *msyncinfo)
      {
        return PAMI_UNIMPL;
      };


      inline pami_result_t multicombine(pami_multicombine_t *mcombineinfo)
      {
        return PAMI_UNIMPL;
      };



    private:

      pami_client_t  _client;
      pami_context_t _context;
      size_t        _clientid;
      size_t        _contextid;

      PAMI::Memory::MemoryManager _mm;  // TODO why do I have to do this for sys dep?
      SysDep _sysdep;

      void * _dispatch[1024];
      //void* _get; //use for now..remove later
      MemoryAllocator<1024, 16> _request;


      //ContextLock _lock;
      ProtocolAllocator _protocol;
      PlatformDeviceList *_devices;

  }; // end PAMI::Context
}; // end namespace PAMI
#undef TRACE_ERR
#endif // __common_socklinux_Context_h__
