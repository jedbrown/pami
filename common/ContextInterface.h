///
/// \file common/ContextInterface.h
/// \brief PAMI context interface.
///
#ifndef __common_ContextInterface_h__
#define __common_ContextInterface_h__

#include <stdlib.h>
#include <string.h>

#include "sys/pami.h"

namespace PAMI
{
  namespace Interface
  {
    template <class T_Context>
    class Context
    {
      public:
        inline Context (pami_client_t client, size_t id) {}

        inline pami_client_t getClient ();

        inline size_t getId ();

        inline pami_result_t destroy ();

        inline pami_result_t post (pami_work_t *state, pami_work_function work_fn, void * cookie);

        inline size_t advance (size_t maximum, pami_result_t & result);

        inline pami_result_t lock ();

        inline pami_result_t trylock ();

        inline pami_result_t unlock ();

        inline pami_result_t send (pami_send_t * parameters);

        inline pami_result_t send (pami_send_immediate_t * parameters);

        inline pami_result_t send (pami_send_typed_t * parameters);

        inline pami_result_t put (pami_put_simple_t * parameters);

        inline pami_result_t put_typed (pami_put_typed_t * parameters);

        inline pami_result_t get (pami_get_simple_t * parameters);

        inline pami_result_t get_typed (pami_get_typed_t * parameters);

        inline pami_result_t rmw (pami_rmw_t * parameters);

        inline pami_result_t memregion_register (void            * address,
                                                size_t            bytes,
                                                pami_memregion_t * memregion);

        inline pami_result_t memregion_deregister (pami_memregion_t * memregion);

        inline pami_result_t memregion_query (pami_memregion_t    memregion,
                                             void            ** address,
                                             size_t           * bytes,
                                             size_t           * task);

        inline pami_result_t rput (pami_rput_simple_t * parameters);

        inline pami_result_t rput_typed (pami_rput_typed_t * parameters);

        inline pami_result_t rget (pami_rget_simple_t * parameters);

        inline pami_result_t rget_typed (pami_rget_typed_t * parameters);

        inline pami_result_t purge_totask (size_t *dest, size_t count);

        inline pami_result_t resume_totask (size_t *dest, size_t count);

        inline pami_result_t fence_begin ();

        inline pami_result_t fence_end ();

        inline pami_result_t fence_all (pami_event_function   done_fn,
                                       void               * cookie);

        inline pami_result_t fence_task (pami_event_function   done_fn,
                                        void               * cookie,
                                        size_t               task);

      inline pami_result_t geometry_algorithms_num (pami_geometry_t geometry,
                                                   pami_xfer_type_t ctype,
                                                   int *lists_lengths);

      inline pami_result_t geometry_algorithms_info (pami_geometry_t geometry,
                                                     pami_xfer_type_t   colltype,
                                                     pami_algorithm_t  *algs0,
                                                     pami_metadata_t   *mdata0,
                                                     int               num0,
                                                     pami_algorithm_t  *algs1,
                                                     pami_metadata_t   *mdata1,
                                                     int               num1);

        inline pami_result_t collective (pami_xfer_t * parameters);

        inline pami_result_t multisend_getroles(size_t          dispatch,
                                               int            *numRoles,
                                               int            *replRole);

        inline pami_result_t multicast(pami_multicast_t *mcastinfo);

        inline pami_result_t manytomany(pami_manytomany_t *m2minfo);

        inline pami_result_t multisync(pami_multisync_t *msyncinfo);

        inline pami_result_t multicombine(pami_multicombine_t *mcombineinfo);


