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
    class Client : public XMI::Queue::Element
    {
      public:
        inline Client (const char * name, xmi_result_t & result) :
            XMI::Queue::Element ()
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
