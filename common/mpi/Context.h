///
/// \file common/mpi/Context.h
/// \brief PAMI MPI specific context implementation.
///
#ifndef __common_mpi_Context_h__
#define __common_mpi_Context_h__

#define ENABLE_GENERIC_DEVICE
#define ENABLE_SHMEM_DEVICE

#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include "common/ContextInterface.h"
#include "algorithms/geometry/Geometry.h"
#include "WakeupManager.h"
#include "components/devices/mpi/mpidevice.h"
#include "components/devices/mpi/mpipacketmodel.h"
#include "components/devices/mpi/mpimessage.h"

#include "p2p/protocols/send/eager/Eager.h"
#include "p2p/protocols/send/composite/Composite.h"

#include "SysDep.h"
#include "components/devices/generic/Device.h"
#include "components/devices/misc/ProgressFunctionMsg.h"
#include "components/devices/misc/AtomicBarrierMsg.h"
#include "components/devices/workqueue/WQRingReduceMsg.h"
#include "components/devices/workqueue/WQRingBcastMsg.h"
#include "components/devices/workqueue/LocalAllreduceWQMessage.h"
#include "components/devices/workqueue/LocalReduceWQMessage.h"
#include "components/devices/workqueue/LocalBcastWQMessage.h"
#include "components/devices/workqueue/WQRingBcastMsg.h"
#include "components/devices/workqueue/LocalBcastWQMessage.h"
#include "components/devices/mpi/MPIBcastMsg.h"
#include "components/devices/mpi/MPISyncMsg.h"
#include "algorithms/geometry/CCMICollRegistration.h"
#include "algorithms/geometry/PGASCollRegistration.h"
#include "algorithms/geometry/OldCCMICollRegistration.h"
#include "Mapping.h"
#include <new>
#include <map>
#include "components/atomic/counter/CounterMutex.h"
#include "components/atomic/gcc/GccCounter.h"
#include <sched.h>

#ifdef ENABLE_SHMEM_DEVICE
#include "components/devices/shmem/ShmemDevice.h"
#include "components/devices/shmem/ShmemPacketModel.h"
#include "util/fifo/FifoPacket.h"
#include "util/fifo/LinearFifo.h"
#endif

#include "components/devices/mpi/mpimulticastprotocol.h"
#include "components/devices/mpi/mpimulticastmodel.h"
#include "components/devices/mpi/mpimultisyncmodel.h"
#include "components/devices/mpi/mpimulticombinemodel.h"
#include "components/devices/mpi/mpimanytomanymodel.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x)// fprintf x
#endif

namespace PAMI
{
    // This won't work with XL
    typedef PAMI::Mutex::CounterMutex<PAMI::Counter::GccProcCounter>  ContextLock;
    typedef Device::MPIMessage MPIMessage;
    typedef Device::MPIDevice MPIDevice;
    typedef Device::MPIPacketModel<MPIDevice,MPIMessage> MPIPacketModel;
    typedef PAMI::Protocol::Send::Eager <MPIPacketModel,MPIDevice> EagerMPI;

    // \todo I do not distinguish local vs non-local so no eager shmem protocol here... just EagerMPI
  typedef PAMI::Protocol::MPI::P2PMcastProto<MPIDevice,
                                            EagerMPI,
                                            PAMI::Device::MPIBcastMdl,
                                            PAMI::Device::MPIBcastDev> P2PMcastProto;
    typedef PAMI::Mutex::CounterMutex<PAMI::Counter::GccProcCounter>  ContextLock;

#ifdef ENABLE_SHMEM_DEVICE
    typedef Fifo::FifoPacket <64, 1024> ShmemPacket;
    typedef Fifo::LinearFifo<Atomic::GccBuiltin, ShmemPacket, 128> ShmemFifo;
    typedef Device::ShmemDevice<ShmemFifo> ShmemDevice;
    typedef Device::Shmem::PacketModel<ShmemDevice> ShmemModel;
#endif

    typedef MemoryAllocator<1024, 16> ProtocolAllocator;

