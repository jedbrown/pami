///
/// \file components/client/bgp/BgpClient.h
/// \brief XMI client interface specific for the BGP platform.
///
#ifndef   __components_client_bgp_bgpclient_h__
#define   __components_client_bgp_bgpclient_h__

#include <stdlib.h>

#include "../Client.h"

#include "components/context/bgp/BgpContext.h"

#define XMI_CLIENT_CLASS XMI::Client::BgpClient

namespace XMI
{
  namespace Client
  {
    class BgpClient : public Client<XMI::Client::BgpClient,XMI::Context::BgpContext>
    {
      public:
        inline BgpClient (char * name) :
          Client<XMI::Client::BgpClient,XMI::Context::BgpContext>(name),
          _client ((xmi_client_t) this),
          _references (1)
        {
        }

        inline ~BgpClient ()
        {
        }

        static XMI::Client::BgpClient * generate_impl (char * name)
        {
          int rc = 0;

          //__client_list->lock();

          // If a client with this name is not already initialized...
          XMI::Client::BgpClient * client = NULL;
          //if ((client = __client_list->contains (name)) == NULL)
          //{
            rc = posix_memalign((void **)&client, 16, sizeof (XMI::Client::BgpClient));
            if (rc != 0) assert(0);
            memset ((void *)client, 0x00, sizeof(XMI::Client::BgpClient));
            new (client) XMI::Client::BgpClient (name);
            //__client_list->pushHead ((QueueElem *) client);
          //}
          //else
          //{
            //client->incReferenceCount ();
          //}

          //__client_list->unlock();

          return client;
        }

        static void destroy_impl (XMI::Client::BgpClient * client)
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

        inline XMI::Context::BgpContext * createContext_impl (xmi_configuration_t configuration[],
                                                       size_t              count)
        {
          //_context_list->lock ();

          XMI::Context::BgpContext * context = NULL;
          int rc = posix_memalign((void **)&context, 16, sizeof (XMI::Context::BgpContext));
          if (rc != 0) assert(0);
          memset ((void *)context, 0x00, sizeof(XMI::Context::BgpContext));
          new (context) XMI::Context::BgpContext (this->getClientId());
          //_context_list->pushHead ((QueueElem *) context);

          //_context_list->unlock ();

          return context;
        }
        
        inline xmi_result_t destroyContext_impl (XMI::Context::BgpContext * context)
        {
          //_context_list->lock ();
          //_context_list->remove (context);
          return context->destroy ();
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

    }; // end class XMI::Client::BgpClient
  }; // end namespace Client
}; // end namespace XMI


#endif // __components_client_bgp_bgpclient_h__
