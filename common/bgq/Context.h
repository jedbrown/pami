///
/// \file common/bgq/Context.h
/// \brief XMI Blue Gene\Q specific context implementation.
///
#ifndef __common_bgq_Context_h__
#define __common_bgq_Context_h__

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

#include "components/devices/shmem/ShmemDevice.h"
#include "util/fifo/FifoPacket.h"
#include "util/fifo/LinearFifo.h"

#include "components/devices/bgq/mu/MUDevice.h"
#include "components/devices/bgq/mu/MUPacketModel.h"
#include "components/devices/bgq/mu/MUInjFifoMessage.h"

#include "components/atomic/gcc/GccBuiltin.h"
#include "components/atomic/l2/L2Counter.h"
//#include "components/atomic/pthread/Pthread.h"
//#include "components/atomic/bgq/BgqAtomic.h"

#include "components/memory/MemoryAllocator.h"

#include "SysDep.h"
#include "Memregion.h"

#include "p2p/protocols/send/eager/Eager.h"
#include "p2p/protocols/send/eager/EagerImmediate.h"
#include "p2p/protocols/send/eager/EagerSimple.h"

#include "p2p/protocols/get/Get.h"
#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif

#define MU_COLL_DEVICE
//#define MU_DEVICE

#ifdef MU_COLL_DEVICE
#include "components/devices/bgq/mu/MUCollDevice.h"
#include "components/devices/bgq/mu/MUMulticastModel.h"
#include "components/devices/bgq/mu/MUMultisyncModel.h"
#include "components/devices/bgq/mu/MUMulticombineModel.h"
#include "common/bgq/NativeInterface.h"
#endif

namespace XMI
{
#ifdef MU_COLL_DEVICE
  typedef Device::MU::MUCollDevice MUDevice;
  typedef BGQNativeInterface<MUDevice,
                             Device::MU::MUMulticastModel,
                             Device::MU::MUMultisyncModel,
                             Device::MU::MUMulticombineModel> MUGlobalNI;

  #define MU_DEVICE
#elif defined(MU_DEVICE)
  typedef Device::MU::MUDevice MUDevice;
#endif

  typedef XMI::Mutex::CounterMutex<XMI::Counter::GccProcCounter>  ContextLock;

  //typedef Fifo::FifoPacket <32, 992> ShmemPacket;
  typedef Fifo::FifoPacket <32, 512> ShmemPacket;
  typedef Fifo::LinearFifo<Atomic::L2Counter, ShmemPacket, 16> ShmemFifo;
  //typedef Fifo::LinearFifo<Atomic::GccBuiltin, ShmemPacket, 16> ShmemFifo;
  //typedef Device::Fifo::LinearFifo<Atomic::Pthread,ShmemPacket,16> ShmemFifo;
  //typedef Fifo::LinearFifo<Atomic::BgqAtomic,ShmemPacket,16> ShmemFifo;

  typedef Device::ShmemDevice<ShmemFifo> ShmemDevice;
  typedef Device::ShmemModel<ShmemDevice> ShmemModel;

  //
  // >> Point-to-point protocol typedefs and dispatch registration.
  typedef XMI::Protocol::Send::Eager <ShmemModel, ShmemDevice> EagerShmem;
  typedef XMI::Protocol::Send::Eager < XMI::Device::MU::MUPacketModel,MUDevice > EagerMu;
  // << Point-to-point protocol typedefs and dispatch registration.
  //

