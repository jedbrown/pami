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
#include "Mapping.h"
#include "common/lapiunix/lapifunc.h"
#include "common/lapiunix/Client.h"
#include "common/ContextInterface.h"
#include "components/devices/BaseDevice.h"
#include "components/memory/MemoryManager.h"

// Geometry
#include "algorithms/geometry/LapiGeometry.h"

// Components
#include "components/devices/generic/Device.h"
#include "../lapi/include/Context.h"

// CAU Components
#include "components/devices/cau/caudevice.h"
#include "components/devices/cau/caumessage.h"
#include "components/devices/cau/caumulticastmodel.h"
#include "components/devices/cau/caumultisyncmodel.h"
#include "components/devices/cau/caumulticombinemodel.h"
#include "components/devices/cau/caumulticombinemodel.h"

// BSR Components
#include "components/devices/bsr/bsrdevice.h"
#include "components/devices/bsr/bsrmultisyncmodel.h"

// P2P Protocols
#include "p2p/protocols/Send.h"
#include "p2p/protocols/SendPWQ.h"
#include "p2p/protocols/send/eager/Eager.h"
#include "p2p/protocols/send/composite/Composite.h"

// P2P Shared memory protocols
#include "components/devices/shmem/ShmemDevice.h"
#include "components/devices/shmem/ShmemPacketModel.h"
#include "components/atomic/indirect/IndirectCounter.h"
#include "components/atomic/counter/CounterMutex.h"
#include "components/atomic/native/NativeCounter.h"
#include "components/atomic/native/NativeMutex.h"
#include "components/atomic/native/YieldingNativeMutex.h"
#include "components/fifo/FifoPacket.h"
#include "components/fifo/linear/LinearFifo.h"
#include "components/devices/shmem/shaddr/NoShaddr.h"

// Collective Protocols
#include "algorithms/geometry/PGASCollRegistration.h"
#include "algorithms/geometry/P2PCCMIRegistration.h"
#include "algorithms/geometry/CAUCollRegistration.h"
#include "algorithms/geometry/ClassRouteId.h"

// Collective shmem device
#include "components/devices/cshmem/CollSharedMemoryManager.h"
#include "components/devices/cshmem/CollShmDevice.h"
#include "algorithms/geometry/CCMICSMultiRegistration.h"

#include "components/devices/NativeInterface.h"


namespace PAMI
{
  //  A simple wrapper class for Send, and LAPI "Device"
  //  This class is a "dummy" device, used to wrapper the lapi state
  //  object, and present it to the NativeInterface and Send
  //  Protocols as a Proper PAMI device object.
  //  This is used for the P2P Collectives "Over Send"
  class DeviceWrapper: public PAMI::Device::Interface::BaseDevice<DeviceWrapper>
  {
  public:
    DeviceWrapper():
      _lapi_state(NULL)
      {}
    inline void          init(lapi_state_t *lapi_state) {_lapi_state=lapi_state;}
    inline lapi_state_t *getState() { return _lapi_state;}
    inline int advance_impl()
      {
        LapiImpl::Context *cp      = (LapiImpl::Context *)_lapi_state;
        return (cp->*(cp->pAdvance))(1);
      }
  private:
    lapi_state_t                          *_lapi_state;
  };

  //  An Implementation of a send protocol "Over LAPI internals"
  //  This device implements the send protocol interface,
  //  but uses the LAPI component internal objects.
  //  This object could probably be put somewhere else, but
  //  it has a limited use for P2P collectives at this point
  class SendWrapper: public Protocol::Send::Send
  {
  public:
    SendWrapper(size_t                      dispatch,
                pami_dispatch_p2p_function  dispatch_fn,
                void                      * cookie,
                DeviceWrapper             & device,
                pami_endpoint_t             origin,
                pami_dispatch_hint_t        hint,
                pami_result_t             & result):
      Send()
      {
        _lapi_state                = device.getState();
        LapiImpl::Context *cp      = (LapiImpl::Context *)_lapi_state;
        internal_rc_t rc = (cp->*(cp->pDispatchSet))(dispatch,
                                                        (void *)dispatch_fn,
                                                        cookie,
                                                        hint,
                                                        INTERFACE_PAMI);
        result = PAMI_RC(rc);
        return;
      }
    ~SendWrapper()
      {
      }
    inline pami_result_t immediate(pami_send_immediate_t * send)
      {
        LapiImpl::Context *cp = (LapiImpl::Context *)_lapi_state;
        internal_rc_t rc = (cp->*(cp->pSendSmall))(send->dest, send->dispatch,
                                                      send->header.iov_base, send->header.iov_len,
                                                      send->data.iov_base, send->data.iov_len,
                                                      send->hints, FLAG_NULL);
        return PAMI_RC(rc);
      }
    inline pami_result_t simple (pami_send_t * simple)
      {
        LapiImpl::Context *cp = (LapiImpl::Context *)_lapi_state;
        internal_rc_t rc = (cp->*(cp->pSend))(simple->send.dest, simple->send.dispatch,
                                                 simple->send.header.iov_base, simple->send.header.iov_len,
                                                 simple->send.data.iov_base, simple->send.data.iov_len,
                                                 simple->send.hints,
                                                 simple->events.local_fn, simple->events.remote_fn,
                                                 simple->events.cookie,
                                                 NULL, NULL, NULL, NULL, NULL,
                                                 INTERFACE_PAMI, FLAG_NULL);
        return PAMI_RC(rc);
      }
    inline pami_result_t getAttributes (pami_configuration_t  configuration[],
                                        size_t                num_configs)
      {
        PAMI_abort();
        return PAMI_INVAL;
      }


