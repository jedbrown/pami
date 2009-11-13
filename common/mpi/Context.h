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
#include "components/devices/mpi/mpidevice.h"
#include "components/devices/mpi/mpimodel.h"
#include "components/devices/mpi/mpimessage.h"
#include "p2p/protocols/send/adaptive/Adaptive.h"
#include "p2p/protocols/send/eager/Eager.h"
#include "p2p/protocols/send/eager/EagerSimple.h"
#include "p2p/protocols/send/eager/EagerImmediate.h"
#include "SysDep.h"
#include "components/geometry/mpi/mpicollfactory.h"
#include "components/geometry/mpi/mpicollregistration.h"
#ifdef ENABLE_GENERIC_DEVICE
#include "components/devices/generic/GenericDevice.h"
#endif
#include "Mapping.h"
#include <new>
#include <map>
#include "components/atomic/counter/CounterMutex.h"
#include "components/atomic/gcc/GccCounter.h"
#include <sched.h>


#warning shmem device must become sub-device of generic device
#include "components/devices/shmem/ShmemDevice.h"
#include "components/devices/shmem/ShmemModel.h"
#include "components/devices/shmem/ShmemMessage.h"
#include "util/fifo/FifoPacket.h"
#include "util/fifo/LinearFifo.h"



#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif

namespace XMI
{
    // This won't work with XL
    typedef XMI::Mutex::CounterMutex<XMI::Counter::GccProcCounter>  ContextLock;

    typedef Device::MPIMessage MPIMessage;
    typedef Device::MPIDevice<SysDep> MPIDevice;
    typedef Device::MPIModel<MPIDevice,MPIMessage> MPIModel;
    typedef Geometry::Common<XMI_MAPPING_CLASS> MPIGeometry;
    typedef CollFactory::MPI<MPIDevice, SysDep> MPICollfactory;
    typedef CollRegistration::MPI<MPIGeometry, MPICollfactory, MPIDevice, SysDep> MPICollreg;
    typedef XMI::Protocol::Send::Eager <MPIModel,MPIDevice> EagerMPI;
    //typedef XMI::Protocol::Send::Adaptive <MPIModel,MPIDevice> EagerMPI;

    typedef XMI::Mutex::CounterMutex<XMI::Counter::GccProcCounter>  ContextLock;
    typedef Fifo::FifoPacket <16, 240> ShmemPacket;
//    typedef Fifo::LinearFifo<Counter::LockBoxProcCounter, ShmemPacket, 128> ShmemFifo;
    typedef Fifo::LinearFifo<Atomic::GccBuiltin, ShmemPacket, 128> ShmemFifo;
    typedef Device::ShmemMessage<ShmemPacket> ShmemMessage;
    typedef Device::ShmemDevice<ShmemFifo, ShmemPacket> ShmemDevice;
    typedef Device::ShmemModel<ShmemDevice, ShmemMessage> ShmemModel;

    typedef MemoryAllocator<1024, 16> ProtocolAllocator;

    class Work : public Queue
    {
      private:
        class WorkObject : public QueueElem
        {
          public:
            inline WorkObject (xmi_event_function   fn,
                               void               * cookie) :
              QueueElem (),
              _fn (fn),
              _cookie (cookie)
            {};
            inline ~WorkObject() { };

            xmi_event_function   _fn;
            void               * _cookie;
        };

        xmi_context_t _context;
        ContextLock   _lock;
        MemoryAllocator<sizeof(WorkObject),16> _allocator;

      public:
        inline Work (xmi_context_t context, SysDep * sysdep) :
          Queue (),
          _context (context),
          _lock (),
          _allocator ()
        {
          _lock.init (sysdep);
        };
        inline void post (xmi_event_function   fn,
                          void               * cookie)
        {
          _lock.acquire ();
          WorkObject * obj = (WorkObject *) _allocator.allocateObject ();
          new (obj) WorkObject (fn, cookie);
          pushTail ((QueueElem *) obj);
          _lock.release ();
        };

        inline size_t advance ()
        {
          size_t events = 0;
          if (_lock.tryAcquire ())
          {
            WorkObject * obj = NULL;
            while ((obj = (WorkObject *) popHead()) != NULL)
            {
              obj->_fn(_context, obj->_cookie, XMI_SUCCESS);
              events++;
            }
            _lock.release ();
          }
          return events;
        };
    };


