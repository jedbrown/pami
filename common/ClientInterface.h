///
/// \file common/ClientInterface.h
/// \brief PAMI client interface.
///
#ifndef __common_ClientInterface_h__
#define __common_ClientInterface_h__

#include <stdlib.h>
#include <string.h>

#include <pami.h>

#include "util/queue/Queue.h"
#include "Context.h"

namespace PAMI
{
  namespace Interface
  {
    template <class T_Client>
    class Client
    {
      public:
        inline Client (const char * name, pami_result_t & result) 
        {
          result = PAMI_UNIMPL;
        }

        inline ~Client () {}

        static pami_result_t generate (const char           *name,
                                       pami_client_t        *client,
                                       pami_configuration_t  configuration[],
                                       size_t                num_configs);

        static void destroy (pami_client_t client);

        inline char * getName () const;

        ///
        /// \param[in] configuration
        /// \param[in] count
        /// \param[out] contexts	array of contexts created
        /// \param[in,out] ncontexts	num contexts requested (in), created (out)
        ///
        inline pami_result_t createContext (pami_configuration_t   configuration[],
                                           size_t                count,
                                           pami_context_t       * context,
                                           size_t                ncontexts);

        inline pami_result_t destroyContext (pami_context_t context); // deprecated
        inline pami_result_t destroyContext (pami_context_t *context, size_t ncontexts);

        inline pami_result_t query(pami_configuration_t  configuration[],
                                   size_t                num_configs);

        inline pami_result_t update(pami_configuration_t configuration[],
                                    size_t               num_configs);

        inline pami_result_t geometry_world (pami_geometry_t * world_geometry);

        inline pami_result_t geometry_create_taskrange(pami_geometry_t       *geometry,
                                                       pami_configuration_t   configuration[],
                                                       size_t                 num_configs,
                                                       pami_geometry_t        parent,
                                                       unsigned               id,
                                                       pami_geometry_range_t *rank_slices,
                                                       size_t                 slice_count,
                                                       pami_context_t         context,
                                                       pami_event_function    fn,
                                                       void                  *cookie);

        inline pami_result_t geometry_create_tasklist(pami_geometry_t       *geometry,
                                                      pami_configuration_t   configuration[],
                                                      size_t                 num_configs,
                                                      pami_geometry_t        parent,
                                                      unsigned               id,
                                                      pami_task_t           *tasks,
                                                      size_t                 task_count,
                                                      pami_context_t         context,
                                                      pami_event_function    fn,
                                                      void                  *cookie);

        inline pami_result_t geometry_create_topology(pami_geometry_t       *geometry,
                                                      pami_configuration_t   configuration[],
                                                      size_t                 num_configs,
                                                      pami_geometry_t        parent,
                                                      unsigned               id,
                                                      pami_topology_t       *topology,
                                                      pami_context_t         context,
                                                      pami_event_function    fn,
                                                      void                  *cookie);

	inline pami_result_t geometry_query(pami_geometry_t       geometry,
					    pami_configuration_t  configuration[],
					    size_t                num_configs);

	inline pami_result_t geometry_update(pami_geometry_t       geometry,
					     pami_configuration_t  configuration[],
					     size_t                num_configs,
					     pami_context_t        context,
					     pami_event_function   fn,
					     void                 *cookie);

        inline pami_result_t geometry_algorithms_num (pami_geometry_t   geometry,
                                                      pami_xfer_type_t  ctype,
                                                      size_t           *lists_lengths);

        inline pami_result_t geometry_algorithms_info (pami_geometry_t    geometry,
                                                       pami_xfer_type_t   colltype,
                                                       pami_algorithm_t  *algs0,
                                                       pami_metadata_t   *mdata0,
                                                       size_t             num0,
                                                       pami_algorithm_t  *algs1,
                                                       pami_metadata_t   *mdata1,
                                                       size_t             num1);

