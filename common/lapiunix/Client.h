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
#include "../lapi/include/Client.h"

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

        // Fence the create
        // Todo:  remove this, does it violate the API?
        CheckLapiRC(lapi_gfence (_main_lapi_handle));

        size_t min_rank, max_rank, num_local, *local_ranks;
        generateMapCache(myrank,
                         mysize,
                         min_rank,
                         max_rank,
                         num_local,
                         &local_ranks);
        __global.mapping.set_mapcache(_mapcache,
                                      _peers,
                                      _npeers);

        // Generate the "generic" device queues
	// We do this here because some devices need a properly initialized
	// mapping to correctly build (like shmem device, for example.  shmem
	// device requires to know the "local" nodes, so a proper mapping
	// must be built).
        _platdevs.generate(_clientid, _maxctxts, _mm);
	_platdevs.init(_clientid,0,_client,(pami_context_t)_contexts[0],&_mm);

        // Initialize the optimized collectives
        _contexts[0]->initCollectives();

        // Return error code
        result                         = rc;
      }

    inline ~Client ()
      {
        _lapiClientAlloc.returnObject(_lapiClient);
      }


    static void map_cache_fn(pami_context_t   context,
                             void           * cookie,
                             pami_result_t    result)
      {
        int *flag = (int*)cookie;
        *flag = 0;
      }

    pami_result_t generateMapCache(size_t   myrank,
                                   size_t   mysize,
                                   size_t  &min_rank,
                                   size_t  &max_rank,
                                   size_t  &num_local,
                                   size_t **local_ranks)
      {
        size_t  r,q,nSize,tSize;
        int     err,nz,tz,str_len=128;
        char   *host,*hosts,*s;

        // local node process/rank info
        _mapcache=(uint32_t*)malloc(sizeof(*_mapcache) * mysize);
        PAMI_assertf(_mapcache != NULL, "memory alloc failed");
        _peers = (size_t*)malloc(sizeof(*_peers) * mysize);
        PAMI_assertf(_peers != NULL, "memory alloc failed");
        host=(char*)malloc(str_len);
        PAMI_assertf(host != NULL, "memory alloc failed");
        hosts=(char*)malloc(str_len*mysize);
        PAMI_assertf(hosts != NULL, "memory alloc failed");
        err = gethostname(host, str_len);
        PAMI_assertf(err == 0, "gethostname failed, errno %d", errno);


	// Do an allgather to collect the map
        pami_xfer_type_t   colltype = PAMI_XFER_ALLGATHER;
        pami_algorithm_t   alg;
        pami_metadata_t    mdata;
        pami_xfer_t        xfer;
        pami_result_t      rc;
        volatile int       flag = 1;
        _world_geometry->algorithms_info(colltype,&alg,&mdata,1,NULL,NULL,0,0);
        xfer.cb_done                        = map_cache_fn;
        xfer.cookie                         = (void*)&flag;
        xfer.algorithm                      = alg;
        xfer.cmd.xfer_allgather.sndbuf      = host;
        xfer.cmd.xfer_allgather.stype       = &PAMI_BYTE;
        xfer.cmd.xfer_allgather.stypecount  = str_len;
        xfer.cmd.xfer_allgather.rcvbuf      = hosts;
        xfer.cmd.xfer_allgather.rtype       = &PAMI_BYTE;
        xfer.cmd.xfer_allgather.rtypecount  = str_len;

	// We can only advance the lapi device here because our other devices
	// are not initialized;  they rely on the map that we are building.
        _contexts[0]->collective(&xfer);
        while(flag)
          _contexts[0]->advance_only_lapi(10,rc);

        PAMI_assertf(err == 0, "allgather failed, err %d", err);

        nSize = 0;
        tSize = 1;
        _npeers = 0;
        for (r = 0; r < mysize; ++r) {
          // search backwards for anyone with the same hostname...
          for (q = r - 1; (int)q >= 0 && strcmp(hosts + str_len * r, hosts + str_len * q) != 0; --q);
          if ((int)q >= 0) {
            // already saw this hostname... add new peer...
            uint32_t u = _mapcache[q];
            uint32_t t = (u & 0x0000ffff) + 1;
            _mapcache[r] = (u & 0xffff0000) | t;
            if (t >= tSize) tSize = t + 1;
          } else {
            // new hostname... first one for that host... give it T=0
            _mapcache[r] = (nSize << 16) | 0;
            ++nSize;
          }
          if (strcmp(host, hosts + str_len * r) == 0) {
            _peers[_npeers++] = r;
          }
        }
        free(host);
        free(hosts);

        // if all ranks are local, then see if an ENV variable
        // gives us permission to spice things up.
        nz = tz = 0;
        s = getenv("PAMI_MAPPING_TSIZE");
        if (s) {
          tz = strtol(s, NULL, 0);
        }
        s = getenv("PAMI_MAPPING_NSIZE");
        if (s) {
          nz = strtol(s, NULL, 0);
        }
        if (nSize == 1 && (nz > 0 || tz > 0)) {
          uint32_t t = 0;
          uint32_t n = 0;
          if (nz > 0) {
            tz = 0;
            // remap using N-first sequence
            nSize = nz;
            for (r = 0; r < mysize; ++r) {
              if (n >= nSize) { ++t; n = 0; }
              _mapcache[r] = (n << 16) | t;
              ++n;
            }
            tSize = t + 1;
          } else if (tz > 0) {
            // remap using T-first sequence
            tSize = tz;
            for (r = 0; r < mysize; ++r) {
              if (t >= tSize) { ++n; t = 0; }
              _mapcache[r] = (n << 16) | t;
              ++t;
            }
            nSize = n + 1;
          }
          // now, must recompute _peers, _npeers...
          _npeers = 0;
          n = _mapcache[myrank] & 0xffff0000;
          for (r = 0; r < mysize; ++r) {
            if ((_mapcache[r] & 0xffff0000) == n) {
              _peers[_npeers++] = r;
            }
          }
        }

        // local ranks could be represented as rectangle...
        // but, let Global.h use Topology analyze if it wants.
        *local_ranks = _peers;
        num_local    = _npeers;
        // global ranks could be represented as rectangle...
        min_rank     = 0;
        max_rank = mysize-1;
        //
        // At this point, _mapcache[rank] -> [index1]|[index2], where:
        // (at target node)_peers[index2] -> rank
        // coordinates = (index1,index2)

        return PAMI_SUCCESS;
      }



    static pami_result_t generate_impl (const char * name, pami_client_t * client,
                                        pami_configuration_t   configuration[],
                                        size_t                 num_configs)
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
	      _platdevs.init(_clientid,i,_client,(pami_context_t)_contexts[i],&_mm);
              if(rc) RETURN_ERR_PAMI(PAMI_ERROR, "createContext failed with rc %d\n", rc);
              _contexts[i]->setWorldGeometry(_world_geometry);
              _contexts[i]->initP2PCollectives();
              _contexts[i]->initCollectives();
            }
        return PAMI_SUCCESS;
      }

    // DEPRECATED!
    inline pami_result_t destroyContext_impl (pami_context_t context)
      {
        PAMI::Context *c = (PAMI::Context*) context;
        pami_result_t rc = PAMI_SUCCESS;
        rc               = c->destroy();
        _contextAlloc.returnObject(context);
        return rc;
      }
    inline pami_result_t destroyContext_impl (pami_context_t *context, size_t ncontexts)
      {
	PAMI_assertf(ncontexts == _ncontexts, "destroyContext called without all contexts");
	size_t i;
        pami_result_t rc = PAMI_SUCCESS;
	for (i = 0; i < _ncontexts; ++i)
	  {
            context[i]       = NULL;
            PAMI::Context *c = _contexts[i];
            _contexts[i]     = NULL;
            pami_result_t r  = c->destroy();
	    if (r != PAMI_SUCCESS) rc = r;
            _contextAlloc.returnObject((void *)c);
          }
	_ncontexts = 0;
        return rc;
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
                  configuration[i].value.intval = 1; // real value TBD
                  break;
                case PAMI_CLIENT_CONST_CONTEXTS:
                  configuration[i].value.intval = 0; // real value TBD
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
                  configuration[i].value.doubleval =__global.time.tick();
                  break;
                case PAMI_CLIENT_MEM_SIZE:
                case PAMI_CLIENT_PROCESSOR_NAME:
                default:
                {
                  pami_result_t rc;
                  lapi_state_t *lp = _contexts[0]->getLapiState();
                  LapiImpl::Context *cp = (LapiImpl::Context *)lp;
                  rc = (cp->*(cp->pConfigQuery))(configuration);
                  if(rc != PAMI_SUCCESS)
                    result = PAMI_INVAL;
                }
              }
          }
        return result;
      }

    inline pami_result_t update_impl (pami_configuration_t configuration[],
                                      size_t               num_configs)
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


    static void cr_func(pami_context_t  context,
                        void           *cookie,
                        uint64_t       *reduce_result,
                        LAPIGeometry   *g,
                        pami_result_t   result )
      {
        Client *c = (Client*)cookie;
        for(size_t n=0; n<c->_ncontexts; n++)
          {
            c->_contexts[n]->_pgas_collreg->analyze_global(n,g,reduce_result[0]);
            c->_contexts[n]->_p2p_ccmi_collreg->analyze_global(n,g,reduce_result[1]);
            c->_contexts[n]->_cau_collreg->analyze_global(n,g,reduce_result[2]);
          }
      }

    inline pami_result_t geometry_create_taskrange_impl(pami_geometry_t       * geometry,
                                                        pami_configuration_t    configuration[],
                                                        size_t                  num_configs,
                                                        pami_geometry_t         parent,
                                                        unsigned                id,
                                                        pami_geometry_range_t * rank_slices,
                                                        size_t                  slice_count,
                                                        pami_context_t          context,
                                                        pami_event_function     fn,
                                                        void                  * cookie)
      {
        LAPIGeometry              *new_geometry = NULL;
        uint64_t                  to_reduce[3];
        // If our new geometry is NOT NULL, we will create a new geometry
        // for this client.  This new geometry will be populated with a
        // set of algorithms.
        if(geometry != NULL)
          {
            new_geometry=(LAPIGeometry*) malloc(sizeof(*new_geometry));
            new(new_geometry)LAPIGeometry((LAPIGeometry*)parent,
                                         &__global.mapping,
                                         id,
                                         slice_count,
                                         rank_slices);
            for(size_t n=0; n<_ncontexts; n++)
              {
                _contexts[n]->_pgas_collreg->analyze_local(n,new_geometry,&to_reduce[0]);
                _contexts[n]->_p2p_ccmi_collreg->analyze_local(n,new_geometry,&to_reduce[1]);
                _contexts[n]->_cau_collreg->analyze_local(n,new_geometry,&to_reduce[2]);
#ifdef _COLLSHM
                    // coll shm device is currently only enabled for world_geometry
                    // _contexts[n]->_coll_shm_collreg->analyze(n, new_geometry);
#endif // _COLLSHM
              }
            *geometry=(LAPIGeometry*) new_geometry;
          }

        // Now we must take care of the synchronization of this new geometry
        // First, if a new geometry is created, we will perform an allreduce
        // on the new geometry.
        // If we have a parent geometry, we perform synchronization on that
        // geometry.  If we don't have a parent geometry, we perform an
        // "unexpected" barrier on only the new geometry.  In either case,
        // any node creating a new geometry will allocate a class route.
        // When the synchronization is completed (barrier or uebarrier), it
        // will chain into an allreduce operation, which performs the allocation
        // of the classroute.  When the classroute has been allocated,
        // The "done" event is delivered to the user.
        LAPIGeometry      *bargeom = (LAPIGeometry*)parent;
        PAMI::Context    *ctxt    = (PAMI::Context *)context;
        if(new_geometry)
          {
            pami_algorithm_t  alg;
            new_geometry->algorithms_info(PAMI_XFER_ALLREDUCE,
                                          &alg,
                                          NULL,
                                          1,
                                          NULL,
                                          NULL,
                                          0,
                                          ctxt->getId());
            Geometry::Algorithm<LAPIGeometry> *ar_algo = (Geometry::Algorithm<LAPIGeometry> *)alg;
            LAPIClassRouteId *cr = (LAPIClassRouteId *)malloc(sizeof(LAPIClassRouteId));
            new(cr)LAPIClassRouteId(ar_algo,
                                   new_geometry,
                                   to_reduce,
                                   3,
                                   cr_func,
                                   (void*)this,
                                   fn,
                                   cookie);
            if(bargeom)
              bargeom->default_barrier(LAPIClassRouteId::get_classroute, cr, ctxt->getId(), context);
            else
              new_geometry->ue_barrier(LAPIClassRouteId::get_classroute, cr, ctxt->getId(), context);
          }
        else
          {
            if(bargeom)
              bargeom->default_barrier(fn, cookie, ctxt->getId(), context);
            else
              {
                // Null parent and null new geometry?  Why are you here?
                return PAMI_INVAL;
              }
          }
        return PAMI_SUCCESS;
      }
      
    inline pami_result_t geometry_create_tasklist_impl(pami_geometry_t       * geometry,
                                                       pami_configuration_t    configuration[],
                                                       size_t                  num_configs,
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
        size_t bytes     = 8192*1024;
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
        PAMI_abort();
        _mm.init (malloc (n), n);

        return;
      }

  private:
    // LAPI Client Pointer
    LapiImpl::Client                            *_lapiClient;

    // Array of PAMI Contexts associated with this Client
    PAMI::Context                               *_contexts[64];

    // The rank map cache
    uint32_t                                     *_mapcache;

    // The local peers cache
    size_t                                       *_peers;

    // The number of local peers
    size_t                                       _npeers;

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
