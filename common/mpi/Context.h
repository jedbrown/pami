///
/// \file common/mpi/Context.h
/// \brief XMI MPI specific context implementation.
///
#ifndef __common_mpi_Context_h__
#define __common_mpi_Context_h__

#define ENABLE_GENERIC_DEVICE

#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include "common/ContextInterface.h"
#include "Geometry.h"
#include "WakeupManager.h"
#include "components/devices/mpi/mpidevice.h"
#include "components/devices/mpi/mpipacketmodel.h"
#include "components/devices/mpi/mpimessage.h"
#include "p2p/protocols/send/adaptive/Adaptive.h"
#include "p2p/protocols/send/eager/Eager.h"
#include "p2p/protocols/send/eager/EagerSimple.h"
#include "p2p/protocols/send/eager/EagerImmediate.h"
#include "SysDep.h"
#include "components/geometry/mpi/mpicollfactory.h"
#include "components/geometry/mpi/mpicollregistration.h"
#include "components/devices/generic/GenericDevice.h"
#include "Mapping.h"
#include <new>
#include <map>
#include "components/atomic/counter/CounterMutex.h"
#include "components/atomic/gcc/GccCounter.h"
#include <sched.h>


#warning shmem device must become sub-device of generic device
#include "components/devices/shmem/ShmemDevice.h"
#include "components/devices/shmem/ShmemModel.h"
#include "util/fifo/FifoPacket.h"
#include "util/fifo/LinearFifo.h"




#include "components/devices/mpi/mpimulticastprotocol.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif

namespace XMI
{
    // This won't work with XL
    typedef XMI::Mutex::CounterMutex<XMI::Counter::GccProcCounter>  ContextLock;

    typedef Device::MPIMessage MPIMessage;
    typedef Device::MPIDevice<SysDep> MPIDevice;
    typedef Device::MPIPacketModel<MPIDevice,MPIMessage> MPIPacketModel;
    typedef Geometry::Common<XMI_MAPPING_CLASS> MPIGeometry;
    typedef CollFactory::MPI<MPIDevice, SysDep> MPICollfactory;
    typedef CollRegistration::MPI<MPIGeometry, MPICollfactory, MPIDevice, SysDep> MPICollreg;
    typedef XMI::Protocol::Send::Eager <MPIPacketModel,MPIDevice> EagerMPI;


    // \todo I do not distinguish local vs non-local so no eager shmem protocol here... just EagerMPI
    typedef XMI::Protocol::MPI::P2pDispatchMulticastProtocol<MPIDevice,EagerMPI,XMI::Device::MPIBcastMdl> P2pDispatchMulticastProtocol;

    typedef XMI::Mutex::CounterMutex<XMI::Counter::GccProcCounter>  ContextLock;

    typedef Fifo::FifoPacket <16, 240> ShmemPacket;
    typedef Fifo::LinearFifo<Atomic::GccBuiltin, ShmemPacket, 128> ShmemFifo;
    typedef Device::ShmemDevice<ShmemFifo> ShmemDevice;
    typedef Device::ShmemModel<ShmemDevice> ShmemModel;

    typedef MemoryAllocator<1024, 16> ProtocolAllocator;

    class Context : public Interface::Context<XMI::Context>
    {
	static void __work_done(xmi_context_t ctx, void *cookie, xmi_result_t result) {
		XMI::Context *context = (XMI::Context *)ctx;
		context->_workAllocator.returnObject(cookie);
	}
    public:
      inline Context (xmi_client_t client, size_t id, size_t num,
				XMI::Device::Generic::Device *generics,
				void * addr, size_t bytes) :
        Interface::Context<XMI::Context> (client, id),
        _client (client),
        _contextid (id),
        _mm (addr, bytes),
	_sysdep(_mm),
        _lock (),
	_workAllocator (),
	_generic(generics[id]),
        _shmem(),
        _mpi(&__global.mpi_device),

