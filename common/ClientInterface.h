///
/// \file common/ClientInterface.h
/// \brief XMI client interface.
///
#ifndef __common_ClientInterface_h__
#define __common_ClientInterface_h__

#include <stdlib.h>
#include <string.h>

#include "sys/xmi.h"

#include "util/queue/Queue.h"
#include "Context.h"

namespace XMI
{
  namespace Interface
  {
    template <class T_Client>
    class Client : public XMI::QueueElem
    {
      public:
        inline Client (const char * name, xmi_result_t & result) :
            XMI::QueueElem ()
        {
          result = XMI_UNIMPL;
        }

        inline ~Client () {}

        static xmi_result_t generate (const char * name, xmi_client_t * client);

        static void destroy (xmi_client_t client);

        inline char * getName () const;

        ///
        /// \param[in] configuration
        /// \param[in] count
        /// \param[out] contexts	array of contexts created
        /// \param[in,out] ncontexts	num contexts requested (in), created (out)
        ///
        inline xmi_result_t createContext (xmi_configuration_t   configuration[],
                                           size_t                count,
                                           xmi_context_t       * context,
                                           size_t                ncontexts);

        inline xmi_result_t destroyContext (xmi_context_t context);

        inline xmi_result_t queryConfiguration (xmi_configuration_t * configuration);

        inline xmi_result_t geometry_world (xmi_geometry_t * world_geometry);

        inline xmi_result_t geometry_create_taskrange(xmi_geometry_t       * geometry,
                                                      xmi_geometry_t         parent,
                                                      unsigned               id,
                                                      xmi_geometry_range_t * rank_slices,
                                                      size_t                 slice_count,
                                                      xmi_context_t          context,
                                                      xmi_event_function     fn,
                                                      void                 * cookie);

        inline xmi_result_t geometry_create_tasklist(xmi_geometry_t       * geometry,
                                                     xmi_geometry_t         parent,
                                                     unsigned               id,
                                                     xmi_task_t           * tasks,
                                                     size_t                 task_count,
                                                     xmi_context_t          context,
                                                     xmi_event_function     fn,
                                                     void                 * cookie);

        inline xmi_result_t geometry_destroy(xmi_geometry_t geometry);
    }; // end class XMI::Client::Client

    template <class T_Client>
    xmi_result_t Client<T_Client>::generate (const char * name, xmi_client_t * client)
    {
      return T_Client::generate_impl(name, client);
    }

    template <class T_Client>
    void Client<T_Client>::destroy (xmi_client_t client)
    {
      T_Client::destroy_impl(client);
    }

    template <class T_Client>
    inline char * Client<T_Client>::getName () const
    {
      return static_cast<T_Client*>(this)->getName_impl();
    }

    template <class T_Client>
    inline xmi_result_t Client<T_Client>::createContext (xmi_configuration_t   configuration[],
                                                         size_t                count,
                                                         xmi_context_t       * context,
                                                         size_t                ncontexts)
    {
      return static_cast<T_Client*>(this)->createContext_impl(configuration, count, context, ncontexts);
    }

    template <class T_Client>
    inline xmi_result_t Client<T_Client>::destroyContext (xmi_context_t context)
    {
      return static_cast<T_Client*>(this)->destroyContext_impl(context);
    }

    template <class T_Client>
    xmi_result_t Client<T_Client>::queryConfiguration (xmi_configuration_t * configuration)
    {
      return static_cast<T_Client*>(this)->queryConfiguration_impl(configuration);
    }

    template <class T_Client>
    xmi_result_t Client<T_Client>::geometry_world (xmi_geometry_t * world_geometry)
    {
      return static_cast<T_Client*>(this)->geometry_world_impl(world_geometry);
    }

    template <class T_Client>
    xmi_result_t Client<T_Client>::geometry_create_taskrange (xmi_geometry_t       * geometry,
                                                              xmi_geometry_t         parent,
                                                              unsigned               id,
                                                              xmi_geometry_range_t * task_slices,
                                                              size_t                 slice_count,
                                                              xmi_context_t          context,
                                                              xmi_event_function     fn,
                                                              void                 * cookie)
    {
      return static_cast<T_Client*>(this)->geometry_create_taskrange_impl(geometry,
                                                                          parent,
                                                                          id,
                                                                          task_slices,
                                                                          slice_count,
                                                                          context,
                                                                          fn,
                                                                          cookie);
    }

    template <class T_Client>
    xmi_result_t Client<T_Client>::geometry_create_tasklist (xmi_geometry_t       * geometry,
                                                             xmi_geometry_t         parent,
                                                             unsigned               id,
                                                             xmi_task_t           * tasks,
                                                             size_t                 task_count,
                                                             xmi_context_t          context,
                                                             xmi_event_function     fn,
                                                             void                 * cookie)
    {
      return static_cast<T_Client*>(this)->geometry_create_tasklist_impl(geometry,
                                                                          parent,
                                                                          id,
                                                                          tasks,
                                                                          task_count,
                                                                          context,
                                                                          fn,
                                                                          cookie);
    }

    template <class T_Client>
    xmi_result_t Client<T_Client>::geometry_destroy (xmi_geometry_t geometry)
    {
      return static_cast<T_Client*>(this)->geometry_destroy_impl(geometry);
    }

  }; // end namespace Interface
}; // end namespace XMI
#endif // __xmi_client_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
