///
/// \file common/default/Client.h
/// \brief PAMI default client implementation.
///
#ifndef __common_default_Client_h__
#define __common_default_Client_h__

#include <stdlib.h>

#include "Context.h"
#include "components/memory/shmem/SharedMemoryManager.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x)  fprintf x
#endif

namespace PAMI
{
  namespace Common
  {
    template <class T_Client>
    class Client
    {
      public:

        /// \todo Make the number of bytes of client-specific shared memory configurable
        inline Client (const char * name, pami_client_t client, pami_result_t &result) :
            _client (client),
            _references (1),
            _ncontexts (0),
            _mm ()
        {
          TRACE_ERR((stderr, ">> Client::Client(), this = %p\n", this));

          static size_t next_client_id = 0; // move this to a global resource manager...
          _clientid = next_client_id++;

          // Set the client name string.
          memset ((void *)_name, 0x00, sizeof(_name));
          strncpy (_name, name, sizeof(_name) - 1);
	  initializeMemoryManager();

          result = PAMI_SUCCESS;
          TRACE_ERR((stderr, "<< Client::Client()\n"));
        }

        inline ~Client ()
        {
          TRACE_ERR((stderr, ">> Client::~Client()\n"));
          TRACE_ERR((stderr, "<< Client::~Client()\n"));
        }

        static pami_result_t generate_impl (const char           * name,
                                            pami_client_t        * client,
                                            pami_configuration_t   configuration[],
                                            size_t                 num_configs)
        {
          TRACE_ERR((stderr, ">> Client::generate_impl(\"%s\", %p)\n", name, client));

          pami_result_t result = PAMI_ERROR;
          pami_result_t rc;

          // Needs error-checking and locks for thread safety
          T_Client * clientp = NULL;
          rc = __global.heap_mm->memalign((void **) & clientp, 16, sizeof (T_Client));

          if (rc != PAMI_SUCCESS) PAMI_abort();

          new (clientp) T_Client (name, (pami_client_t) clientp, result);
          *client = clientp;

          TRACE_ERR((stderr, "<< Client::generate_impl(\"%s\", %p), *client = %p, result = %d\n", name, client, *client, result));

          return result;
        }

        static void destroy_impl (pami_client_t client)
        {
          // Needs error-checking and locks for thread safety
          T_Client * c = (T_Client *) client;
          c->~Client();
          free ((void *) client);
        }

        inline char * getName_impl ()
        {
          return _name;
        }
        /*
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

                if (_ncontexts + n > 4)
                  {
                    n = 4 - _ncontexts;
                  }

                if (n <= 0)   // impossible?
                  {
                    return PAMI_ERROR;
                  }

                int rc = posix_memalign((void **) & _contexts, 16, sizeof(*_contexts) * n);
                PAMI_assertf(rc == 0, "posix_memalign failed for _contexts[%d], errno=%d\n", n, errno);
                int x;

                _platdevs.generate(_clientid, n, _mm);

                // This memset has been removed due to the amount of cycles it takes
                // on simulators.  Lower level initializers should be setting the
                // relevant fields of the context, so this memset should not be
                // needed anyway.
                //memset((void *)_contexts, 0, sizeof(PAMI::Context) * n);
                size_t bytes = _mm.available() / n - 16;

                for (x = 0; x < n; ++x)
                  {
                    context[x] = (pami_context_t) & _contexts[x];
                    new (&_contexts[x]) PAMI::Context(this->getClient(), _clientid, x, n,
                                                     &_platdevs, &_mm, bytes);
                    //_context_list->pushHead((QueueElem *)&context[x]);
                    //_context_list->unlock();
                  }

                  TRACE_ERR((stderr, "<< Client::createContext_impl()\n"));
                return PAMI_SUCCESS;
                }
        */

        inline pami_result_t destroyContext_impl (pami_context_t *context, size_t ncontexts)
        {
          PAMI_assertf(ncontexts == _ncontexts, "destroyContext called without all contexts (%zu != %zu)", _ncontexts, ncontexts);
          pami_result_t res = PAMI_SUCCESS;
          size_t i;

          for (i = 0; i < _ncontexts; ++i)
            {
              context[i] = NULL;
              PAMI::Context * ctx = &_contexts[i];
              pami_result_t rc = ctx->destroy ();

              if (rc != PAMI_SUCCESS) res = rc;
            }

          free(_contexts);
          _contexts = NULL;
          _ncontexts = 0;
          return res;
        }

        inline pami_result_t query_single (pami_configuration_t & configuration)
        {
          pami_result_t result = PAMI_SUCCESS;

          switch (configuration.name)
            {
              case PAMI_CLIENT_TASK_ID:
                configuration.value.intval = __global.mapping.task();
                break;
              case PAMI_CLIENT_NUM_TASKS:
                configuration.value.intval = __global.mapping.size();
                break;
              case PAMI_CLIENT_CLOCK_MHZ:
              case PAMI_CLIENT_WTIMEBASE_MHZ:
                configuration.value.intval = __global.time.clockMHz();
                break;
              case PAMI_CLIENT_WTICK:
                configuration.value.doubleval = __global.time.tick();
                break;
                //case PAMI_CLIENT_MEM_SIZE:
                //case PAMI_CLIENT_PROCESSOR_NAME:
              default:
                result = PAMI_INVAL;
            }

          return result;
        }

