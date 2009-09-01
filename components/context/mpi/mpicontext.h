///
/// \file xmi/mpi/mpicontext.h
/// \brief XMI MPI specific context implementation.
///
#ifndef   __xmi_mpi_mpicontext_h__
#define   __xmi_mpi_mpicontext_h__

#include <stdlib.h>
#include <string.h>

#include "components/context/Context.h"

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
        }
        
        inline xmi_client_t getClientId_impl ()
        {
          return _client;
        }

        inline xmi_client_t destroy_impl ()
        {
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

        inline xmi_result_t dispatch_impl (xmi_dispatch_t             id,
                                           xmi_dispatch_callback_fn   fn,
                                           void                     * cookie,
                                           xmi_send_hint_t            options)
        {
          return XMI_UNIMPL;
        }

      private:
      
        xmi_client_t _client;

        void * _dispatch[1024];

    }; // end XMI::Context::MPI
  }; // end namespace Context
}; // end namespace XMI

#endif // __xmi_mpi_mpicontext_h__
