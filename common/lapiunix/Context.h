/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* This is an automatically generated copyright prolog.             */
/* After initializing,  DO NOT MODIFY OR MOVE                       */
/*  --------------------------------------------------------------- */
/* Licensed Materials - Property of IBM                             */
/* Blue Gene/Q 5765-PER 5765-PRP                                    */
/*                                                                  */
/* (C) Copyright IBM Corp. 2011, 2012 All Rights Reserved           */
/* US Government Users Restricted Rights -                          */
/* Use, duplication, or disclosure restricted                       */
/* by GSA ADP Schedule Contract with IBM Corp.                      */
/*                                                                  */
/*  --------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
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
#include "common/default/Dispatch.h"
#include "common/lapiunix/lapifunc.h"
#include "common/lapiunix/Client.h"
#include "common/ContextInterface.h"
#include "components/devices/BaseDevice.h"
#include "components/memory/MemoryManager.h"

// Geometry
#include "algorithms/geometry/Geometry.h"

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
#include "algorithms/geometry/FCACollRegistration.h"
#include "algorithms/geometry/ClassRouteId.h"

// Collective shmem device
#include "components/devices/cshmem/CollSharedMemoryManager.h"
#include "components/devices/cshmem/CollShmDevice.h"
#include "components/devices/NativeInterface.h"
#include "common/NativeInterfaceFactory.h"


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
        Context *cp      = (Context *)_lapi_state;
        return PAMI_Context_advance((pami_context_t) cp, 1);
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
        (void)origin;
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
    inline pami_result_t typed (pami_send_typed_t * typed)
      {
        LapiImpl::Context *cp = (LapiImpl::Context *)_lapi_state;
        // No error code conversion required for typed send
        return (cp->*(cp->pSendTyped))(typed);
      }
    inline void setPWQAllocator(void * allocator)
      {
        _pwqAllocator = allocator;
      }
    inline void * getPWQAllocator()
       {
         return _pwqAllocator;
       }
    inline pami_result_t getAttributes (pami_configuration_t  configuration[],
                                        size_t                num_configs)
      {
        LapiImpl::Context *cp = (LapiImpl::Context *)_lapi_state;
        pami_result_t result = PAMI_SUCCESS;
        size_t i;
        for(i=0; i<num_configs; i++)
        {
           internal_rc_t rc;
           rc = (cp->*(cp->pConfigQuery))(configuration);
           if(rc != SUCCESS)
             result = PAMI_INVAL;
        }
        return result;
      }


     template <class T_MemoryManager>
     static inline SendWrapper * generate (size_t                      dispatch,
                                           pami_dispatch_p2p_function  dispatch_fn,
                                           void                      * cookie,
                                           PAMI::DeviceWrapper       & device,
                                           pami_endpoint_t             origin,
                                           pami_context_t              context,
                                           pami_dispatch_hint_t        hint,
                                           T_MemoryManager           * mm,
                                           pami_result_t             & result)
      {
        SendWrapper * sw = NULL;
        result = mm->memalign((void **)&sw, 16, sizeof(SendWrapper));
        PAMI_assert_alwaysf(result == PAMI_SUCCESS, "Failed to get memory for send wrapper");
        new ((void *)sw) SendWrapper (dispatch, dispatch_fn, cookie, device, origin, hint, result);
        if (result != PAMI_SUCCESS)
            {
              mm->free(sw);
              sw = NULL;
            }
        return sw;
      }
  private:
    lapi_state_t                          *_lapi_state;
    void                                  *_pwqAllocator;
  };

  // Device Typedefs
  typedef Device::CAUDevice                                           CAUDevice;
  typedef Device::BSRDevice                                           BSRDevice;
  typedef Device::BSRDevice::BSRMsyncMessage                          BSRMsyncMessage;
  typedef Device::BSRDevice::BSRMcastMessage                          BSRMcastMessage;
  typedef Device::BSRDevice::BSRMcombineMessage                       BSRMcombineMessage;
  typedef Device::BSRMultisyncModel<BSRDevice,BSRMsyncMessage>        BSRMultisyncModel;
  typedef Device::BSRMulticastModel<BSRDevice,BSRMcastMessage>        BSRMulticastModel;
  typedef Device::BSRMulticombineModel<BSRDevice,BSRMcombineMessage>  BSRMulticombineModel;

  // P2P Message Typedefs
  typedef PAMI::SendWrapper                                           LAPISendBase;
  typedef PAMI::Protocol::Send::SendPWQ < LAPISendBase >              LAPISend;

  // Shared Memory P2P Typedefs
  typedef Fifo::FifoPacket <P2PSHM_HDRSIZE,P2PSHM_PKTSIZE>            ShmemPacket;
  typedef Fifo::LinearFifo<ShmemPacket, Counter::Indirect<Counter::Native> > ShmemFifo;
  typedef Device::ShmemDevice<ShmemFifo,
                              Counter::Indirect<Counter::Native>,
                              Device::Shmem::NoShaddr,
                              128,
                              4096 > ShmemDevice;
  typedef Device::Shmem::PacketModel<ShmemDevice>                     ShmemPacketModel;
  typedef Protocol::Send::Eager <ShmemPacketModel>                    ShmemEagerBase;
  typedef PAMI::Protocol::Send::SendWrapperPWQ < ShmemEagerBase >     ShmemEager;

  // "New" Collective Message Typedefs
  typedef Device::CAUMsyncMessage                                     CAUMsyncMessage;
  typedef Device::CAUMcastMessage                                     CAUMcastMessage;
  typedef Device::CAUMcombineMessage                                  CAUMcombineMessage;

  // P2P Model Classes:  None here, LAPI component implements p2p

  // "New" Collective Model typedefs
  typedef Device::CAUMultisyncModel<CAUDevice,CAUMsyncMessage>        CAUMultisyncModel;
  typedef Device::CAUMulticastModel<CAUDevice,CAUMcastMessage>        CAUMulticastModel;
  typedef Device::CAUMulticombineModel<CAUDevice,
                                       CAUMcombineMessage>            CAUMulticombineModel;


  // "OverP2P Collective Native Interface Typedefs
  typedef PAMI::NativeInterfaceActiveMessage<LAPISend>                LAPISendNI_AM;
  typedef PAMI::NativeInterfaceAllsided<LAPISend>                     LAPISendNI_AS;
  typedef PAMI::NativeInterfaceActiveMessage<ShmemEager>              ShmemEagerNI_AM;
  typedef PAMI::NativeInterfaceAllsided<ShmemEager>                   ShmemEagerNI_AS;
  typedef PAMI::NativeInterfaceActiveMessage< Protocol::Send::SendPWQ<Protocol::Send::Send> > CompositeNI_AM;
  typedef PAMI::NativeInterfaceAllsided< Protocol::Send::SendPWQ<Protocol::Send::Send> >      CompositeNI_AS;
  // For active message based collectives we need support for sending large headers 
  typedef PAMI::NativeInterfaceActiveMessage<LAPISend, 2>                LAPISendNI_AM_AMC;
  typedef PAMI::NativeInterfaceAllsided<LAPISend, 2>                     LAPISendNI_AS_AMC;
  typedef PAMI::NativeInterfaceActiveMessage<ShmemEager, 2>              ShmemEagerNI_AM_AMC;
  typedef PAMI::NativeInterfaceAllsided<ShmemEager, 2>                   ShmemEagerNI_AS_AMC;
  typedef PAMI::NativeInterfaceActiveMessage< Protocol::Send::SendPWQ<Protocol::Send::Send>, 2 > CompositeNI_AM_AMC;
  typedef PAMI::NativeInterfaceAllsided< Protocol::Send::SendPWQ<Protocol::Send::Send>, 2 >      CompositeNI_AS_AMC;
  // Geometry Typedefs
  typedef Geometry::Common                                            PEGeometry;

  // Protocol Typedefs

  // Memory Allocator Typedefs
  typedef MemoryAllocator<1024, 16> ProtocolAllocator;


  // Collective Shmem Protocol Typedefs
  typedef Atomic::NativeAtomic                                                   LAPICSAtomic;
  typedef Counter::Native                                                        LAPICSCounter;
  typedef Mutex::YieldingNative                                                  LAPICSMutex;
  typedef PAMI::Memory::CollSharedMemoryManager<LAPICSAtomic,
                                                LAPICSMutex,
                                                LAPICSCounter,
                                                COLLSHM_CTLCNT,COLLSHM_BUFCNT,COLLSHM_LGBUFCNT,
                                                COLLSHM_WINGROUPSZ,COLLSHM_BUFSZ,COLLSHM_LGBUFSZ>
  LAPICSMemoryManager;


  // PGAS RT Typedefs/Coll Registration
  typedef xlpgas::CollectiveManager<CompositeNI_AM> LAPINBCollManager;
  typedef CollRegistration::PGASRegistration<PEGeometry,
                                             CompositeNI_AM,
                                             ProtocolAllocator,
                                             LAPISend,
                                             ShmemEager,
                                             DeviceWrapper,
                                             ShmemDevice,
                                             LAPINBCollManager,
                                             LAPICSMemoryManager> PGASCollreg;

  // Over P2P CCMI Protocol Typedefs
  typedef NativeInterfaceCommon::NativeInterfaceFactory <ProtocolAllocator,  
                                                         LAPISendNI_AM,  
                                                         LAPISendNI_AS, 
                                                         LAPISend, 
                                                         DeviceWrapper>       LapiNIFactory;

  typedef NativeInterfaceCommon::NativeInterfaceFactory2Device <ProtocolAllocator, 
                                                                CompositeNI_AM, 
                                                                CompositeNI_AS, 
                                                                ShmemEager, 
                                                                ShmemDevice, 
                                                                LAPISend, 
                                                                DeviceWrapper> CompositeNIFactory;  

  typedef NativeInterfaceCommon::NativeInterfaceFactory <ProtocolAllocator,
                                                         LAPISendNI_AM_AMC,
                                                         LAPISendNI_AS_AMC,
                                                         LAPISend,
                                                         DeviceWrapper>       LapiNIFactory_AMC;

  typedef NativeInterfaceCommon::NativeInterfaceFactory2Device <ProtocolAllocator, 
                                                                CompositeNI_AM_AMC, 
                                                                CompositeNI_AS_AMC, 
                                                                ShmemEager, 
                                                                ShmemDevice, 
                                                                LAPISend, 
                                                                DeviceWrapper> CompositeNIFactory_AMC;  

  typedef CollRegistration::P2P::CCMIRegistration<PEGeometry,
                                                  ProtocolAllocator,
                                                  CCMI::Adaptor::P2PBarrier::BinomialBarrier,
                                                  CCMI::Adaptor::P2PBarrier::BinomialBarrierFactory > P2PCCMICollreg;


  // Collective Shmem Protocol Typedefs
  typedef Atomic::NativeAtomic                                                   LAPICSAtomic;
  typedef Counter::Native                                                        LAPICSCounter;
  typedef Mutex::YieldingNative                                                  LAPICSMutex;
  typedef PAMI::Memory::CollSharedMemoryManager<LAPICSAtomic,
                                                LAPICSMutex,
                                                LAPICSCounter,
                                                COLLSHM_CTLCNT,COLLSHM_BUFCNT,COLLSHM_LGBUFCNT,
                                                COLLSHM_WINGROUPSZ,COLLSHM_BUFSZ,COLLSHM_LGBUFSZ> LAPICSMemoryManager;

  typedef PAMI::Device::CollShm::CollShmDevice<LAPICSAtomic,
                                               LAPICSMemoryManager,
                                               COLLSHM_DEVICE_NUMSYNCS,
                                               COLLSHM_DEVICE_SYNCCOUNT>         LAPICSDevice;
  typedef PAMI::Device::CollShm::CollShmModel<LAPICSDevice, LAPICSMemoryManager> LAPICollShmModel;
  typedef PAMI::Device::CSNativeInterface<LAPICollShmModel>                      LAPICSNativeInterface;

  // "New" CCMI Protocol Typedefs
  typedef PAMI::Device::DeviceNativeInterface<CAUDevice,
                                              CAUMulticastModel,
                                              CAUMultisyncModel,
                                              CAUMulticombineModel>   CAUNativeInterface;


  typedef PAMI::Device::DeviceNativeInterface<BSRDevice,
                                              BSRMulticastModel,
                                              BSRMultisyncModel,
                                              BSRMulticombineModel>   BSRNativeInterface;


  typedef CollRegistration::CAU::CAURegistration<PEGeometry,
                                                 PAMI::Device::Generic::Device,
                                                 BSRDevice,
                                                 CAUDevice,
                                                 LAPICSNativeInterface,
                                                 CAUNativeInterface,
                                                 BSRNativeInterface,
                                                 LAPICollShmModel,
                                                 DeviceWrapper,
                                                 LAPISendNI_AM,
                                                 ProtocolAllocator,
                                                 LAPISend,
                                                 LAPICSMemoryManager>  CAUCollreg;
  typedef CollRegistration::FCA::FCARegistration<PEGeometry>            FCACollreg;

  typedef Geometry::ClassRouteId<PEGeometry> LAPIClassRouteId;



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

#define SHM_DIRECT 0
    class Context : public Interface::Context<PAMI::Context>
    {
    public:
    inline Context (pami_client_t                        client,
                    size_t                               clientid,
                    char                                *clientname,
                    size_t                               id,
                    PlatformDeviceList                  *devices):
        Interface::Context<PAMI::Context> (client, id),
        _client (client),
        _context((pami_context_t) this),
        _contextid (id),
        _clientid (clientid),
        _clientname(clientname),
        _dispatch_id(4095),
        _pgas_collreg(NULL),
        _p2p_ccmi_collreg(NULL),
        _cau_collreg(NULL),
        _fca_collreg(NULL),
        _devices(devices)
#if SHM_DIRECT==1        
        ,_dispatch(this)
#endif        
      {
#if SHM_DIRECT==1
        char *env = getenv("MP_S_SHM_DIRECT");
        _shm_direct_on = false;
        if(env && atoi(env)) _shm_direct_on = true;
#endif
      }

      inline pami_result_t initP2P(pami_configuration_t cxt_config[],
                                   size_t         num_configs,
                                   size_t        *out_myrank,
                                   size_t        *out_mysize,
                                   lapi_handle_t *out_lapi_handle)
        {
          LapiImpl::Client* lp_client = (LapiImpl::Client*)_client;

          //TODO: need to get context configuration
          LapiImpl::Context::Config config(lp_client->GetConfig(), cxt_config, num_configs);

          pami_result_t rc = PAMI_SUCCESS;

          try {
            LapiImpl::Context::Create(lp_client, config, (LapiImpl::Context *)_lapi_state);
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

          *out_mysize        = _Lapi_env.MP_procs;
          *out_myrank        = _Lapi_env.MP_child;
          *out_lapi_handle   = _lapi_handle;

          return PAMI_SUCCESS;
        }

      inline pami_result_t initDevices(bool      affinity_checked,
                                       unsigned  cau_uniqifier)
        {
          _cau_device.init((lapi_state_t*)_lapi_state,
                           _lapi_handle,
                           _client,
                           _clientid,
                           _context,
                           _contextid,
                           cau_uniqifier,
                          &_dispatch_id);

          _bsr_device.init(_client, 
                           _context, 
                           _contextid, 
                           _Lapi_env.MP_child,
                           affinity_checked);

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
          // destroy is non-blocking now and can return EGAIN
          if(_cau_collreg) {
              _cau_collreg->invalidateContext();
              _cau_collreg = NULL;
          }
          LapiImpl::Context *cp = (LapiImpl::Context *)&_lapi_state[0];
          internal_rc_t rc = (cp->*(cp->pTerm))();
          return PAMI_RC(rc);
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
          if (cp->IsMultiThreaded() && (ERR_EAGAIN == (cp->*(cp->pTryLock))())) {
              result = PAMI_EAGAIN;
              return 0;
          }

          while (maximum --)
           {
             size_t events = _devices->advance(_clientid, _contextid);
             if (0 == (cp->*(cp->pAdvance))() || events > 0) {
               result = PAMI_SUCCESS;
               if (cp->IsMultiThreaded()) (cp->*(cp->pUnlock))();
               return 1;
             }
           }
          result = PAMI_EAGAIN;
          if (cp->IsMultiThreaded()) (cp->*(cp->pUnlock))();
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


      inline pami_result_t plock ()
        {
          LapiImpl::Context *cp = (LapiImpl::Context *)&_lapi_state[0];
          (cp->*(cp->pLock))();
          return PAMI_SUCCESS;
        }

      inline pami_result_t punlock ()
        {
          LapiImpl::Context *cp = (LapiImpl::Context *)&_lapi_state[0];
          (cp->*(cp->pUnlock))();
          return PAMI_SUCCESS;
        }

      inline bool shm_direct_on()
        {
#if SHM_DIRECT==1
          return _shm_direct_on;
#else
          return false;
#endif          
        }

      inline pami_result_t send_impl (pami_send_t * parameters)
        {
          // Compiler should optimize out these branches when SHM_DIRECT=0
          pami_result_t rc = PAMI_ERROR;
          if(SHM_DIRECT && shm_direct_on())
          {
#if SHM_DIRECT==1
            rc = _dispatch.send (parameters);
#endif            
          }
          if(rc != PAMI_SUCCESS)
          {
            LapiImpl::Context *cp = (LapiImpl::Context *)&_lapi_state[0];
            internal_rc_t rc = (cp->*(cp->pSend))(parameters->send.dest,
                                                  parameters->send.dispatch,         // hdr_hdl
                                                  parameters->send.header.iov_base,  // uhdr
                                                  parameters->send.header.iov_len,   // uhdr_len
                                                  parameters->send.data.iov_base,    // udata
                                                  parameters->send.data.iov_len,     // udata_len
                                                  parameters->send.hints,            // send hints
                                                  parameters->events.local_fn,       //
                                                  parameters->events.remote_fn,      //
                                                  parameters->events.cookie,         //
                                                  NULL, NULL,                        //  unused send completion handler
                                                  NULL, NULL, NULL,                  // unused counter
                                                  INTERFACE_PAMI,                    // caller
                                                  FLAG_NULL);
            return PAMI_RC(rc);
          }
          return rc;
        }

      inline pami_result_t send_impl (pami_send_immediate_t * send)
        {
          // Compiler should optimize out these branches when SHM_DIRECT=0
          pami_result_t rc = PAMI_ERROR;
          if(SHM_DIRECT && shm_direct_on())
          {
#if SHM_DIRECT==1            
            rc = _dispatch.send (send);
#endif            
          }
          if(rc != PAMI_SUCCESS)
          {
            LapiImpl::Context *cp = (LapiImpl::Context *)&_lapi_state[0];
            internal_rc_t rc = (cp->*(cp->pSendSmall))(send->dest, send->dispatch,
                                                       send->header.iov_base, send->header.iov_len,
                                                       send->data.iov_base, send->data.iov_len, send->hints, FLAG_NULL);
            return PAMI_RC(rc);
          }
          return rc;
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
          LapiImpl::Context *cp = (LapiImpl::Context *)&_lapi_state[0];
          internal_rc_t rc =
              (cp->*(cp->pUnregisterMem))(memregion, PAMI_CLIENT_MEMREGION_SIZE_STATIC);
          memset(memregion, 0, sizeof(pami_memregion_t));
          return PAMI_RC(rc);
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
          internal_rc_t rc = SUCCESS;
          for (int i=0; i<(ssize_t)count; i++) {
              pami_endpoint_t *tgt = (pami_endpoint_t *)dest + i;
              rc = (cp->*(cp->pPurge))(*tgt, INTERFACE_PAMI);
          }
          return PAMI_RC(rc);
        }

      inline pami_result_t resume_totask_impl (pami_endpoint_t * dest, size_t count)
        {
          LapiImpl::Context *cp = (LapiImpl::Context *)_lapi_state;
          internal_rc_t rc = SUCCESS;
          for (int i=0; i<(ssize_t)count; i++) {
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
          pami_result_t rc;
          plock();
          PEGeometry::ContextMap *cm = (PEGeometry::ContextMap*)parameters->algorithm;
          rc = (*cm)[_contextid].generate(parameters);
          punlock();
          return rc;
        }

    inline pami_result_t amcollective_dispatch_impl (pami_algorithm_t               algorithm,
                                                    size_t                          dispatch,
                                                    pami_dispatch_callback_function fn,
                                                    void                           *cookie,
                                                    pami_collective_hint_t          options)
        {
          std::map<size_t,Geometry::Algorithm<PEGeometry> > *algo =
            (std::map<size_t,Geometry::Algorithm<PEGeometry> > *)algorithm;
          return (*algo)[0].dispatch_set(_contextid, dispatch, fn, cookie, options);
        }



      inline pami_result_t dispatch_impl (size_t                          id,
                                          pami_dispatch_callback_function fn,
                                          void                          * cookie,
                                          pami_dispatch_hint_t            options)
        {
#if SHM_DIRECT==1          
          pami_result_t   result;
          pami_endpoint_t self = PAMI_ENDPOINT_INIT(_clientid, __global.mapping.task(), _contextid);
          Protocol::Send::Send * send = ShmemEagerBase::generate (id,
                                                                  fn.p2p,
                                                                  cookie,
                                                                  _devices->_shmem[_contextid],
                                                                  self,
                                                                  _context,
                                                                  options,
                                                                  __global.heap_mm,
                                                                  result);
          _dispatch.set (id, send);
          assert(result == PAMI_SUCCESS);
#endif          
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
          (void)dispatch;
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
                    rc = (cp->*(cp->pConfigQuery))(&configuration[i]);
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
          (void)dispatch;
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
                    rc = (cp->*(cp->pConfigUpdate))(&configuration[i]);
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
                    rc = (cp->*(cp->pConfigQuery))(&configuration[i]);
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
                    rc = (cp->*(cp->pConfigUpdate))(&configuration[i]);
                    if(rc != SUCCESS)
                      result = PAMI_INVAL;
                  }
                }
            }
          return result;
        }
      inline void registerUnexpBarrier_impl (unsigned     comm,
                                             pami_quad_t &info,
                                             unsigned     peer,
                                             unsigned     algorithm)
      {
        Geometry::UnexpBarrierQueueElement *ueb =
          (Geometry::UnexpBarrierQueueElement *) _ueb_allocator.allocateObject();
        new (ueb) Geometry::UnexpBarrierQueueElement (comm, _contextid, info, peer, algorithm);
        _ueb_queue.pushTail(ueb);
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
  public:
      /*  This is the "per context dispatch", and is used by    */
      /*  collectives, should start from 255 and decrease       */
      int                                    _dispatch_id;

      /*  The over lapi devices                                 */
      DeviceWrapper                          _lapi_device;
      CAUDevice                              _cau_device;
      BSRDevice                              _bsr_device;

      /*  Protocol allocator                                    */
      ProtocolAllocator                      _protocol;

      /*  Collective Registrations                              */
      PGASCollreg                           *_pgas_collreg;
      P2PCCMICollreg                        *_p2p_ccmi_collreg;
      CAUCollreg                            *_cau_collreg;
      FCACollreg                            *_fca_collreg;

      lapi_handle_t                          _lapi_handle;
      PlatformDeviceList                    *_devices;

      //  Unexpected Barrier match queue
      MatchQueue<>                           _ueb_queue;

      //  Unexpected Barrier allocator
      MemoryAllocator <sizeof(PAMI::Geometry::UnexpBarrierQueueElement), 16> _ueb_allocator;

      // shared memory direct, using bg device
#if SHM_DIRECT==1
      bool                                   _shm_direct_on;
      Dispatch<4096>                         _dispatch;
#endif      
  private:
    }; // end PAMI::Context
}; // end namespace PAMI

#endif // __pami_lapi_lapicontext_h__