        inline pami_result_t query_impl (pami_configuration_t configuration[],
                                         size_t               num_configs)
        {
          pami_result_t result = PAMI_SUCCESS;
          size_t i;

          for (i = 0; i < num_configs; i++)
            {
              if (query_single(configuration[i]) == PAMI_INVAL)
                result = PAMI_INVAL;
            }

          return result;
        }

        inline pami_result_t update_impl (pami_configuration_t configuration[],
                                          size_t               num_configs)
        {
          return PAMI_INVAL;
        }
        inline size_t getNumContexts()
        {
          return _ncontexts;
        }

        inline PAMI::Context *getContext(size_t ctx)
        {
          return _contexts + ctx;
        }

        inline PAMI::Context *getContexts()
        {
          return _contexts;
        }

        inline size_t getClientId()
        {
          return _clientid;
        }

        inline pami_result_t geometry_world_impl (pami_geometry_t * world_geometry)
        {
          PAMI_abort();
          return PAMI_SUCCESS;
        }

        inline pami_result_t geometry_create_topology_impl(pami_geometry_t       *geometry,
                                                           pami_configuration_t   configuration[],
                                                           size_t                 num_configs,
                                                           pami_geometry_t        parent,
                                                           unsigned               id,
                                                           pami_topology_t       *topology,
                                                           pami_context_t         context,
                                                           pami_event_function    fn,
                                                           void                  *cookie)
        {
          PAMI_abort();
          return PAMI_SUCCESS;
        }

        inline pami_result_t geometry_create_taskrange_impl(pami_geometry_t       *geometry,
                                                            pami_configuration_t   configuration[],
                                                            size_t                 num_configs,
                                                            pami_geometry_t        parent,
                                                            unsigned               id,
                                                            pami_geometry_range_t *rank_slices,
                                                            size_t                 slice_count,
                                                            pami_context_t         context,
                                                            pami_event_function    fn,
                                                            void                  *cookie)
        {
          PAMI_abort();
          return PAMI_SUCCESS;
        }


        inline pami_result_t geometry_create_tasklist_impl(pami_geometry_t       *geometry,
                                                           pami_configuration_t   configuration[],
                                                           size_t                 num_configs,
                                                           pami_geometry_t        parent,
                                                           unsigned               id,
                                                           pami_task_t           *tasks,
                                                           size_t                 task_count,
                                                           pami_context_t         context,
                                                           pami_event_function    fn,
                                                           void                  *cookie)
        {
          // todo:  implement this routine
          PAMI_abort();
          return PAMI_SUCCESS;
        }

        inline pami_result_t geometry_query_impl (pami_geometry_t        geometry,
                                                  pami_configuration_t   configuration[],
                                                  size_t                 num_configs)
        {
          return PAMI_UNIMPL;
        }

        inline pami_result_t geometry_update_impl (pami_geometry_t        geometry,
                                                   pami_configuration_t   configuration[],
                                                   size_t                 num_configs,
                                                   pami_context_t         context,
                                                   pami_event_function    fn,
                                                   void                 * cookie)
        {
          return PAMI_UNIMPL;
        }

        inline pami_result_t geometry_destroy_impl (pami_geometry_t geometry)
        {
          PAMI_abort();
          return PAMI_UNIMPL;
        }

        inline pami_client_t getClient () const
        {
          return _client;
        }


        pami_client_t   _client;
        size_t          _clientid;
        size_t          _references;
        size_t          _ncontexts;
        PAMI::Context * _contexts;
        PlatformDeviceList _platdevs;
        char            _name[256];

        Memory::SharedMemoryManager _mm;

        inline void initializeMemoryManager ()
        {
          size_t bytes     = 1024 * 1024;
          size_t pagesize  = 4096;

          char * jobstr = getenv ("PAMI_JOB_ID");
	  char shmemfile[PAMI::Memory::MemoryManager::MMKEYSIZE];

          if (jobstr)
            snprintf (shmemfile, sizeof(shmemfile) - 1, "/pami-client-%s-%s", _name, jobstr);
          else
            snprintf (shmemfile, sizeof(shmemfile) - 1, "/pami-client-%s", _name);

          // Round up to the page size
          size_t size = (bytes + pagesize - 1) & ~(pagesize - 1);

          _mm.init(__global.shared_mm, size, 1, 0, shmemfile);

          return;
        }

    }; // end class PAMI::Common::Client
  }; // end namespace PAMI::Common
}; // end namespace PAMI

#undef TRACE_ERR
#endif // __common_default_Client_h__
