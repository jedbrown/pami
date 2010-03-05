///
/// \file common/ContextInterface.h
/// \brief XMI context interface.
///
#ifndef __common_ContextInterface_h__
#define __common_ContextInterface_h__

#include <stdlib.h>
#include <string.h>

#include "sys/xmi.h"

namespace XMI
{
  namespace Interface
  {
    template <class T_Context>
    class Context
    {
      public:
        inline Context (xmi_client_t client, size_t id) {}

        inline xmi_client_t getClient ();

        inline size_t getId ();

        inline xmi_result_t destroy ();

        inline xmi_result_t post (xmi_work_t *state, xmi_work_function work_fn, void * cookie);

        inline size_t advance (size_t maximum, xmi_result_t & result);

        inline xmi_result_t lock ();

        inline xmi_result_t trylock ();

        inline xmi_result_t unlock ();

        inline xmi_result_t send (xmi_send_t * parameters);

        inline xmi_result_t send (xmi_send_immediate_t * parameters);

        inline xmi_result_t send (xmi_send_typed_t * parameters);

        inline xmi_result_t put (xmi_put_simple_t * parameters);

        inline xmi_result_t put_typed (xmi_put_typed_t * parameters);

        inline xmi_result_t get (xmi_get_simple_t * parameters);

        inline xmi_result_t get_typed (xmi_get_typed_t * parameters);

        inline xmi_result_t rmw (xmi_rmw_t * parameters);

        inline xmi_result_t memregion_register (void            * address,
                                                size_t            bytes,
                                                xmi_memregion_t * memregion);

        inline xmi_result_t memregion_deregister (xmi_memregion_t * memregion);

        inline xmi_result_t memregion_query (xmi_memregion_t    memregion,
                                             void            ** address,
                                             size_t           * bytes,
                                             size_t           * task);

        inline xmi_result_t rput (xmi_rput_simple_t * parameters);

        inline xmi_result_t rput_typed (xmi_rput_typed_t * parameters);

        inline xmi_result_t rget (xmi_rget_simple_t * parameters);

        inline xmi_result_t rget_typed (xmi_rget_typed_t * parameters);

        inline xmi_result_t purge_totask (size_t *dest, size_t count);

        inline xmi_result_t resume_totask (size_t *dest, size_t count);

        inline xmi_result_t fence_begin ();

        inline xmi_result_t fence_end ();

        inline xmi_result_t fence_all (xmi_event_function   done_fn,
                                       void               * cookie);

        inline xmi_result_t fence_task (xmi_event_function   done_fn,
                                        void               * cookie,
                                        size_t               task);

      inline xmi_result_t geometry_algorithms_num (xmi_geometry_t geometry,
                                                   xmi_xfer_type_t ctype,
                                                   int *lists_lengths);

      inline xmi_result_t geometry_algorithms_info (xmi_geometry_t geometry,
                                                     xmi_xfer_type_t   colltype,
                                                     xmi_algorithm_t  *algs0,
                                                     xmi_metadata_t   *mdata0,
                                                     int               num0,
                                                     xmi_algorithm_t  *algs1,
                                                     xmi_metadata_t   *mdata1,
                                                     int               num1);

        inline xmi_result_t collective (xmi_xfer_t * parameters);

        inline xmi_result_t multisend_getroles(size_t          dispatch,
                                               int            *numRoles,
                                               int            *replRole);

        inline xmi_result_t multicast(xmi_multicast_t *mcastinfo);

        inline xmi_result_t manytomany(xmi_manytomany_t *m2minfo);

        inline xmi_result_t multisync(xmi_multisync_t *msyncinfo);

        inline xmi_result_t multicombine(xmi_multicombine_t *mcombineinfo);


        inline xmi_result_t dispatch (size_t                     dispatch,
                                      xmi_dispatch_callback_fn   fn,
                                      void                     * cookie,
                                      xmi_send_hint_t            options);
	//#ifdef __xmi_target_mpi__
        inline xmi_result_t dispatch_new (size_t                 dispatch,
                                          xmi_dispatch_callback_fn   fn,
                                          void                     * cookie,
                                          xmi_dispatch_hint_t        options);
	//#endif
        inline xmi_result_t amcollective_dispatch(xmi_algorithm_t            algorithm,
                                                  size_t                     dispatch,
                                                  xmi_dispatch_callback_fn   fn,
                                                  void                     * cookie,
                                                  xmi_collective_hint_t      options);
    }; // end class XMI::Context::Context

    template <class T_Context>
    xmi_client_t Context<T_Context>::getClient ()
    {
      return static_cast<T_Context*>(this)->getClient_impl();
    }

