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

namespace XMI
{
  namespace Interface
  {
    template <class T_Client, class T_Context>
    class Client : public QueueElem
    {
      public:
        inline Client (const char * name, xmi_result_t & result) :
          QueueElem ()
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
                                           xmi_context_t *contexts,
					   int *ncontexts);

        inline xmi_result_t destroyContext (xmi_context_t context);

    }; // end class XMI::Client::Client

    template <class T_Client, class T_Context>
    xmi_result_t Client<T_Client,T_Context>::generate (const char * name, xmi_client_t * client)
    {
      return T_Client::generate_impl(name, client);
    }

    template <class T_Client, class T_Context>
    void Client<T_Client,T_Context>::destroy (xmi_client_t client)
    {
      T_Client::destroy_impl(client);
    }

    template <class T_Client, class T_Context>
    inline char * Client<T_Client,T_Context>::getName () const
    {
      return static_cast<T_Client*>(this)->getName_impl();
    }

    template <class T_Client, class T_Context>
    inline xmi_result_t Client<T_Client,T_Context>::createContext (xmi_configuration_t configuration[],
                                           size_t                count,
                                           xmi_context_t *contexts,
					   int *ncontexts) {
	return static_cast<T_Client*>(this)->createContext_impl(configuration, count, contexts, ncontexts);
    }

    template <class T_Client, class T_Context>
    inline xmi_result_t Client<T_Client,T_Context>::destroyContext (xmi_context_t context)
    {
      return static_cast<T_Client*>(this)->destroyContext_impl(context);
    }

  }; // end namespace Interface
}; // end namespace XMI


#endif // __xmi_client_h__
