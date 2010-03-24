///
/// \file common/bgp/Client.h
/// \brief PAMI client interface specific for the BGP platform.
///
#ifndef __common_bgp_Client_h__
#define __common_bgp_Client_h__

#include <stdlib.h>

#include "common/ClientInterface.h"

#include "Global.h"
#include "Context.h"

#include "components/memory/MemoryManager.h"

namespace PAMI
{
  class Client : public Interface::Client<PAMI::Client>
  {
    public:
      inline Client (const char * name, pami_result_t &result) :
          Interface::Client<PAMI::Client>(name, result),
          _client ((pami_client_t) this),
          _references (1),
          _ncontexts (0),
          _mm ()
      {
        static size_t next_client_id = 0;
        // Set the client name string.
        memset ((void *)_name, 0x00, sizeof(_name));
        strncpy (_name, name, sizeof(_name) - 1);

        _clientid = next_client_id++;
        // assert(_clientid < PAMI_MAX_NUM_CLIENTS);

        // Get some shared memory for this client
        initializeMemoryManager ();

        result = PAMI_SUCCESS;
      }

      inline ~Client ()
      {
      }

      static pami_result_t generate_impl (const char * name, pami_client_t * client)
      {
        pami_result_t result;
        int rc = 0;

        //__client_list->lock();

        // If a client with this name is not already initialized...
        PAMI::Client * clientp = NULL;
        //if ((client = __client_list->contains (name)) == NULL)
        //{
        rc = posix_memalign((void **) & clientp, 16, sizeof (PAMI::Client));

        if (rc != 0) assert(0);

        memset ((void *)clientp, 0x00, sizeof(PAMI::Client));
        new (clientp) PAMI::Client (name, result);
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

      static void destroy_impl (pami_client_t client)
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

      inline pami_result_t createContext_impl (pami_configuration_t   configuration[],
                                              size_t                count,
                                              pami_context_t       * context,
                                              size_t                ncontexts)
      {
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
            void *base = NULL;
            _mm.enable();
            _mm.memalign((void **)&base, 16, bytes);
            _mm.disable();
            PAMI_assertf(base != NULL, "out of sharedmemory in context create\n");
            new (&_contexts[x]) PAMI::Context(this->getClient(), _clientid, x, n,
                                             &_platdevs, base, bytes);
            //_context_list->pushHead((QueueElem *)&context[x]);
            //_context_list->unlock();
          }

        return PAMI_SUCCESS;
      }

      inline pami_result_t destroyContext_impl (pami_context_t context)
      {
        //_context_list->lock ();
        //_context_list->remove (context);
        return ((PAMI::Context *)context)->destroy ();
        //_context_list->unlock ();
      }

      inline pami_result_t queryConfiguration_impl (pami_configuration_t * configuration)
      {
        pami_result_t result = PAMI_ERROR;

        switch (configuration->name)
          {
            case PAMI_NUM_CONTEXTS:
              configuration->value.intval = 4; // modified by runmode?
              result = PAMI_SUCCESS;
              break;
            case PAMI_CONST_CONTEXTS:
              configuration->value.intval = 1; // .TRUE.
              result = PAMI_SUCCESS;
              break;
            case PAMI_TASK_ID:
              configuration->value.intval = __global.mapping.task();
              result = PAMI_SUCCESS;
              break;
            case PAMI_NUM_TASKS:
              configuration->value.intval = __global.mapping.size();
              result = PAMI_SUCCESS;
              break;
            case PAMI_CLOCK_MHZ:
            case PAMI_WTIMEBASE_MHZ:
              configuration->value.intval = __global.time.clockMHz();
              result = PAMI_SUCCESS;
              break;
            case PAMI_WTICK:
              configuration->value.doubleval = __global.time.tick();
              result = PAMI_SUCCESS;
              break;
            case PAMI_PROCESSOR_NAME:
              {
                int rc;
                char* pn = __global.processor_name;
                rc = snprintf(pn, 128, "Task %u of %u", __global.mapping.task(), __global.mapping.size());
                pn[128-1] = 0;
                configuration->value.chararray = pn;
                if (rc>0)
                  result = PAMI_SUCCESS;
              }
              break;
            case PAMI_MEM_SIZE:
            default:
              break;
          }

        return result;
      }

      // the friend clause is actually global, but this helps us remember why...
      //friend class PAMI::Device::Generic::Device;
      //friend class pami.cc

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

    inline pami_result_t geometry_create_taskrange_impl(pami_geometry_t       * geometry,
                                                       pami_geometry_t         parent,
                                                       unsigned               id,
                                                       pami_geometry_range_t * rank_slices,
                                                       size_t                 slice_count,
                                                       pami_context_t          context,
                                                       pami_event_function     fn,
                                                       void                 * cookie)
      {
        PAMI_abort();
        return PAMI_SUCCESS;
      }


    inline pami_result_t geometry_create_tasklist_impl(pami_geometry_t       * geometry,
                                                      pami_geometry_t         parent,
                                                      unsigned               id,
                                                      pami_task_t           * tasks,
                                                      size_t                 task_count,
                                                      pami_context_t          context,
                                                      pami_event_function     fn,
                                                      void                 * cookie)
      {
        // todo:  implement this routine
        PAMI_abort();
        return PAMI_SUCCESS;
      }


    inline pami_result_t geometry_destroy_impl (pami_geometry_t geometry)
      {
        PAMI_abort();
        return PAMI_UNIMPL;
      }

    protected:

      inline pami_client_t getClient () const
      {
        return _client;
      }

    private:

      pami_client_t _client;
      size_t _clientid;

      size_t       _references;
      size_t       _ncontexts;
      PAMI::Context *_contexts;
      PAMI::PlatformDeviceList _platdevs;

      char         _name[256];

      Memory::MemoryManager _mm;

      inline void initializeMemoryManager ()
      {
        char   shmemfile[1024];
        size_t bytes     = 1024 * 1024;
        //size_t pagesize  = 4096;

        snprintf (shmemfile, 1023, "/pami-client-%s", _name);

        // Round up to the page size
        //size_t size = (bytes + pagesize - 1) & ~(pagesize - 1);

        int fd, rc;
        size_t n = bytes;

        // CAUTION! The following sequence MUST ensure that "rc" is "-1" iff failure.
        rc = shm_open (shmemfile, O_CREAT | O_RDWR, 0600);

        if ( rc != -1 )
          {
            fd = rc;
            rc = ftruncate( fd, n );

            if ( rc != -1 )
              {
                void * ptr = mmap( NULL, n, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

                if ( ptr != MAP_FAILED )
                  {
                    _mm.init (ptr, n);
                    return;
                  }
              }
          }

        // Failed to create shared memory .. fake it using the heap ??
        _mm.init (malloc (n), n);

        return;
      }
  }; // end class PAMI::Client
}; // end namespace PAMI
#endif // __components_client_bgp_bgpclient_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
