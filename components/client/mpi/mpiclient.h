///
/// \file xmi/mpi/mpiclient.h
/// \brief XMI client interface specific for the MPI platform.
///
#ifndef   __xmi_mpi_mpiclient_h__
#define   __xmi_mpi_mpiclient_h__

#include <stdlib.h>

#define XMI_CLIENT_CLASS XMI::Client::MPI

#include "components/client/Client.h"
#include "components/context/mpi/mpicontext.h"
#include "components/geometry/common/commongeometry.h"

namespace XMI
{
  namespace Client
  {

    
    class MPI : public Client<XMI::Client::MPI,XMI::Context::MPI>
    {
    public:

      static void shutdownfunc()
        {
          MPI_Finalize();
        }
      inline MPI (char * name, xmi_result_t & result) :
        Client<XMI::Client::MPI,XMI::Context::MPI>(name, result),
        _client ((xmi_client_t) this),
        _references (1)
        {
          static int initialized = 0;
          if(initialized==0)
              {
                int rc = MPI_Init(0, NULL);
                if(rc != MPI_SUCCESS)
                    {
                      fprintf(stderr, "Unable to initialize context:  MPI_Init failure\n");
                      XMI_abort();
                    }
                initialized=1;
                atexit(shutdownfunc);
              }

        }

      inline ~MPI ()
        {
        }
      
      static xmi_result_t generate_impl (char * name, xmi_client_t * in_client)
        {
          int rc = 0;
          XMI::Client::MPI * client;
          client = (XMI::Client::MPI *)malloc(sizeof (XMI::Client::MPI));
          assert(client != NULL);
          memset ((void *)client, 0x00, sizeof(XMI::Client::MPI));
          xmi_result_t res;
          new (client) XMI::Client::MPI (name, res);
          *in_client = (xmi_client_t*) client;
          return XMI_SUCCESS;
        }

      static void destroy_impl (xmi_client_t client)
        {
          free (client);
        }

      inline char * getName_impl ()
        {
          return "";
        }

      inline xmi_context_t createContext_impl (xmi_configuration_t configuration[],
                                               size_t              count,
                                               xmi_result_t       & result)
        {
          XMI::Context::MPI * context = NULL;
          context = (XMI::Context::MPI*) malloc(sizeof(XMI::Context::MPI));
          assert(context != NULL);
          memset ((void *)context, 0x00, sizeof(XMI::Context::MPI));
          new (context) XMI::Context::MPI (this->getClientId());
          result = XMI_SUCCESS;
          return context;
        }
        
      inline xmi_result_t destroyContext_impl (xmi_context_t context)
        {
          free(context);
          return XMI_SUCCESS;
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