    template <class T_Context>
    size_t Context<T_Context>::getId ()
    {
      return static_cast<T_Context*>(this)->getId_impl();
    }

    template <class T_Context>
    xmi_result_t Context<T_Context>::destroy ()
    {
      return static_cast<T_Context*>(this)->destroy_impl();
    }

    template <class T_Context>
    xmi_result_t Context<T_Context>::post (xmi_work_t *state, xmi_work_function work_fn, void * cookie)
    {
      return static_cast<T_Context*>(this)->post_impl(state, work_fn, cookie);
    }

    template <class T_Context>
    size_t Context<T_Context>::advance (size_t maximum, xmi_result_t & result)
    {
      return static_cast<T_Context*>(this)->advance_impl(maximum, result);
    }

    template <class T_Context>
    xmi_result_t Context<T_Context>::lock ()
    {
      return static_cast<T_Context*>(this)->lock_impl();
    }

    template <class T_Context>
    xmi_result_t Context<T_Context>::trylock ()
    {
      return static_cast<T_Context*>(this)->trylock_impl();
    }

    template <class T_Context>
    xmi_result_t Context<T_Context>::unlock ()
    {
      return static_cast<T_Context*>(this)->unlock_impl();
    }

    template <class T_Context>
    xmi_result_t Context<T_Context>::send (xmi_send_t * parameters)
    {
      return static_cast<T_Context*>(this)->send_impl(parameters);
    }

    template <class T_Context>
    xmi_result_t Context<T_Context>::send (xmi_send_immediate_t * parameters)
    {
      return static_cast<T_Context*>(this)->send_impl(parameters);
    }

    template <class T_Context>
    xmi_result_t Context<T_Context>::send (xmi_send_typed_t * parameters)
    {
      return static_cast<T_Context*>(this)->send_impl(parameters);
    }

    template <class T_Context>
    xmi_result_t Context<T_Context>::put (xmi_put_simple_t * parameters)
    {
      return static_cast<T_Context*>(this)->put_impl(parameters);
    }

    template <class T_Context>
    xmi_result_t Context<T_Context>::put_typed (xmi_put_typed_t * parameters)
    {
      return static_cast<T_Context*>(this)->put_typed_impl(parameters);
    }

    template <class T_Context>
    xmi_result_t Context<T_Context>::get (xmi_get_simple_t * parameters)
    {
      return static_cast<T_Context*>(this)->get_impl(parameters);
    }

    template <class T_Context>
    xmi_result_t Context<T_Context>::get_typed (xmi_get_typed_t * parameters)
    {
      return static_cast<T_Context*>(this)->get_typed_impl(parameters);
    }

    template <class T_Context>
    xmi_result_t Context<T_Context>::rmw (xmi_rmw_t * parameters)
    {
      return static_cast<T_Context*>(this)->rmw_impl(parameters);
    }

    template <class T_Context>
    xmi_result_t Context<T_Context>::memregion_register (void            * address,
                                                         size_t            bytes,
                                                         xmi_memregion_t * memregion)
    {
      return static_cast<T_Context*>(this)->memregion_register_impl(address,
                                                                    bytes,
                                                                    memregion);
    }

    template <class T_Context>
    xmi_result_t Context<T_Context>::memregion_deregister (xmi_memregion_t * memregion)
    {
      return static_cast<T_Context*>(this)->memregion_deregister_impl(memregion);
    }