     template <class T_Allocator>
     static inline SendWrapper * generate (size_t                      dispatch,
                                           pami_dispatch_p2p_function  dispatch_fn,
                                           void                      * cookie,
                                           PAMI::DeviceWrapper       & device,
                                           pami_endpoint_t             origin,
                                           pami_context_t              context,
                                           pami_dispatch_hint_t        hint,
                                           T_Allocator               & allocator,
                                           pami_result_t             & result)
      {
        COMPILE_TIME_ASSERT(sizeof(SendWrapper) <= T_Allocator::objsize);
        SendWrapper * sw = (SendWrapper *) allocator.allocateObject ();
        new ((void *)sw) SendWrapper (dispatch, dispatch_fn, cookie, device, origin, hint, result);
        if (result != PAMI_SUCCESS)
            {
              allocator.returnObject (sw);
              sw = NULL;
            }
        return sw;
      }
  private:
    lapi_state_t                          *_lapi_state;
  };

  // Device Typedefs
  typedef Device::CAUDevice                                           CAUDevice;
  typedef Device::BSRDevice                                           BSRDevice;
  typedef Device::BSRMsyncMessage                                     BSRMsyncMessage;
  typedef Device::BSRMcastMessage                                     BSRMcastMessage;
  typedef Device::BSRMcombineMessage                                  BSRMcombineMessage;
  typedef Device::BSRMultisyncModel<BSRDevice,BSRMsyncMessage>        BSRMultisyncModel;
  typedef Device::BSRMulticastModel<BSRDevice,BSRMcastMessage>        BSRMulticastModel;
  typedef Device::BSRMulticombineModel<BSRDevice,BSRMcombineMessage>  BSRMulticombineModel;

  // P2P Message Typedefs
  typedef PAMI::SendWrapper                                           LAPISendBase;
  typedef PAMI::Protocol::Send::SendPWQ < LAPISendBase >              LAPISend;

  // Shared Memory P2P Typedefs
  typedef Fifo::FifoPacket <P2PSHM_HDRSIZE,P2PSHM_PKTSIZE>            ShmemPacket;
  typedef Fifo::LinearFifo<ShmemPacket, Counter::Indirect<Counter::Native> > ShmemFifo;
  typedef Device::ShmemDevice<ShmemFifo, Counter::Indirect<Counter::Native>, Device::Shmem::NoShaddr, 128 > ShmemDevice;
  typedef Device::Shmem::PacketModel<ShmemDevice>                     ShmemPacketModel;
  typedef Protocol::Send::Eager <ShmemPacketModel>                    ShmemEagerBase;
  typedef PAMI::Protocol::Send::SendWrapperPWQ < ShmemEagerBase >            ShmemEager;

  // "New" Collective Message Typedefs
  typedef Device::CAUMsyncMessage                                     CAUMsyncMessage;
  typedef Device::CAUMcastSendMessage                                 CAUMcastSendMessage;
  typedef Device::CAUMcombineMessage                                  CAUMcombineMessage;
  typedef Device::CAUM2MMessage                                       CAUM2MMessage;

  // P2P Model Classes:  None here, LAPI component implements p2p

  // "New" Collective Model typedefs
  typedef Device::CAUMultisyncModel<CAUDevice,CAUMsyncMessage>        CAUMultisyncModel;
  typedef Device::CAUMulticastModel<CAUDevice,CAUMcastSendMessage>    CAUMulticastModel;
  typedef Device::CAUMulticombineModel<CAUDevice,
                                       CAUMcombineMessage>            CAUMulticombineModel;


  // "OverP2P Collective Native Interface Typedefs
  typedef PAMI::NativeInterfaceActiveMessage<LAPISend>                LAPISendNI_AM;
  typedef PAMI::NativeInterfaceAllsided<LAPISend>                     LAPISendNI_AS;
  typedef PAMI::NativeInterfaceActiveMessage<ShmemEager>              ShmemEagerNI_AM;
  typedef PAMI::NativeInterfaceAllsided<ShmemEager>                   ShmemEagerNI_AS;
  typedef PAMI::NativeInterfaceActiveMessage< Protocol::Send::SendPWQ<Protocol::Send::Send> > CompositeNI_AM;
  typedef PAMI::NativeInterfaceAllsided< Protocol::Send::SendPWQ<Protocol::Send::Send> >      CompositeNI_AS;
  
  // Geometry Typedefs
  typedef Geometry::Lapi                                              LAPIGeometry;

  // Protocol Typedefs

  // Memory Allocator Typedefs
  typedef MemoryAllocator<1024, 16> ProtocolAllocator;


