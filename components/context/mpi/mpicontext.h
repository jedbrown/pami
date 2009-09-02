///
/// \file xmi/mpi/mpicontext.h
/// \brief XMI MPI specific context implementation.
///
#ifndef   __xmi_mpi_mpicontext_h__
#define   __xmi_mpi_mpicontext_h__

#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include "components/context/Context.h"
#include <platform.h>
#include <new>

#define XMI_CONTEXT_CLASS XMI::Context::MPI

namespace XMI
{
  namespace Context
  {
    class MPI : public Context<XMI::Context::MPI>
    {
    public:
      inline MPI (xmi_client_t client) :
        Context<XMI::Context::MPI> (client),
        _client (client)
        {
          // Initialize MPI.  We can only do this once
          static int initialized = 0;
          if(initialized==0)
              {
                int rc = MPI_Init(0, NULL);
                if(rc != MPI_SUCCESS)
                    {
                      fprintf(stderr, "Unable to initialize context:  MPI_Init failure\n");
                      XMI_abort();
                    }
                initialized=1;
              }
          MPI_Comm_rank(MPI_COMM_WORLD,&_myrank);
          MPI_Comm_size(MPI_COMM_WORLD,&_mysize); 

          _ranklist = (unsigned*)malloc(sizeof(unsigned)*_mysize);
          for (int i=0; i<_mysize; i++) _ranklist[i]=i;
          
          // Initialize Global Geometry for the context
          _world_geometry =
            (XMI::Geometry::Geometry<XMI_GEOMETRY_CLASS>*)
            malloc(sizeof(*_world_geometry));
          new(_world_geometry)
            XMI::Geometry::Geometry<XMI_GEOMETRY_CLASS>(NULL,        // Mapping
                                                        _ranklist,   // Ranks
                                                        _mysize,     // NumRanks
                                                        0,           // Comm id
                                                        0,           // numcolors
                                                        1);          // isglobal?
        }
        
      inline xmi_client_t getClientId_impl ()
        {
          return _client;
        }

      inline xmi_client_t destroy_impl ()
        {
          // Do not call finalize because if we do
          // it is not valid to call init again
          // per the MPI spec.
        }

      inline xmi_result_t post_impl (xmi_event_function work_fn, void * cookie)
        {
          return XMI_UNIMPL;
        }

      inline size_t advance_impl (size_t maximum, xmi_result_t & result)
        {
          result = XMI_UNIMPL;
          return 0;
        }

      inline xmi_result_t lock_impl ()
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t trylock_impl ()
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t unlock_impl ()
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t send_impl (xmi_send_simple_t * parameters)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t send_impl (xmi_send_immediate_t * parameters)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t send_impl (xmi_send_typed_t * parameters)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t put_impl (xmi_put_simple_t * parameters)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t put_typed_impl (xmi_put_typed_t * parameters)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t get_impl (xmi_get_simple_t * parameters)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t get_typed_impl (xmi_get_typed_t * parameters)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t rmw_impl (xmi_rmw_t * parameters)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t memregion_register_impl (void            * address,
                                                   size_t            bytes,
                                                   xmi_memregion_t * memregion)
        {
          return XMI_UNIMPL;
        }
        
      inline xmi_result_t memregion_deregister_impl (xmi_memregion_t * memregion)
        {
          return XMI_UNIMPL;
        }


      inline xmi_result_t memregion_query_impl (xmi_memregion_t    memregion,
                                                void            ** address,
                                                size_t           * bytes,
                                                size_t           * task)
        {
          return XMI_UNIMPL;
        }
        

      inline xmi_result_t memregion_register_impl (xmi_rmw_t * parameters)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t rput_impl (xmi_rput_simple_t * parameters)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t rput_typed_impl (xmi_rput_typed_t * parameters)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t rget_impl (xmi_rget_simple_t * parameters)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t rget_typed_impl (xmi_rget_typed_t * parameters)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t purge_totask_impl (size_t * dest, size_t count)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t resume_totask_impl (size_t * dest, size_t count)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t fence_begin_impl ()
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t fence_end_impl ()
        {
          return XMI_UNIMPL;
        }
        
      inline xmi_result_t fence_all_impl (xmi_event_function   done_fn,
                                          void               * cookie)
        {
          return XMI_UNIMPL;
        }
        
      inline  xmi_result_t fence_task_impl (xmi_event_function   done_fn,
                                            void               * cookie,
                                            size_t               task)
        {
          return XMI_UNIMPL;
        }
        
      inline xmi_result_t geometry_initialize_impl (xmi_geometry_t       * geometry,
                                                    unsigned               id,
                                                    xmi_geometry_range_t * rank_slices,
                                                    unsigned               slice_count)
        {
          return XMI_UNIMPL;
        }
            

      inline xmi_result_t geometry_world_impl (xmi_geometry_t * world_geometry)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t geometry_algorithm_impl (xmi_geometry_t   geometry,
                                                   xmi_algorithm_t *algorithm,
                                                   int             *num)
        {
          return XMI_UNIMPL;
        }
        
      inline xmi_result_t geometry_finalize_impl (xmi_geometry_t geometry)
        {
          return XMI_UNIMPL;
        }


      inline xmi_result_t collective_impl (xmi_xfer_t * parameters)
        {
          return XMI_UNIMPL;
        }
        
      inline xmi_result_t multisend_getroles_impl(xmi_dispatch_t  dispatch,
                                                  int            *numRoles,
                                                  int            *replRole)
        {
          return XMI_UNIMPL;
        }
        
      inline xmi_result_t multicast_impl(xmi_multicast_t *mcastinfo)
        {
          return XMI_UNIMPL;
        }
        
      inline xmi_result_t manytomany_impl(xmi_manytomany_t *m2minfo)
        {
          return XMI_UNIMPL;
        }
        
      inline xmi_result_t multisync_impl(xmi_multisync_t *msyncinfo)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t multicombine_impl(xmi_multicombine_t *mcombineinfo)
        {
          return XMI_UNIMPL;
        }
      inline xmi_result_t dispatch_impl (xmi_dispatch_t             id,
                                         xmi_dispatch_callback_fn   fn,
                                         void                     * cookie,
                                         xmi_send_hint_t            options)
        {
          return XMI_UNIMPL;
        }

    private:
      xmi_client_t _client;
      int          _myrank;
      int          _mysize;
      unsigned    *_ranklist;
      void        *_dispatch[1024];
      XMI::Geometry::Geometry<XMI_GEOMETRY_CLASS> *_world_geometry;
    }; // end XMI::Context::MPI
  }; // end namespace Context
}; // end namespace XMI

#endif // __xmi_mpi_mpicontext_h__
