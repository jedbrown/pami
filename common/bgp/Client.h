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

#undef TRACE_ERR
#define TRACE_ERR(x) //fprintf x

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
          _world_geometry((BGPGeometry*)_world_geometry_storage),
          _mm ()
      {
        TRACE_ERR((stderr,  "%s enter\n", __PRETTY_FUNCTION__));
        static size_t next_client_id = 0;
        // Set the client name string.
        memset ((void *)_name, 0x00, sizeof(_name));
        strncpy (_name, name, sizeof(_name) - 1);

        _clientid = next_client_id++;
        // PAMI_assert(_clientid < PAMI_MAX_NUM_CLIENTS);

        // Get some shared memory for this client
        initializeMemoryManager ();

        _world_range.lo=0;
        _world_range.hi=__global.mapping.size()-1;
        new(_world_geometry_storage) BGPGeometry(NULL, &__global.mapping,0, 1,&_world_range);

        result = PAMI_SUCCESS;
      }

      inline ~Client ()
      {
      }

      static pami_result_t generate_impl (const char * name, pami_client_t * client,
                                          pami_configuration_t  configuration[],
                                          size_t                num_configs)
      {
        TRACE_ERR((stderr,  "%s enter\n", __PRETTY_FUNCTION__));
        pami_result_t result;
        int rc = 0;

        //__client_list->lock();

        // If a client with this name is not already initialized...
        PAMI::Client * clientp = NULL;
        //if ((client = __client_list->contains (name)) == NULL)
        //{
        rc = posix_memalign((void **) & clientp, 16, sizeof (PAMI::Client));

        if (rc != 0) PAMI_abort();

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
        TRACE_ERR((stderr,  "%s enter\n", __PRETTY_FUNCTION__));
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
        TRACE_ERR((stderr, "BGP::Client::createContext mm available %zu\n", _mm.available()));
        _platdevs.generate(_clientid, n, _mm);

        // This memset has been removed due to the amount of cycles it takes
        // on simulators.  Lower level initializers should be setting the
        // relevant fields of the context, so this memset should not be
        // needed anyway.
        //memset((void *)_contexts, 0, sizeof(PAMI::Context) * n);
        size_t bytes = _mm.available() / n - 16;
        TRACE_ERR((stderr, "BGP::Client::createContext mm available %zu, bytes %zu\n", _mm.available(), bytes));

        for (x = 0; x < n; ++x)
          {
          TRACE_ERR((stderr, "BGP::Client::createContext %u\n", x));
            context[x] = (pami_context_t) & _contexts[x];
            void *base = NULL;
            _mm.enable();
            _mm.memalign((void **)&base, 16, bytes);
            _mm.disable();
            PAMI_assertf(base != NULL, "out of sharedmemory in context create x=%d,n=%d,bytes=%zu,mm.size=%zu,mm.available=%zu\n", x, n, bytes, _mm.size(), _mm.available());
            new (&_contexts[x]) PAMI::Context(this->getClient(), _clientid, x, n,
                                             &_platdevs, base, bytes, _world_geometry);
            //_context_list->pushHead((QueueElem *)&context[x]);
            //_context_list->unlock();
          }
        _ncontexts = (size_t)n;
        TRACE_ERR((stderr,  "%s ncontexts %zu exit\n", __PRETTY_FUNCTION__,_ncontexts));

        return PAMI_SUCCESS;
      }

      // DEPRECATED!
      inline pami_result_t destroyContext_impl (pami_context_t context)
      {
        return ((PAMI::Context *)context)->destroy ();
      }

      inline pami_result_t destroyContext_impl (pami_context_t *context, size_t ncontexts)
      {
	PAMI_assertf(ncontexts == _ncontexts, "destroyContext called without all contexts");
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
                  configuration[i].value.intval = 4; // modified by runmode?
                  break;
                case PAMI_CLIENT_CONST_CONTEXTS:
                  configuration[i].value.intval = 1; // .TRUE.
                  break;
                case PAMI_CLIENT_TASK_ID:
                  configuration[i].value.intval = __global.mapping.task();
                  break;
                case PAMI_CLIENT_NUM_TASKS:
                  configuration[i].value.intval = __global.mapping.size();
                  break;
                case PAMI_CLIENT_CLOCK_MHZ:
                case PAMI_CLIENT_WTIMEBASE_MHZ:
                  configuration[i].value.intval = __global.time.clockMHz();
                  break;
                case PAMI_CLIENT_WTICK:
                  configuration[i].value.doubleval = __global.time.tick();
                  break;
                case PAMI_CLIENT_PROCESSOR_NAME:
                {
                  int rc;
                  char* pn = __global.processor_name;
                  rc = snprintf(pn, 128, "Task %u of %u", __global.mapping.task(), __global.mapping.size());
                  pn[128-1] = 0;
                  configuration[i].value.chararray = pn;
                  if (rc>0)
                    result = PAMI_INVAL;
                }
                break;
                case PAMI_CLIENT_MEM_SIZE:
                  // #warning todo:  these should be dispatch scoped
                case PAMI_DISPATCH_SEND_IMMEDIATE_MAX:
                case PAMI_DISPATCH_RECV_IMMEDIATE_MAX:
                default:
                  result = PAMI_INVAL;
              }
          }
        return result;
      }


    inline pami_result_t update_impl (pami_configuration_t configuration[],
                                      size_t               num_configs)
      {
        return PAMI_INVAL;
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
        *world_geometry = _world_geometry;
        return PAMI_SUCCESS;
      }

    inline pami_result_t geometry_create_taskrange_impl(pami_geometry_t       * geometry,
                                                        pami_configuration_t   configuration[],
                                                        size_t                 num_configs,
                                                       pami_geometry_t         parent,
                                                       unsigned               id,
                                                       pami_geometry_range_t * rank_slices,
                                                       size_t                 slice_count,
                                                       pami_context_t          context,
                                                       pami_event_function     fn,
                                                       void                 * cookie)
      {
        TRACE_ERR((stderr,  "%s enter\n", __PRETTY_FUNCTION__));

        BGPGeometry              *new_geometry;

        if(geometry != NULL)
        {
          new_geometry=(BGPGeometry*) malloc(sizeof(*new_geometry)); /// \todo use allocator
          new(new_geometry) BGPGeometry((PAMI::Geometry::Common*)parent,
                                    &__global.mapping,
                                    id,
                                    slice_count,
                                            rank_slices);
          for(size_t n=0; n<_ncontexts; n++)
          {
            _contexts[n].analyze(n,(BGPGeometry*)new_geometry);
          }
          *geometry = (pami_geometry_t) new_geometry;
          /// \todo  deliver completion to the appropriate context
	  new_geometry->processUnexpBarrier();
        }
        BGPGeometry *bargeom = (BGPGeometry*)parent;
        PAMI::Context *ctxt = (PAMI::Context *)context;
        bargeom->default_barrier(fn, cookie, ctxt->getId(), context);
        return PAMI_SUCCESS;
      }


    inline pami_result_t geometry_create_tasklist_impl(pami_geometry_t       * geometry,
                                                        pami_configuration_t   configuration[],
                                                        size_t                 num_configs,
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
      BGPGeometry                  *_world_geometry;
      uint8_t                       _world_geometry_storage[sizeof(BGPGeometry)];
      pami_geometry_range_t         _world_range;

      Memory::MemoryManager _mm;

      inline void initializeMemoryManager ()
      {
        TRACE_ERR((stderr,  "%s enter\n", __PRETTY_FUNCTION__));
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

#undef TRACE_ERR

#endif // __components_client_bgp_bgpclient_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