  // PGAS RT Typedefs/Coll Registration
  typedef TSPColl::NBCollManager<CompositeNI_AM> LAPINBCollManager;
  typedef CollRegistration::PGASRegistration<LAPIGeometry,
                                             CompositeNI_AM,
                                             ProtocolAllocator,
                                             LAPISend,
                                             ShmemEager,
                                             DeviceWrapper,
                                             ShmemDevice,
                                             LAPINBCollManager> PGASCollreg;

  // Over P2P CCMI Protocol Typedefs
  typedef CollRegistration::P2P::CCMIRegistration<LAPIGeometry,
                                                  ShmemDevice,
                                                  DeviceWrapper,
                                                  ProtocolAllocator,
                                                  ShmemEager,
                                                  ShmemDevice,
                                                  ShmemEagerNI_AM,
                                                  ShmemEagerNI_AS,
                                                  LAPISend,
                                                  DeviceWrapper,
                                                  LAPISendNI_AM,
                                                  LAPISendNI_AS,
                                                  CompositeNI_AM,
                                                  CompositeNI_AS> P2PCCMICollreg;


  // Collective Shmem Protocol Typedefs
  typedef Atomic::NativeAtomic                                                   LAPICSAtomic;
  typedef Counter::Native                                                        LAPICSCounter;
  typedef Mutex::YieldingNative                                                  LAPICSMutex;
  typedef PAMI::Memory::CollSharedMemoryManager<LAPICSAtomic,LAPICSMutex,LAPICSCounter,COLLSHM_SEGSZ,COLLSHM_PAGESZ,
                                    COLLSHM_WINGROUPSZ,COLLSHM_BUFSZ>            LAPICSMemoryManager;
  typedef PAMI::Device::CollShm::CollShmDevice<LAPICSAtomic, LAPICSMemoryManager,
                             COLLSHM_DEVICE_NUMSYNCS, COLLSHM_DEVICE_SYNCCOUNT>  LAPICSDevice;
  typedef PAMI::Device::CollShm::CollShmModel<LAPICSDevice, LAPICSMemoryManager> LAPICollShmModel;
  typedef PAMI::Device::CSNativeInterface<LAPICollShmModel>                      LAPICSNativeInterface;
  typedef PAMI::CollRegistration::CCMICSMultiRegistration<LAPIGeometry,
                   LAPICSNativeInterface, LAPICSMemoryManager, LAPICollShmModel> LAPICollShmCollreg;

  // "New" CCMI Protocol Typedefs
  typedef PAMI::Device::DeviceNativeInterface<CAUDevice,
                                              CAUMulticastModel,
                                              CAUMultisyncModel,
                                              CAUMulticombineModel>   CAUNativeInterface;


  typedef PAMI::Device::DeviceNativeInterface<BSRDevice,
                                              BSRMulticastModel,
                                              BSRMultisyncModel,
                                              BSRMulticombineModel>   BSRNativeInterface;


  typedef CollRegistration::CAU::CAURegistration<LAPIGeometry,
                                                 PAMI::Device::Generic::Device,
                                                 BSRDevice,
                                                 CAUDevice,
                                                 LAPICSNativeInterface,
                                                 CAUNativeInterface,
                                                 BSRNativeInterface,
                                                 LAPICollShmModel,
                                                 LAPICSMemoryManager>  CAUCollreg;

  typedef Geometry::ClassRouteId<LAPIGeometry> LAPIClassRouteId;



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
    inline pami_result_t generate(size_t clientid, size_t num_ctx, Memory::MemoryManager &mm, bool disable_shmem=false) {
        // these calls create (allocate and construct) each element.
        // We don't know how these relate to contexts, they are semi-opaque.
        _generics = PAMI::Device::Generic::Device::Factory::generate(clientid, num_ctx, mm, NULL);
        if(disable_shmem==false)
          _shmem    = ShmemDevice::Factory::generate(clientid, num_ctx, mm, _generics);
        else
          _shmem    = NULL;
        
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
     * \param[in] clientid     Client ID (index)
     * \param[in] num_ctx      Number of contexts in this client
     * \param[in] ctx          Context opaque entity
     * \param[in] contextid    Context ID (index)
     */
    inline pami_result_t init(size_t clientid, size_t contextid, pami_client_t clt, pami_context_t ctx, PAMI::Memory::MemoryManager *mm, bool disable_shmem=false) {
        PAMI::Device::Generic::Device::Factory::init(_generics, clientid, contextid, clt, ctx, mm, _generics);
        if(disable_shmem==false)
          ShmemDevice::Factory::init(_shmem, clientid, contextid, clt, ctx, mm, _generics);
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
        if(_shmem)
          events += ShmemDevice::Factory::advance(_shmem, clientid, contextid);
        return events;
    }
    PAMI::Device::Generic::Device        *_generics; // need better name...
    ShmemDevice                          *_shmem;
  }; // class PlatformDeviceList


