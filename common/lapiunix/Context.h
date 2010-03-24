///
/// \file common/lapiunix/Context.h
/// \brief PAMI LAPI specific context implementation.
///
#ifndef __common_lapiunix_Context_h__
#define __common_lapiunix_Context_h__

// Standard includes
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <new>
#include <map>

// common includes
#include "SysDep.h"
#include "Mapping.h"
#include "common/lapiunix/lapifunc.h"
#include "lapi.h"
#include "common/ContextInterface.h"

// Geometry
#include "algorithms/geometry/Geometry.h"

// Components
#include "components/devices/lapiunix/lapiunixdevice.h"
#include "components/devices/lapiunix/lapiunixpacketmodel.h"
#include "components/devices/lapiunix/lapiunixmessage.h"
#include "p2p/protocols/send/eager/Eager.h"
#include "SysDep.h"
#include "components/devices/generic/Device.h"
#include "components/devices/misc/ProgressFunctionMsg.h"
#include "components/devices/misc/AtomicBarrierMsg.h"
#include "components/devices/workqueue/WQRingReduceMsg.h"
#include "components/devices/workqueue/WQRingBcastMsg.h"
#include "components/devices/workqueue/LocalAllreduceWQMessage.h"
#include "components/devices/workqueue/LocalReduceWQMessage.h"
#include "components/devices/workqueue/LocalBcastWQMessage.h"
#include "components/devices/lapiunix/lapiunixmulticastmodel.h"
#include "components/devices/lapiunix/lapiunixmultisyncmodel.h"
#include "components/devices/lapiunix/lapiunixmulticombinemodel.h"
#include "components/devices/lapiunix/lapiunixmanytomanymodel.h"
#include "components/devices/shmem/ShmemDevice.h"
#include "components/devices/shmem/ShmemPacketModel.h"
#include "components/atomic/counter/CounterMutex.h"
#include "components/atomic/gcc/GccCounter.h"

// P2P Protocols
#include "p2p/protocols/send/eager/Eager.h"

// Collective Protocols
#include "algorithms/geometry/CCMICollRegistration.h"
#include "algorithms/geometry/PGASCollRegistration.h"
#include "algorithms/geometry/OldCCMICollRegistration.h"

namespace PAMI
{
    typedef PAMI::Mutex::CounterMutex<PAMI::Counter::GccProcCounter>  ContextLock;

  // Device Typedefs
  typedef Device::LAPIDevice                                          LAPIDevice;
  typedef Fifo::FifoPacket <32, 512>                                  ShmemPacket;
  typedef Fifo::LinearFifo<Atomic::GccBuiltin, ShmemPacket, 128>      ShmemFifo;
  typedef Device::ShmemDevice<ShmemFifo>                              ShmemDevice;
  // P2P Message Typedefs
    typedef Device::LAPIMessage LAPIMessage;

  // "Old" Collective Typedefs
  typedef Device::OldLAPIMcastMessage                                 OldLAPIMcastMessage;
  typedef Device::OldLAPIM2MMessage                                   OldLAPIM2MMessage;

  // "New" Collective Message Typedefs
  typedef Device::LAPIMsyncMessage                                    LAPIMsyncMessage;
  typedef Device::LAPIMcastMessage                                    LAPIMcastMessage;
  typedef Device::LAPIMcombineMessage                                 LAPIMcombineMessage;
  typedef Device::LAPIM2MMessage                                      LAPIM2MMessage;

  // P2P Model Classes
    typedef Device::LAPIPacketModel<LAPIDevice,LAPIMessage> LAPIPacketModel;

  // "New" Collective Model typedefs
  typedef Device::LAPIMultisyncModel<LAPIDevice,LAPIMsyncMessage>     LAPIMultisyncModel;
  typedef Device::LAPIMulticastModel<LAPIDevice,LAPIMcastMessage>     LAPIMulticastModel;
  typedef Device::LAPIMulticombineModel<LAPIDevice,
                                       LAPIMcombineMessage>           LAPIMulticombineModel;
  typedef Device::LAPIManytomanyModel<LAPIDevice,LAPIM2MMessage>      LAPIManytomanyModel;

  // "Old" Collective Model Typedefs
  typedef PAMI::Device::LAPIOldmulticastModel<LAPIDevice,
                                             OldLAPIMcastMessage>     LAPIOldMcastModel;
  typedef PAMI::Device::LAPIOldm2mModel<LAPIDevice,
                                       OldLAPIM2MMessage,
                                       size_t>                        LAPIOldM2MModel;

  // Geometry Typedefs
  typedef Geometry::Common                                            LAPIGeometry;

