///
/// \file common/lapiunix/Client.h
/// \brief PAMI client interface specific for the LAPI platform.
///
#ifndef __common_lapiunix_Client_h__
#define __common_lapiunix_Client_h__

#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#include "common/ClientInterface.h"
#include "common/lapiunix/Context.h"
#include "algorithms/geometry/Geometry.h"
#include "components/lapi/include/Client.h"

namespace PAMI
{
  class Client : public Interface::Client<PAMI::Client>
  {
  public:
    inline Client (const char * name, pami_result_t & result) :
      Interface::Client<PAMI::Client>(name, result),
      _lapiClient(NULL),
      _client ((pami_client_t) this),
      _maxctxts(0),
      _ncontexts (0),
      _mm ()
      {
        pami_result_t   rc               = PAMI_SUCCESS;
        static size_t   next_client_id   = 0;
        size_t          myrank=0, mysize = 0;
        _clientid                        = next_client_id++;

        // Todo:  Support more contexts
        _maxctxts = 1;

        // Get storage and allocate a LAPI Client Object
        _lapiClient                    = (LapiImpl::Client*) _lapiClientAlloc.allocateObject();
        new(_lapiClient) LapiImpl::Client(name);

        // Initialize the shared memory manager
        initializeMemoryManager ();

        // Generate the Device queues
        _platdevs.generate(_clientid, _maxctxts, _mm);

        // Create an initial context for initialization
        rc = createOneContext(&_contexts[0],0);
        if(rc) {result=rc;  return;}
        // Initialize Point to Point Communication
        // This is used to build an initial "simple" mapping
        // Using rank and size
        rc = _contexts[0]->initP2P(&myrank, &mysize, &_main_lapi_handle);
        if(rc) {result=rc;  return;}

        // Initialize the mapping to be used for collectives
        __global.mapping.init(myrank, mysize);

        // Initialize world geometry object, and set the geometry ptr for the context
        _world_geometry = (LAPIGeometry*)_geometryAlloc.allocateObject();
        _world_range.lo = 0;
        _world_range.hi = mysize-1;
        new(_world_geometry) LAPIGeometry(NULL, &__global.mapping,0, 1,&_world_range);
        _contexts[0]->setWorldGeometry(_world_geometry);

        // Initialize "Safe" Collectives, which will be used
        // To build the mappings and topologies for the optimized
        // collectives and to set up token management for collectives
        _contexts[0]->initP2PCollectives();

        // #warning:  TODO, here is where we want to have a more
        // sophisticated mapping, use P2P collectives to
        // rebuild a better mapping for optimized collectives

        // --> Ok to do collectives after here, build better mappings
        // -->

        // Initialize the optimized collectives
        _contexts[0]->initCollectives();

        // Fence the create
        // Todo:  remove this, does it violate the API?
        CheckLapiRC(lapi_gfence (_main_lapi_handle));

        // Return error code
        result                         = rc;
      }

    inline ~Client ()
      {
        _lapiClientAlloc.returnObject(_lapiClient);
      }

    static pami_result_t generate_impl (const char * name, pami_client_t * client)
      {
        int rc = 0;
        PAMI::Client * clientp;
        clientp = (PAMI::Client *)malloc(sizeof (PAMI::Client));
        assert(clientp != NULL);
        memset ((void *)clientp, 0x00, sizeof(PAMI::Client));
        pami_result_t res;
        new (clientp) PAMI::Client (name, res);
        *client = (pami_client_t) clientp;
        return res;
      }

    static void destroy_impl (pami_client_t client)
      {
        Client *c = (Client *) client;
        c->~Client();
        free (client);
      }

    inline char * getName_impl ()
      {
        return _lapiClient->GetName();
      }


    inline pami_result_t createOneContext(PAMI::Context **ctxt,
                                          int             index)
      {
        PAMI::Context *c = (PAMI::Context *) _contextAlloc.allocateObject();
        new (c) PAMI::Context(this->getClient(),      /* Client ptr       */
                              _clientid,              /* Client  id       */
                              _lapiClient->GetName(), /* Client String    */
                              index,                  /* Context id       */
                              &_platdevs,             /* Platform Devices */
                              &_mm);                  /* Memory Manager   */
        *ctxt = c;
        _ncontexts++;
        return PAMI_SUCCESS;
      }