    class Context : public Interface::Context<PAMI::Context>
    {
    public:
    inline Context (pami_client_t                        client,
                    size_t                               clientid,
                    char                                *clientname,
                    size_t                               id,
                    PlatformDeviceList                  *devices,
                    std::map<unsigned, pami_geometry_t> *geometry_map):
        Interface::Context<PAMI::Context> (client, id),
        _client (client),
        _clientid (clientid),
        _clientname(clientname),
        _dispatch_id(255),
        _context((pami_context_t) this),
        _contextid (id),
        _world_geometry(NULL),
        _geometry_map(geometry_map),
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
          LapiImpl::Client* lp_client = (LapiImpl::Client*)_client;

          LapiImpl::Context::Config config(lp_client->GetConfig(), NULL, 0);

          pami_result_t rc = PAMI_SUCCESS;

          try {
            LapiImpl::Context::Create(config, (LapiImpl::Context *)_lapi_state);
          } catch (int lapi_err) {
            /* convert to general PAMI error */
            rc = PAMI_ERROR;
          } catch (internal_rc_t int_err) {
            rc = PAMI_RC(int_err);
          } catch (std::bad_alloc) {
            rc = PAMI_ENOMEM;
          }

          // This lock prevents any dispatches from happening
          // before we can initialize the message layer
          lock();

          if (PAMI_SUCCESS != rc)
            return rc;

          _lapi_handle = ((lapi_state_t*)_lapi_state)->my_hndl;

          // Initialize the lapi device for collectives
          _lapi_device.init((lapi_state_t*)_lapi_state);
          _cau_device.init((lapi_state_t*)_lapi_state,
                           _lapi_handle,
                           _client,
                           _clientid,
                           _context,
                           _contextid,
                          &_dispatch_id);
#ifndef _LAPI_LINUX
          _bsr_device.init(_client, _context, _contextid, _Lapi_env.MP_child);
#endif
          *out_mysize        = _Lapi_env.MP_procs;
          *out_myrank        = _Lapi_env.MP_child;
          *out_lapi_handle   = _lapi_handle;

          return PAMI_SUCCESS;
        }

      inline pami_result_t initP2PCollectives()
        {
          // Initalize Collective Registration
	  pami_result_t rc;
	  rc = __global.heap_mm->memalign((void **)&_pgas_collreg, 0,
								sizeof(*_pgas_collreg));
	  PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc PGASCollreg");
          new(_pgas_collreg) PGASCollreg(_client,_context,_clientid,_contextid,
                                         _protocol,_lapi_device,_devices->_shmem[_contextid],
                                         &_dispatch_id,_geometry_map, false);
          _world_geometry->resetUEBarrier(); // Reset so pgas will select the UE barrier
          _pgas_collreg->analyze(_contextid,_world_geometry);
          return PAMI_SUCCESS;
        }

      inline pami_result_t initCollectives(Memory::MemoryManager               *mm,
                                           bool                                 disable_shm)
        {
          Memory::MemoryManager               *mm_ptr;
          if(disable_shm) mm_ptr = NULL;
          else            mm_ptr = mm;
          PAMI::Topology *local_master_topo = (PAMI::Topology *) ((PAMI::Geometry::Lapi *)_world_geometry)->getTopology(PAMI::Geometry::MASTER_TOPOLOGY_INDEX);
	  pami_result_t rc;
          uint64_t *invec;
	  rc = __global.heap_mm->memalign((void **)&invec, 0,
                                          (3 + local_master_topo->size()) * sizeof(uint64_t));
	  PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc invec");
          // for cau classroute initialization
          invec[2]  = 0xFFFFFFFFFFFFFFFFULL;
          for (int i = 0; i < local_master_topo->size(); ++i)  invec[3+i] = 0ULL;
	  rc = __global.heap_mm->memalign((void **)&_p2p_ccmi_collreg, 0,
						sizeof(*_p2p_ccmi_collreg));
	  PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc P2PCCMICollreg");
          size_t numpeers=0, numtasks;
          __global.mapping.nodePeers(numpeers);
          numtasks = __global.mapping.size();

#ifndef _LAPI_LINUX          
          _bsr_device.setGenericDevices(_devices->_generics);
#endif          
          _cau_device.setGenericDevices(_devices->_generics);
	  rc = __global.heap_mm->memalign((void **)&_cau_collreg, 0,
						sizeof(*_cau_collreg));
	  PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc CAUCollreg");
          new(_cau_collreg) CAUCollreg(_client,
                                       _context,
                                       _contextid,
                                       _clientid,
                                       *_devices->_generics,
                                       _bsr_device,
                                       _cau_device,
                                       __global.mapping,
                                       _lapi_handle,
                                       &_dispatch_id,
                                       _geometry_map,
                                       mm_ptr);
          // We analyze global here to get the proper device specific info
          _cau_collreg->receive_global(_contextid, _world_geometry, &invec[2], 1);

          new(_p2p_ccmi_collreg) P2PCCMICollreg(_client,
                                                _context,
                                                _contextid,
                                                _clientid,
                                                _devices->_shmem[_contextid],
                                                _lapi_device,
                                                _protocol,
                                                mm_ptr?1:0,  //use shared memory
                                                1,  //use "global" device
                                                numtasks,
                                                numpeers,
                                                &_dispatch_id,
                                                _geometry_map);
          _p2p_ccmi_collreg->analyze(_contextid, _world_geometry);
          _pgas_collreg->setGenericDevice(&_devices->_generics[_contextid]);

          __global.heap_mm->free(invec);
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
          LapiImpl::Context *ep = (LapiImpl::Context *)(lapi_state_t*)_lapi_state;
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
          work = new (state) PAMI::Device::Generic::GenericThread(work_fn, cookie);
          _devices->_generics[_contextid].postThread(work);
          return PAMI_SUCCESS;
        }