        inline pami_result_t geometry_destroy(pami_geometry_t geometry);
        inline pami_geometry_t mapidtogeometry (int comm);
        inline void registerUnexpBarrier(unsigned     comm,
                                         pami_quad_t &info,
                                         unsigned     peer,
                                         unsigned     algorithm);
        inline double wtime();      
        inline unsigned long long wtimebase();      
    }; // end class PAMI::Client::Client

    template <class T_Client>
    pami_result_t Client<T_Client>::generate (const char           *name,
                                              pami_client_t        *client,
                                              pami_configuration_t  configuration[],
                                              size_t                num_configs)
    {
      return T_Client::generate_impl(name, client, configuration, num_configs);
    }

    template <class T_Client>
    void Client<T_Client>::destroy (pami_client_t client)
    {
      T_Client::destroy_impl(client);
    }

    template <class T_Client>
    inline char * Client<T_Client>::getName () const
    {
      return static_cast<T_Client*>(this)->getName_impl();
    }

    template <class T_Client>
    inline pami_result_t Client<T_Client>::createContext (pami_configuration_t   configuration[],
                                                         size_t                count,
                                                         pami_context_t       * context,
                                                         size_t                ncontexts)
    {
      return static_cast<T_Client*>(this)->createContext_impl(configuration, count, context, ncontexts);
    }

    template <class T_Client>
    inline pami_result_t Client<T_Client>::destroyContext (pami_context_t context)
    {
      return static_cast<T_Client*>(this)->destroyContext_impl(context);
    }
    template <class T_Client>
    inline pami_result_t Client<T_Client>::destroyContext (pami_context_t *context, size_t ncontexts)
    {
      return static_cast<T_Client*>(this)->destroyContext_impl(context, ncontexts);
    }

    template <class T_Client>
    pami_result_t Client<T_Client>::query(pami_configuration_t configuration[],
                                          size_t               num_configs)
    {
      return static_cast<T_Client*>(this)->query_impl(configuration,num_configs);
    }

    template <class T_Client>
    pami_result_t Client<T_Client>::update(pami_configuration_t configuration[],
                                           size_t               num_configs)
    {
      return static_cast<T_Client*>(this)->update_impl(configuration,num_configs);
    }

    template <class T_Client>
    pami_result_t Client<T_Client>::geometry_world (pami_geometry_t * world_geometry)
    {
      return static_cast<T_Client*>(this)->geometry_world_impl(world_geometry);
    }

    template <class T_Client>
    pami_result_t Client<T_Client>::geometry_create_taskrange (pami_geometry_t       *geometry,
                                                               pami_configuration_t   configuration[],
                                                               size_t                 num_configs,
                                                               pami_geometry_t        parent,
                                                               unsigned               id,
                                                               pami_geometry_range_t *task_slices,
                                                               size_t                 slice_count,
                                                               pami_context_t         context,
                                                               pami_event_function    fn,
                                                               void                  *cookie)
    {
      return static_cast<T_Client*>(this)->geometry_create_taskrange_impl(geometry,
                                                                          configuration,
                                                                          num_configs,
                                                                          parent,
                                                                          id,
                                                                          task_slices,
                                                                          slice_count,
                                                                          context,
                                                                          fn,
                                                                          cookie);
    }

    template <class T_Client>
    pami_result_t Client<T_Client>::geometry_create_tasklist (pami_geometry_t       *geometry,
                                                              pami_configuration_t   configuration[],
                                                              size_t                 num_configs,
                                                              pami_geometry_t        parent,
                                                              unsigned               id,
                                                              pami_task_t           *tasks,
                                                              size_t                 task_count,
                                                              pami_context_t         context,
                                                              pami_event_function    fn,
                                                              void                  *cookie)
    {
      return static_cast<T_Client*>(this)->geometry_create_tasklist_impl(geometry,
                                                                         configuration,
                                                                         num_configs,
                                                                         parent,
                                                                         id,
                                                                         tasks,
                                                                         task_count,
                                                                         context,
                                                                         fn,
                                                                         cookie);
    }