  typedef XMI::Protocol::Get::Get <ShmemModel, ShmemDevice> GetShmem;

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
    inline xmi_result_t init(size_t clientid, size_t num_ctx) {
       // these calls create (allocate and construct) each element.
       // We don't know how these relate to contexts, they are semi-opaque.
       _generics = XMI::Device::Generic::Device::create(clientid, num_ctx);
       _shmem = ShmemDevice::create(clientid, num_ctx, _generics);
       _progfunc = XMI::Device::ProgressFunctionDev::create(clientid, num_ctx, _generics);
       _atombarr = XMI::Device::AtomicBarrierDev::create(clientid, num_ctx, _generics);
       _wqringreduce = XMI::Device::WQRingReduceDev::create(clientid, num_ctx, _generics);
       _wqringbcast = XMI::Device::WQRingBcastDev::create(clientid, num_ctx, _generics);
       _localallreduce = XMI::Device::LocalAllreduceWQDevice::create(clientid, num_ctx, _generics);
       _localbcast = XMI::Device::LocalBcastWQDevice::create(clientid, num_ctx, _generics);
       _localreduce = XMI::Device::LocalReduceWQDevice::create(clientid, num_ctx, _generics);
#ifdef MU_DEVICE
       _mu = MUDevice::create(clientid, num_ctx, _generics);
#endif
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
    inline xmi_result_t dev_init(XMI::SysDep *sd, size_t clientid, size_t num_ctx, xmi_context_t ctx, size_t contextid) {
       _generics->init(ctx, clientid, contextid, num_ctx);
       _shmem->init(sd, clientid, num_ctx, ctx, contextid);
       _progfunc->init(sd, clientid, num_ctx, ctx, contextid);
       _atombarr->init(sd, clientid, num_ctx, ctx, contextid);
       _wqringreduce->init(sd, clientid, num_ctx, ctx, contextid);
       _wqringbcast->init(sd, clientid, num_ctx, ctx, contextid);
       _localallreduce->init(sd, clientid, num_ctx, ctx, contextid);
       _localbcast->init(sd, clientid, num_ctx, ctx, contextid);
       _localreduce->init(sd, clientid, num_ctx, ctx, contextid);
#ifdef MU_DEVICE
       _mu->init(sd, clientid, num_ctx, ctx, contextid);
#endif
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
       events += _generics->advance(clientid, contextid);
       events += _shmem->advance(clientid, contextid);
       events += _progfunc->advance(clientid, contextid);
       events += _atombarr->advance(clientid, contextid);
       events += _wqringreduce->advance(clientid, contextid);
       events += _wqringbcast->advance(clientid, contextid);
       events += _localallreduce->advance(clientid, contextid);
       events += _localbcast->advance(clientid, contextid);
       events += _localreduce->advance(clientid, contextid);
#ifdef MU_DEVICE
       events += _mu->advance(clientid, contextid);
#endif
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
#ifdef MU_DEVICE
    MUDevice *_mu;
#endif
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
	  _devices(devices)
      {
        // ----------------------------------------------------------------
        // Compile-time assertions
        // ----------------------------------------------------------------

        // Make sure the memory allocator is large enough for all
        // protocol classes.
        COMPILE_TIME_ASSERT(sizeof(EagerShmem) <= ProtocolAllocator::objsize);
        COMPILE_TIME_ASSERT(sizeof(EagerMu) <= ProtocolAllocator::objsize);
        COMPILE_TIME_ASSERT(sizeof(GetShmem) <= ProtocolAllocator::objsize);

        // ----------------------------------------------------------------
        // Compile-time assertions
        // ----------------------------------------------------------------

#ifdef MU_COLL_DEVICE
        // Can't construct NI until device is init()'d.  Ctor into member storage.
        _global_mu_ni = new (_global_mu_ni_storage) MUGlobalNI(_devices->_mu[_contextid], _client, _clientid, _context, _contextid);
        xmi_result_t status;
#endif
	_devices->dev_init(&_sysdep, _clientid, num, _context, _contextid);

        xmi_result_t result ;
#warning This should not be here?
	_get = (void *) _request.allocateObject ();
	new ((void *)_get) GetShmem(_devices->_shmem[_contextid], result);

        // dispatch_impl relies on the table being initialized to NULL's.
        memset(_dispatch, 0x00, sizeof(_dispatch));

      }
#ifdef MU_COLL_DEVICE
      // \brief For testing NativeInterface.
      inline MUDevice* getMu(){return &_devices->_mu[_contextid];}
#endif
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
	work = new (work) XMI::Device::Generic::GenericThread(work_fn, cookie);
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
        _lock.acquire ();
	return XMI_SUCCESS;
      }

      inline xmi_result_t trylock_impl ()
      {
        if (_lock.tryAcquire ()) {
		return XMI_SUCCESS;
	}
	return XMI_EAGAIN;
      }

      inline xmi_result_t unlock_impl ()
      {
        _lock.release ();
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
        ((GetShmem*)_get)->getimpl (	parameters->rma.done_fn,
                                parameters->rma.cookie,
                                parameters->rma.dest,
                                parameters->rget.bytes,
                                (Memregion*)parameters->rget.local_mr,
                                (Memregion*)parameters->rget.remote_mr,
                                parameters->rget.local_offset,
                                parameters->rget.remote_offset);
        return XMI_SUCCESS;
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

      inline xmi_result_t geometry_algorithms_num_impl (xmi_geometry_t geometry,
                                                        xmi_xfer_type_t ctype,
                                                        int *lists_lengths)
      {
        return XMI_UNIMPL;
      }

      inline
      xmi_result_t geometry_algorithms_info_impl (xmi_geometry_t geometry,
                                                  xmi_xfer_type_t colltype,
                                                  xmi_algorithm_t *algs,
                                                  xmi_metadata_t *mdata,
                                                  int algorithm_type,
                                                  int num)
        {
          return XMI_UNIMPL;

        }

      inline xmi_result_t dispatch_impl (size_t                     id,
                                         xmi_dispatch_callback_fn   fn,
                                         void                     * cookie,
                                         xmi_send_hint_t            options)
      {
        xmi_result_t result = XMI_ERROR;

        if (_dispatch[id] == NULL)
          {
            // Allocate memory for the protocol object.
            _dispatch[id] = (void *) _protocolAllocator.allocateObject ();

#warning Eager/ShmemModel needs to change to accept array of devices
#ifdef MU_DEVICE
            new ((void *)_dispatch[id]) EagerMu (id, fn, cookie, _devices->_mu[_contextid], result);
#else
            new ((void *)_dispatch[id]) EagerShmem (id, fn, cookie, _devices->_shmem[_contextid], result);
#endif
          }

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
#ifdef MU_COLL_DEVICE
      TRACE_ERR((stderr, ">> dispatch_new_impl multicast %zd\n", id));
      if (_dispatch[id] == NULL)
      {
        _dispatch[id] = (void *)_global_mu_ni; // Only have one multicast right now
        return _global_mu_ni->setDispatch(fn, cookie);
        TRACE_ERR((stderr, ">> dispatch_new_impl multicast %zd\n", id));
        XMI_assertf(_protocolAllocator.objsize >= sizeof(XMI::Device::MU::MUMulticastModel),"%zd >= %zd\n",_protocolAllocator.objsize,sizeof(XMI::Device::MU::MUMulticastModel));
        // Allocate memory for the protocol object.
        _dispatch[id] = (void *) _protocolAllocator.allocateObject ();

        XMI::Device::MU::MUMulticastModel * model = new ((void*)_dispatch[id]) XMI::Device::MU::MUMulticastModel(result, _devices->_mu[_contextid]);
        model->registerMcastRecvFunction(id, fn.multicast, cookie);

      }
#endif
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
#ifdef MU_COLL_DEVICE
        TRACE_ERR((stderr, ">> multicast_impl multicast %zd, %p\n", mcastinfo->dispatch, mcastinfo));
        CCMI::Interfaces::NativeInterface * ni = (CCMI::Interfaces::NativeInterface *) _dispatch[mcastinfo->dispatch];
        return ni->multicast(mcastinfo); // this version of ni allocates/frees our request storage for us.
#else
          return XMI_UNIMPL;
#endif
      };


      inline xmi_result_t manytomany(xmi_manytomany_t *m2minfo)
      {
        return XMI_UNIMPL;
      };


      inline xmi_result_t multisync(xmi_multisync_t *msyncinfo)
      {
#ifdef MU_COLL_DEVICE
        TRACE_ERR((stderr, ">> multisync_impl multisync %p\n", msyncinfo));
        return _global_mu_ni->multisync(msyncinfo); // Only have one multisync right now

#else
          return XMI_UNIMPL;
#endif
      };


      inline xmi_result_t multicombine(xmi_multicombine_t *mcombineinfo)
      {
#ifdef MU_COLL_DEVICE
        TRACE_ERR((stderr, ">> multicombine_impl multicombine %p\n", mcombineinfo));
        return _global_mu_ni->multicombine(mcombineinfo);// Only have one multicombine right now
#else
          return XMI_UNIMPL;
#endif
      };



    private:

      xmi_client_t  _client;
      xmi_context_t _context;
      size_t        _clientid;
      size_t        _contextid;

      XMI::Memory::MemoryManager _mm;
      SysDep _sysdep;

#ifdef MU_COLL_DEVICE
      MUGlobalNI * _global_mu_ni;
      uint8_t      _global_mu_ni_storage[sizeof(MUGlobalNI)];
#endif
      ShmemDevice          _shmem;

      void * _dispatch[1024];
      void* _get; //use for now..remove later
      MemoryAllocator<1024, 16> _request;
      ContextLock _lock;
      ProtocolAllocator _protocolAllocator;
      PlatformDeviceList *_devices;
  }; // end XMI::Context
}; // end namespace XMI

#endif // __components_context_bgq_bgqcontext_h__