      inline size_t advance_impl (size_t maximum, pami_result_t & result)
        {
          LapiImpl::Context *cp = (LapiImpl::Context *)&_lapi_state[0];
          (cp->*(cp->pLock))();
          while (maximum --)
           {
             size_t events = _devices->advance(_clientid, _contextid);
             internal_rc_t rc = (cp->*(cp->pAdvance))(1);
             if (rc == 0 || events > 0) {
               result = PAMI_SUCCESS;
               (cp->*(cp->pUnlock))();
               return 1;
             }
           }
          result = PAMI_EAGAIN;
          (cp->*(cp->pUnlock))();
          return 0;
        }

      inline size_t advance_only_lapi (size_t maximum, pami_result_t & result)
        {
          LapiImpl::Context *cp = (LapiImpl::Context *)&_lapi_state[0];
          internal_rc_t rc = (cp->*(cp->pAdvance))(maximum);
          result = PAMI_RC(rc);
          return 0;
        }

      inline pami_result_t lock_impl ()
        {
          LapiImpl::Context *ep = (LapiImpl::Context *)&_lapi_state[0];
          ep->mutex.Lock<true>();
          return PAMI_SUCCESS;
        }

      inline pami_result_t trylock_impl ()
        {
          LapiImpl::Context *ep = (LapiImpl::Context *)&_lapi_state[0];
          if (0 == ep->mutex.TryLock<true>())
            return PAMI_SUCCESS;
          else
            return PAMI_EAGAIN;
        }

      inline pami_result_t unlock_impl ()
        {
          LapiImpl::Context *ep = (LapiImpl::Context *)&_lapi_state[0];
          ep->mutex.Unlock<true>();
          return PAMI_SUCCESS;
        }
      inline pami_result_t send_impl (pami_send_t * parameters)
        {
          LapiImpl::Context *cp = (LapiImpl::Context *)&_lapi_state[0];
          internal_rc_t rc = (cp->*(cp->pSend))(parameters->send.dest,
                                                   parameters->send.dispatch,         // hdr_hdl
                                                   parameters->send.header.iov_base,  // uhdr
                                                   parameters->send.header.iov_len,   // uhdr_len
                                                   parameters->send.data.iov_base,    // udata
                                                   parameters->send.data.iov_len,     // udata_len
                                                   parameters->send.hints,                             // send hints
                                                   parameters->events.local_fn,       //
                                                   parameters->events.remote_fn,      //
                                                   parameters->events.cookie,         //
                                                   NULL, NULL,                        //  unused send completion handler
                                                   NULL, NULL, NULL,                  // unused counter
                                                   INTERFACE_PAMI,                    // caller
                                                   FLAG_NULL); 
          return PAMI_RC(rc);
        }

      inline pami_result_t send_impl (pami_send_immediate_t * send)
        {
          LapiImpl::Context *cp = (LapiImpl::Context *)&_lapi_state[0];
          internal_rc_t rc = (cp->*(cp->pSendSmall))(send->dest, send->dispatch,
                  send->header.iov_base, send->header.iov_len,
                  send->data.iov_base, send->data.iov_len, send->hints, FLAG_NULL);
          return PAMI_RC(rc);
        }

      inline pami_result_t send_impl (pami_send_typed_t * send_typed)
        {
          LapiImpl::Context *cp = (LapiImpl::Context *)&_lapi_state[0];
          return (cp->*(cp->pSendTyped))(send_typed);
        }

      inline pami_result_t put_impl (pami_put_simple_t * put)
        {
          LapiImpl::Context *cp = (LapiImpl::Context *)&_lapi_state[0];
          internal_rc_t rc = (cp->*(cp->pPut))
              (put->rma.dest, put->addr.local, NULL,
               put->addr.remote, NULL, put->rma.bytes,
               put->rma.hints, INTERFACE_PAMI,
               (void*)put->rma.done_fn, (void*)put->put.rdone_fn,
               put->rma.cookie, NULL, NULL, NULL);
          return PAMI_RC(rc);
        }

      inline pami_result_t put_typed_impl (pami_put_typed_t * put_typed)
        {
          LapiImpl::Context *cp = (LapiImpl::Context *)&_lapi_state[0];
          return (cp->*(cp->pPutTyped))(put_typed);
        }