        inline pami_result_t dispatch (size_t                     dispatch,
                                      pami_dispatch_callback_fn   fn,
                                      void                     * cookie,
                                      pami_send_hint_t            options);
	//#ifdef __pami_target_mpi__
        inline pami_result_t dispatch_new (size_t                 dispatch,
                                          pami_dispatch_callback_fn   fn,
                                          void                     * cookie,
                                          pami_dispatch_hint_t        options);
	//#endif
        inline pami_result_t amcollective_dispatch(pami_algorithm_t            algorithm,
                                                  size_t                     dispatch,
                                                  pami_dispatch_callback_fn   fn,
                                                  void                     * cookie,
                                                  pami_collective_hint_t      options);
    }; // end class PAMI::Context::Context

    template <class T_Context>
    pami_client_t Context<T_Context>::getClient ()
    {
      return static_cast<T_Context*>(this)->getClient_impl();
    }

    template <class T_Context>
    size_t Context<T_Context>::getId ()
    {
      return static_cast<T_Context*>(this)->getId_impl();
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::destroy ()
    {
      return static_cast<T_Context*>(this)->destroy_impl();
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::post (pami_work_t *state, pami_work_function work_fn, void * cookie)
    {
      return static_cast<T_Context*>(this)->post_impl(state, work_fn, cookie);
    }

    template <class T_Context>
    size_t Context<T_Context>::advance (size_t maximum, pami_result_t & result)
    {
      return static_cast<T_Context*>(this)->advance_impl(maximum, result);
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::lock ()
    {
      return static_cast<T_Context*>(this)->lock_impl();
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::trylock ()
    {
      return static_cast<T_Context*>(this)->trylock_impl();
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::unlock ()
    {
      return static_cast<T_Context*>(this)->unlock_impl();
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::send (pami_send_t * parameters)
    {
      return static_cast<T_Context*>(this)->send_impl(parameters);
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::send (pami_send_immediate_t * parameters)
    {
      return static_cast<T_Context*>(this)->send_impl(parameters);
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::send (pami_send_typed_t * parameters)
    {
      return static_cast<T_Context*>(this)->send_impl(parameters);
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::put (pami_put_simple_t * parameters)
    {
      return static_cast<T_Context*>(this)->put_impl(parameters);
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::put_typed (pami_put_typed_t * parameters)
    {
      return static_cast<T_Context*>(this)->put_typed_impl(parameters);
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::get (pami_get_simple_t * parameters)
    {
      return static_cast<T_Context*>(this)->get_impl(parameters);
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::get_typed (pami_get_typed_t * parameters)
    {
      return static_cast<T_Context*>(this)->get_typed_impl(parameters);
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::rmw (pami_rmw_t * parameters)
    {
      return static_cast<T_Context*>(this)->rmw_impl(parameters);
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::memregion_register (void            * address,
                                                         size_t            bytes,
                                                         pami_memregion_t * memregion)
    {
      return static_cast<T_Context*>(this)->memregion_register_impl(address,
                                                                    bytes,
                                                                    memregion);
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::memregion_deregister (pami_memregion_t * memregion)
    {
      return static_cast<T_Context*>(this)->memregion_deregister_impl(memregion);
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::memregion_query (pami_memregion_t    memregion,
                                                      void            ** address,
                                                      size_t           * bytes,
                                                      size_t           * task)
    {
      return static_cast<T_Context*>(this)->memregion_query_impl(memregion,
                                                                 address,
                                                                 bytes,
                                                                 task);
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::rput (pami_rput_simple_t * parameters)
    {
      return static_cast<T_Context*>(this)->rput_impl(parameters);
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::rput_typed (pami_rput_typed_t * parameters)
    {
      return static_cast<T_Context*>(this)->rput_typed_impl(parameters);
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::rget (pami_rget_simple_t * parameters)
    {
      return static_cast<T_Context*>(this)->rget_impl(parameters);
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::rget_typed (pami_rget_typed_t * parameters)
    {
      return static_cast<T_Context*>(this)->rget_typed_impl(parameters);
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::purge_totask (size_t *dest, size_t count)
    {
      return static_cast<T_Context*>(this)->purge_totask_impl(dest, count);
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::resume_totask (size_t *dest, size_t count)
    {
      return static_cast<T_Context*>(this)->resume_totask_impl(dest, count);
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::fence_begin ()
    {
      return static_cast<T_Context*>(this)->fence_begin_impl();
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::fence_end ()
    {
      return static_cast<T_Context*>(this)->fence_end_impl();
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::fence_all (pami_event_function   done_fn,
                                                void               * cookie)
    {
      return static_cast<T_Context*>(this)->fence_all_impl(done_fn, cookie);
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::fence_task (pami_event_function   done_fn,
                                                 void               * cookie,
                                                 size_t               task)
    {
      return static_cast<T_Context*>(this)->fence_task_impl(done_fn, cookie, task);
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::geometry_algorithms_num (pami_geometry_t geometry,
                                                              pami_xfer_type_t coll_type,
                                                              int *lists_lengths)
    {
      return static_cast<T_Context*>(this)->geometry_algorithms_num_impl(geometry,
                                                                         coll_type,
                                                                         lists_lengths);
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::geometry_algorithms_info (pami_geometry_t geometry,
                                                              pami_xfer_type_t   colltype,
                                                              pami_algorithm_t  *algs0,
                                                              pami_metadata_t   *mdata0,
                                                              int               num0,
                                                              pami_algorithm_t  *algs1,
                                                              pami_metadata_t   *mdata1,
                                                              int               num1)
    {
      return static_cast<T_Context*>(this)->geometry_algorithms_info_impl(geometry,
                                                                          colltype,
                                                                          algs0,
                                                                          mdata0,
                                                                          num0,
                                                                          algs1,
                                                                          mdata1,
                                                                          num1);
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::collective(pami_xfer_t * parameters)
    {
      return static_cast<T_Context*>(this)->collective_impl(parameters);
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::multisend_getroles(size_t          dispatch,
                                                        int            *numRoles,
                                                        int            *replRole)
    {
        return static_cast<T_Context*>(this)->multisend_getroles_impl(dispatch,numRoles,replRole);
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::multicast(pami_multicast_t *mcastinfo)
    {
        return static_cast<T_Context*>(this)->multicast_impl(mcastinfo);
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::manytomany(pami_manytomany_t *m2minfo)
    {
        return static_cast<T_Context*>(this)->manytomany_impl(m2minfo);
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::multisync(pami_multisync_t *msyncinfo)
    {
        return static_cast<T_Context*>(this)->multisync_impl(msyncinfo);
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::multicombine(pami_multicombine_t *mcombineinfo)
    {
        return static_cast<T_Context*>(this)->multicombine_impl(mcombineinfo);
    }

    template <class T_Context>
    pami_result_t Context<T_Context>::dispatch (size_t                     dispatch,
                                               pami_dispatch_callback_fn   fn,
                                               void                     * cookie,
                                               pami_send_hint_t            options)
    {
        return static_cast<T_Context*>(this)->dispatch_impl(dispatch,fn,cookie,options);
    }
//#ifdef __pami_target_mpi__
    template <class T_Context>
    pami_result_t Context<T_Context>::dispatch_new (size_t                 dispatch,
                                               pami_dispatch_callback_fn   fn,
                                               void                     * cookie,
                                               pami_dispatch_hint_t        options)
    {
        return static_cast<T_Context*>(this)->dispatch_new_impl(dispatch,fn,cookie,options);
    }
//#endif
    template <class T_Context>
    pami_result_t Context<T_Context>::amcollective_dispatch(pami_algorithm_t            algorithm,
                                                           size_t                     dispatch,
                                                           pami_dispatch_callback_fn   fn,
                                                           void                     * cookie,
                                                           pami_collective_hint_t      options)
    {
      return static_cast<T_Context*>(this)->amcollective_dispatch_impl(algorithm,
                                                                       dispatch,
                                                                       fn,
                                                                       cookie,
                                                                       options);
    }
  }; // end namespace Interface
}; // end namespace PAMI

#endif // __components_context_context_h__
