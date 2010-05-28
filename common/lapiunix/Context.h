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
#include "common/ContextInterface.h"

// Geometry
#include "algorithms/geometry/Geometry.h"

// Components
#include "components/devices/generic/Device.h"
#include "components/devices/lapiunix/lapiunixdevice.h"
#include "components/devices/lapiunix/lapiunixpacketmodel.h"
#include "components/devices/lapiunix/lapiunixmessage.h"
#include "components/devices/lapiunix/lapiunixmulticastmodel.h"
#include "components/devices/lapiunix/lapiunixmultisyncmodel.h"
#include "components/devices/lapiunix/lapiunixmulticombinemodel.h"
#include "components/devices/lapiunix/lapiunixmanytomanymodel.h"
#include "components/lapi/include/Context.h"

// P2P Protocols

// Collective Protocols
#include "algorithms/geometry/CCMICollRegistration.h"
#include "algorithms/geometry/PGASCollRegistration.h"
#include "algorithms/geometry/OldCCMICollRegistration.h"

namespace PAMI
{
  // Device Typedefs
  typedef Device::LAPIDevice                                          LAPIDevice;
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
        _generics = PAMI::Device::Generic::Device::Factory::generate(clientid, num_ctx, mm, NULL);
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
        return events;
    }
    PAMI::Device::Generic::Device        *_generics; // need better name...
  }; // class PlatformDeviceList


    class Context : public Interface::Context<PAMI::Context>
    {
    public:
    inline Context (pami_client_t                  client,
                    size_t                         clientid,
                    char                          *clientname,
                    size_t                         id,
                    PlatformDeviceList            *devices,
                    Memory::MemoryManager         *mm):
        Interface::Context<PAMI::Context> (client, id),
        _client (client),
        _clientid (clientid),
        _clientname(clientname),
        _contextid (id),
        _world_geometry(NULL),
        _devices(devices)
      {
      }

      inline void setWorldGeometry(LAPIGeometry *world_geometry)
        {
          _world_geometry = world_geometry;
        }

      inline pami_result_t initP2P(size_t        *out_myrank,
                                   size_t        *out_mysize,
                                   lapi_handle_t *out_lapi_handle)
        {
          // Bring up the LAPI P2P Contexts
          lapi_info_t  init_info;
          memset(&init_info, 0, sizeof(init_info));
          init_info.protocol_name  = _clientname;

          // TODO: Honor the configuration passed in
          int rc = LAPI__Init(&_lapi_handle, &init_info);
          if (rc) {
            RETURN_ERR_PAMI(PAMI_ERROR, "LAPI__Init failed with rc %d\n", rc);
          }
          _lapi_state = _Lapi_port[_lapi_handle];

          // Initialize the lapi device for collectives
          _lapi_device.init(_mm, _clientid, 0, _context, _contextid);
          _lapi_device.setLapiHandle(_lapi_handle);

          // Initialize Platform and Collective "per context" Devices
          _devices->init(_clientid,_contextid,_client,_context,_mm);

          // Query My Rank and My Size
          // TODO:  Use LAPI Internals, instead of
          // doing an upcall to LAPI
          int orank=0, osz=0;
          CheckLapiRC(lapi_qenv(_lapi_handle,
                                NUM_TASKS,
                                &osz));

          CheckLapiRC(lapi_qenv(_lapi_handle,
                                TASK_ID,
                                &orank));

          *out_mysize        = osz;
          *out_myrank        = orank;
          *out_lapi_handle   = _lapi_handle;

          return PAMI_SUCCESS;
        }

      inline pami_result_t initP2PCollectives()
        {
          // Initalize Collective Registration
          _pgas_collreg=(PGASCollreg*) malloc(sizeof(*_pgas_collreg));
          new(_pgas_collreg) PGASCollreg(_client,_context,_contextid,_lapi_device);
          _pgas_collreg->analyze(_contextid,_world_geometry);
          return PAMI_SUCCESS;
        }

      inline pami_result_t initCollectives()
        {
          _oldccmi_collreg=(OldCCMICollreg*) malloc(sizeof(*_oldccmi_collreg));
          new(_oldccmi_collreg) OldCCMICollreg(_client, _context,_contextid,_sd,_lapi_device);
          _oldccmi_collreg->analyze(_contextid, _world_geometry);

          _ccmi_collreg=(CCMICollreg*) malloc(sizeof(*_ccmi_collreg));
          new(_ccmi_collreg) CCMICollreg(_client, (pami_context_t)this, _contextid ,_clientid,_lapi_device);
          _ccmi_collreg->analyze(_contextid, _world_geometry);

          return PAMI_SUCCESS;
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
          LapiImpl::Context *ep = (LapiImpl::Context *)_lapi_state;
          int rc = LAPI__Term(ep->my_hndl);
          if (rc) {
            RETURN_ERR_PAMI(PAMI_ERROR, "LAPI__Term failed with rc %d\n", rc);
          }
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
#if 0
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
#endif
          // Todo:  Add collective devices
          // Todo:  Add Generic devices
          LapiImpl::Context *cp = (LapiImpl::Context *)_lapi_state;
          result = (cp->*(cp->pAdvance))(maximum);
          return 1;
        }

      inline pami_result_t lock_impl ()
        {
          LapiImpl::Context *ep = (LapiImpl::Context *)_lapi_state;
          return (ep->*(ep->pLock))();
        }

      inline pami_result_t trylock_impl ()
        {
          LapiImpl::Context *ep = (LapiImpl::Context *)_lapi_state;
          return (ep->*(ep->pTryLock))();
        }

      inline pami_result_t unlock_impl ()
        {
          LapiImpl::Context *ep = (LapiImpl::Context *)_lapi_state;
          return (ep->*(ep->pUnlock))();
        }
      inline pami_result_t send_impl (pami_send_t * parameters)
        {
          assert(0);
          return PAMI_SUCCESS;
        }

      inline pami_result_t send_impl (pami_send_immediate_t * send)
        {
          LapiImpl::Context *cp = (LapiImpl::Context *)_lapi_state;
          return (cp->*(cp->pSendSmall))(send->dest, send->dispatch,
                                         send->header.iov_base, send->header.iov_len,
                                         send->data.iov_base, send->data.iov_len,
                                         *(send_hint_t *)&send->hints);
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
          LapiImpl::Context *cp = (LapiImpl::Context *)_lapi_state;
          return (cp->*(cp->pGet))(parameters->rma.dest, parameters->addr.local, NULL,
                                   parameters->addr.remote, NULL, parameters->rma.bytes,
                                   *(send_hint_t*)&parameters->rma.hints, INTERFACE_PAMI,
                                   (void *)parameters->rma.done_fn, parameters->rma.cookie, NULL, NULL);
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

      inline pami_result_t memregion_create_impl (void             * address,
                                                  size_t             bytes_in,
                                                  size_t           * bytes_out,
                                                  pami_memregion_t * memregion)
        {
          assert(0);
          return PAMI_UNIMPL;
        }

      inline pami_result_t memregion_destroy_impl (pami_memregion_t * memregion)
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
          assert(0);
          return PAMI_UNIMPL;
        }

      inline pami_result_t multicombine_impl(pami_multicombine_t *mcombineinfo)
        {
          assert(0);
          return PAMI_UNIMPL;
        }
      inline pami_result_t dispatch_impl (size_t                      id,
                                          pami_dispatch_callback_fn   fn,
                                          void                      * cookie,
                                          pami_send_hint_t            options)
        {
          LapiImpl::Context  *cp = (LapiImpl::Context *)_lapi_state;
          return (cp->*(cp->pDispatchSet))(id, (void *)fn.p2p, cookie,
                                           *(send_hint_t *)&options, INTERFACE_PAMI);
        }

    inline pami_result_t dispatch_new_impl (size_t                     id,
                                           pami_dispatch_callback_fn   fn,
                                           void                     * cookie,
                                           pami_dispatch_hint_t        options)
    {
      assert(0);
      pami_result_t result        = PAMI_ERROR;
      return result;
    }

      inline lapi_state_t *getLapiState()
        {
          return _lapi_state;
        }


    private:
      /*  PAMI Client Pointer associated with this PAMI Context */
      pami_client_t                          _client;

      /*  Lapi State Object.  use this for direct access        */
      lapi_state_t                          *_lapi_state;

      /*  Context pointer to this client                        */
      pami_context_t                         _context;

      /*  Context id (offset) of this context in the client     */
      size_t                                 _contextid;

      /*  Client id of this client                              */
      size_t                                 _clientid;

      /*  Pointer to the client name string                     */
      char                                  *_clientname;

      /*  Memory Manager Pointer                                */
      Memory::MemoryManager                 *_mm;

      /*  The over lapi device                                  */
      LAPIDevice                             _lapi_device;
  public:
      /*  Collective Registrations                              */
      CCMICollreg                           *_ccmi_collreg;
      PGASCollreg                           *_pgas_collreg;
      OldCCMICollreg                        *_oldccmi_collreg;

      /*  World Geometry Pointer for this context               */
      LAPIGeometry                          *_world_geometry;
  private:
      lapi_handle_t                          _lapi_handle;
      PlatformDeviceList                    *_devices;
      SysDep                                 _sd;

    }; // end PAMI::Context
}; // end namespace PAMI

#endif // __pami_lapi_lapicontext_h__
