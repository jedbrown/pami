///
/// \file xmi/bgp/bgpclient.h
/// \brief XMI client interface specific for the BGP platform.
///
#ifndef   __xmi_bgp_bgpclient_h__
#define   __xmi_bgp_bgpclient_h__

#include <stdlib.h>

#include "xmi/Client.h"

#include "bgpcontext.h"

#define XMI_CLIENT_CLASS XMI::Client::BGP

namespace XMI
{
  namespace Client
  {
    class BGP : public Client<XMI::Client::BGP,XMI::Context::BGP>
    {
      public:
        inline BGP (char * name) :
          Client<XMI::Client::BGP,XMI::Context::BGP>(name),
          _client ((xmi_client_t) this),
          _references (1)
        {
        }

        inline ~BGP ()
        {
        }

        static XMI::Client::BGP * generate_impl (char * name)
        {
          int rc = 0;

          //__client_list->lock();

          // If a client with this name is not already initialized...
          XMI::Client::BGP * client = NULL;
          //if ((client = __client_list->contains (name)) == NULL)
          //{
            rc = posix_memalign((void **)&client, 16, sizeof (XMI::Client::BGP));
            if (rc != 0) assert(0);
            memset ((void *)client, 0x00, sizeof(XMI::Client::BGP));
            new (client) XMI::Client::BGP (name);
            //__client_list->pushHead ((QueueElem *) client);
          //}
          //else
          //{
            //client->incReferenceCount ();
          //}

          //__client_list->unlock();

          return client;
        }

        static void destroy_impl (XMI::Client::BGP * client)
        {
          //__client_list->lock ();
          //client->decReferenceCount ();
          //if (client->getReferenceCount () == 0)
          //{
            //__client_list->remove (client);
            free (client);
          //}
          //__client_list->unlock ();
        }

        inline char * getName_impl ()
        {
          return "";
        }

        inline XMI::Context::BGP * createContext_impl (xmi_configuration_t configuration[],
                                                       size_t              count)
        {
          //_context_list->lock ();

          XMI::Context::BGP * context = NULL;
          int rc = posix_memalign((void **)&context, 16, sizeof (XMI::Context::BGP));
          if (rc != 0) assert(0);
          memset ((void *)context, 0x00, sizeof(XMI::Context::BGP));
          new (context) XMI::Context::BGP (this->getClientId());
          //_context_list->pushHead ((QueueElem *) context);

          //_context_list->unlock ();

          return context;
        }
        
        inline void destroyContext_impl (XMI::Context::BGP * context)
        {
          //_context_list->lock ();
          //_context_list->remove (context);
          context->destroy ();
          //_context_list->unlock ();
        }
        
      protected:
      
        inline xmi_client_t getClientId () const
        {
          return _client;
        }

      private:

        xmi_client_t _client;
        size_t       _references;

    }; // end class XMI::Client::BGP
  }; // end namespace Client
}; // end namespace XMI


#endif // __xmi_bgp_bgpclient_h__
