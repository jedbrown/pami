///
/// \file common/lapiunix/Context.h
/// \brief XMI LAPI specific context implementation.
///
#ifndef __common_lapiunix_Context_h__
#define __common_lapiunix_Context_h__

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "common/lapiunix/lapifunc.h"
#include "common/ContextInterface.h"
#include "Geometry.h"
#include "components/devices/lapiunix/lapiunixdevice.h"
#include "components/devices/lapiunix/lapiunixpacketmodel.h"
#include "components/devices/lapiunix/lapiunixmessage.h"
#include "p2p/protocols/send/eager/Eager.h"
#include "SysDep.h"
#include "common/default/CollFactory.h"
#include "common/default/CollRegistration.h"
#include "components/devices/generic/GenericDevice.h"
#include "components/devices/misc/ProgressFunctionMsg.h"
#include "components/devices/misc/AtomicBarrierMsg.h"
#include "components/devices/workqueue/WQRingReduceMsg.h"
#include "components/devices/workqueue/WQRingBcastMsg.h"
#include "components/devices/workqueue/LocalAllreduceWQMessage.h"
#include "components/devices/workqueue/LocalReduceWQMessage.h"
#include "components/devices/workqueue/LocalBcastWQMessage.h"
#include "components/atomic/counter/CounterMutex.h"
#include "components/atomic/gcc/GccCounter.h"
#include "Mapping.h"
#include <new>
#include <map>

namespace XMI
{
    typedef XMI::Mutex::CounterMutex<XMI::Counter::GccProcCounter>  ContextLock;
    typedef Device::LAPIMessage LAPIMessage;
    typedef Device::LAPIDevice<SysDep> LAPIDevice;
    typedef Device::LAPIPacketModel<LAPIDevice,LAPIMessage> LAPIPacketModel;
    typedef Geometry::Common<XMI_MAPPING_CLASS> LAPIGeometry;
    typedef CollFactory::Default<LAPIDevice, SysDep, LAPIMcastModel> LAPICollfactory;
    typedef CollRegistration::Default<LAPIGeometry, LAPICollfactory, LAPIDevice, SysDep> LAPICollreg;
    typedef XMI::Protocol::Send::Eager <LAPIPacketModel,LAPIDevice> EagerLAPI;
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
     * \param[in] clientid     Client ID (index)
     * \param[in] contextid    Context ID (index)
     */
    inline xmi_result_t init(size_t clientid, size_t num_ctx) {
       // these calls create (allocate and construct) each element.
       // We don't know how these relate to contexts, they are semi-opaque.
       _generics = XMI::Device::Generic::Device::create(clientid, num_ctx);
       //_shmem = ShmemDevice::create(clientid, num_ctx, _generics);
       _progfunc = XMI::Device::ProgressFunctionDev::create(clientid, num_ctx, _generics);
       _atombarr = XMI::Device::AtomicBarrierDev::create(clientid, num_ctx, _generics);
       _wqringreduce = XMI::Device::WQRingReduceDev::create(clientid, num_ctx, _generics);
       _wqringbcast = XMI::Device::WQRingBcastDev::create(clientid, num_ctx, _generics);
       _localallreduce = XMI::Device::LocalAllreduceWQDevice::create(clientid, num_ctx, _generics);
       _localbcast = XMI::Device::LocalBcastWQDevice::create(clientid, num_ctx, _generics);
       _localreduce = XMI::Device::LocalReduceWQDevice::create(clientid, num_ctx, _generics);
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
       //_shmem->init(sd, clientid, num_ctx, ctx, contextid);
       _progfunc->init(sd, clientid, num_ctx, ctx, contextid);
       _atombarr->init(sd, clientid, num_ctx, ctx, contextid);
       _wqringreduce->init(sd, clientid, num_ctx, ctx, contextid);
       _wqringbcast->init(sd, clientid, num_ctx, ctx, contextid);
       _localallreduce->init(sd, clientid, num_ctx, ctx, contextid);
       _localbcast->init(sd, clientid, num_ctx, ctx, contextid);
       _localreduce->init(sd, clientid, num_ctx, ctx, contextid);
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
       //events += _shmem->advance(clientid, contextid);
       events += _progfunc->advance(clientid, contextid);
       events += _atombarr->advance(clientid, contextid);
       events += _wqringreduce->advance(clientid, contextid);
       events += _wqringbcast->advance(clientid, contextid);
       events += _localallreduce->advance(clientid, contextid);
       events += _localbcast->advance(clientid, contextid);
       events += _localreduce->advance(clientid, contextid);
       return events;
    }

