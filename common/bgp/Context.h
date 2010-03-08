///
/// \file common/bgp/Context.h
/// \brief XMI BGP specific context implementation.
///
#ifndef __common_bgp_Context_h__
#define __common_bgp_Context_h__

#include <stdlib.h>
#include <string.h>

#include "sys/xmi.h"
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

namespace XMI
{
  typedef XMI::Mutex::CounterMutex<XMI::Counter::GccProcCounter>  ContextLock;

  typedef Fifo::FifoPacket <16, 256> ShmemPacket;
  typedef Fifo::LinearFifo<Counter::LockBoxProcCounter, ShmemPacket, 128> ShmemFifo;
  //typedef Fifo::LinearFifo<Atomic::GccBuiltin, ShmemPacket, 128> ShmemFifo;
  typedef Device::ShmemDevice<ShmemFifo> ShmemDevice;
  typedef Device::ShmemPacketModel<ShmemDevice> ShmemModel;

  //
  // >> Point-to-point protocol typedefs and dispatch registration.
  //typedef XMI::Protocol::Send::Eager <ShmemModel, ShmemDevice> EagerShmem;
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
    inline xmi_result_t generate(size_t clientid, size_t num_ctx, Memory::MemoryManager &mm) {
	// these calls create (allocate and construct) each element.
	// We don't know how these relate to contexts, they are semi-opaque.
        _generics = XMI::Device::Generic::Device::Factory::generate(clientid, num_ctx, mm);
        _shmem = ShmemDevice::Factory::generate(clientid, num_ctx, mm);
	_progfunc = XMI::Device::ProgressFunctionDev::Factory::generate(clientid, num_ctx, mm);
	_atombarr = XMI::Device::AtomicBarrierDev::Factory::generate(clientid, num_ctx, mm);
	_wqringreduce = XMI::Device::WQRingReduceDev::Factory::generate(clientid, num_ctx, mm);
	_wqringbcast = XMI::Device::WQRingBcastDev::Factory::generate(clientid, num_ctx, mm);
	_localallreduce = XMI::Device::LocalAllreduceWQDevice::Factory::generate(clientid, num_ctx, mm);
	_localbcast = XMI::Device::LocalBcastWQDevice::Factory::generate(clientid, num_ctx, mm);
	_localreduce = XMI::Device::LocalReduceWQDevice::Factory::generate(clientid, num_ctx, mm);
	// BGP-specific devices...
	_gibarr = XMI::Device::BGP::giDevice::Factory::generate(clientid, num_ctx, mm);
	_cnallred = XMI::Device::BGP::CNAllreduceDevice::Factory::generate(clientid, num_ctx, mm);
	_cnppallred = XMI::Device::BGP::CNAllreducePPDevice::Factory::generate(clientid, num_ctx, mm);
	_cn2pallred = XMI::Device::BGP::CNAllreduce2PDevice::Factory::generate(clientid, num_ctx, mm);
	_cnbcast = XMI::Device::BGP::CNBroadcastDevice::Factory::generate(clientid, num_ctx, mm);
	return XMI_SUCCESS;
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
    inline xmi_result_t init(size_t clientid, size_t contextid, xmi_client_t clt, xmi_context_t ctx, XMI::SysDep *sd) {
	XMI::Device::Generic::Device::Factory::init(_generics, clientid, contextid, clt, ctx, sd, _generics);
	ShmemDevice::Factory::init(_shmem, clientid, contextid, clt, ctx, sd, _generics);
	XMI::Device::ProgressFunctionDev::Factory::init(_progfunc, clientid, contextid, clt, ctx, sd, _generics);
	XMI::Device::AtomicBarrierDev::Factory::init(_atombarr, clientid, contextid, clt, ctx, sd, _generics);
	XMI::Device::WQRingReduceDev::Factory::init(_wqringreduce, clientid, contextid, clt, ctx, sd, _generics);
	XMI::Device::WQRingBcastDev::Factory::init(_wqringbcast, clientid, contextid, clt, ctx, sd, _generics);
	XMI::Device::LocalAllreduceWQDevice::Factory::init(_localallreduce, clientid, contextid, clt, ctx, sd, _generics);
	XMI::Device::LocalBcastWQDevice::Factory::init(_localbcast, clientid, contextid, clt, ctx, sd, _generics);
	XMI::Device::LocalReduceWQDevice::Factory::init(_localreduce, clientid, contextid, clt, ctx, sd, _generics);
	XMI::Device::BGP::giDevice::Factory::init(_gibarr, clientid, contextid, clt, ctx, sd, _generics);
	XMI::Device::BGP::CNAllreduceDevice::Factory::init(_cnallred, clientid, contextid, clt, ctx, sd, _generics);
	XMI::Device::BGP::CNAllreducePPDevice::Factory::init(_cnppallred, clientid, contextid, clt, ctx, sd, _generics);
	XMI::Device::BGP::CNAllreduce2PDevice::Factory::init(_cn2pallred, clientid, contextid, clt, ctx, sd, _generics);
	XMI::Device::BGP::CNBroadcastDevice::Factory::init(_cnbcast, clientid, contextid, clt, ctx, sd, _generics);
	return XMI_SUCCESS;
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
        events += XMI::Device::Generic::Device::Factory::advance(_generics, clientid, contextid);
        events += ShmemDevice::Factory::advance(_shmem, clientid, contextid);
        events += XMI::Device::ProgressFunctionDev::Factory::advance(_progfunc, clientid, contextid);
	events += XMI::Device::AtomicBarrierDev::Factory::advance(_atombarr, clientid, contextid);
	events += XMI::Device::WQRingReduceDev::Factory::advance(_wqringreduce, clientid, contextid);
	events += XMI::Device::WQRingBcastDev::Factory::advance(_wqringbcast, clientid, contextid);
	events += XMI::Device::LocalAllreduceWQDevice::Factory::advance(_localallreduce, clientid, contextid);
	events += XMI::Device::LocalBcastWQDevice::Factory::advance(_localbcast, clientid, contextid);
	events += XMI::Device::LocalReduceWQDevice::Factory::advance(_localreduce, clientid, contextid);
	// BGP-specific devices...
	events += XMI::Device::BGP::giDevice::Factory::advance(_gibarr, clientid, contextid);
	events += XMI::Device::BGP::CNAllreduceDevice::Factory::advance(_cnallred, clientid, contextid);
	events += XMI::Device::BGP::CNAllreducePPDevice::Factory::advance(_cnppallred, clientid, contextid);
	events += XMI::Device::BGP::CNAllreduce2PDevice::Factory::advance(_cn2pallred, clientid, contextid);
	events += XMI::Device::BGP::CNBroadcastDevice::Factory::advance(_cnbcast, clientid, contextid);
	return events;
    }

    XMI::Device::Generic::Device *_generics; // need better name...
    ShmemDevice *_shmem;
    XMI::Device::ProgressFunctionDev *_progfunc;
    XMI::Device::AtomicBarrierDev *_atombarr;
    XMI::Device::WQRingReduceDev *_wqringreduce;
    XMI::Device::WQRingBcastDev *_wqringbcast;;
    XMI::Device::LocalAllreduceWQDevice *_localallreduce;
    XMI::Device::LocalBcastWQDevice *_localbcast;
    XMI::Device::LocalReduceWQDevice *_localreduce;
    // BGP-specific devices...
    XMI::Device::BGP::giDevice *_gibarr;
    XMI::Device::BGP::CNAllreduceDevice *_cnallred;
    XMI::Device::BGP::CNAllreducePPDevice *_cnppallred;
    XMI::Device::BGP::CNAllreduce2PDevice *_cn2pallred;
    XMI::Device::BGP::CNBroadcastDevice *_cnbcast;
  }; // class PlatformDeviceList