      inline pami_result_t get_impl (pami_get_simple_t * get)
        {
          LapiImpl::Context *cp = (LapiImpl::Context *)&_lapi_state[0];
          internal_rc_t rc = (cp->*(cp->pGet))(get->rma.dest, get->addr.local, NULL,
                  get->addr.remote, NULL, get->rma.bytes,
                  get->rma.hints, INTERFACE_PAMI,
                  (void *)get->rma.done_fn, get->rma.cookie, NULL, NULL);
          return PAMI_RC(rc);
        }

      inline pami_result_t get_typed_impl (pami_get_typed_t * get_typed)
        {
          LapiImpl::Context *cp = (LapiImpl::Context *)&_lapi_state[0];
          return (cp->*(cp->pGetTyped))(get_typed);
        }

      inline pami_result_t rmw_impl (pami_rmw_t * rmw)
        {
          LapiImpl::Context *cp = (LapiImpl::Context *)&_lapi_state[0];
          size_t            len = ((TypeCode *)rmw->type)->GetDataSize();
          RMW_input_t       input;

          if (4 == len) {
            if (rmw->operation & ~(_OP_FETCH_COMPARE))
                input.int32.in_val   = *(int32_t *)(rmw->value);
            if (rmw->operation & _OP_COMPARE)
                input.int32.test_val = *(int32_t *)(rmw->test);
          } else {
            if (rmw->operation & ~(_OP_FETCH_COMPARE))
                input.int64.in_val   = *(int64_t *)(rmw->value);
            if (rmw->operation & _OP_COMPARE)
                input.int64.test_val = *(int64_t *)(rmw->test);
          }

          // disable fetching compare result on PAMI flow
          void *local = (rmw->operation & _OP_FETCH) ? rmw->local : NULL;

          internal_rc_t rc = 
            (cp->*(cp->pRmw))(rmw->dest, local, rmw->remote, len,
                (AtomicOps)rmw->operation, input, rmw->hints, INTERFACE_PAMI,
                (void*)rmw->done_fn, rmw->cookie, NULL);

          return PAMI_RC(rc);
        }

      inline pami_result_t memregion_create_impl (void             * address,
                                                  size_t             bytes_in,
                                                  size_t           * bytes_out,
                                                  pami_memregion_t * memregion)
        {
          LapiImpl::Context *cp = (LapiImpl::Context *)&_lapi_state[0];
          internal_rc_t rc =
              (cp->*(cp->pRegisterMem))(address, bytes_in, bytes_out,
                      (void*)memregion, PAMI_CLIENT_MEMREGION_SIZE_STATIC);
          return PAMI_RC(rc);
        }

      inline pami_result_t memregion_destroy_impl (pami_memregion_t * memregion)
        {
          // Do nothing but set memregion to a invalid value
          memset(memregion, 0, sizeof(pami_memregion_t));
          return PAMI_SUCCESS;
        }


      inline pami_result_t rput_impl (pami_rput_simple_t * parameters)
        {
          LapiImpl::Context *cp = (LapiImpl::Context *)&_lapi_state[0];
          MemRegion *local_mr   = (MemRegion*)(parameters->rdma.local.mr);
          void      *local_buf  = (void*)(local_mr->basic.user_ptr + parameters->rdma.local.offset);
          MemRegion *remote_mr  = (MemRegion*)(parameters->rdma.remote.mr);
          void      *remote_buf = (void*)(remote_mr->basic.user_ptr + parameters->rdma.remote.offset);
          /* pass both local_mr and remote_mr will enable eager rdma 
           * if hint.use_rdma != force_off */
          internal_rc_t rc = (cp->*(cp->pPut))
              (parameters->rma.dest, local_buf, local_mr,
               remote_buf, remote_mr, parameters->rma.bytes,
               parameters->rma.hints, INTERFACE_PAMI,
               (void*)parameters->rma.done_fn, (void*)parameters->put.rdone_fn,
               parameters->rma.cookie, NULL, NULL, NULL);
          return PAMI_RC(rc);
        }

      inline pami_result_t rput_typed_impl (pami_rput_typed_t * parameters)
        {
          LapiImpl::Context *cp = (LapiImpl::Context *)&_lapi_state[0];
          /* we use PAMI_Put_typed for now */
          MemRegion *local_mr   = (MemRegion*)(parameters->rdma.local.mr);
          void      *local_buf  = (void*)(local_mr->basic.user_ptr + parameters->rdma.local.offset);
          MemRegion *remote_mr  = (MemRegion*)(parameters->rdma.remote.mr);
          void      *remote_buf = (void*)(remote_mr->basic.user_ptr + parameters->rdma.remote.offset);

          pami_put_typed_t put_typed;
          put_typed.rma         = parameters->rma;
          put_typed.type        = parameters->type;
          put_typed.put         = parameters->put;
          put_typed.addr.local  = local_buf;
          put_typed.addr.remote = remote_buf;

          return (cp->*(cp->pPutTyped))(&put_typed);
        }

