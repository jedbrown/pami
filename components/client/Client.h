///
/// \file xmi/Client.h
/// \brief XMI client interface.
///
#ifndef   __xmi_client_h__
#define   __xmi_client_h__

#include <stdlib.h>
#include <string.h>

#include "queueing/Queue.h"

namespace XMI
{
  namespace Client
  {
    template <class T_Client, class T_Context>
    class Client : public DCMF::Queueing::QueueElem
    {
      public:
        inline Client (char * name) :
          DCMF::Queueing::QueueElem ()
        {
        }

        inline ~Client () {}

        static T_Client * generate (char * name);

        static void destroy (T_Client * client);

        inline char * getName () const;

        inline T_Context * createContext (xmi_configuration_t configuration[],
                                          size_t              count);

        inline void destroyContext (T_Context * context);

    }; // end class XMI::Client::Client

    template <class T_Client, class T_Context>
    T_Client * Client<T_Client,T_Context>::generate (char * name)
    {
      return T_Client::generate_impl(name);
    }

    template <class T_Client, class T_Context>
    void Client<T_Client,T_Context>::destroy (T_Client * client)
    {
      T_Client::destroy_impl(client);
    }

    template <class T_Client, class T_Context>
    inline char * Client<T_Client,T_Context>::getName () const
    {
      return static_cast<T_Client*>(this)->getName_impl();
    }

    template <class T_Client, class T_Context>
    inline T_Context * Client<T_Client,T_Context>::createContext (xmi_configuration_t configuration[],
                                                                  size_t              count)
    {
      return static_cast<T_Client*>(this)->createContext_impl(configuration, count);
    }

    template <class T_Client, class T_Context>
    inline void Client<T_Client,T_Context>::destroyContext (T_Context * context)
    {
      static_cast<T_Client*>(this)->destroyContext_impl(context);
    }

  }; // end namespace Client
}; // end namespace XMI


#endif // __xmi_client_h__