  class Context : public Interface::Context<XMI::Context>
  {
    public:
      inline Context (xmi_client_t client, size_t clientid, size_t id, size_t num,
      				PlatformDeviceList *devices,
				void * addr, size_t bytes) :
          Interface::Context<XMI::Context> (client, id),
          _client (client),
          _context ((xmi_context_t)this),
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

        _lock.init(&_sysdep);
	_devices->init(_clientid, _contextid, _client, _context, &_sysdep);

        // dispatch_impl relies on the table being initialized to NULL's.
        memset(_dispatch, 0x00, sizeof(_dispatch));
      }

      inline xmi_client_t getClient_impl ()
      {
        return _client;
      }

      inline size_t getId_impl ()
      {
        return _contextid;
      }

      inline xmi_result_t destroy_impl ()
      {
        return XMI_SUCCESS;
      }

      inline xmi_result_t post_impl (xmi_work_t *state, xmi_work_function work_fn, void * cookie)
      {
        XMI::Device::Generic::GenericThread *work;
	COMPILE_TIME_ASSERT(sizeof(*state) >= sizeof(*work));
	work = new (state) XMI::Device::Generic::GenericThread(work_fn, cookie);
	work->setStatus(XMI::Device::OneShot);
	_devices->_generics[_contextid].postThread(work);
        return XMI_SUCCESS;
      }

