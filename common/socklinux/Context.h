///
/// \file common/socklinux/Context.h
/// \brief XMI Sockets on Linux specific context implementation.
///
#ifndef __common_socklinux_Context_h__
#define __common_socklinux_Context_h__

#define ENABLE_SHMEM_DEVICE
//#define ENABLE_UDP_DEVICE

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

#include "p2p/protocols/get/Get.h"
#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif

namespace XMI
{
  //typedef XMI::Mutex::CounterMutex<XMI::Counter::GccProcCounter>  ContextLock;
#ifdef ENABLE_UDP_DEVICE
  typedef Device::UDP::UdpDevice<SysDep> UdpDevice;
  typedef Device::UDP::UdpModel<UdpDevice,Device::UDP::UdpSendMessage> UdpModel;
  typedef XMI::Protocol::Send::Datagram < UdpModel, UdpDevice > DatagramUdp;
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
    inline xmi_result_t generate(size_t clientid, size_t num_ctx, Memory::MemoryManager &mm) {
       // these calls create (allocate and construct) each element.
       // We don't know how these relate to contexts, they are semi-opaque.
        _generics = XMI::Device::Generic::Device::Factory::generate(clientid, num_ctx, mm);
#ifdef ENABLE_SHMEM_DEVICE
        _shmem = ShmemDevice::Factory::generate(clientid, num_ctx, mm);
#endif
#ifdef ENABLE_UDP_DEVICE
        _udp = UdpDevice::generate(clientid, num_ctx, mm);
#endif
	_progfunc = XMI::Device::ProgressFunctionDev::Factory::generate(clientid, num_ctx, mm);
	_atombarr = XMI::Device::AtomicBarrierDev::Factory::generate(clientid, num_ctx, mm);
	_wqringreduce = XMI::Device::WQRingReduceDev::Factory::generate(clientid, num_ctx, mm);
	_wqringbcast = XMI::Device::WQRingBcastDev::Factory::generate(clientid, num_ctx, mm);
	_localallreduce = XMI::Device::LocalAllreduceWQDevice::Factory::generate(clientid, num_ctx, mm);
	_localbcast = XMI::Device::LocalBcastWQDevice::Factory::generate(clientid, num_ctx, mm);
	_localreduce = XMI::Device::LocalReduceWQDevice::Factory::generate(clientid, num_ctx, mm);
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
     * \param[in] sd           SysDep object
     * \param[in] clientid     Client ID (index)
     * \param[in] num_ctx      Number of contexts in this client
     * \param[in] ctx          Context opaque entity
     * \param[in] contextid    Context ID (index)
     */
    inline xmi_result_t init(size_t clientid, size_t contextid, xmi_client_t clt, xmi_context_t ctx, XMI::SysDep *sd) {
	XMI::Device::Generic::Device::Factory::init(_generics, clientid, contextid, clt, ctx, sd, _generics);
#ifdef ENABLE_SHMEM_DEVICE
	ShmemDevice::Factory::init(_shmem, clientid, contextid, clt, ctx, sd, _generics);
#endif
#ifdef ENABLE_UDP_DEVICE
	UdpDevice::Factory::init(_udp, clientid, contextid, clt, ctx, sd, _generics);
#endif
	XMI::Device::ProgressFunctionDev::Factory::init(_progfunc, clientid, contextid, clt, ctx, sd, _generics);
	XMI::Device::AtomicBarrierDev::Factory::init(_atombarr, clientid, contextid, clt, ctx, sd, _generics);
	XMI::Device::WQRingReduceDev::Factory::init(_wqringreduce, clientid, contextid, clt, ctx, sd, _generics);
	XMI::Device::WQRingBcastDev::Factory::init(_wqringbcast, clientid, contextid, clt, ctx, sd, _generics);
	XMI::Device::LocalAllreduceWQDevice::Factory::init(_localallreduce, clientid, contextid, clt, ctx, sd, _generics);
	XMI::Device::LocalBcastWQDevice::Factory::init(_localbcast, clientid, contextid, clt, ctx, sd, _generics);
	XMI::Device::LocalReduceWQDevice::Factory::init(_localreduce, clientid, contextid, clt, ctx, sd, _generics);
	return XMI_SUCCESS;
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
        events += XMI::Device::Generic::Device::Factory::advance(_generics, clientid, contextid);
#ifdef ENABLE_SHMEM_DEVICE
        events += ShmemDevice::Factory::advance(_shmem, clientid, contextid);
#endif
#ifdef ENABLE_UDP_DEVICE
        events += UdpDevice::Factory::advance(_shmem, clientid, contextid);
#endif
        events += XMI::Device::ProgressFunctionDev::Factory::advance(_progfunc, clientid, contextid);
	events += XMI::Device::AtomicBarrierDev::Factory::advance(_atombarr, clientid, contextid);
	events += XMI::Device::WQRingReduceDev::Factory::advance(_wqringreduce, clientid, contextid);
	events += XMI::Device::WQRingBcastDev::Factory::advance(_wqringbcast, clientid, contextid);
	events += XMI::Device::LocalAllreduceWQDevice::Factory::advance(_localallreduce, clientid, contextid);
	events += XMI::Device::LocalBcastWQDevice::Factory::advance(_localbcast, clientid, contextid);
	events += XMI::Device::LocalReduceWQDevice::Factory::advance(_localreduce, clientid, contextid);
	return events;
    }

    XMI::Device::Generic::Device *_generics; // need better name...
#ifdef ENABLE_SHMEM_DEVICE
    ShmemDevice *_shmem;
#endif
#ifdef ENABLE_UDP_DEVICE
    UdpDevice *_udp;
#endif
    XMI::Device::ProgressFunctionDev *_progfunc;
    XMI::Device::AtomicBarrierDev *_atombarr;
    XMI::Device::WQRingReduceDev *_wqringreduce;
    XMI::Device::WQRingBcastDev *_wqringbcast;;
    XMI::Device::LocalAllreduceWQDevice *_localallreduce;
    XMI::Device::LocalBcastWQDevice *_localbcast;
    XMI::Device::LocalReduceWQDevice *_localreduce;
  }; // class PlatformDeviceList