  // Collective Types
  typedef Geometry::Common                                           MPIGeometry;
  typedef Device::MPIMsyncMessage                                    MPIMsyncMessage;
  typedef Device::MPIMultisyncModel<MPIDevice,MPIMsyncMessage>       MPIMultisyncModel;
  typedef Device::MPIMcastMessage                                    MPIMcastMessage;
  typedef Device::MPIMulticastModel<MPIDevice,MPIMcastMessage>       MPIMulticastModel;
  typedef Device::MPIMcombineMessage                                 MPIMcombineMessage;
  typedef Device::MPIMulticombineModel<MPIDevice,
                                       MPIMcombineMessage>           MPIMulticombineModel;
  typedef Device::MPIM2MMessage                                      MPIM2MMessage;
  typedef Device::MPIManytomanyModel<MPIDevice,MPIM2MMessage>        MPIManytomanyModel;
  typedef PAMI::Device::MPIOldmulticastModel<PAMI::Device::MPIDevice,
                                            PAMI::Device::MPIMessage> MPIOldMcastModel;
  typedef PAMI::Device::MPIOldm2mModel<PAMI::Device::MPIDevice,
                                      PAMI::Device::MPIMessage,
                                      size_t>                        MPIOldM2MModel;
  // "New" CCMI Typedefs/Coll Registration
  typedef PAMI::MPINativeInterface<MPIDevice,
                                  MPIMulticastModel,
                                  MPIMultisyncModel,
                                  MPIMulticombineModel,
                                  OneSided>              DefaultNativeInterface;

  typedef PAMI::MPINativeInterface<MPIDevice,
                                  MPIMulticastModel,
                                  MPIMultisyncModel,
                                  MPIMulticombineModel,
                                  AllSided>              DefaultNativeInterfaceAS;

  typedef CollRegistration::CCMIRegistration<MPIGeometry,
                                             DefaultNativeInterface,
                                             DefaultNativeInterfaceAS,
                                             MPIDevice> CCMICollreg;
  // PGAS RT Typedefs/Coll Registration
  typedef PAMI::Device::MPIOldmulticastModel<PAMI::Device::MPIDevice,
                                            PAMI::Device::MPIMessage> MPIOldMcastModel;
  typedef TSPColl::NBCollManager<MPIOldMcastModel> MPINBCollManager;
  typedef CollRegistration::PGASRegistration<MPIGeometry,
                                             MPIOldMcastModel,
                                             MPIDevice,
                                             MPINBCollManager> PGASCollreg;

  typedef CollRegistration::OldCCMIRegistration<MPIGeometry,
                                                MPIOldMcastModel,
                                                MPIOldM2MModel,
                                                MPIDevice,
                                                SysDep> OldCCMICollreg;

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
        _progfunc = PAMI::Device::ProgressFunctionDev::Factory::generate(clientid, num_ctx, mm);
        _atombarr = PAMI::Device::AtomicBarrierDev::Factory::generate(clientid, num_ctx, mm);
        _wqringreduce = PAMI::Device::WQRingReduceDev::Factory::generate(clientid, num_ctx, mm);
        _wqringbcast = PAMI::Device::WQRingBcastDev::Factory::generate(clientid, num_ctx, mm);
        _localallreduce = PAMI::Device::LocalAllreduceWQDevice::Factory::generate(clientid, num_ctx, mm);
        _localbcast = PAMI::Device::LocalBcastWQDevice::Factory::generate(clientid, num_ctx, mm);
        _localreduce = PAMI::Device::LocalReduceWQDevice::Factory::generate(clientid, num_ctx, mm);
        _mpimsync = PAMI::Device::MPISyncDev::Factory::generate(clientid, num_ctx, mm);
        _mpimcast = PAMI::Device::MPIBcastDev::Factory::generate(clientid, num_ctx, mm);
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
        PAMI::Device::ProgressFunctionDev::Factory::init(_progfunc, clientid, contextid, clt, ctx, mm, _generics);
        PAMI::Device::AtomicBarrierDev::Factory::init(_atombarr, clientid, contextid, clt, ctx, mm, _generics);
        PAMI::Device::WQRingReduceDev::Factory::init(_wqringreduce, clientid, contextid, clt, ctx, mm, _generics);
        PAMI::Device::WQRingBcastDev::Factory::init(_wqringbcast, clientid, contextid, clt, ctx, mm, _generics);
        PAMI::Device::LocalAllreduceWQDevice::Factory::init(_localallreduce, clientid, contextid, clt, ctx, mm, _generics);
        PAMI::Device::LocalBcastWQDevice::Factory::init(_localbcast, clientid, contextid, clt, ctx, mm, _generics);
        PAMI::Device::LocalReduceWQDevice::Factory::init(_localreduce, clientid, contextid, clt, ctx, mm, _generics);