        _empty_advance(0)
        {
          MPI_Comm_rank(MPI_COMM_WORLD,&_myrank);
          MPI_Comm_size(MPI_COMM_WORLD,&_mysize);
          _world_geometry=(MPIGeometry*) malloc(sizeof(*_world_geometry));
	  _world_range.lo=0;
	  _world_range.hi=_mysize-1;
          new(_world_geometry) MPIGeometry(&__global.mapping,0, 1,&_world_range);

	  _collreg=(MPICollreg*) malloc(sizeof(*_collreg));
	  new(_collreg) MPICollreg(_mpi, &_sysdep, getClient(), (xmi_context_t)(void *)this, getId());

          _world_collfactory=_collreg->analyze(_world_geometry);
	  _world_geometry->setKey(XMI::Geometry::XMI_GKEY_COLLFACTORY, _world_collfactory);

#ifdef USE_WAKEUP_VECTORS
	  _wakeupManager.init(1, 0x57550000 | id); // check errors?
#endif // USE_WAKEUP_VECTORS
	  _generic.init (_sysdep, (xmi_context_t)this, id, num, generics);
          _shmem.init(&_sysdep);
          _lock.init(&_sysdep);

          // this barrier is here because the shared memory init
          // needs to be synchronized
          // we shoudl find a way to remove this

          MPI_Barrier(MPI_COMM_WORLD);
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
          // Do not call finalize because if we do
          // it is not valid to call init again
          // per the MPI spec.
          return XMI_SUCCESS;
        }

      inline xmi_result_t post_impl (xmi_work_function work_fn, void * cookie)
        {
          XMI::Device::ProgressFunctionMsg *work =
		(XMI::Device::ProgressFunctionMsg *)_workAllocator.allocateObject();
	  work->setFunc(work_fn);
	  work->setCookie(cookie);
	  work->setDone((xmi_callback_t){__work_done, (void *)work});
	  work->postWorkDirect();
          return XMI_SUCCESS;
        }