  class Context : public Interface::Context<XMI::Context>
  {
    public:
      inline Context (xmi_client_t client, size_t clientid, size_t contextid, size_t num,
                      PlatformDeviceList *devices, void * addr, size_t bytes) :
          Interface::Context<XMI::Context> (client, contextid),
          _client (client),
          _context ((xmi_context_t)this),
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
	_devices->init(_clientid, _contextid, _client, _context, &_sysdep);
        TRACE_ERR((stderr, "<< Context::Context()\n"));
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
        //return XMI_UNIMPL;
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

        //std::cout << "<" << __global.mapping.task() << ">: advance  max= " << maximum << std::endl;
        for (i = 0; i < maximum && events == 0; i++)
          {
		events += _devices->advance(_clientid, _contextid);
          }
        //std::cout << "<" << __global.mapping.task() << ">: advance  events= " << events << std::endl;

        if (events > 0) result = XMI_SUCCESS;

        return events;
      }

      inline xmi_result_t lock_impl ()
      {
        //_lock.acquire ();
	return XMI_SUCCESS;
      }

      inline xmi_result_t trylock_impl ()
      {
        //if (_lock.tryAcquire ()) {
		return XMI_SUCCESS;
	//}
	//return XMI_EAGAIN;
      }

      inline xmi_result_t unlock_impl ()
      {
        //_lock.release ();
	return XMI_SUCCESS;
      }