      inline pami_result_t rget_impl (pami_rget_simple_t * parameters)
        {
          LapiImpl::Context *cp = (LapiImpl::Context *)&_lapi_state[0];
          MemRegion *local_mr   = (MemRegion*)(parameters->rdma.local.mr);
          void      *local_buf  = (void*)(local_mr->basic.user_ptr + parameters->rdma.local.offset);
          MemRegion *remote_mr  = (MemRegion*)(parameters->rdma.remote.mr);
          void      *remote_buf = (void*)(remote_mr->basic.user_ptr + parameters->rdma.remote.offset);
          /* pass both local_mr and remote_mr will enable eager rdma 
           * if hint.use_rdma != force_off */
          internal_rc_t rc = 
              (cp->*(cp->pGet))(parameters->rma.dest,
                      local_buf, local_mr, remote_buf, remote_mr, parameters->rma.bytes,
                      parameters->rma.hints, INTERFACE_PAMI,
                      (void *)parameters->rma.done_fn, parameters->rma.cookie, NULL, NULL);
          return PAMI_RC(rc);
        }

      inline pami_result_t rget_typed_impl (pami_rget_typed_t * parameters)
        {
          LapiImpl::Context *cp = (LapiImpl::Context *)&_lapi_state[0];
          /* we use PAMI_Get_typed for now */
          MemRegion *local_mr   = (MemRegion*)(parameters->rdma.local.mr);
          void      *local_buf  = (void*)(local_mr->basic.user_ptr + parameters->rdma.local.offset);
          MemRegion *remote_mr  = (MemRegion*)(parameters->rdma.remote.mr);
          void      *remote_buf = (void*)(remote_mr->basic.user_ptr + parameters->rdma.remote.offset);

          pami_get_typed_t get_typed;
          get_typed.rma         = parameters->rma;
          get_typed.type        = parameters->type;
          get_typed.addr.local  = local_buf;
          get_typed.addr.remote = remote_buf;

          return (cp->*(cp->pGetTyped))(&get_typed);
        }

      inline pami_result_t purge_totask_impl (pami_endpoint_t * dest, size_t count)
        {
          LapiImpl::Context *cp = (LapiImpl::Context *)_lapi_state;
          internal_rc_t rc;
          for (int i=0; i<count; i++) {
              pami_endpoint_t *tgt = (pami_endpoint_t *)dest + i;
              rc = (cp->*(cp->pPurge))(*tgt, INTERFACE_PAMI);
          }
          return PAMI_RC(rc);
        }

      inline pami_result_t resume_totask_impl (pami_endpoint_t * dest, size_t count)
        {
          LapiImpl::Context *cp = (LapiImpl::Context *)_lapi_state;
          internal_rc_t rc;
          for (int i=0; i<count; i++) {
              pami_endpoint_t *tgt = (pami_endpoint_t *)dest + i;
              rc = (cp->*(cp->pResume))(*tgt, INTERFACE_PAMI);
          }
          return PAMI_RC(rc);
        }

      inline pami_result_t fence_begin_impl ()
        {
          // This function is no-op in PERCS for now
          return PAMI_SUCCESS; 
        }

      inline pami_result_t fence_end_impl ()
        {
          // This function is no-op in PERCS for now
          return PAMI_SUCCESS; 
        }

      inline pami_result_t fence_all_impl (pami_event_function   done_fn,
                                          void               * cookie)
        {
          LapiImpl::Context *cp = (LapiImpl::Context *)_lapi_state;
          return (cp->*(cp->pFenceAll))(done_fn, cookie);
        }

      inline  pami_result_t fence_endpoint_impl (pami_event_function   done_fn,
                                                 void                * cookie,
                                                 pami_endpoint_t       endpoint)
        {
          LapiImpl::Context *cp = (LapiImpl::Context *)_lapi_state;
          return (cp->*(cp->pFenceEndpoint))(done_fn, cookie, endpoint);
        }


    inline pami_result_t collective_impl (pami_xfer_t * parameters)
        {
        Geometry::Algorithm<LAPIGeometry> *algo = (Geometry::Algorithm<LAPIGeometry> *)parameters->algorithm;
        return algo->generate(parameters);
        }

    inline pami_result_t amcollective_dispatch_impl (pami_algorithm_t            algorithm,
                                                    size_t                     dispatch,
                                                    pami_dispatch_callback_function fn,
                                                    void                     * cookie,
                                                    pami_collective_hint_t      options)
        {
        Geometry::Algorithm<LAPIGeometry> *algo = (Geometry::Algorithm<LAPIGeometry> *)algorithm;
        return algo->dispatch_set(dispatch, fn, cookie, options);
        }


      inline pami_result_t geometry_algorithms_num_impl (pami_geometry_t geometry,
                                                        pami_xfer_type_t colltype,
                                                        size_t *lists_lengths)
        {
        LAPIGeometry *_geometry = (LAPIGeometry*) geometry;
        return _geometry->algorithms_num(colltype, lists_lengths, _contextid);
        }

      inline pami_result_t geometry_algorithms_info_impl (pami_geometry_t    geometry,
                                                          pami_xfer_type_t   colltype,
                                                          pami_algorithm_t  *algs0,
                                                          pami_metadata_t   *mdata0,
                                                          size_t                num0,
                                                          pami_algorithm_t  *algs1,
                                                          pami_metadata_t   *mdata1,
                                                          size_t                num1)
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