      inline size_t advance_impl (size_t maximum, xmi_result_t & result)
      {
//          result = XMI_EAGAIN;
        result = XMI_SUCCESS;
        size_t events = 0;

        unsigned i;

        for (i = 0; i < maximum && events == 0; i++)
          {
            events += _devices->advance(_clientid, _contextid);
          }

        //if (events > 0) result = XMI_SUCCESS;

        return events;
      }

      inline xmi_result_t lock_impl ()
      {
        TRACE_ERR((stderr, ">> lock_impl()\n"));
        _lock.acquire ();
        TRACE_ERR((stderr, "<< lock_impl()\n"));
        return XMI_SUCCESS;
      }

      inline xmi_result_t trylock_impl ()
      {
        TRACE_ERR((stderr, ">> trylock_impl()\n"));

        if (_lock.tryAcquire ())
          {
            TRACE_ERR((stderr, "<< trylock_impl(), XMI_SUCCESS\n"));
            return XMI_SUCCESS;
          }

        TRACE_ERR((stderr, "<< trylock_impl(), XMI_EAGAIN\n"));
        return XMI_EAGAIN;
      }

      inline xmi_result_t unlock_impl ()
      {
        TRACE_ERR((stderr, ">> release_impl()\n"));
        _lock.release ();
        TRACE_ERR((stderr, "<< release_impl()\n"));
        return XMI_SUCCESS;
      }

      inline xmi_result_t send_impl (xmi_send_t * parameters)
      {
        size_t id = (size_t)(parameters->send.dispatch);
        TRACE_ERR((stderr, ">> send_impl('simple'), _dispatch[%zd] = %p\n", id, _dispatch[id]));
        XMI_assert_debug (_dispatch[id] != NULL);

        XMI::Protocol::Send::Send * send =
          (XMI::Protocol::Send::Send *) _dispatch[id];
        xmi_result_t result = send->simple (parameters);

        TRACE_ERR((stderr, "<< send_impl('simple')\n"));
        return result;
      }

      inline xmi_result_t send_impl (xmi_send_immediate_t * parameters)
      {
        size_t id = (size_t)(parameters->dispatch);
        TRACE_ERR((stderr, ">> send_impl('immediate'), _dispatch[%zd] = %p\n", id, _dispatch[id]));
        XMI_assert_debug (_dispatch[id] != NULL);

        XMI::Protocol::Send::Send * send =
          (XMI::Protocol::Send::Send *) _dispatch[id];
        xmi_result_t result = send->immediate (parameters);

        TRACE_ERR((stderr, "<< send_impl('immediate')\n"));
        return result;
      }