    template <class T_Context>
    xmi_result_t Context<T_Context>::memregion_query (xmi_memregion_t    memregion,
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
    xmi_result_t Context<T_Context>::rput (xmi_rput_simple_t * parameters)
    {
      return static_cast<T_Context*>(this)->rput_impl(parameters);
    }

    template <class T_Context>
    xmi_result_t Context<T_Context>::rput_typed (xmi_rput_typed_t * parameters)
    {
      return static_cast<T_Context*>(this)->rput_typed_impl(parameters);
    }

    template <class T_Context>
    xmi_result_t Context<T_Context>::rget (xmi_rget_simple_t * parameters)
    {
      return static_cast<T_Context*>(this)->rget_impl(parameters);
    }

    template <class T_Context>
    xmi_result_t Context<T_Context>::rget_typed (xmi_rget_typed_t * parameters)
    {
      return static_cast<T_Context*>(this)->rget_typed_impl(parameters);
    }

    template <class T_Context>
    xmi_result_t Context<T_Context>::purge_totask (size_t *dest, size_t count)
    {
      return static_cast<T_Context*>(this)->purge_totask_impl(dest, count);
    }

    template <class T_Context>
    xmi_result_t Context<T_Context>::resume_totask (size_t *dest, size_t count)
    {
      return static_cast<T_Context*>(this)->resume_totask_impl(dest, count);
    }

    template <class T_Context>
    xmi_result_t Context<T_Context>::fence_begin ()
    {
      return static_cast<T_Context*>(this)->fence_begin_impl();
    }

    template <class T_Context>
    xmi_result_t Context<T_Context>::fence_end ()
    {
      return static_cast<T_Context*>(this)->fence_end_impl();
    }

    template <class T_Context>
    xmi_result_t Context<T_Context>::fence_all (xmi_event_function   done_fn,
                                                void               * cookie)
    {
      return static_cast<T_Context*>(this)->fence_all_impl(done_fn, cookie);
    }

    template <class T_Context>
    xmi_result_t Context<T_Context>::fence_task (xmi_event_function   done_fn,
                                                 void               * cookie,
                                                 size_t               task)
    {
      return static_cast<T_Context*>(this)->fence_task_impl(done_fn, cookie, task);
    }

    template <class T_Context>
    xmi_result_t Context<T_Context>::geometry_algorithms_num (xmi_geometry_t geometry,
                                                              xmi_xfer_type_t coll_type,
                                                              int *lists_lengths)
    {
      return static_cast<T_Context*>(this)->geometry_algorithms_num_impl(geometry,
                                                                         coll_type,
                                                                         lists_lengths);
    }

    template <class T_Context>
    xmi_result_t Context<T_Context>::geometry_algorithms_info (xmi_geometry_t geometry,
                                                              xmi_xfer_type_t   colltype,
                                                              xmi_algorithm_t  *algs0,
                                                              xmi_metadata_t   *mdata0,
                                                              int               num0,
                                                              xmi_algorithm_t  *algs1,
                                                              xmi_metadata_t   *mdata1,
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
    xmi_result_t Context<T_Context>::collective(xmi_xfer_t * parameters)
    {
      return static_cast<T_Context*>(this)->collective_impl(parameters);
    }

    template <class T_Context>
    xmi_result_t Context<T_Context>::multisend_getroles(size_t          dispatch,
                                                        int            *numRoles,
                                                        int            *replRole)
    {
        return static_cast<T_Context*>(this)->multisend_getroles_impl(dispatch,numRoles,replRole);
    }

    template <class T_Context>
    xmi_result_t Context<T_Context>::multicast(xmi_multicast_t *mcastinfo)
    {
        return static_cast<T_Context*>(this)->multicast_impl(mcastinfo);
    }

    template <class T_Context>
    xmi_result_t Context<T_Context>::manytomany(xmi_manytomany_t *m2minfo)
    {
        return static_cast<T_Context*>(this)->manytomany_impl(m2minfo);
    }

    template <class T_Context>
    xmi_result_t Context<T_Context>::multisync(xmi_multisync_t *msyncinfo)
    {
        return static_cast<T_Context*>(this)->multisync_impl(msyncinfo);
    }

    template <class T_Context>
    xmi_result_t Context<T_Context>::multicombine(xmi_multicombine_t *mcombineinfo)
    {
        return static_cast<T_Context*>(this)->multicombine_impl(mcombineinfo);
    }

    template <class T_Context>
    xmi_result_t Context<T_Context>::dispatch (size_t                     dispatch,
                                               xmi_dispatch_callback_fn   fn,
                                               void                     * cookie,
                                               xmi_send_hint_t            options)
    {
        return static_cast<T_Context*>(this)->dispatch_impl(dispatch,fn,cookie,options);
    }
//#ifdef __xmi_target_mpi__
    template <class T_Context>
    xmi_result_t Context<T_Context>::dispatch_new (size_t                 dispatch,
                                               xmi_dispatch_callback_fn   fn,
                                               void                     * cookie,
                                               xmi_dispatch_hint_t        options)
    {
        return static_cast<T_Context*>(this)->dispatch_new_impl(dispatch,fn,cookie,options);
    }
//#endif
    template <class T_Context>
    xmi_result_t Context<T_Context>::amcollective_dispatch(xmi_algorithm_t            algorithm,
                                                           size_t                     dispatch,
                                                           xmi_dispatch_callback_fn   fn,
                                                           void                     * cookie,
                                                           xmi_collective_hint_t      options)
    {
      return static_cast<T_Context*>(this)->amcollective_dispatch_impl(algorithm,
                                                                       dispatch,
                                                                       fn,
                                                                       cookie,
                                                                       options);
    }
  }; // end namespace Interface
}; // end namespace XMI

#endif // __components_context_context_h__