    inline pami_result_t createContext_impl (pami_configuration_t configuration[],
                                             size_t               count,
                                             pami_context_t      *contexts,
                                             size_t               ncontexts)
      {
        // Todo:  Support multiple contexts (more than 1)
        if(ncontexts > _maxctxts)
          return PAMI_ERROR;

        // Set up return for first context
        if(ncontexts > 0)
          contexts[0] = (pami_context_t) _contexts[0];

        // Todo:  Configure Contexts
        // This should work, but crosstalk isn't enabled yet for lapi
        for(int i=1; i< ncontexts; i++)
            {
              pami_result_t rc;
              lapi_handle_t t_lhandle;
              size_t        t_myrank, t_mysize;
              contexts[i] = (pami_context_t*)_contexts[i];
              rc = createOneContext(&_contexts[i],i);
              rc = _contexts[i]->initP2P(&t_myrank, &t_mysize, &t_lhandle);
              if(rc) RETURN_ERR_PAMI(PAMI_ERROR, "createContext failed with rc %d\n", rc);
              _contexts[i]->setWorldGeometry(_world_geometry);
              _contexts[i]->initP2PCollectives();
              _contexts[i]->initCollectives();
            }
        return PAMI_SUCCESS;
      }

    inline pami_result_t destroyContext_impl (pami_context_t context)
      {
        PAMI::Context *c = (PAMI::Context*) context;
        pami_result_t rc = PAMI_SUCCESS;
        rc               = c->destroy();
        _contextAlloc.returnObject(context);
        return rc;
      }

    inline pami_result_t queryConfiguration_impl (pami_configuration_t * configuration)
      {
        pami_result_t result = PAMI_ERROR;
        switch (configuration->name)
            {
                case PAMI_NUM_CONTEXTS:
                  configuration->value.intval = 1; // real value TBD
                  result = PAMI_SUCCESS;
                  break;
                case PAMI_CONST_CONTEXTS:
                  configuration->value.intval = 0; // real value TBD
                  result = PAMI_SUCCESS;
                  break;
#if 0
                // These are passed to LAPI for now
                case PAMI_TASK_ID:
                  configuration->value.intval = __global.mapping.task();
                  result = PAMI_SUCCESS;
                  break;
                case PAMI_NUM_TASKS:
                  configuration->value.intval = __global.mapping.size();
                  result = PAMI_SUCCESS;
                  break;
#endif                  
                case PAMI_CLOCK_MHZ:
                case PAMI_WTIMEBASE_MHZ:
                  configuration->value.intval = __global.time.clockMHz();
                  result = PAMI_SUCCESS;
                  break;
                case PAMI_WTICK:
                  configuration->value.doubleval =__global.time.tick();
                  result = PAMI_SUCCESS;
                  break;
                case PAMI_MEM_SIZE:
                case PAMI_PROCESSOR_NAME:
                default:
                  break;
            }
        if(result == PAMI_SUCCESS)
          return result;

        // Todo:  Change if we have client and context queries
        // Lapi stores the configuration off the context
        // Use context 0 to query.  It should be created in the
        // current implementation because we create the
        // context at client create time
        lapi_state_t *lp = _contexts[0]->getLapiState();
        LapiImpl::Context *cp = (LapiImpl::Context *)lp;
        return (cp->*(cp->pConfigQuery))(configuration);
      }


    inline pami_result_t updateConfiguration_impl (pami_configuration_t * configuration)
      {
        // Todo:  Change if we have client and context queries

        // Lapi stores the configuration off the context
        // Use context 0 to query.  It should be created in the
        // current implementation because we create the
        // context at client create time
        lapi_state_t *lp = _contexts[0]->getLapiState();
        LapiImpl::Context *cp = (LapiImpl::Context *)lp;
        return (cp->*(cp->pConfigUpdate))(configuration);
      }

    inline PAMI::Context *getContext(size_t ctx)
      {
        return _contexts[ctx];
      }

    inline pami_result_t geometry_world_impl (pami_geometry_t * world_geometry)
      {
        *world_geometry = _world_geometry;
        return PAMI_SUCCESS;
      }