    XMI::Device::Generic::Device *_generics; // need better name...
    //ShmemDevice *_shmem;
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
	static void __work_done(xmi_context_t ctx, void *cookie, xmi_result_t result) {
		XMI::Context *context = (XMI::Context *)ctx;
		context->_workAllocator.returnObject(cookie);
	}
    public:
      inline Context (xmi_client_t client, size_t clientid, size_t id, size_t num,
				PlatformDeviceList *devices,
				void * addr, size_t bytes) :
        Interface::Context<XMI::Context> (client, id),
        _client (client),
        _clientid (clientid),
        _contextid (id),
        _mm (addr, bytes),
	_sysdep(_mm),
        _lock (),
        _empty_advance(0),
	_workAllocator (),
        _minterface(&_lapi_device,this,_contextid,clientid),
	_devices(devices)
        {
          lapi_info_t   * lapi_info;     /* used as argument to LAPI_Init */
          lapi_extend_t * extend_info;   /* holds IP addresses and ports */
          lapi_udp_t    * udp_info;      /* List of ip, port info to pass to LAPI */
          int             num_tasks;     /* number of tasks (from LAPI_LIST_NAME) */
          char          * list_name;     /* name of UDP host/port list file */
          FILE          * fp;
          int             i;

          /* ------------------------------------------------------------ */
          /*        initialize request allocation pool                    */
          /* ------------------------------------------------------------ */
//	    __pgasrt_lapi_pool_init();

          /* ------------------------------------------------------------ */
          /*       allocate and initialize lapi_info                      */
          /* ------------------------------------------------------------ */

          CheckNULL(lapi_info,(lapi_info_t *)malloc(sizeof(lapi_info_t)));
          memset(lapi_info, 0, sizeof(lapi_info_t));

          /* ------------------------------------------------------------ */
          /* collect UDP hostnames and ports into udp_info data structure */
          /* ------------------------------------------------------------ */

          udp_info = NULL;
          list_name=getenv("LAPI_LIST_NAME");
          if (list_name)
              {
                if ((fp = fopen (list_name, "r")) == NULL) {
                  printf ("Cannot find LAPI_LIST_NAME\n");
                  abort();
                }
                fscanf(fp, "%u", &num_tasks);
                CheckNULL(udp_info,(lapi_udp_t *) malloc(num_tasks*sizeof(lapi_udp_t)));
                for (i = 0; i < num_tasks; i++)
                    {
                      char ip[256];
                      unsigned port;
                      fscanf(fp, "%s %u", ip, &port);
                      udp_info[i].ip_addr = inet_addr(ip);
                      udp_info[i].port_no = port;
                    }
                /* ------------------------------------------------------------ */
                /*        link up udp_info, extend_info and lapi_info           */
                /* ------------------------------------------------------------ */
                CheckNULL(extend_info,(lapi_extend_t *)malloc(sizeof(lapi_extend_t)));
                memset(extend_info, 0, sizeof(lapi_extend_t));
                extend_info->add_udp_addrs = udp_info;
                extend_info->num_udp_addr  = num_tasks;
                extend_info->udp_hndlr     = 0;
                lapi_info->add_info        = extend_info;
              }
          else
              {
                lapi_info->add_info        = NULL;
              }
          /* ------------------------------------------------------------ */
          /*                call LAPI_Init                                */
          /* ------------------------------------------------------------ */
          int intval = 0;
          lapi_info->protocol_name = "xmi";
          CheckLapiRC(lapi_init(&_lapi_handle, lapi_info));
          CheckLapiRC(lapi_senv(_lapi_handle,INTERRUPT_SET, intval));
          CheckLapiRC(lapi_qenv(_lapi_handle,TASK_ID,
                                       (int *)&_myrank));
          CheckLapiRC(lapi_qenv(_lapi_handle,NUM_TASKS,
                                (int *)&_mysize));
          free(lapi_info);

          _lapi_device.init(&_sysdep, _context, _contextid);
          _lapi_device.setLapiHandle(_lapi_handle);
          __global.mapping.init(_myrank, _mysize);

          _world_geometry=(LAPIGeometry*) malloc(sizeof(*_world_geometry));
          _world_range.lo=0;
          _world_range.hi=_mysize-1;
          new(_world_geometry) LAPIGeometry(&__global.mapping,0, 1,&_world_range);

          _collreg=(LAPICollreg*) malloc(sizeof(*_collreg));
          new(_collreg) LAPICollreg(&_lapi_device, &_sysdep,getClient(), (xmi_context_t)(void *)this, getId(),_clientid);

          _world_collfactory=_collreg->analyze(_world_geometry);
          _world_geometry->setKey(XMI::Geometry::XMI_GKEY_COLLFACTORY, _world_collfactory);

          CheckLapiRC(lapi_gfence (_lapi_handle));

          // dispatch_impl relies on the table being initialized to NULL's.
          memset(_dispatch, 0x00, sizeof(_dispatch));

	  _devices->dev_init(&_sysdep, _clientid, num, _context, _contextid);

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
          LAPI_Gfence (_lapi_handle);
          CheckLapiRC(lapi_term(_lapi_handle));
          return XMI_SUCCESS;
        }

