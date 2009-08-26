///
/// \file xmi/mpi/mpiclient.h
/// \brief XMI client interface specific for the MPI platform.
///
#ifndef   __xmi_mpi_mpiclient_h__
#define   __xmi_mpi_mpiclient_h__

#include <stdlib.h>

#include "components/client/Client.h"

#include "components/context/mpi/mpicontext.h"

#define XMI_CLIENT_CLASS XMI::Client::MPI

namespace XMI
{
  namespace Client
  {
    class MPI : public Client<XMI::Client::MPI,XMI::Context::MPI>
    {
      public:
        inline MPI (char * name) :
          Client<XMI::Client::MPI,XMI::Context::MPI>(name),
          _client ((xmi_client_t) this),
          _references (1)
        {
        }

        inline ~MPI ()
        {
        }

        static XMI::Client::MPI * generate_impl (char * name)
        {
          int rc = 0;

          //__client_list->lock();

          // If a client with this name is not already initialized...
          XMI::Client::MPI * client = NULL;
          //if ((client = __client_list->contains (name)) == NULL)
          //{
          //            rc = posix_memalign((void **)&client, 16, sizeof (XMI::Client::MPI));
          //if (rc != 0) assert(0);
          client = (XMI::Client::MPI *)malloc(sizeof (XMI::Client::MPI));
          assert(client != NULL);
          memset ((void *)client, 0x00, sizeof(XMI::Client::MPI));
          new (client) XMI::Client::MPI (name);
          //__client_list->pushHead ((QueueElem *) client);
          //}
          //else
          //{
            //client->incReferenceCount ();
          //}

          //__client_list->unlock();

          return client;
        }

        static void destroy_impl (XMI::Client::MPI * client)
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

        inline XMI::Context::MPI * createContext_impl (xmi_configuration_t configuration[],
                                                       size_t              count)
        {
          //_context_list->lock ();

          XMI::Context::MPI * context = NULL;
          //int rc = posix_memalign((void **)&context, 16, sizeof (XMI::Context::MPI));
          //if (rc != 0) assert(0);
          context = (XMI::Context::MPI*) malloc(sizeof(XMI::Context::MPI));
          assert(context != NULL);
          memset ((void *)context, 0x00, sizeof(XMI::Context::MPI));
          new (context) XMI::Context::MPI (this->getClientId());
          //_context_list->pushHead ((QueueElem *) context);

          //_context_list->unlock ();

          return context;
        }
        
        inline void destroyContext_impl (XMI::Context::MPI * context)
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

    }; // end class XMI::Client::MPI
  }; // end namespace Client
}; // end namespace XMI


#endif // __xmi_mpi_mpiclient_h__