    inline pami_result_t geometry_create_taskrange_impl(pami_geometry_t       * geometry,
                                                        pami_geometry_t         parent,
                                                        unsigned                id,
                                                        pami_geometry_range_t * rank_slices,
                                                        size_t                  slice_count,
                                                        pami_context_t          context,
                                                        pami_event_function     fn,
                                                        void                  * cookie)
      {
        LAPIGeometry              *new_geometry;

        if(geometry != NULL)
            {
              new_geometry = (LAPIGeometry*)_geometryAlloc.allocateObject();
              new(new_geometry) LAPIGeometry((PAMI::Geometry::Common*)parent,
                                             &__global.mapping,
                                             id,
                                             slice_count,
                                             rank_slices);
              for(size_t n=0; n<_ncontexts; n++)
                  {
                    _contexts[n]->_pgas_collreg->analyze(n,new_geometry);
                    _contexts[n]->_oldccmi_collreg->analyze(n,new_geometry);
                    _contexts[n]->_ccmi_collreg->analyze(n,new_geometry);
                  }
              *geometry=(LAPIGeometry*) new_geometry;
              // todo:  deliver completion to the appropriate context
            }
        LAPIGeometry *bargeom = (LAPIGeometry*)parent;
        PAMI::Context *ctxt = (PAMI::Context *)context;
        bargeom->default_barrier(fn, cookie, ctxt->getId(), context);
        return PAMI_SUCCESS;
      }

    inline pami_result_t geometry_create_tasklist_impl(pami_geometry_t       * geometry,
                                                       pami_geometry_t         parent,
                                                       unsigned                id,
                                                       pami_task_t           * tasks,
                                                       size_t                  task_count,
                                                       pami_context_t          context,
                                                       pami_event_function     fn,
                                                       void                  * cookie)
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
    inline void initializeMemoryManager ()
      {
        char   shmemfile[1024];
        size_t bytes     = 1024*1024;
        size_t pagesize  = 4096;

        snprintf (shmemfile, 1023, "/pami-client-%s", _lapiClient->GetName());
        // Round up to the page size
        size_t size = (bytes + pagesize - 1) & ~(pagesize - 1);
        int fd, rc;
        size_t n = bytes;

        // CAUTION! The following sequence MUST ensure that "rc" is "-1" iff failure.
        rc = shm_open (shmemfile, O_CREAT|O_RDWR,0600);
        if ( rc != -1 )
            {
              fd = rc;
              rc = ftruncate( fd, n );
              void * ptr = mmap( NULL, n, PROT_READ | PROT_WRITE, MAP_FILE|MAP_SHARED, fd, 0);
              if ( ptr != MAP_FAILED )
                  {
                    _mm.init (ptr, n);
                    return;
                  }
            }
        // Failed to create shared memory .. fake it using the heap ??
        _mm.init (malloc (n), n);

        return;
      }

  private:
    // LAPI Client Pointer
    LapiImpl::Client                            *_lapiClient;

    // Array of PAMI Contexts associated with this Client
    PAMI::Context                               *_contexts[64];

    // Maximum number of contexts
    size_t                                       _maxctxts;

    // Number of PAMI Contexts associated with this Client
    size_t                                       _ncontexts;

    // A handle to this client for API purposes
    pami_client_t                                _client;

    // An integer identifier for this Client
    size_t                                       _clientid;

    // Platform Device List
    PAMI::PlatformDeviceList                     _platdevs;

    // World Geometry object for collectives
    LAPIGeometry                                *_world_geometry;

    // Geometry Range Object for the initial world geometry
    pami_geometry_range_t                        _world_range;

    // Shared Memory Manager for this Client
    Memory::MemoryManager                        _mm;

    // Initial LAPI handle for building geometries,
    // fencing, and other comm needed for initialization
    lapi_handle_t                                _main_lapi_handle;

    // PAMI Context Allocator
    MemoryAllocator<sizeof(PAMI::Context),16>    _contextAlloc;

    // LAPI Client Allocator
    MemoryAllocator<sizeof(LapiImpl::Client),16> _lapiClientAlloc;

    // Geometry Allocator
    MemoryAllocator<sizeof(LAPIGeometry),16>     _geometryAlloc;

  }; // end class PAMI::Client
}; // end namespace PAMI


#endif
// __pami_lapi_lapiclient_h__