      inline size_t advance_impl (size_t maximum, xmi_result_t & result)
        {
          result = XMI_SUCCESS;
          size_t events = 0;

          unsigned i;
          for (i=0; i<maximum && events==0; i++)
              {

        events += _mpi->advance_impl();
        events += _generic.advance();
        events += _shmem.advance_impl();


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
          int local;
//          if(__global.mapping.isPeer(parameters->send.task, __global.mapping.task())) \todo isPeer should support XMI_MAPPLING_TSIZE
          if(__global.topology_local.isRankMember(parameters->send.task))
            local=1;
          else
            local=0;
          XMI_assert_debug (_dispatch[id][local] != NULL);
          XMI::Protocol::Send::Send * send =
            (XMI::Protocol::Send::Send *) _dispatch[id][local];
          send->simple (parameters);
          return XMI_SUCCESS;
        }

      inline xmi_result_t send_impl (xmi_send_immediate_t * parameters)
        {
          size_t id = (size_t)(parameters->dispatch);
          XMI_assert_debug (_dispatch[id][0] != NULL);

          int local;
//          if(__global.mapping.isPeer(parameters->send.task, __global.mapping.task())) \todo isPeer should support XMI_MAPPLING_TSIZE
          if(__global.topology_local.isRankMember(parameters->task))
            local=1;
          else
            local=0;
          TRACE_ERR((stderr, ">> send_impl('immediate') dispatch[%zd][%d] = %p\n", id, local,_dispatch[id][local]));

          XMI::Protocol::Send::Send * send =
            (XMI::Protocol::Send::Send *) _dispatch[id][local];
          send->immediate (parameters);

          TRACE_ERR((stderr, "<< send_impl('immediate')\n"));
          return XMI_SUCCESS;
        }

      inline xmi_result_t send_impl (xmi_send_typed_t * parameters)
        {
          XMI_abort();
          return XMI_UNIMPL;
        }

      inline xmi_result_t put_impl (xmi_put_simple_t * parameters)
        {
          XMI_abort();
          return XMI_UNIMPL;
        }

      inline xmi_result_t put_typed_impl (xmi_put_typed_t * parameters)
        {
          XMI_abort();
          return XMI_UNIMPL;
        }

      inline xmi_result_t get_impl (xmi_get_simple_t * parameters)
        {
          XMI_abort();
          return XMI_UNIMPL;
        }

      inline xmi_result_t get_typed_impl (xmi_get_typed_t * parameters)
        {
          XMI_abort();
          return XMI_UNIMPL;
        }

      inline xmi_result_t rmw_impl (xmi_rmw_t * parameters)
        {
          XMI_abort();
          return XMI_UNIMPL;
        }

      inline xmi_result_t memregion_register_impl (void            * address,
                                                   size_t            bytes,
                                                   xmi_memregion_t * memregion)
        {
          XMI_abort();
          return XMI_UNIMPL;
        }

      inline xmi_result_t memregion_deregister_impl (xmi_memregion_t * memregion)
        {
          XMI_abort();
          return XMI_UNIMPL;
        }


      inline xmi_result_t memregion_query_impl (xmi_memregion_t    memregion,
                                                void            ** address,
                                                size_t           * bytes,
                                                size_t           * task)
        {
          XMI_abort();
          return XMI_UNIMPL;
        }


      inline xmi_result_t memregion_register_impl (xmi_rmw_t * parameters)
        {
          XMI_abort();
          return XMI_UNIMPL;
        }

      inline xmi_result_t rput_impl (xmi_rput_simple_t * parameters)
        {
          XMI_abort();
          return XMI_UNIMPL;
        }

      inline xmi_result_t rput_typed_impl (xmi_rput_typed_t * parameters)
        {
          XMI_abort();
          return XMI_UNIMPL;
        }

      inline xmi_result_t rget_impl (xmi_rget_simple_t * parameters)
        {
          XMI_abort();
          return XMI_UNIMPL;
        }

      inline xmi_result_t rget_typed_impl (xmi_rget_typed_t * parameters)
        {
          XMI_abort();
          return XMI_UNIMPL;
        }

      inline xmi_result_t purge_totask_impl (size_t * dest, size_t count)
        {
          XMI_abort();
          return XMI_UNIMPL;
        }

      inline xmi_result_t resume_totask_impl (size_t * dest, size_t count)
        {
          XMI_abort();
          return XMI_UNIMPL;
        }

      inline xmi_result_t fence_begin_impl ()
        {
          XMI_abort();
          return XMI_UNIMPL;
        }

      inline xmi_result_t fence_end_impl ()
        {
          XMI_abort();
          return XMI_UNIMPL;
        }

      inline xmi_result_t fence_all_impl (xmi_event_function   done_fn,
                                          void               * cookie)
        {
          XMI_abort();
          return XMI_UNIMPL;
        }

      inline  xmi_result_t fence_task_impl (xmi_event_function   done_fn,
                                            void               * cookie,
                                            size_t               task)
        {
          XMI_abort();
          return XMI_UNIMPL;
        }

      inline xmi_result_t geometry_initialize_impl (xmi_geometry_t       * geometry,
                                                    unsigned               id,
                                                    xmi_geometry_range_t * rank_slices,
                                                    size_t                 slice_count)
        {
	  MPIGeometry              *new_geometry;
	  MPICollfactory           *new_collfactory;
          new_geometry=(MPIGeometry*) malloc(sizeof(*new_geometry));
          new(new_geometry) MPIGeometry(&__global.mapping,id, slice_count,rank_slices);
          new_collfactory=_collreg->analyze(new_geometry);
	  new_geometry->setKey(XMI::Geometry::XMI_GKEY_COLLFACTORY, new_collfactory);
	  *geometry=(MPIGeometry*) new_geometry;
          return XMI_SUCCESS;
        }


      inline xmi_result_t geometry_world_impl (xmi_geometry_t * world_geometry)
        {
	  *world_geometry = _world_geometry;
          return XMI_SUCCESS;
        }

      inline xmi_result_t geometry_finalize_impl (xmi_geometry_t geometry)
        {
          XMI_abort();
          return XMI_UNIMPL;
        }


      inline xmi_result_t collective_impl (xmi_xfer_t * parameters)
        {
	  MPICollfactory           *collfactory;
	  // This is ok...we can avoid a switch because all the xmi structs
	  // have the same layout.let's just use barrier for now
	  MPIGeometry              *new_geometry = (MPIGeometry*)parameters->xfer_barrier.geometry;
	  collfactory =(MPICollfactory*) new_geometry->getKey(XMI::Geometry::XMI_GKEY_COLLFACTORY);
          return collfactory->collective(parameters);
        }
        inline xmi_result_t geometry_algorithms_num_impl (xmi_geometry_t geometry,
                                                          xmi_xfer_type_t colltype,
                                                          int *lists_lengths)
        {
          MPIGeometry *new_geometry = (MPIGeometry*) geometry;
          MPICollfactory  *collfactory =  (MPICollfactory*)
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
          MPIGeometry *new_geometry = (MPIGeometry*) geometry;
          MPICollfactory  *collfactory;
          collfactory = (MPICollfactory*)
            new_geometry->getKey(XMI::Geometry::XMI_GKEY_COLLFACTORY);
          return collfactory->algorithms_info(colltype, algs,
                                              mdata, algorithm_type, num);

        }


      inline xmi_result_t multisend_getroles_impl(size_t          dispatch,
                                                  int            *numRoles,
                                                  int            *replRole)
        {
          XMI_abort();
          return XMI_UNIMPL;
        }

      inline xmi_result_t multicast_impl(xmi_multicast_t *mcastinfo)
      {
      size_t id = (size_t)(mcastinfo->dispatch);
      TRACE_ERR((stderr, ">> multicast_impl, _dispatch[%zd/%zd] = %p\n", id, mcastinfo->dispatch, _dispatch[id][0]));
      XMI_assert_debug (_dispatch[id][0] != NULL);

      // \todo A COMPLETE TEMPORARY HACK - since Mike gave us two dispatch table entries, we used the
      // second (unused by mcast) entry to store an id of what we put in the first entry.  Now
      // we know what class to pull out of _dispatch[][0] and (eventually) what allocator to use.
      XMI_assert_debug (_dispatch[id][1] > (void*)0 && _dispatch[id][1] < (void*)6);


      if(_dispatch[id][1] == (void*)2) // see HACK comment above
      {
        P2pDispatchMulticastProtocol * multicast = (P2pDispatchMulticastProtocol *) _dispatch[id][0];
        TRACE_ERR((stderr, ">> multicast_impl, one sided multicast %p\n", multicast));
        multicast->multicast(mcastinfo);
      }
      else if(_dispatch[id][1] == (void*)3) // see HACK comment above
      {
        XMI::Device::WQRingBcastMdl * multicast = (XMI::Device::WQRingBcastMdl*) _dispatch[id][0];
        TRACE_ERR((stderr, ">> multicast_impl, all sided ring multicast %p\n", multicast));
        if(mcastinfo->request==NULL) // some tests have removed this field so malloc it (\todo memory leak)
        {
          char *msgbuf = new char[XMI::Device::WQRingBcastMdl::sizeof_msg];
          mcastinfo->request = msgbuf;
        }
        multicast->postMulticast(mcastinfo);
      }
      else if(_dispatch[id][1] == (void*)4) // see HACK comment above
      {
        XMI::Device::LocalBcastWQModel  * multicast = (XMI::Device::LocalBcastWQModel *) _dispatch[id][0];
        TRACE_ERR((stderr, ">> multicast_impl, all sided multicast %p\n", multicast));
        if(mcastinfo->request==NULL) // some tests have removed this field so malloc it (\todo memory leak)
        {
          char *msgbuf = new char[XMI::Device::LocalBcastWQModel::sizeof_msg];
          mcastinfo->request = msgbuf;
        }
        multicast->postMulticast(mcastinfo);
      }
      else if(_dispatch[id][1] == (void*)5) // see HACK comment above
      {
        XMI::Device::MPIBcastMdl  * multicast = (XMI::Device::MPIBcastMdl *) _dispatch[id][0];
        TRACE_ERR((stderr, ">> multicast_impl, all sided global multicast %p\n", multicast));
        if(mcastinfo->request==NULL) // some tests have removed this field so malloc it (\todo memory leak)
        {
          char *msgbuf = new char[XMI::Device::MPIBcastMdl::sizeof_msg];
          mcastinfo->request = msgbuf;
        }
        multicast->postMulticast(mcastinfo);
      }
      else
        XMI_abort();
      return XMI_SUCCESS;
      }

      inline xmi_result_t manytomany_impl(xmi_manytomany_t *m2minfo)
        {
          XMI_abort();
          return XMI_UNIMPL;
        }

      inline xmi_result_t multisync_impl(xmi_multisync_t *msyncinfo)
        {
          XMI_abort();
          return XMI_UNIMPL;
        }

      inline xmi_result_t multicombine_impl(xmi_multicombine_t *mcombineinfo)
        {
          XMI_abort();
          return XMI_UNIMPL;
        }
      inline xmi_result_t dispatch_impl (size_t                     id,
                                         xmi_dispatch_callback_fn   fn,
                                         void                     * cookie,
                                         xmi_send_hint_t            options)
        {
          size_t index = (size_t) id;
          // Off node registration
          // This is for communication off node
          if (_dispatch[(size_t)id][0] != NULL) return XMI_ERROR;
          _dispatch[(size_t)id][0]      = (void *) _request.allocateObject ();
          xmi_result_t result        = XMI_ERROR;
          XMI_assert(_request.objsize >= sizeof(EagerMPI));
          new (_dispatch[(size_t)id][0]) EagerMPI (id, fn, cookie, *_mpi,
                                                __global.mapping.task(),
                                                _context, _contextid, result);
          if(result!=XMI_SUCCESS)
          {
             XMI_abort();
             goto result_error;
          }
          // Shared Memory Registration
          // This is for communication on node
          TRACE_ERR((stderr, ">> dispatch_impl(), _dispatch[%zd] = %p\n", index, _dispatch[index][0]));

          // currently, shared memory is off, because this dispatch_id will  not be null
//          if (_dispatch[index][1] == NULL)
              {
                TRACE_ERR((stderr, "   dispatch_impl(), before protocol init\n"));

                if (options.no_long_header == 1)
                    {
                      _dispatch[id][1] = _protocol.allocateObject ();
                      new (_dispatch[id][1])
                        Protocol::Send::Eager <ShmemModel, ShmemDevice, false>
                        (id, fn, cookie, _shmem, __global.mapping.task(),
                         _context, _contextid, result);
                    }
                else
                    {
                      _dispatch[id][1] = _protocol.allocateObject ();
                      new (_dispatch[id][1])
                        Protocol::Send::Eager <ShmemModel, ShmemDevice, true>
                        (id, fn, cookie, _shmem, __global.mapping.task(),
                         _context, _contextid, result);
                    }

                TRACE_ERR((stderr, "   dispatch_impl(),  after protocol init, result = %zd\n", result));
                if (result != XMI_SUCCESS)
                    {
                      _protocol.returnObject (_dispatch[id][1]);
                      _dispatch[id][1] = NULL;
                    }
              }
              XMI_assert(result == XMI_SUCCESS);

          result_error:
          TRACE_ERR((stderr, "<< dispatch_impl(), result = %zd, _dispatch[%zd] = %p\n", result, index, _dispatch[index][0]));
          return result;
        }

    inline xmi_result_t dispatch_new_impl (size_t                     id,
                                           xmi_dispatch_callback_fn   fn,
                                           void                     * cookie,
                                           xmi_dispatch_hint_t        options)
    {
      xmi_result_t result        = XMI_ERROR;
      // Off node registration
      // This is for communication off node
      if(_dispatch[(size_t)id][0] != NULL)
      {
        XMI_abort();
        goto result_error;
      }
      _dispatch[(size_t)id][0]      = (void *) _request.allocateObject ();
      if(options.type == XMI_MULTICAST)
      {
        // \todo A COMPLETE TEMPORARY HACK - since Mike gave us two dispatch table entries, use the
        // second (unused by mcast) entry to store an id of what we put in the first entry.
        // sample:
        //  _dispatch[(size_t)id][1] = (void*) ARBITRARY_ID;

        if(options.hint.multicast.one_sided)
        {
          _dispatch[(size_t)id][1] = (void*) 2; // see HACK comments above
          XMI_assertf(_request.objsize >= sizeof(P2pDispatchMulticastProtocol),"%zd >= %zd(%zd,%zd)\n",_request.objsize,sizeof(P2pDispatchMulticastProtocol),sizeof(EagerMPI),sizeof(XMI::Device::MPIBcastMdl));
          new (_dispatch[(size_t)id][0]) P2pDispatchMulticastProtocol(id, fn.multicast, cookie,
                                                                      *_mpi,
                                                                      this->_client,
                                                                      this->_context,
                                                                      this->_contextid,
                                                                      result);
          TRACE_ERR((stderr, "<< dispatch_impl(), mcast local onesided _dispatch[%zd] = %p\n", id, _dispatch[id][0]));
        }
        else if((options.hint.multicast.all_sided) && (options.hint.multicast.local))
        {
          if(options.hint.multicast.ring_wq ) // \todo bogus!  the problem with hints is ....
          {
            _dispatch[(size_t)id][1] = (void*) 3; // see HACK comments above
            XMI_assertf(_request.objsize >= sizeof(XMI::Device::WQRingBcastMdl),"%zd >= %zd\n",_request.objsize,sizeof(XMI::Device::WQRingBcastMdl));
            new (_dispatch[(size_t)id][0]) XMI::Device::WQRingBcastMdl(result);
            TRACE_ERR((stderr, "<< dispatch_impl(), mcast local allsided ring _dispatch[%zd] = %p\n", id, _dispatch[id][0]));
          }
        else
        {
            _dispatch[(size_t)id][1] = (void*) 4; // see HACK comments above
            XMI_assertf(_request.objsize >= sizeof(XMI::Device::LocalBcastWQModel),"%zd >= %zd\n",_request.objsize,sizeof(XMI::Device::LocalBcastWQModel));
            new (_dispatch[(size_t)id][0]) XMI::Device::LocalBcastWQModel(result);
            TRACE_ERR((stderr, "<< dispatch_impl(), mcast local allsided _dispatch[%zd] = %p\n", id, _dispatch[id][0]));
          }
        }
        else if((options.hint.multicast.all_sided) && (options.hint.multicast.global))
        {
          _dispatch[(size_t)id][1] = (void*) 5; // see HACK comments above
          XMI_assertf(_request.objsize >= sizeof(XMI::Device::MPIBcastMdl),"%zd >= %zd\n",_request.objsize,sizeof(XMI::Device::MPIBcastMdl));
          new (_dispatch[(size_t)id][0]) XMI::Device::MPIBcastMdl(result);
          TRACE_ERR((stderr, "<< dispatch_impl(), mcast global allsided _dispatch[%zd] = %p\n", id, _dispatch[id][0]));
        }
        else // !experimental collective and !local allsided shmem and !global allsided
        {
          XMI_abort();
          return XMI_UNIMPL;
        }

        if(result!=XMI_SUCCESS)
        {
          XMI_assertf(result==XMI_SUCCESS,"result %d\n",result);
          goto result_error;
        }
      }
      else if(options.type == XMI_P2P_SEND)
      {
        XMI_assert(_request.objsize >= sizeof(EagerMPI));
        new (_dispatch[(size_t)id][0]) EagerMPI (id, fn, cookie, *_mpi,
                                                 __global.mapping.task(),
                                                 _context, _contextid, result);
        if(result!=XMI_SUCCESS)
        {
          XMI_abort();
          goto result_error;
        }
        // Shared Memory Registration
        // This is for communication on node
        TRACE_ERR((stderr, ">> dispatch_impl(), _dispatch[%zd] = %p\n", id, _dispatch[id][0]));

        // currently, shared memory is off, because this dispatch_id will  not be null
//          if (_dispatch[id][1] == NULL)
        {
          TRACE_ERR((stderr, "   dispatch_impl(), before protocol init\n"));

          if(options.hint.send.no_long_header == 1)
          {
            _dispatch[id][1] = _protocol.allocateObject ();
            new (_dispatch[id][1])
            Protocol::Send::Eager <ShmemModel, ShmemDevice, false>
            (id, fn, cookie, _shmem, __global.mapping.task(),
             _context, _contextid, result);
          }
          else
          {
            _dispatch[id][1] = _protocol.allocateObject ();
            new (_dispatch[id][1])
            Protocol::Send::Eager <ShmemModel, ShmemDevice, true>
            (id, fn, cookie, _shmem, __global.mapping.task(),
             _context, _contextid, result);
          }

          TRACE_ERR((stderr, "   dispatch_impl(),  after protocol init, result = %zd\n", result));
          if(result != XMI_SUCCESS)
          {
            _protocol.returnObject (_dispatch[id][1]);
            _dispatch[id][1] = NULL;
          }
        }
        XMI_assert(result == XMI_SUCCESS);
      }
      else // !XMI_P2P_SEND and !XMI_MULTICAST
      {
        XMI_abort();
        return XMI_UNIMPL;
      }

      result_error:
      TRACE_ERR((stderr, "<< dispatch_impl(), result = %zd, _dispatch[%zd] = %p\n", result, id, _dispatch[id][0]));
      return result;
    }

    private:
      std::map <unsigned, xmi_geometry_t>   _geometry_id;
      xmi_client_t              _client;
      xmi_context_t             _context;
      size_t                    _contextid;
      void                     *_dispatch[1024][2];
      ProtocolAllocator         _protocol;
      Memory::MemoryManager     _mm;
      SysDep                    _sysdep;
      ContextLock _lock;

      MemoryAllocator<XMI::Device::ProgressFunctionMdl::sizeof_msg, 16> _workAllocator;

      XMI::Device::Generic::Device &_generic;
#ifdef USE_WAKEUP_VECTORS
      XMI::WakeupManager _wakeupManager;
#endif /* USE_WAKEUP_VECTORS */
      ShmemDevice               _shmem;
      MemoryAllocator<2048,16>  _request;
      MPIDevice                *_mpi;
      MPICollreg               *_collreg;
      MPIGeometry              *_world_geometry;
      MPICollfactory           *_world_collfactory;
      unsigned                  _empty_advance;
      xmi_geometry_range_t      _world_range;
      int                       _myrank;
      int                       _mysize;
      unsigned                 *_ranklist;
    }; // end XMI::Context
}; // end namespace XMI

#undef TRACE_ERR

#endif // __xmi_mpi_mpicontext_h__