  // Protocol Typedefs
    typedef PAMI::Protocol::Send::Eager <LAPIPacketModel,LAPIDevice> EagerLAPI;

  // "New" CCMI Protocol Typedefs
  typedef PAMI::LAPINativeInterface<LAPIDevice,
                                   LAPIMulticastModel,
                                   LAPIMultisyncModel,
                                   LAPIMulticombineModel>              DefaultNativeInterface;
  typedef CollRegistration::CCMIRegistration<LAPIGeometry,
                                             DefaultNativeInterface,
                                             DefaultNativeInterface,
                                             LAPIDevice> CCMICollreg;

  // PGAS RT Typedefs/Coll Registration
  typedef PAMI::Device::LAPIOldmulticastModel<LAPIDevice,
                                             OldLAPIMcastMessage> LAPIOldMcastModel;
  typedef TSPColl::NBCollManager<LAPIOldMcastModel> LAPINBCollManager;
  typedef CollRegistration::PGASRegistration<LAPIGeometry,
                                             LAPIOldMcastModel,
                                             LAPIDevice,
                                             LAPINBCollManager> PGASCollreg;

  // "Old" CCMI Protocol Typedefs
  typedef CollRegistration::OldCCMIRegistration<LAPIGeometry,
                                                LAPIOldMcastModel,
                                                LAPIOldM2MModel,
                                                LAPIDevice,
                                                SysDep> OldCCMICollreg;
  // Memory Allocator Typedefs
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
    inline pami_result_t generate(size_t clientid, size_t num_ctx, Memory::MemoryManager &mm) {
        // these calls create (allocate and construct) each element.
        // We don't know how these relate to contexts, they are semi-opaque.
        _generics = PAMI::Device::Generic::Device::Factory::generate(clientid, num_ctx, mm);
//	_shmem = ShmemDevice::Factory::generate(clientid, num_ctx, mm);
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
//	ShmemDevice::Factory::init(_shmem, clientid, contextid, clt, ctx, mm, _generics);
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
//	events += ShmemDevice::Factory::advance(_shmem, clientid, contextid);
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
//    ShmemDevice *_shmem;
    PAMI::Device::ProgressFunctionDev *_progfunc;
    PAMI::Device::AtomicBarrierDev *_atombarr;
    PAMI::Device::WQRingReduceDev *_wqringreduce;
    PAMI::Device::WQRingBcastDev *_wqringbcast;
    PAMI::Device::LocalAllreduceWQDevice *_localallreduce;
    PAMI::Device::LocalBcastWQDevice *_localbcast;
    PAMI::Device::LocalReduceWQDevice *_localreduce;
  }; // class PlatformDeviceList


    class Context : public Interface::Context<PAMI::Context>
    {
    public:
    inline Context (pami_client_t                  client,
                    size_t                        clientid,
                    size_t                        id,
                    size_t                        num,
                    PlatformDeviceList *devices,
                    void                         *addr,
                    size_t                        bytes,
                    LAPIGeometry                 *world_geometry,
                    lapi_handle_t                 lapi_handle) :
        Interface::Context<PAMI::Context> (client, id),
        _client (client),
        _clientid (clientid),
        _contextid (id),
        _mm (addr, bytes),
        _sysdep(_mm),
        _lock (),
        _world_geometry(world_geometry),
        _minterface(_lapi_device,_client,this,_contextid,_clientid),
        _empty_advance(0),
        _lapi_handle(lapi_handle),
        _devices(devices)
      {
        _lapi_device.init(&_mm, _clientid, 0, _context, _contextid);
        _lapi_device.setLapiHandle(_lapi_handle);

        _pgas_collreg=(PGASCollreg*) malloc(sizeof(*_pgas_collreg));
        new(_pgas_collreg) PGASCollreg(client, (pami_context_t)this, id,_lapi_device);
        _pgas_collreg->analyze(_contextid,_world_geometry);

        _oldccmi_collreg=(OldCCMICollreg*) malloc(sizeof(*_oldccmi_collreg));
        new(_oldccmi_collreg) OldCCMICollreg(client, (pami_context_t)this, id,_sysdep,_lapi_device);
        _oldccmi_collreg->analyze(_contextid, _world_geometry);

        _ccmi_collreg=(CCMICollreg*) malloc(sizeof(*_ccmi_collreg));
        new(_ccmi_collreg) CCMICollreg(client, (pami_context_t)this, id,clientid,_lapi_device);
        _ccmi_collreg->analyze(_contextid, _world_geometry);

          // dispatch_impl relies on the table being initialized to NULL's.
          memset(_dispatch, 0x00, sizeof(_dispatch));

//	  _devices->dev_init(&_sysdep, _clientid, num, _context, _contextid);
          _devices->init(_clientid, _contextid, _client, _context, &_mm);
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
          LAPI_Gfence (_lapi_handle);
          CheckLapiRC(lapi_term(_lapi_handle));
          return PAMI_SUCCESS;
        }

