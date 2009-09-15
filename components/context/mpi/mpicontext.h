///
/// \file xmi/mpi/mpicontext.h
/// \brief XMI MPI specific context implementation.
///
#ifndef   __xmi_mpi_mpicontext_h__
#define   __xmi_mpi_mpicontext_h__

#define XMI_CONTEXT_CLASS XMI::Context::MPI

#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include "components/context/Context.h"
#include "components/geometry/common/commongeometry.h"
#include "components/devices/mpi/mpidevice.h"
#include "components/devices/mpi/mpimodel.h"
#include "components/devices/mpi/mpimessage.h"
#include "p2p/protocols/send/eager/EagerSimple.h"
#include "components/sysdep/mpi/mpisysdep.h"
#include "components/geometry/mpi/mpicollfactory.h"
#include "components/geometry/mpi/mpicollregistration.h"
#include "components/mapping/mpi/mpimapping.h"
#include <new>
#include <map>

namespace XMI
{
  namespace Context
  {
    typedef Device::MPIMessage MPIMessage;
    typedef Device::MPIDevice<SysDep::MPISysDep> MPIDevice;
    typedef Device::MPIModel<MPIDevice,MPIMessage> MPIModel;
    typedef Geometry::Common<XMI_MAPPING_CLASS> MPIGeometry;
    typedef CollFactory::MPI<MPIDevice> MPICollfactory;
    typedef CollRegistration::MPI<MPIGeometry, MPICollfactory, MPIDevice> MPICollreg;


    class MPI : public Context<XMI::Context::MPI>
    {
    public:
      inline MPI (xmi_client_t client) :
        Context<XMI::Context::MPI> (client),
        _client (client)
        {
          MPI_Comm_rank(MPI_COMM_WORLD,&_myrank);
          MPI_Comm_size(MPI_COMM_WORLD,&_mysize); 
          _world_geometry=(MPIGeometry*) malloc(sizeof(*_world_geometry));
	  _world_range.lo=0;
	  _world_range.hi=_mysize-1;
          new(_world_geometry) MPIGeometry(&_sysdep.mapping,1,&_world_range);
	  
	  _collreg=(MPICollreg*) malloc(sizeof(*_collreg));
	  new(_collreg) MPICollreg(&_mpi);

          _world_collfactory=_collreg->analyze(_world_geometry);
	  _world_geometry->setKey(XMI::Geometry::COLLFACTORY, _world_collfactory);

        }
        
      inline xmi_client_t getClientId_impl ()
        {
          return _client;
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
                    configuration->value.intval = _sysdep.mapping.task();
                    result = XMI_SUCCESS;
                    break;
                  case XMI_NUM_TASKS:
                    configuration->value.intval = _sysdep.mapping.size();
                    result = XMI_SUCCESS;
                    break;
                  default:
                    break;
              };

          return result;
        }      
      
      inline xmi_result_t post_impl (xmi_event_function work_fn, void * cookie)
        {
          assert(0);
          return XMI_UNIMPL;
        }

      inline size_t advance_impl (size_t maximum, xmi_result_t & result)
        {
          result = XMI_SUCCESS;
          size_t events = 0;
          unsigned i;
          for (i=0; i<maximum && events==0; i++)
              {
                events += _mpi.advance_impl();
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

      inline xmi_result_t send_impl (xmi_send_simple_t * parameters)
        {
          size_t id = (size_t)(parameters->send.dispatch);
          assert (_dispatch[id] != NULL);

          XMI::Protocol::Send::Simple * send =
            (XMI::Protocol::Send::Simple *) _dispatch[id];
          send->start (parameters->simple.local_fn,
                       parameters->simple.remote_fn,
                       parameters->send.cookie,
                       parameters->send.task,
                       parameters->simple.addr,
                       parameters->simple.bytes,
                       parameters->send.header.addr,
                       parameters->send.header.bytes);

          return XMI_SUCCESS;
        }

      inline xmi_result_t send_impl (xmi_send_immediate_t * parameters)
        {
          assert(0);
          return XMI_UNIMPL;
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
                                                    unsigned               slice_count)
        {
	  MPIGeometry              *new_geometry;
	  MPICollfactory           *new_collfactory;
          new_geometry=(MPIGeometry*) malloc(sizeof(*new_geometry));
          new(new_geometry) MPIGeometry(&_sysdep.mapping,slice_count,rank_slices);
          new_collfactory=_collreg->analyze(new_geometry);
	  new_geometry->setKey(XMI::Geometry::COLLFACTORY, new_collfactory);
	  *geometry=(MPIGeometry*) new_geometry;
          return XMI_SUCCESS;
        }
            

      inline xmi_result_t geometry_world_impl (xmi_geometry_t * world_geometry)
        {
	  *world_geometry = _world_geometry;
          return XMI_SUCCESS;
        }

      inline xmi_result_t geometry_algorithm_impl (xmi_xfer_type_t  colltype,
						   xmi_geometry_t   geometry,
                                                   xmi_algorithm_t *algorithm,
                                                   int             *num)
        {
	  MPICollfactory           *collfactory;
	  MPIGeometry              *new_geometry = (MPIGeometry*) geometry;
	  collfactory =(MPICollfactory*) new_geometry->getKey(XMI::Geometry::COLLFACTORY);
	  return collfactory->algorithm(colltype,algorithm,num);
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
	  collfactory =(MPICollfactory*) new_geometry->getKey(XMI::Geometry::COLLFACTORY);
          return collfactory->collective(parameters);
        }
        
      inline xmi_result_t multisend_getroles_impl(xmi_dispatch_t  dispatch,
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
      inline xmi_result_t dispatch_impl (xmi_dispatch_t             id,
                                         xmi_dispatch_callback_fn   fn,
                                         void                     * cookie,
                                         xmi_send_hint_t            options)
        {
          typedef XMI::Protocol::Send::EagerSimple <MPIModel,MPIDevice,MPIMessage> EagerSimpleMPI;

          if (_dispatch[(size_t)id] != NULL) return XMI_ERROR;
          _dispatch[(size_t)id]      = (void *) _request.allocateObject ();
          xmi_result_t result        = XMI_ERROR;
          new (_dispatch[(size_t)id]) EagerSimpleMPI (id, fn, cookie, _mpi, _sysdep.mapping.task(), _context, result);
          return result;
        }

    private:
      std::map <unsigned, xmi_geometry_t>   _geometry_id;
      xmi_client_t              _client;
      xmi_context_t             _context;
      void                     *_dispatch[1024];
      SysDep::MPISysDep         _sysdep;
      MemoryAllocator<1024,16>  _request;
      MPIDevice                 _mpi;
      MPICollreg               *_collreg;
      MPIGeometry              *_world_geometry;
      MPICollfactory           *_world_collfactory;
      xmi_geometry_range_t      _world_range;
      int                       _myrank;
      int                       _mysize;
      unsigned                 *_ranklist;
      
    }; // end XMI::Context::MPI
  }; // end namespace Context
}; // end namespace XMI

#endif // __xmi_mpi_mpicontext_h__
