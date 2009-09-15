///
/// \file components/client/bgq/BgqClient.h
/// \brief XMI client interface specific for the Blue Gene\Q platform.
///
#ifndef   __components_client_bgq_bgqclient_h__
#define   __components_client_bgq_bgqclient_h__

#define XMI_CLIENT_CLASS XMI::Client::BgqClient

#include <stdlib.h>

#include "../Client.h"

#include "components/context/bgq/BgqContext.h"

namespace XMI
{
  namespace Client
  {
    class BgqClient : public Client<XMI::Client::BgqClient,XMI::Context::BgqContext>
    {
      public:
        inline BgqClient (char * name, xmi_result_t &result) :
          Client<XMI::Client::BgqClient,XMI::Context::BgqContext>(name, result),
          _client ((xmi_client_t) this),
          _references (1)
        {
          // Set the client name string.
          memset ((void *)_name, 0x00, sizeof(_name));
          strncpy (_name, name, sizeof(_name) - 1);

          result = XMI_SUCCESS;
        }

        inline ~BgqClient ()
        {
        }

        static xmi_result_t generate_impl (char * name, xmi_client_t * client)
        {
          xmi_result_t result;
          int rc = 0;

          //__client_list->lock();

          // If a client with this name is not already initialized...
          XMI::Client::BgqClient * clientp = NULL;
          //if ((client = __client_list->contains (name)) == NULL)
          //{
            rc = posix_memalign((void **)&clientp, 16, sizeof (XMI::Client::BgqClient));
            if (rc != 0) assert(0);
            memset ((void *)clientp, 0x00, sizeof(XMI::Client::BgqClient));
            new (clientp) XMI::Client::BgqClient (name, result);
            *client = clientp;
            //__client_list->pushHead ((QueueElem *) client);
          //}
          //else
          //{
            //client->incReferenceCount ();
          //}

          //__client_list->unlock();

          return result;
        }

        static void destroy_impl (xmi_client_t client)
        {
          //__client_list->lock ();
          //client->decReferenceCount ();
          //if (client->getReferenceCount () == 0)
          //{
            //__client_list->remove (client);
            free ((void *) client);
          //}
          //__client_list->unlock ();
        }

        inline char * getName_impl ()
        {
          return _name;
        }

        inline xmi_context_t createContext_impl (xmi_configuration_t   configuration[],
                                                 size_t                count,
                                                 xmi_result_t        & result)
        {
          //_context_list->lock ();

          XMI::Context::BgqContext * context = NULL;
          int rc = posix_memalign((void **)&context, 16, sizeof (XMI::Context::BgqContext));
          if (rc != 0) assert(0);
          memset ((void *)context, 0x00, sizeof(XMI::Context::BgqContext));
          new (context) XMI::Context::BgqContext (this->getClientId());
          //_context_list->pushHead ((QueueElem *) context);

          //_context_list->unlock ();

          result = XMI_SUCCESS;
          return (xmi_context_t) context;
        }
        
        inline xmi_result_t destroyContext_impl (xmi_context_t context)
        {
          //_context_list->lock ();
          //_context_list->remove (context);
          return ((XMI::Context::BgqContext *)context)->destroy ();
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
        char         _name[256];

    }; // end class XMI::Client::BgqClient
  }; // end namespace Client
}; // end namespace XMI


#endif // __components_client_bgq_bgqclient_h__