        PAMI::Device::MPISyncDev::Factory::init(_mpimsync, clientid, contextid, clt, ctx, mm, _generics);
        PAMI::Device::MPIBcastDev::Factory::init(_mpimcast, clientid, contextid, clt, ctx, mm, _generics);
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
        events += PAMI::Device::ProgressFunctionDev::Factory::advance(_progfunc, clientid, contextid);
        events += PAMI::Device::AtomicBarrierDev::Factory::advance(_atombarr, clientid, contextid);
        events += PAMI::Device::WQRingReduceDev::Factory::advance(_wqringreduce, clientid, contextid);
        events += PAMI::Device::WQRingBcastDev::Factory::advance(_wqringbcast, clientid, contextid);
        events += PAMI::Device::LocalAllreduceWQDevice::Factory::advance(_localallreduce, clientid, contextid);
        events += PAMI::Device::LocalBcastWQDevice::Factory::advance(_localbcast, clientid, contextid);
        events += PAMI::Device::LocalReduceWQDevice::Factory::advance(_localreduce, clientid, contextid);
        events += PAMI::Device::MPISyncDev::Factory::advance(_mpimsync, clientid, contextid);
        events += PAMI::Device::MPIBcastDev::Factory::advance(_mpimcast, clientid, contextid);
        return events;
    }

    PAMI::Device::Generic::Device *_generics; // need better name...
#ifdef ENABLE_SHMEM_DEVICE
    ShmemDevice *_shmem;