    template <class T_Client>
    pami_result_t Client<T_Client>::geometry_create_topology(pami_geometry_t       *geometry,
                                                             pami_configuration_t   configuration[],
                                                             size_t                 num_configs,
                                                             pami_geometry_t        parent,
                                                             unsigned               id,
                                                             pami_topology_t       *topology,
                                                             pami_context_t         context,
                                                             pami_event_function    fn,
                                                             void                  *cookie)
    {
      return static_cast<T_Client*>(this)->geometry_create_topology_impl(geometry,
                                                                         configuration,
                                                                         num_configs,
                                                                         parent,
                                                                         id,
                                                                         topology,
                                                                         context,
                                                                         fn,
                                                                         cookie);
    }

    template <class T_Client>
    pami_result_t Client<T_Client>::geometry_query(pami_geometry_t       geometry,
						   pami_configuration_t  configuration[],
						   size_t                num_configs)
    {
      return static_cast<T_Client*>(this)->geometry_query_impl(geometry,
                                                               configuration,
                                                               num_configs);
    }

    template <class T_Client>
    pami_result_t Client<T_Client>::geometry_update(pami_geometry_t       geometry,
						    pami_configuration_t  configuration[],
						    size_t                num_configs,
						    pami_context_t        context,
						    pami_event_function   fn,
						    void                 *cookie)
    {
      return static_cast<T_Client*>(this)->geometry_update_impl(geometry,
                                                                configuration,
                                                                num_configs,
								context,
								fn,
								cookie);
    }

    template <class T_Client>
    pami_result_t Client<T_Client>::geometry_algorithms_num (pami_geometry_t geometry,
                                                              pami_xfer_type_t coll_type,
                                                              size_t *lists_lengths)
    {
      return static_cast<T_Client*>(this)->geometry_algorithms_num_impl(geometry,
                                                                         coll_type,
                                                                         lists_lengths);
    }

    template <class T_Client>
    pami_result_t Client<T_Client>::geometry_algorithms_info (pami_geometry_t geometry,
                                                              pami_xfer_type_t   colltype,
                                                              pami_algorithm_t  *algs0,
                                                              pami_metadata_t   *mdata0,
                                                              size_t             num0,
                                                              pami_algorithm_t  *algs1,
                                                              pami_metadata_t   *mdata1,
                                                              size_t             num1)
    {
      return static_cast<T_Client*>(this)->geometry_algorithms_info_impl(geometry,
                                                                         colltype,
                                                                         algs0,
                                                                         mdata0,
                                                                         num0,
                                                                         algs1,
                                                                         mdata1,
                                                                         num1);
    }


    template <class T_Client>
    pami_result_t Client<T_Client>::geometry_destroy (pami_geometry_t geometry)
    {
      return static_cast<T_Client*>(this)->geometry_destroy_impl(geometry);
    }
    
    template <class T_Client>
    pami_geometry_t Client<T_Client>::mapidtogeometry (int comm)
    {
      return static_cast<T_Client*>(this)->mapidtogeometry_impl(comm);
    }

    template <class T_Client>
    inline void Client<T_Client>::registerUnexpBarrier (unsigned     comm,
                                                        pami_quad_t &info,
                                                        unsigned     peer,
                                                        unsigned     algorithm)
    {
      static_cast<T_Client*>(this)->registerUnexpBarrier_impl(comm,
                                                              info,
                                                              peer,
                                                              algorithm);      
    }

    template <class T_Client>
    inline double Client<T_Client>::wtime ( )
    {
      return static_cast<T_Client*>(this)->wtime_impl();
    }

    template <class T_Client>
    inline unsigned long long Client<T_Client>::wtimebase ( )
    {
      return static_cast<T_Client*>(this)->wtimebase_impl();
    }

  }; // end namespace Interface
}; // end namespace PAMI
#endif // __pami_client_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