      inline xmi_result_t send_impl (xmi_send_t * parameters)
      {
        size_t id = (size_t)(parameters->send.dispatch);
        TRACE_ERR((stderr, ">> send_impl('simple'), _dispatch[%zd] = %p\n", id, _dispatch[id]));
        XMI_assert_debug (_dispatch[id] != NULL);

        XMI::Protocol::Send::Send * send =
          (XMI::Protocol::Send::Send *) _dispatch[id];
        send->simple (parameters);

        TRACE_ERR((stderr, "<< send_impl('simple')\n"));
        return XMI_SUCCESS;
      }

      inline xmi_result_t send_impl (xmi_send_immediate_t * parameters)
      {
        size_t id = (size_t)(parameters->dispatch);
        TRACE_ERR((stderr, ">> send_impl('immediate'), _dispatch[%zd] = %p\n", id, _dispatch[id]));
        XMI_assert_debug (_dispatch[id] != NULL);

        XMI::Protocol::Send::Send * send =
          (XMI::Protocol::Send::Send *) _dispatch[id];
        send->immediate (parameters);

        TRACE_ERR((stderr, "<< send_impl('immediate')\n"));
        return XMI_SUCCESS;
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

      inline xmi_result_t dispatch_impl (size_t                     id,
                                         xmi_dispatch_callback_fn   fn,
                                         void                     * cookie,
                                         xmi_send_hint_t            options)
      {
        xmi_result_t result = XMI_ERROR;
TRACE_ERR((stderr, ">> socklinux::dispatch_impl .. _dispatch[%zu] = %p, result = %d\n", id, _dispatch[id], result));

        if (_dispatch[id] == NULL)
          {
            // Allocate memory for the protocol object.
            _dispatch[id] = (void *) _protocolAllocator.allocateObject ();
#ifdef ENABLE_UDP_DEVICE
            COMPILE_TIME_ASSERT(sizeof(DatagramUdp) <= ProtocolAllocator::objsize);
            new ((void *)_dispatch[id])
              DatagramUdp
                (id, fn, cookie, _devices->_udp[_contextid], result);
#else
#ifdef ENABLE_SHMEM_DEVICE
            COMPILE_TIME_ASSERT(sizeof(Protocol::Send::Eager <ShmemModel, ShmemDevice, true>) <= ProtocolAllocator::objsize);
            new ((void *)_dispatch[id])
              Protocol::Send::Eager <ShmemModel, ShmemDevice, true>
                (id, fn, cookie, _devices->_shmem[_contextid], result);
#endif
#endif
            if (result != XMI_SUCCESS)
              {
                _protocolAllocator.returnObject (_dispatch[id]);
                _dispatch[id] = NULL;
              }
          }
TRACE_ERR((stderr, "<< socklinux::dispatch_impl .. result = %d\n", result));
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
        return result;
    }

      inline xmi_result_t multisend_getroles(size_t          dispatch,
                                             int            *numRoles,
                                             int            *replRole)
      {
        return XMI_UNIMPL;
      };

      inline xmi_result_t multicast(xmi_multicast_t *mcastinfo)
      {
        return XMI_UNIMPL;
      };


      inline xmi_result_t manytomany(xmi_manytomany_t *m2minfo)
      {
        return XMI_UNIMPL;
      };


      inline xmi_result_t multisync(xmi_multisync_t *msyncinfo)
      {
        return XMI_UNIMPL;
      };


      inline xmi_result_t multicombine(xmi_multicombine_t *mcombineinfo)
      {
        return XMI_UNIMPL;
      };



    private:

      xmi_client_t  _client;
      xmi_context_t _context;
      size_t        _clientid;
      size_t        _contextid;

      XMI::Memory::MemoryManager _mm;  // TODO why do I have to do this for sys dep?
      SysDep _sysdep;

      void * _dispatch[1024];
      //void* _get; //use for now..remove later
      MemoryAllocator<1024, 16> _request;


      //ContextLock _lock;
      ProtocolAllocator _protocolAllocator;
      PlatformDeviceList *_devices;

  }; // end XMI::Context
}; // end namespace XMI

#endif // __common_socklinux_Context_h__