#endif
    PAMI::Device::ProgressFunctionDev *_progfunc;
    PAMI::Device::AtomicBarrierDev *_atombarr;
    PAMI::Device::WQRingReduceDev *_wqringreduce;
    PAMI::Device::WQRingBcastDev *_wqringbcast;
    PAMI::Device::LocalAllreduceWQDevice *_localallreduce;
    PAMI::Device::LocalBcastWQDevice *_localbcast;
    PAMI::Device::LocalReduceWQDevice *_localreduce;
    PAMI::Device::MPISyncDev *_mpimsync;
    PAMI::Device::MPIBcastDev *_mpimcast;
  }; // class PlatformDeviceList

    class Context : public Interface::Context<PAMI::Context>
    {
    public:
      inline Context (pami_client_t client, size_t clientid, size_t id, size_t num,
                                PlatformDeviceList *devices,
                                void * addr, size_t bytes,
                                MPIGeometry *world_geometry) :
        Interface::Context<PAMI::Context> (client, id),
        _client (client),
        _clientid (clientid),
        _contextid (id),
        _mm (addr, bytes),
        _sysdep(_mm),
        _lock (),
#warning This needs to be done elsewhere - not per-context if in __global!
        _mpi(&__global.mpi_device),
        _world_geometry(world_geometry),
        _minterface(*_mpi, client, this, _contextid, clientid),
        _empty_advance(0),
        _devices(devices)
        {
          // dispatch_impl relies on the table being initialized to NULL's.
          memset(_dispatch, 0x00, sizeof(_dispatch));

        _pgas_collreg=(PGASCollreg*) malloc(sizeof(*_pgas_collreg));
        new(_pgas_collreg) PGASCollreg(client, (pami_context_t)this, id,*_mpi);
        _pgas_collreg->analyze(_contextid,_world_geometry);

        _oldccmi_collreg=(OldCCMICollreg*) malloc(sizeof(*_oldccmi_collreg));
        new(_oldccmi_collreg) OldCCMICollreg(client, (pami_context_t)this, id,_sysdep,*_mpi);
        _oldccmi_collreg->analyze(_contextid, _world_geometry);

        _ccmi_collreg=(CCMICollreg*) malloc(sizeof(*_ccmi_collreg));
        new(_ccmi_collreg) CCMICollreg(client, (pami_context_t)this, id,clientid,*_mpi);
        _ccmi_collreg->analyze(_contextid, _world_geometry);

#ifdef USE_WAKEUP_VECTORS
          _wakeupManager.init(1, 0x57550000 | id); // check errors?
#endif // USE_WAKEUP_VECTORS
          _devices->init(_clientid, _contextid, _client, _context, &_mm);
          _mpi->init(&_mm, _clientid, num, (pami_context_t)this, id);
          _lock.init(&_mm);

          // this barrier is here because the shared memory init
          // needs to be synchronized
          // we shoudl find a way to remove this
          MPI_Barrier(MPI_COMM_WORLD);
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
          // Do not call finalize because if we do
          // it is not valid to call init again
          // per the MPI spec.
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
          result = PAMI_SUCCESS;
          size_t events = 0;

          unsigned i;
          for (i=0; i<maximum && events==0; i++)
              {
#warning Does this _mpi device still belong here?
        events += _mpi->advance_impl();
        events += _devices->advance(_clientid, _contextid);

                if(events == 0)
                  _empty_advance++;
                else
                  _empty_advance=0;
              }
          if(_empty_advance==10)
              {
                sched_yield();
                _empty_advance=0;
              }

#ifdef USE_WAKEUP_VECTORS
          // this is only for advances that are allowed to sleep...
          if (!events) {
                  _wakeupManager.sleep(xxx); // check errors?
                goto repeat;
          }
#endif // USE_WAKEUP_VECTORS

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
        TRACE_ERR((stderr, ">> Context::send_impl('simple'), _dispatch[%zu][0] = %p\n", id, _dispatch[id][0]));
        PAMI_assert_debug (_dispatch[id][0] != NULL);

        PAMI::Protocol::Send::Send * send =
          (PAMI::Protocol::Send::Send *) _dispatch[id][0];


        TRACE_ERR((stderr, "<< Context::send_impl('simple')\n"));
        return send->simple (parameters);
        }

      inline pami_result_t send_impl (pami_send_immediate_t * parameters)
        {
        size_t id = (size_t)(parameters->dispatch);
        TRACE_ERR((stderr, ">> Context::send_impl('immediate'), _dispatch[%zu][0] = %p\n", id, _dispatch[id][0]));
        PAMI_assert_debug (_dispatch[id][0] != NULL);

        PAMI::Protocol::Send::Send * send =
          (PAMI::Protocol::Send::Send *) _dispatch[id][0];


        TRACE_ERR((stderr, "<< Context::send_impl('immediate')\n"));
        return send->immediate (parameters);
        }

      inline pami_result_t send_impl (pami_send_typed_t * parameters)
        {
          PAMI_abort();
          return PAMI_UNIMPL;
        }

      inline pami_result_t put_impl (pami_put_simple_t * parameters)
        {
          PAMI_abort();
          return PAMI_UNIMPL;
        }

      inline pami_result_t put_typed_impl (pami_put_typed_t * parameters)
        {
          PAMI_abort();
          return PAMI_UNIMPL;
        }

      inline pami_result_t get_impl (pami_get_simple_t * parameters)
        {
          PAMI_abort();
          return PAMI_UNIMPL;
        }

      inline pami_result_t get_typed_impl (pami_get_typed_t * parameters)
        {
          PAMI_abort();
          return PAMI_UNIMPL;
        }

      inline pami_result_t rmw_impl (pami_rmw_t * parameters)
        {
          PAMI_abort();
          return PAMI_UNIMPL;
        }

      inline pami_result_t memregion_register_impl (void            * address,
                                                   size_t            bytes,
                                                   pami_memregion_t * memregion)
        {
          PAMI_abort();
          return PAMI_UNIMPL;
        }

      inline pami_result_t memregion_deregister_impl (pami_memregion_t * memregion)
        {
          PAMI_abort();
          return PAMI_UNIMPL;
        }


      inline pami_result_t memregion_query_impl (pami_memregion_t    memregion,
                                                void            ** address,
                                                size_t           * bytes,
                                                size_t           * task)
        {
          PAMI_abort();
          return PAMI_UNIMPL;
        }


      inline pami_result_t memregion_register_impl (pami_rmw_t * parameters)
        {
          PAMI_abort();
          return PAMI_UNIMPL;
        }

      inline pami_result_t rput_impl (pami_rput_simple_t * parameters)
        {
          PAMI_abort();
          return PAMI_UNIMPL;
        }

      inline pami_result_t rput_typed_impl (pami_rput_typed_t * parameters)
        {
          PAMI_abort();
          return PAMI_UNIMPL;
        }

      inline pami_result_t rget_impl (pami_rget_simple_t * parameters)
        {
          PAMI_abort();
          return PAMI_UNIMPL;
        }

      inline pami_result_t rget_typed_impl (pami_rget_typed_t * parameters)
        {
          PAMI_abort();
          return PAMI_UNIMPL;
        }

      inline pami_result_t purge_totask_impl (size_t * dest, size_t count)
        {
          PAMI_abort();
          return PAMI_UNIMPL;
        }

      inline pami_result_t resume_totask_impl (size_t * dest, size_t count)
        {
          PAMI_abort();
          return PAMI_UNIMPL;
        }

      inline pami_result_t fence_begin_impl ()
        {
          PAMI_abort();
          return PAMI_UNIMPL;
        }

      inline pami_result_t fence_end_impl ()
        {
          PAMI_abort();
          return PAMI_UNIMPL;
        }

      inline pami_result_t fence_all_impl (pami_event_function   done_fn,
                                          void               * cookie)
        {
          PAMI_abort();
          return PAMI_UNIMPL;
        }

      inline  pami_result_t fence_task_impl (pami_event_function   done_fn,
                                            void               * cookie,
                                            size_t               task)
        {
          PAMI_abort();
          return PAMI_UNIMPL;
        }

        inline pami_result_t geometry_algorithms_num_impl (pami_geometry_t geometry,
                                                          pami_xfer_type_t colltype,
                                                          int *lists_lengths)
        {
        MPIGeometry *_geometry = (MPIGeometry*) geometry;
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
        MPIGeometry *_geometry = (MPIGeometry*) geometry;
        return _geometry->algorithms_info(colltype,
                                          algs0,
                                          mdata0,
                                          num0,
                                          algs1,
                                          mdata1,
                                          num1,
                                          _contextid);
      }

    inline pami_result_t collective_impl (pami_xfer_t * parameters)
      {
        Geometry::Algorithm<MPIGeometry> *algo = (Geometry::Algorithm<MPIGeometry> *)parameters->algorithm;
        return algo->generate(parameters);
        }

    inline pami_result_t amcollective_dispatch_impl (pami_algorithm_t            algorithm,
                                                    size_t                     dispatch,
                                                    pami_dispatch_callback_fn   fn,
                                                    void                     * cookie,
                                                    pami_collective_hint_t      options)
      {
        Geometry::Algorithm<MPIGeometry> *algo = (Geometry::Algorithm<MPIGeometry> *)algorithm;
        return algo->dispatch_set(dispatch, fn, cookie, options);
      }

      inline pami_result_t multisend_getroles_impl(size_t          dispatch,
                                                  int            *numRoles,
                                                  int            *replRole)
        {
          PAMI_abort();
          return PAMI_UNIMPL;
        }
      inline pami_result_t multicast_impl(pami_multicast_t *mcastinfo)
      {
      size_t id = (size_t)(mcastinfo->dispatch);
      TRACE_ERR((stderr, ">> multicast_impl, _dispatch[%zu/%zu] = %p\n", id, mcastinfo->dispatch, _dispatch[id][0]));
      PAMI_assert_debug (_dispatch[id][0] != NULL);

      // \todo A COMPLETE TEMPORARY HACK - since Mike gave us two dispatch table entries, we used the
      // second (unused by mcast) entry to store an id of what we put in the first entry.  Now
      // we know what class to pull out of _dispatch[][0] and (eventually) what allocator to use.
      PAMI_assert_debug (_dispatch[id][1] > (void*)0 && _dispatch[id][1] < (void*)6);


      if(_dispatch[id][1] == (void*)2) // see HACK comment above
      {
        typedef uint8_t mcast_storage_t[P2PMcastProto::sizeof_msg];
        P2PMcastProto * multicast = (P2PMcastProto *) _dispatch[id][0];
        TRACE_ERR((stderr, ">> multicast_impl, one sided multicast %p\n", multicast));
        mcast_storage_t * msgbuf = (mcast_storage_t*)malloc(P2PMcastProto::sizeof_msg);/// \todo memleak
        multicast->multicast(*msgbuf,mcastinfo);
      }
      else if(_dispatch[id][1] == (void*)3) // see HACK comment above
      {
        typedef uint8_t mcast_storage_t[PAMI::Device::WQRingBcastMdl::sizeof_msg];
        PAMI::Device::WQRingBcastMdl * multicast = (PAMI::Device::WQRingBcastMdl*) _dispatch[id][0];
        TRACE_ERR((stderr, ">> multicast_impl, all sided ring multicast %p\n", multicast));
        mcast_storage_t * msgbuf = (mcast_storage_t*)malloc(PAMI::Device::WQRingBcastMdl::sizeof_msg);/// \todo memleak
        multicast->postMulticast(*msgbuf,mcastinfo);
      }
      else if(_dispatch[id][1] == (void*)4) // see HACK comment above
      {
        typedef uint8_t mcast_storage_t[PAMI::Device::LocalBcastWQModel::sizeof_msg];
        PAMI::Device::LocalBcastWQModel  * multicast = (PAMI::Device::LocalBcastWQModel *) _dispatch[id][0];
        TRACE_ERR((stderr, ">> multicast_impl, all sided multicast %p\n", multicast));
        mcast_storage_t * msgbuf = (mcast_storage_t*)malloc(PAMI::Device::LocalBcastWQModel::sizeof_msg);/// \todo memleak
        multicast->postMulticast(*msgbuf,mcastinfo);
      }
      else if(_dispatch[id][1] == (void*)5) // see HACK comment above
      {
        typedef uint8_t mcast_storage_t[PAMI::Device::MPIBcastMdl::sizeof_msg];
        PAMI::Device::MPIBcastMdl  * multicast = (PAMI::Device::MPIBcastMdl *) _dispatch[id][0];
        TRACE_ERR((stderr, ">> multicast_impl, all sided global multicast %p\n", multicast));
        mcast_storage_t * msgbuf = (mcast_storage_t*)malloc(PAMI::Device::MPIBcastMdl::sizeof_msg);/// \todo memleak
        mcastinfo->client = this->_clientid;  // \todo:  this protocol is not consistent with the other protocols here
                                              // It assumes info->client is client_id.  The hack here is to fix up the
                                              // client.  this perturbs user storage
        multicast->postMulticast(*msgbuf,mcastinfo);
      }
      else
        PAMI_abort();
      return PAMI_SUCCESS;
      }
      inline pami_result_t manytomany_impl(pami_manytomany_t *m2minfo)
        {
          PAMI_abort();
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
          PAMI_abort();
          return PAMI_UNIMPL;
        }

      inline pami_result_t dispatch_impl (size_t                     id,
                                         pami_dispatch_callback_fn   fn,
                                         void                     * cookie,
                                         pami_send_hint_t            options)
      {
        pami_result_t result = PAMI_ERROR;
        TRACE_ERR((stderr, ">> Context::dispatch_impl .. _dispatch[%zu][0] = %p, result = %d\n", id, _dispatch[id][0], result));

        if (_dispatch[id][0] == NULL)
          {
            if (options.no_shmem == 1)
            {
              // Register only the "mpi" eager protocol
              //
              // This mpi eager protocol code should be changed to respect the
              // "long header" option
              //
              _dispatch[id][0] = (Protocol::Send::Send *)
                EagerMPI::generate (id, fn, cookie, *_mpi, _protocol, result);
            }
            else if (options.use_shmem == 1)
            {
              // Register only the "shmem" eager protocol
              if (options.no_long_header == 1)
                {
                  _dispatch[id][0] = (Protocol::Send::Send *)
                    Protocol::Send::Eager <ShmemModel, ShmemDevice, false>::
                      generate (id, fn, cookie, _devices->_shmem[_contextid], _protocol, result);
                }
              else
                {
                  _dispatch[id][0] = (Protocol::Send::Send *)
                    Protocol::Send::Eager <ShmemModel, ShmemDevice, true>::
                      generate (id, fn, cookie, _devices->_shmem[_contextid], _protocol, result);
                }
            }
            else
            {
              // Register both the "mpi" and "shmem" eager protocols
              //
              // This mpi eager protocol code should be changed to respect the
              // "long header" option
              //
              EagerMPI * eagermpi =
                EagerMPI::generate (id, fn, cookie, *_mpi, _protocol, result);
#ifdef ENABLE_SHMEM_DEVICE
              if (options.no_long_header == 1)
                {
                  Protocol::Send::Eager <ShmemModel, ShmemDevice, false> * eagershmem =
                    Protocol::Send::Eager <ShmemModel, ShmemDevice, false>::
                      generate (id, fn, cookie, _devices->_shmem[_contextid], _protocol, result);

                  _dispatch[id][0] = (Protocol::Send::Send *) Protocol::Send::Factory::
                      generate (eagershmem, eagermpi, _protocol, result);
                }
              else
                {
                  Protocol::Send::Eager <ShmemModel, ShmemDevice, true> * eagershmem =
                    Protocol::Send::Eager <ShmemModel, ShmemDevice, true>::
                      generate (id, fn, cookie, _devices->_shmem[_contextid], _protocol, result);

                  _dispatch[id][0] = (Protocol::Send::Send *) Protocol::Send::Factory::
                      generate (eagershmem, eagermpi, _protocol, result);
                }
#else
              _dispatch[id][0] = eagermpi;
#endif
            }
          }

        TRACE_ERR((stderr, "<< Context::dispatch_impl .. result = %d\n", result));
        return result;
      }

    inline pami_result_t dispatch_new_impl (size_t                     id,
                                           pami_dispatch_callback_fn   fn,
                                           void                     * cookie,
                                           pami_dispatch_hint_t        options)
    {
      pami_result_t result        = PAMI_ERROR;
      // Off node registration
      // This is for communication off node
      if(_dispatch[(size_t)id][0] != NULL)
      {
        PAMI_assertf(0,"Error:  Dispatch already set, id=%d\n", id);
        goto result_error;
      }
      _dispatch[(size_t)id][0]      = (void *) _request.allocateObject ();
      if(options.type == PAMI_MULTICAST)
      {
        // \todo A COMPLETE TEMPORARY HACK - since Mike gave us two dispatch table entries, use the
        // second (unused by mcast) entry to store an id of what we put in the first entry.
        // sample:
        //  _dispatch[(size_t)id][1] = (void*) ARBITRARY_ID;

        if(options.hint.multicast.one_sided)
        {
          _dispatch[(size_t)id][1] = (void*) 2; // see HACK comments above
          PAMI_assertf(_request.objsize >= sizeof(P2PMcastProto),"%zu >= %zu(%zu,%zu)\n",_request.objsize,sizeof(P2PMcastProto),sizeof(EagerMPI),sizeof(PAMI::Device::MPIBcastMdl));
          new (_dispatch[(size_t)id][0]) P2PMcastProto(id, fn.multicast, cookie,
                                                       *_mpi,
                                                       PAMI::Device::MPIBcastDev::Factory::getDevice(_devices->_mpimcast, _clientid, _contextid),
                                                       this->_client,
                                                       this->_context,
                                                       this->_contextid,
                                                       this->_clientid,
                                                       result);
          TRACE_ERR((stderr, "<< dispatch_impl(), mcast local onesided _dispatch[%zu] = %p\n", id, _dispatch[id][0]));
        }
        else if((options.hint.multicast.all_sided) && (options.hint.multicast.local))
        {
          if(options.hint.multicast.ring_wq ) // \todo bogus!  the problem with hints is ....
          {
            _dispatch[(size_t)id][1] = (void*) 3; // see HACK comments above
            PAMI_assertf(_request.objsize >= sizeof(PAMI::Device::WQRingBcastMdl),"%zu >= %zu\n",_request.objsize,sizeof(PAMI::Device::WQRingBcastMdl));
            new (_dispatch[(size_t)id][0]) PAMI::Device::WQRingBcastMdl(PAMI::Device::WQRingBcastDev::Factory::getDevice(_devices->_wqringbcast, _clientid, _contextid), result);
            TRACE_ERR((stderr, "<< dispatch_impl(), mcast local allsided ring _dispatch[%zu] = %p\n", id, _dispatch[id][0]));
          }
        else
        {
            _dispatch[(size_t)id][1] = (void*) 4; // see HACK comments above
            PAMI_assertf(_request.objsize >= sizeof(PAMI::Device::LocalBcastWQModel),"%zu >= %zu\n",_request.objsize,sizeof(PAMI::Device::LocalBcastWQModel));
            new (_dispatch[(size_t)id][0]) PAMI::Device::LocalBcastWQModel(PAMI::Device::LocalBcastWQDevice::Factory::getDevice(_devices->_localbcast, _clientid, _contextid), result);
            TRACE_ERR((stderr, "<< dispatch_impl(), mcast local allsided _dispatch[%zu] = %p\n", id, _dispatch[id][0]));
          }
        }
        else if((options.hint.multicast.all_sided) && (options.hint.multicast.global))
        {
          _dispatch[(size_t)id][1] = (void*) 5; // see HACK comments above
          PAMI_assertf(_request.objsize >= sizeof(PAMI::Device::MPIBcastMdl),"%zu >= %zu\n",_request.objsize,sizeof(PAMI::Device::MPIBcastMdl));
          new (_dispatch[(size_t)id][0]) PAMI::Device::MPIBcastMdl(PAMI::Device::MPIBcastDev::Factory::getDevice(_devices->_mpimcast, _clientid, _contextid), result);
          TRACE_ERR((stderr, "<< dispatch_impl(), mcast global allsided _dispatch[%zu] = %p\n", id, _dispatch[id][0]));
        }
        else // !experimental collective and !local allsided shmem and !global allsided
        {
          PAMI_assertf(0,"Unknown options for multicast %d\n",options);
          return PAMI_UNIMPL;
        }

        if(result!=PAMI_SUCCESS)
        {
          PAMI_assertf(result==PAMI_SUCCESS,"result %d\n",result);
          goto result_error;
        }
      }
      else if(options.type == PAMI_P2P_SEND)
      {
        result = dispatch_impl (id, fn, cookie, (pami_send_hint_t){0});
        PAMI_assert(result == PAMI_SUCCESS);
      }
      else // !PAMI_P2P_SEND and !PAMI_MULTICAST
      {
        PAMI_abort();
        return PAMI_UNIMPL;
      }

      result_error:
      TRACE_ERR((stderr, "<< dispatch_impl(), result = %zu, _dispatch[%zu] = %p\n", result, id, _dispatch[id][0]));
      return result;
    }

    private:
      std::map <unsigned, pami_geometry_t>   _geometry_id;
      pami_client_t              _client;
      pami_context_t             _context;
      size_t                    _clientid;
      size_t                    _contextid;
      void                     *_dispatch[1024][2];
      ProtocolAllocator         _protocol;
      Memory::MemoryManager     _mm;
      SysDep                    _sysdep;
      ContextLock _lock;

#ifdef USE_WAKEUP_VECTORS
      PAMI::WakeupManager _wakeupManager;
#endif /* USE_WAKEUP_VECTORS */
      MemoryAllocator<4096,16>  _request;
      MPIDevice                *_mpi;
  public:
    CCMICollreg               *_ccmi_collreg;
    PGASCollreg               *_pgas_collreg;
    OldCCMICollreg            *_oldccmi_collreg;
      MPIGeometry              *_world_geometry;
  private:
    DefaultNativeInterface    _minterface;
      unsigned                  _empty_advance;
      unsigned                 *_ranklist;
      PlatformDeviceList *_devices;
    }; // end PAMI::Context
}; // end namespace PAMI

#undef TRACE_ERR

#endif // __pami_mpi_mpicontext_h__