    class Context : public Interface::Context<XMI::Context>
    {
    public:
      inline Context (xmi_client_t client, size_t id, void * addr, size_t bytes) :
        Interface::Context<XMI::Context> (client, id),
        _client (client),
        _contextid (id),
        _mm (addr, bytes),
	_sysdep(_mm),
        _lock (),
        _empty_advance(0),
        _shmem(),
        _work (_context, &_sysdep)
#ifdef ENABLE_GENERIC_DEVICE
	, _generic(_sysdep)
#endif
        {
          MPI_Comm_rank(MPI_COMM_WORLD,&_myrank);
          MPI_Comm_size(MPI_COMM_WORLD,&_mysize);
          _world_geometry=(MPIGeometry*) malloc(sizeof(*_world_geometry));
	  _world_range.lo=0;
	  _world_range.hi=_mysize-1;
          new(_world_geometry) MPIGeometry(&__global.mapping,0, 1,&_world_range);

	  _collreg=(MPICollreg*) malloc(sizeof(*_collreg));
	  new(_collreg) MPICollreg(&_mpi, &_sysdep);

          _world_collfactory=_collreg->analyze(_world_geometry);
	  _world_geometry->setKey(XMI::Geometry::XMI_GKEY_COLLFACTORY, _world_collfactory);

#ifdef ENABLE_GENERIC_DEVICE
	  _generic.init (_sysdep);
#endif
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

      inline xmi_result_t queryConfiguration_impl (xmi_configuration_t * configuration)
        {
          xmi_result_t result = XMI_ERROR;

          switch (configuration->name)
              {
                  case XMI_TASK_ID:
                    configuration->value.intval = __global.mapping.task();
                    result = XMI_SUCCESS;
                    break;
                  case XMI_NUM_TASKS:
                    configuration->value.intval = __global.mapping.size();
                    result = XMI_SUCCESS;
                    break;
                  default:
                    break;
              };

          return result;
        }

      inline xmi_result_t post_impl (xmi_event_function work_fn, void * cookie)
        {
          _work.post (work_fn, cookie);
          return XMI_SUCCESS;
        }

      inline size_t advance_impl (size_t maximum, xmi_result_t & result)
        {
          result = XMI_SUCCESS;
          size_t events = 0;

          // Should this go inside the loop?
          events += _work.advance ();

          unsigned i;
          for (i=0; i<maximum && events==0; i++)
              {
                events += _mpi.advance_impl();
#ifdef ENABLE_GENERIC_DEVICE
	        events += _generic.advance();
#endif
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
          if(__global.mapping.isPeer(parameters->send.task, __global.mapping.task()))
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
          TRACE_ERR((stderr, ">> send_impl('immediate'), _dispatch[%zd] = %p\n", id, _dispatch[id][0]));
          XMI_assert_debug (_dispatch[id][0] != NULL);

          int local;
          if(__global.mapping.isPeer(parameters->task, __global.mapping.task()))
            local=1;
          else
            local=0;

          XMI::Protocol::Send::Send * send =
            (XMI::Protocol::Send::Send *) _dispatch[id][local];
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
          assert(0);
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
        inline xmi_result_t geometry_algorithms_num_impl (xmi_context_t context,
                                                          xmi_geometry_t geometry,
                                                          xmi_xfer_type_t colltype,
                                                          int *lists_lengths)
        {
          MPIGeometry *new_geometry = (MPIGeometry*) geometry;
          MPICollfactory  *collfactory =  (MPICollfactory*)
            new_geometry->getKey(XMI::Geometry::XMI_GKEY_COLLFACTORY);
          return collfactory->algorithms_num(colltype, lists_lengths);
        }

        inline xmi_result_t geometry_algorithms_info_impl (xmi_context_t context,
                                                           xmi_geometry_t geometry,
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
          assert(0);
          return XMI_UNIMPL;
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
          size_t index = (size_t) id;
          // Off node registration
          // This is for communication off node
          if (_dispatch[(size_t)id][0] != NULL) return XMI_ERROR;
          _dispatch[(size_t)id][0]      = (void *) _request.allocateObject ();
          xmi_result_t result        = XMI_ERROR;
          new (_dispatch[(size_t)id][0]) EagerMPI (id, fn, cookie, _mpi,
                                                __global.mapping.task(),
                                                _context, _contextid, result);
          if(result!=XMI_SUCCESS)
              {
                assert(0);
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
              assert(result == XMI_SUCCESS);

          result_error:
          TRACE_ERR((stderr, "<< dispatch_impl(), result = %zd, _dispatch[%zd] = %p\n", result, index, _dispatch[index][0]));
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

      // This is a bringup hack .. it should be replaced with something better
      Work _work;

#ifdef ENABLE_GENERIC_DEVICE
      XMI::Device::Generic::Device _generic;
#endif
      MemoryAllocator<1024,16>  _request;
      MPIDevice                 _mpi;
      MPICollreg               *_collreg;
      MPIGeometry              *_world_geometry;
      MPICollfactory           *_world_collfactory;
      unsigned                  _empty_advance;
      ShmemDevice               _shmem;
      xmi_geometry_range_t      _world_range;
      int                       _myrank;
      int                       _mysize;
      unsigned                 *_ranklist;
    }; // end XMI::Context
}; // end namespace XMI

#undef TRACE_ERR

#endif // __xmi_mpi_mpicontext_h__