      inline pami_result_t post_impl (pami_work_t *state, pami_work_function work_fn, void * cookie)
        {
          PAMI::Device::Generic::GenericThread *work = NULL;
          COMPILE_TIME_ASSERT(sizeof(*state) >= sizeof(*work));
          work = new (work) PAMI::Device::Generic::GenericThread(work_fn, cookie);
          work->setStatus(PAMI::Device::OneShot);
          _devices->_generics[_contextid].postThread(work);
          return PAMI_SUCCESS;
        }

      inline size_t advance_impl (size_t maximum, pami_result_t & result)
        {
          result = PAMI_SUCCESS;
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

      inline pami_result_t lock_impl ()
        {
          assert(0);
          return PAMI_UNIMPL;
        }

      inline pami_result_t trylock_impl ()
        {
          assert(0);
          return PAMI_UNIMPL;
        }

      inline pami_result_t unlock_impl ()
        {
          assert(0);
          return PAMI_UNIMPL;
        }
      inline pami_result_t send_impl (pami_send_t * parameters)
        {
          size_t id = (size_t)(parameters->send.dispatch);
          PAMI_assert_debug (_dispatch[id] != NULL);
          PAMI::Protocol::Send::Send * send =
            (PAMI::Protocol::Send::Send *) _dispatch[id];
          send->simple (parameters);
          return PAMI_SUCCESS;
        }

      inline pami_result_t send_impl (pami_send_immediate_t * parameters)
        {
          size_t id = (size_t)(parameters->dispatch);
          TRACE_ERR((stderr, ">> send_impl('immediate'), _dispatch[%zd] = %p\n", id, _dispatch[id]));
          PAMI_assert_debug (_dispatch[id] != NULL);

          PAMI::Protocol::Send::Send * send =
            (PAMI::Protocol::Send::Send *) _dispatch[id];
          send->immediate (parameters);

          TRACE_ERR((stderr, "<< send_impl('immediate')\n"));
          return PAMI_SUCCESS;
        }

      inline pami_result_t send_impl (pami_send_typed_t * parameters)
        {
          assert(0);
          return PAMI_UNIMPL;
        }

      inline pami_result_t put_impl (pami_put_simple_t * parameters)
        {
          assert(0);
          return PAMI_UNIMPL;
        }

      inline pami_result_t put_typed_impl (pami_put_typed_t * parameters)
        {
          assert(0);
          return PAMI_UNIMPL;
        }

      inline pami_result_t get_impl (pami_get_simple_t * parameters)
        {
          assert(0);
          return PAMI_UNIMPL;
        }

      inline pami_result_t get_typed_impl (pami_get_typed_t * parameters)
        {
          assert(0);
          return PAMI_UNIMPL;
        }

      inline pami_result_t rmw_impl (pami_rmw_t * parameters)
        {
          assert(0);
          return PAMI_UNIMPL;
        }

      inline pami_result_t memregion_register_impl (void            * address,
                                                   size_t            bytes,
                                                   pami_memregion_t * memregion)
        {
          assert(0);
          return PAMI_UNIMPL;
        }

      inline pami_result_t memregion_deregister_impl (pami_memregion_t * memregion)
        {
          assert(0);
          return PAMI_UNIMPL;
        }


      inline pami_result_t memregion_query_impl (pami_memregion_t    memregion,
                                                void            ** address,
                                                size_t           * bytes,
                                                size_t           * task)
        {
          assert(0);
          return PAMI_UNIMPL;
        }


      inline pami_result_t memregion_register_impl (pami_rmw_t * parameters)
        {
          assert(0);
          return PAMI_UNIMPL;
        }

      inline pami_result_t rput_impl (pami_rput_simple_t * parameters)
        {
          assert(0);
          return PAMI_UNIMPL;
        }

      inline pami_result_t rput_typed_impl (pami_rput_typed_t * parameters)
        {
          assert(0);
          return PAMI_UNIMPL;
        }

      inline pami_result_t rget_impl (pami_rget_simple_t * parameters)
        {
          assert(0);
          return PAMI_UNIMPL;
        }

      inline pami_result_t rget_typed_impl (pami_rget_typed_t * parameters)
        {
          assert(0);
          return PAMI_UNIMPL;
        }

      inline pami_result_t purge_totask_impl (size_t * dest, size_t count)
        {
          assert(0);
          return PAMI_UNIMPL;
        }

      inline pami_result_t resume_totask_impl (size_t * dest, size_t count)
        {
          assert(0);
          return PAMI_UNIMPL;
        }

      inline pami_result_t fence_begin_impl ()
        {
          assert(0);
          return PAMI_UNIMPL;
        }

      inline pami_result_t fence_end_impl ()
        {
          assert(0);
          return PAMI_UNIMPL;
        }

      inline pami_result_t fence_all_impl (pami_event_function   done_fn,
                                          void               * cookie)
        {
          assert(0);
          return PAMI_UNIMPL;
        }

      inline  pami_result_t fence_task_impl (pami_event_function   done_fn,
                                            void               * cookie,
                                            size_t               task)
        {
          assert(0);
          return PAMI_UNIMPL;
        }


    inline pami_result_t collective_impl (pami_xfer_t * parameters)
        {
        Geometry::Algorithm<LAPIGeometry> *algo = (Geometry::Algorithm<LAPIGeometry> *)parameters->algorithm;
        return algo->generate(parameters);
        }

    inline pami_result_t amcollective_dispatch_impl (pami_algorithm_t            algorithm,
                                                    size_t                     dispatch,
                                                    pami_dispatch_callback_fn   fn,
                                                    void                     * cookie,
                                                    pami_collective_hint_t      options)
        {
        Geometry::Algorithm<LAPIGeometry> *algo = (Geometry::Algorithm<LAPIGeometry> *)algorithm;
        return algo->dispatch_set(dispatch, fn, cookie, options);
        }


      inline pami_result_t geometry_algorithms_num_impl (pami_geometry_t geometry,
                                                        pami_xfer_type_t colltype,
                                                        int *lists_lengths)
        {
        LAPIGeometry *_geometry = (LAPIGeometry*) geometry;
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
        LAPIGeometry *_geometry = (LAPIGeometry*) geometry;
        return _geometry->algorithms_info(colltype,
                                          algs0,
                                          mdata0,
                                          num0,
                                          algs1,
                                          mdata1,
                                          num1,
                                          _contextid);
        }

      inline pami_result_t multisend_getroles_impl(size_t          dispatch,
                                                  int            *numRoles,
                                                  int            *replRole)
        {
          assert(0);
          return PAMI_UNIMPL;
        }

      inline pami_result_t multicast_impl(pami_multicast_t *mcastinfo)
        {
          assert(0);
          return PAMI_UNIMPL;
        }

      inline pami_result_t manytomany_impl(pami_manytomany_t *m2minfo)
        {
          assert(0);
          return PAMI_UNIMPL;
        }

      inline pami_result_t multisync_impl(pami_multisync_t *msyncinfo)
        {
          // Select the native interface
          // call the multisync for the selected native interface.

          return _minterface.multisync(msyncinfo);
        }

      inline pami_result_t multicombine_impl(pami_multicombine_t *mcombineinfo)
        {
          assert(0);
          return PAMI_UNIMPL;
        }
      inline pami_result_t dispatch_impl (size_t                     id,
                                         pami_dispatch_callback_fn   fn,
                                         void                     * cookie,
                                         pami_send_hint_t            options)
        {
          pami_result_t result        = PAMI_ERROR;
#if 1
          if (_dispatch[(size_t)id] != NULL) return PAMI_ERROR;
          _dispatch[(size_t)id]      = (void *) _request.allocateObject ();
          new (_dispatch[(size_t)id]) EagerLAPI (id,
                                                 fn,
                                                 cookie,
                                                 _lapi_device,
                                                 result);
#endif
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


    private:
      std::map <unsigned, pami_geometry_t>   _geometry_id;
      pami_client_t                          _client;
      pami_context_t                         _context;
      size_t                                _clientid;
      size_t                                _contextid;
      void                                 *_dispatch[1024];
      ProtocolAllocator                     _protocol;
      Memory::MemoryManager                 _mm;
      SysDep                                _sysdep;
      ContextLock                           _lock;
      MemoryAllocator<1024,16>              _request;
      LAPIDevice                            _lapi_device;
  public:
    CCMICollreg                          *_ccmi_collreg;
    PGASCollreg                          *_pgas_collreg;
    OldCCMICollreg                       *_oldccmi_collreg;
      LAPIGeometry                         *_world_geometry;
  private:
    DefaultNativeInterface                _minterface;
      unsigned                              _empty_advance;
      int                                   _myrank;
      int                                   _mysize;
      unsigned                             *_ranklist;
      lapi_handle_t                        _lapi_handle;
      PlatformDeviceList *_devices;
    }; // end PAMI::Context
}; // end namespace PAMI

#endif // __pami_lapi_lapicontext_h__
