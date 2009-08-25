///
/// \file xmi/Context.h
/// \brief XMI context implementation.
///
#ifndef   __xmi_context_h__
#define   __xmi_context_h__

#include <stdlib.h>
#include <string.h>

#include "sys/xmi.h"

namespace XMI
{
  namespace Context
  {
    template <class T_Context>
    class Context
    {
      public:
        inline Context (xmi_client_t client) {}

        inline xmi_client_t getClientId ();

        inline void destroy ();

        inline xmi_result_t post (xmi_event_function work_fn, void * cookie);

        inline size_t advance (size_t maximum, xmi_result_t & result);

        inline xmi_result_t lock ();

        inline xmi_result_t trylock ();

        inline xmi_result_t unlock ();

        inline xmi_result_t send (xmi_send_simple_t * parameters);

        inline xmi_result_t send (xmi_send_immediate_t * parameters);

        inline xmi_result_t send (xmi_send_typed_t * parameters);

        inline xmi_result_t dispatch (xmi_dispatch_t             dispatch,
                                      xmi_dispatch_callback_fn   fn,
                                      void                     * cookie,
                                      xmi_send_hint_t            options);


    }; // end XMI::Context::Context

    template <class T_Context>
    xmi_client_t Context<T_Context>::getClientId ()
    {
      return static_cast<T_Context*>(this)->getClientId_impl();
    }

    template <class T_Context>
    void Context<T_Context>::destroy ()
    {
      static_cast<T_Context*>(this)->destroy_impl();
    }

    template <class T_Context>
    xmi_result_t Context<T_Context>::post (xmi_event_function work_fn, void * cookie)
    {
      return static_cast<T_Context*>(this)->post_impl(work_fn, cookie);
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
    xmi_result_t Context<T_Context>::send (xmi_send_simple_t * parameters)
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
    xmi_result_t Context<T_Context>::dispatch (xmi_dispatch_t             dispatch,
                                               xmi_dispatch_callback_fn   fn,
                                               void                     * cookie,
                                               xmi_send_hint_t            options)
    {
      return static_cast<T_Context*>(this)->dispatch_impl(dispatch,fn,cookie,options);
    }





  }; // end namespace Context
}; // end namespace XMI

#endif // __xmi_context_h__
