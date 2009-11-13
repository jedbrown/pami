///
/// \file common/lapiunix/Client.h
/// \brief XMI client interface specific for the LAPI platform.
///
#ifndef __common_lapiunix_Client_h__
#define __common_lapiunix_Client_h__

#include <stdlib.h>

#include "common/ClientInterface.h"
#include "Context.h"
#include "Geometry.h"
#include "util/lapi/lapi_util.h"

namespace XMI
{
    class Client : public Interface::Client<XMI::Client,XMI::Context>
    {
    public:

      inline Client (const char * name, xmi_result_t & result) :
        Interface::Client<XMI::Client,XMI::Context>(name, result),
        _client ((xmi_client_t) this),
        _references (1),
        _ncontexts (0)
        {
        }

      inline ~Client ()
        {
        }

      static xmi_result_t generate_impl (const char * name, xmi_client_t * in_client)
        {
          int rc = 0;
          XMI::Client * client;
          client = (XMI::Client *)malloc(sizeof (XMI::Client));
          assert(client != NULL);
          memset ((void *)client, 0x00, sizeof(XMI::Client));
          xmi_result_t res;
          new (client) XMI::Client (name, res);
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

      inline xmi_result_t createContext_impl (xmi_configuration_t configuration[],
                                               size_t              count,
                                               xmi_context_t *contexts,
						int *ncontexts)
        {
		//_context_list->lock ();
		int n = *ncontexts;
		if (_ncontexts != 0) {
			*ncontexts = 0;
			return XMI_ERROR;
		}
		if (_ncontexts + n > 4) {
			n = 4 - _ncontexts;
		}
		*ncontexts = n;
		if (n <= 0) { // impossible?
			return XMI_ERROR;
		}
		_generics = (XMI::Device::Generic::Device *)malloc(sizeof(*_generics) * n);
		XMI_assertf(_generics!=0, "malloc failed for _generics[%d], errno=%d\n", n, errno);
		_contexts = (XMI::Context *)malloc(sizeof(XMI::Context) * n);
		XMI_assertf(_contexts!=0, "malloc failed for _contexts[%d], errno=%d\n", n, errno);
		memset((void *)_contexts, 0, sizeof(XMI::Context) * n);
		// size_t bytes = _mm.size() / n;
		int x;
		for (x = 0; x < n; ++x) {
			contexts[x] = &_contexts[x];
			// void *base = NULL;
			// _mm.memalign((void **)&base, 16, bytes);
			// XMI_assertf(base != NULL, "out of sharedmemory in context create\n");
			// new (&context[x]) XMI::Context(this->getClient(), _contexts++, base, bytes);
			new (&_contexts[x]) XMI::Context(this->getClient(), x, n, _generics);
			//_context_list->pushHead((QueueElem *)&context[x]);
			//_context_list->unlock();
		}
		return XMI_SUCCESS;
        }

      inline xmi_result_t destroyContext_impl (xmi_context_t context)
        {
          free(context);
          return XMI_SUCCESS;
        }

	inline size_t getNumContexts()
	{
		return _ncontexts;
	}
	inline XMI::Context *getContexts()
	{
		return _contexts;
	}

    protected:

      inline xmi_client_t getClient () const
        {
          return _client;
        }

    private:
      xmi_client_t _client;
      size_t       _references;
      size_t       _ncontexts;
	XMI::Context *_contexts;
	XMI::Device::Generic::Device *_generics;
    }; // end class XMI::Client
}; // end namespace XMI


#endif // __xmi_lapi_lapiclient_h__