      inline pami_result_t dispatch_impl (size_t                          id,
                                          pami_dispatch_callback_function fn,
                                          void                          * cookie,
                                          pami_dispatch_hint_t            options)
        {
          LapiImpl::Context  *cp = (LapiImpl::Context *)&_lapi_state[0];
          internal_rc_t rc =
              (cp->*(cp->pDispatchSet))(id, (void *)fn.p2p, cookie,
                      options, INTERFACE_PAMI);
          return PAMI_RC(rc);
        }

      inline lapi_state_t *getLapiState()
        {
          return (lapi_state_t*)_lapi_state;
        }

      inline pami_result_t dispatch_query_impl(size_t                dispatch,
                                               pami_configuration_t  configuration[],
                                               size_t                num_configs)
        {
          pami_result_t result = PAMI_SUCCESS;
          size_t i;
          for(i=0; i<num_configs; i++)
            {
              switch (configuration[i].name)
                {
                  default:
                  {
                    internal_rc_t rc;
                    lapi_state_t *lp      = getLapiState();
                    LapiImpl::Context *cp = (LapiImpl::Context *)lp;
                    rc = (cp->*(cp->pConfigQuery))(configuration);
                    if(rc != SUCCESS)
                      result = PAMI_INVAL;
                  }
                }
            }
          return result;
        }

      inline pami_result_t dispatch_update_impl(size_t                dispatch,
                                                pami_configuration_t  configuration[],
                                                size_t                num_configs)
        {
          pami_result_t result = PAMI_SUCCESS;
          size_t i;
          for(i=0; i<num_configs; i++)
            {
              switch (configuration[i].name)
                {
                  default:
                  {
                    internal_rc_t rc;
                    lapi_state_t *lp      = getLapiState();
                    LapiImpl::Context *cp = (LapiImpl::Context *)lp;
                    rc = (cp->*(cp->pConfigUpdate))(configuration);
                    if(rc != SUCCESS)
                      result = PAMI_INVAL;
                  }
                }
            }
          return result;
        }

      inline pami_result_t query_impl(pami_configuration_t  configuration[],
                                      size_t                num_configs)
        {
          pami_result_t result = PAMI_SUCCESS;
          size_t i;
          for(i=0; i<num_configs; i++)
            {
              switch (configuration[i].name)
                {
                  default:
                  {
                    internal_rc_t rc;
                    lapi_state_t *lp      = getLapiState();
                    LapiImpl::Context *cp = (LapiImpl::Context *)lp;
                    rc = (cp->*(cp->pConfigQuery))(configuration);
                    if(rc != SUCCESS)
                      result = PAMI_INVAL;
                  }
                }
            }
          return result;
        }

      inline pami_result_t update_impl(pami_configuration_t  configuration[],
                                       size_t                num_configs)
        {
          pami_result_t result = PAMI_SUCCESS;
          size_t i;
          for(i=0; i<num_configs; i++)
            {
              switch (configuration[i].name)
                {
                  default:
                  {
                    internal_rc_t rc;
                    lapi_state_t *lp      = getLapiState();
                    LapiImpl::Context *cp = (LapiImpl::Context *)lp;
                    rc = (cp->*(cp->pConfigUpdate))(configuration);
                    if(rc != SUCCESS)
                      result = PAMI_INVAL;
                  }
                }
            }
          return result;
        }

    private:
      /*  Lapi State Object.  use this for direct access        */
      /*  Warning!  do not put any variables before this        */
      /*  We rely on this being the first element of the        */
      /*  class for lapi/pami compatibility                     */
      char                                   _lapi_state[sizeof(LapiImpl::Context)];

      /*  PAMI Client Pointer associated with this PAMI Context */
      pami_client_t                          _client;

      /*  Context pointer to this client                        */
      pami_context_t                         _context;

      /*  Context id (offset) of this context in the client     */
      size_t                                 _contextid;

      /*  Client id of this client                              */
      size_t                                 _clientid;

      /*  Pointer to the client name string                     */
      char                                  *_clientname;

      /*  This is the "per context dispatch", and is used by    */
      /*  collectives, should start from 255 and decrease       */
      int                                    _dispatch_id;
      
      /*  Protocol allocator                                    */
      ProtocolAllocator                      _protocol;

      /*  The over lapi devices                                 */
      DeviceWrapper                          _lapi_device;
      CAUDevice                              _cau_device;

      BSRDevice                              _bsr_device;
  public:
      /*  Collective Registrations                              */
      PGASCollreg                           *_pgas_collreg;
      P2PCCMICollreg                        *_p2p_ccmi_collreg;
      CAUCollreg                            *_cau_collreg;

      /*  World Geometry Pointer for this context               */
      LAPIGeometry                          *_world_geometry;

      /*  Client scoped comm id to geomtry map                  */
      std::map<unsigned, pami_geometry_t>   *_geometry_map;
      
  private:
      lapi_handle_t                          _lapi_handle;
      PlatformDeviceList                    *_devices;
    }; // end PAMI::Context
}; // end namespace PAMI

#endif // __pami_lapi_lapicontext_h__