      inline xmi_result_t send_impl (xmi_send_typed_t * parameters)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t put (xmi_put_simple_t * parameters)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t put_typed (xmi_put_typed_t * parameters)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t get (xmi_get_simple_t * parameters)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t get_typed (xmi_get_typed_t * parameters)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t rmw (xmi_rmw_t * parameters)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t memregion_register (void            * address,
                                              size_t            bytes,
                                              xmi_memregion_t * memregion)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t memregion_deregister (xmi_memregion_t * memregion)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t memregion_query (xmi_memregion_t    memregion,
                                           void            ** address,
                                           size_t           * bytes,
                                           size_t           * task)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t rput (xmi_rput_simple_t * parameters)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t rput_typed (xmi_rput_typed_t * parameters)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t rget (xmi_rget_simple_t * parameters)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t rget_typed (xmi_rget_typed_t * parameters)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t purge_totask (size_t *dest, size_t count)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t resume_totask (size_t *dest, size_t count)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t fence_begin ()
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t fence_end ()
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t fence_all (xmi_event_function   done_fn,
                                     void               * cookie)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t fence_task (xmi_event_function   done_fn,
                                      void               * cookie,
                                      size_t               task)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t geometry_initialize (xmi_geometry_t       * geometry,
                                               unsigned               id,
                                               xmi_geometry_range_t * rank_slices,
                                               size_t                 slice_count)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t geometry_world (xmi_geometry_t * world_geometry)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t geometry_finalize (xmi_geometry_t geometry)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t geometry_algorithms_num_impl (xmi_geometry_t geometry,
                                                        xmi_xfer_type_t ctype,
                                                        int *lists_lengths)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t geometry_algorithms_info_impl (xmi_geometry_t geometry,
                                                           xmi_xfer_type_t colltype,
                                                       xmi_algorithm_t  *algs0,
                                                       xmi_metadata_t   *mdata0,
                                                       int               num0,
                                                       xmi_algorithm_t  *algs1,
                                                       xmi_metadata_t   *mdata1,
                                                       int               num1)
      {
	XMI_abort();
	return XMI_SUCCESS;
      }

      inline xmi_result_t collective (xmi_xfer_t * parameters)
      {
        return XMI_UNIMPL;
      }

    inline xmi_result_t amcollective_dispatch_impl (xmi_algorithm_t            algorithm,
                                                    size_t                     dispatch,
                                                    xmi_dispatch_callback_fn   fn,
                                                    void                     * cookie,
                                                    xmi_collective_hint_t      options)
      {
	XMI_abort();
	return XMI_SUCCESS;
      }


      inline xmi_result_t dispatch_impl (size_t                     id,
                                         xmi_dispatch_callback_fn   fn,
                                         void                     * cookie,
                                         xmi_send_hint_t            options)
      {
        xmi_result_t result = XMI_ERROR;
        size_t index = (size_t) id;
        TRACE_ERR((stderr, ">> dispatch_impl(), _dispatch[%zd] = %p\n", index, _dispatch[index]));

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

            TRACE_ERR((stderr, "   dispatch_impl(),  after protocol init, result = %zd\n", result));

            if (result != XMI_SUCCESS)
              {
                _protocol.returnObject (_dispatch[id]);
                _dispatch[id] = NULL;
              }
          }

        TRACE_ERR((stderr, "<< dispatch_impl(), result = %zd, _dispatch[%zd] = %p\n", result, index, _dispatch[index]));
        return result;
      }

      inline xmi_result_t dispatch_new_impl (size_t                     id,
                                             xmi_dispatch_callback_fn   fn,
                                             void                     * cookie,
                                             xmi_dispatch_hint_t        options)
      {
        xmi_result_t result        = XMI_ERROR;
        if(options.type == XMI_P2P_SEND)
        {
          return dispatch_impl (id,
                                fn,
                                cookie,
                                options.hint.send);
        }
          TRACE_ERR((stderr, "<< dispatch_new_impl(), result = %zd, _dispatch[%zd] = %p\n", result, index, _dispatch[index]));
          return result;
      }

      inline xmi_result_t multisend_getroles_impl(size_t          dispatch,
                                                  int            *numRoles,
                                                  int            *replRole)
      {
        return XMI_UNIMPL;
      };

      inline xmi_result_t multicast_impl(xmi_multicast_t *mcastinfo)
      {
        return XMI_UNIMPL;
      };


      inline xmi_result_t manytomany_impl(xmi_manytomany_t *m2minfo)
      {
        return XMI_UNIMPL;
      };


      inline xmi_result_t multisync_impl(xmi_multisync_t *msyncinfo)
      {
        return XMI_UNIMPL;
      };


      inline xmi_result_t multicombine_impl(xmi_multicombine_t *mcombineinfo)
      {
        return XMI_UNIMPL;
      };



    private:

      xmi_client_t  _client;
      xmi_context_t _context;
      size_t        _clientid;
      size_t        _contextid;

      XMI::Memory::MemoryManager _mm;
      SysDep _sysdep;

      // devices...
      ContextLock _lock;

      void * _dispatch[1024];
      ProtocolAllocator _protocol;
      PlatformDeviceList *_devices;

  }; // end XMI::Context
}; // end namespace XMI

#undef TRACE_ERR

#endif // __components_context_bgp_bgpcontext_h__
