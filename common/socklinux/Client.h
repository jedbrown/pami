///
/// \file common/socklinux/Client.h
/// \brief PAMI client interface specific for the Blue Gene\Q platform.
///
#ifndef __common_socklinux_Client_h__
#define __common_socklinux_Client_h__

#include <stdlib.h>

#include "common/ClientInterface.h"
#include "common/default/Client.h"

#include "Context.h"
#include "TypeDefs.h"
#ifndef TRACE_ERR
#define TRACE_ERR(x)  //fprintf x
#endif

namespace PAMI
{
    class Client : public Interface::Client<PAMI::Client>,
                   public Common::Client<PAMI::Client>
    {
      public:

        friend class Interface::Client<PAMI::Client>;

        inline Client (const char * name, pami_client_t client, pami_result_t &result) :
          Interface::Client<PAMI::Client>(name, result),
          Common::Client<PAMI::Client> (name, client, result)
        {
          TRACE_ERR((stderr, ">> Client::Client()\n"));
	  char key[PAMI::Memory::MMKEYSIZE];
	  sprintf(key, "/pami-clt-%s-mm", name);
	  size_t bytes = 1*1024*1024;
	  char *env = getenv("PAMI_CLIENT_SHMEMSIZE");
	  if (env) {
		bytes = strtoull(env, NULL, 0) * 1024 * 1024;
	  }
	  _mm.init(__global.shared_mm, bytes, 16, key);
          TRACE_ERR((stderr, "<< Client::Client()\n"));
        }

        inline ~Client ()
        {
          TRACE_ERR((stderr, ">> Client::~Client()\n"));
          TRACE_ERR((stderr, "<< Client::~Client()\n"));
        }

      protected:

        inline pami_result_t createContext_impl (pami_configuration_t   configuration[],
                                                size_t                count,
                                                pami_context_t       * context,
                                                size_t                ncontexts)
        {
          TRACE_ERR((stderr, ">> Client::createContext_impl()\n"));
        //_context_list->lock ();
        int n = ncontexts;

        if (_ncontexts != 0)
          {
            return PAMI_ERROR;
          }

        if (n > 64)
          {
            return PAMI_INVAL;
          }

        _ncontexts = n;

        pammi_result_t rc = __global.heap_mm->memalign((void **)&_contexts, 16, sizeof(*_contexts) * n);
        PAMI_assertf(rc == PAMI_SUCCESS, "alloc failed for _contexts[%zd]", n);
        int x;

        _platdevs.generate(_clientid, n, _mm);

        // This memset has been removed due to the amount of cycles it takes
        // on simulators.  Lower level initializers should be setting the
        // relevant fields of the context, so this memset should not be
        // needed anyway.
        //memset((void *)_contexts, 0, sizeof(PAMI::Context) * n);
        size_t bytes = (1*1024*1024) / n;
	char *env = getenv("PAMI_CONTEXT_SHMEMSIZE");
	if (env) {
		bytes = strtoull(env, NULL, 0) * 1024 * 1024;
	}

        for (x = 0; x < n; ++x)
          {
            context[x] = (pami_context_t) & _contexts[x];
            new (&_contexts[x]) PAMI::Context(this->getClient(), _clientid, x, n,
                                            &_platdevs, __global.shared_mm, bytes);
            //_context_list->pushHead((QueueElem *)&context[x]);
            //_context_list->unlock();
          }

          TRACE_ERR((stderr, "<< Client::createContext_impl()\n"));
        return PAMI_SUCCESS;
        }

      inline pami_result_t query_impl (pami_configuration_t configuration[],
                                       size_t               num_configs)
        {
          pami_result_t result = PAMI_SUCCESS;
          size_t i;
          for(i=0; i<num_configs; i++)
            {
              switch (configuration[i].name)
                {
                  case PAMI_CLIENT_NUM_CONTEXTS:
                    configuration[i].value.intval = 64; // modified by runmode?
                    break;
                  case PAMI_CLIENT_CONST_CONTEXTS:
                    configuration[i].value.intval = 1; // .TRUE.
                    break;
                  case PAMI_CLIENT_MEM_SIZE:
                  case PAMI_CLIENT_PROCESSOR_NAME:
                  default:
                    result = query_single (configuration[i]);
                    break;
                }
            }
          return result;
        }

    }; // end class PAMI::Client
}; // end namespace PAMI

#undef TRACE_ERR
#endif // __common_socklinux_Client_h__