      inline xmi_result_t post_impl (xmi_work_function work_fn, void * cookie)
        {
          XMI::Device::Generic::GenericThread *work =
		(XMI::Device::Generic::GenericThread *)_workAllocator.allocateObject();
	  new (work) XMI::Device::Generic::GenericThread(work_fn, cookie, (xmi_callback_t){__work_done, (void *)work});
	  work->setStatus(XMI::Device::OneShot);
	  _devices->_generics[_contextid].postThread(work);
          return XMI_SUCCESS;
        }

      inline size_t advance_impl (size_t maximum, xmi_result_t & result)
        {
          result = XMI_SUCCESS;
          size_t events = 0;
          unsigned i;
          for (i=0; i<maximum && events==0; i++)
              {
		// don't we want this advanced too?
		// events += _work.advance ();

                events += _lapi_device.advance_impl();
		events += _devices->advance(_clientid, _contextid);
              }
          return events;
        }

      inline xmi_result_t lock_impl ()
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t trylock_impl ()
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t unlock_impl ()
        {
          assert(0);
          return XMI_UNIMPL;
        }
      inline xmi_result_t send_impl (xmi_send_t * parameters)
        {
          size_t id = (size_t)(parameters->send.dispatch);
          XMI_assert_debug (_dispatch[id] != NULL);
          XMI::Protocol::Send::Send * send =
            (XMI::Protocol::Send::Send *) _dispatch[id];
          send->simple (parameters);
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
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t put_impl (xmi_put_simple_t * parameters)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t put_typed_impl (xmi_put_typed_t * parameters)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t get_impl (xmi_get_simple_t * parameters)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t get_typed_impl (xmi_get_typed_t * parameters)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t rmw_impl (xmi_rmw_t * parameters)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t memregion_register_impl (void            * address,
                                                   size_t            bytes,
                                                   xmi_memregion_t * memregion)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t memregion_deregister_impl (xmi_memregion_t * memregion)
        {
          assert(0);
          return XMI_UNIMPL;
        }


      inline xmi_result_t memregion_query_impl (xmi_memregion_t    memregion,
                                                void            ** address,
                                                size_t           * bytes,
                                                size_t           * task)
        {
          assert(0);
          return XMI_UNIMPL;
        }


      inline xmi_result_t memregion_register_impl (xmi_rmw_t * parameters)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t rput_impl (xmi_rput_simple_t * parameters)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t rput_typed_impl (xmi_rput_typed_t * parameters)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t rget_impl (xmi_rget_simple_t * parameters)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t rget_typed_impl (xmi_rget_typed_t * parameters)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t purge_totask_impl (size_t * dest, size_t count)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t resume_totask_impl (size_t * dest, size_t count)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t fence_begin_impl ()
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t fence_end_impl ()
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t fence_all_impl (xmi_event_function   done_fn,
                                          void               * cookie)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline  xmi_result_t fence_task_impl (xmi_event_function   done_fn,
                                            void               * cookie,
                                            size_t               task)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t geometry_initialize_impl (xmi_geometry_t       * geometry,
                                                    unsigned               id,
                                                    xmi_geometry_range_t * rank_slices,
                                                    size_t                 slice_count)
        {
	  LAPIGeometry              *new_geometry;
	  LAPICollfactory           *new_collfactory;
          new_geometry=(LAPIGeometry*) malloc(sizeof(*new_geometry));
          new(new_geometry) LAPIGeometry(&__global.mapping,id, slice_count,rank_slices);
          new_collfactory=_collreg->analyze(new_geometry);
	  new_geometry->setKey(XMI::Geometry::XMI_GKEY_COLLFACTORY, new_collfactory);
	  *geometry=(LAPIGeometry*) new_geometry;
          return XMI_SUCCESS;
        }


      inline xmi_result_t geometry_world_impl (xmi_geometry_t * world_geometry)
        {
	  *world_geometry = _world_geometry;
          return XMI_SUCCESS;
        }


      inline xmi_result_t geometry_finalize_impl (xmi_geometry_t geometry)
        {
          assert(0);
          return XMI_UNIMPL;
        }


      inline xmi_result_t collective_impl (xmi_xfer_t * parameters)
        {
	  LAPICollfactory           *collfactory;
	  // This is ok...we can avoid a switch because all the xmi structs
	  // have the same layout.let's just use barrier for now
	  LAPIGeometry              *new_geometry = (LAPIGeometry*)parameters->xfer_barrier.geometry;
	  collfactory =(LAPICollfactory*) new_geometry->getKey(XMI::Geometry::XMI_GKEY_COLLFACTORY);
          return collfactory->collective(parameters);
        }

      inline xmi_result_t geometry_algorithms_num_impl (xmi_geometry_t geometry,
                                                        xmi_xfer_type_t colltype,
                                                        int *lists_lengths)
        {
          LAPIGeometry *new_geometry = (LAPIGeometry*) geometry;
          LAPICollfactory  *collfactory =  (LAPICollfactory*)
            new_geometry->getKey(XMI::Geometry::XMI_GKEY_COLLFACTORY);
          return collfactory->algorithms_num(colltype, lists_lengths);
        }

      inline xmi_result_t geometry_algorithms_info_impl (xmi_geometry_t geometry,
                                                         xmi_xfer_type_t colltype,
                                                         xmi_algorithm_t *algs,
                                                         xmi_metadata_t *mdata,
                                                         int algorithm_type,
                                                         int num)
        {
          LAPIGeometry *new_geometry = (LAPIGeometry*) geometry;
          LAPICollfactory  *collfactory;
          collfactory = (LAPICollfactory*)
            new_geometry->getKey(XMI::Geometry::XMI_GKEY_COLLFACTORY);
          return collfactory->algorithms_info(colltype, algs,
                                              mdata, algorithm_type, num);

        }

      inline xmi_result_t multisend_getroles_impl(size_t          dispatch,
                                                  int            *numRoles,
                                                  int            *replRole)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t multicast_impl(xmi_multicast_t *mcastinfo)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t manytomany_impl(xmi_manytomany_t *m2minfo)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline xmi_result_t multisync_impl(xmi_multisync_t *msyncinfo)
        {
          // Select the native interface
          // call the multisync for the selected native interface.

          return _minterface.multisync(msyncinfo);
        }

      inline xmi_result_t multicombine_impl(xmi_multicombine_t *mcombineinfo)
        {
          assert(0);
          return XMI_UNIMPL;
        }
      inline xmi_result_t dispatch_impl (size_t                     id,
                                         xmi_dispatch_callback_fn   fn,
                                         void                     * cookie,
                                         xmi_send_hint_t            options)
        {
          xmi_result_t result        = XMI_ERROR;
#if 1
          if (_dispatch[(size_t)id] != NULL) return XMI_ERROR;
          _dispatch[(size_t)id]      = (void *) _request.allocateObject ();
          new (_dispatch[(size_t)id]) EagerLAPI (id,
                                                 fn,
                                                 cookie,
                                                 _lapi_device,
                                                 result);
#endif
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


    private:
      std::map <unsigned, xmi_geometry_t>   _geometry_id;
      xmi_client_t                          _client;
      xmi_context_t                         _context;
      size_t                                _clientid;
      size_t                                _contextid;
      void                                 *_dispatch[1024];
      ProtocolAllocator                     _protocol;
      Memory::MemoryManager                 _mm;
      SysDep                                _sysdep;
      ContextLock                           _lock;
      MemoryAllocator<sizeof(XMI::Device::Generic::GenericThread), 16> _workAllocator;
      MemoryAllocator<1024,16>              _request;
      LAPIDevice                            _lapi_device;
      LAPICollreg                          *_collreg;
      LAPIGeometry                         *_world_geometry;
      LAPICollfactory                      *_world_collfactory;
      LAPINativeInterface<LAPIDevice>       _minterface;
      xmi_geometry_range_t                  _world_range;
      unsigned                              _empty_advance;
      int                                   _myrank;
      int                                   _mysize;
      unsigned                             *_ranklist;
      lapi_handle_t                        _lapi_handle;
      PlatformDeviceList *_devices;
    }; // end XMI::Context
}; // end namespace XMI

#endif // __xmi_lapi_lapicontext_h__
