///
/// \file components/client/lapi/lapiclient.h
/// \brief XMI client interface specific for the LAPI platform.
///
#ifndef   __xmi_lapi_lapiclient_h__
#define   __xmi_lapi_lapiclient_h__

#include <stdlib.h>

#define XMI_CLIENT_CLASS XMI::Client::LAPI

#include "components/client/Client.h"
#include "components/context/lapiunix/lapiunixcontext.h"
#include "components/geometry/common/commongeometry.h"
#include "util/lapi/lapi_util.h"

namespace XMI
{
  namespace Client
  {
    class LAPI : public Client<XMI::Client::LAPI,XMI::Context::LAPI>
    {
    public:

      inline LAPI (char * name, xmi_result_t & result) :
        Client<XMI::Client::LAPI,XMI::Context::LAPI>(name, result),
        _client ((xmi_client_t) this),
        _references (1),
        _contexts (0)
        {
        }

      inline ~LAPI ()
        {
        }

      static xmi_result_t generate_impl (char * name, xmi_client_t * in_client)
        {
          int rc = 0;
          XMI::Client::LAPI * client;
          client = (XMI::Client::LAPI *)malloc(sizeof (XMI::Client::LAPI));
          assert(client != NULL);
          memset ((void *)client, 0x00, sizeof(XMI::Client::LAPI));
          xmi_result_t res;
          new (client) XMI::Client::LAPI (name, res);
          *in_client = (xmi_client_t*) client;
          return XMI_SUCCESS;
        }

      static void destroy_impl (xmi_client_t client)
        {
          free (client);
        }

      inline char * getName_impl ()
        {
          return '\0';
        }

      inline xmi_context_t createContext_impl (xmi_configuration_t configuration[],
                                               size_t              count,
                                               xmi_result_t       & result)
        {
          XMI::Context::LAPI * context = NULL;
          context = (XMI::Context::LAPI*) malloc(sizeof(XMI::Context::LAPI));
          assert(context != NULL);
          memset ((void *)context, 0x00, sizeof(XMI::Context::LAPI));
          new (context) XMI::Context::LAPI (this->getClient(), _contexts++);
          result = XMI_SUCCESS;
          return context;
        }

      inline xmi_result_t destroyContext_impl (xmi_context_t context)
        {
          free(context);
          return XMI_SUCCESS;
        }

    protected:

      inline xmi_client_t getClient () const
        {
          return _client;
        }

    private:
      xmi_client_t _client;
      size_t       _references;
      size_t       _contexts;
    }; // end class XMI::Client::LAPI
  }; // end namespace Client
}; // end namespace XMI


#endif // __xmi_lapi_lapiclient_h__
