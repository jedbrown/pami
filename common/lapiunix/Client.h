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
#include <map>
#include <vector>
#include "common/ClientInterface.h"
#include "common/lapiunix/Context.h"
#include "algorithms/geometry/LapiGeometry.h"
#include "../lapi/include/Client.h"
#include "api/extension/c/pe_extension/PeExtension.h"

extern pthread_once_t  _Per_proc_lapi_init;
extern void _lapi_perproc_setup(void);
extern PAMI::PamiActiveClients  _pami_act_clients;

void
simplesplit(vector<string> &vec,
            const  string  &str,
            const  string  &del )
{
  size_t  start = 0, end = 0;
  while ( end != string::npos )
    {
      end = str.find( del, start );
      vec.push_back( str.substr(start,
                                (end == string::npos)?string::npos : end - start ) );
      start =((end>(string::npos-del.size()))
              ?string::npos:end+del.size());
    }
}

namespace PAMI
{
  class Client : public Interface::Client<PAMI::Client>
  {
  public:
    inline Client (const char * name, pami_configuration_t configuration[],
            size_t num_configs, pami_result_t & result) :
      Interface::Client<PAMI::Client>(name, result),
      _client ((pami_client_t) this),
      _maxctxts(0),
      _ncontexts (0),
      _world_list(NULL),
      _mm (),
      _disable_shm(true)
      {
        pami_result_t   rc               = PAMI_SUCCESS;
        static size_t   next_client_id   = 0;
        size_t          myrank=0, mysize = 0;
        _clientid                        = next_client_id++;

        // Todo:  Support more contexts
        _maxctxts = 1;

        // Initialize a LAPI Client Object
        new(&_lapiClient[0]) LapiImpl::Client(name, configuration, num_configs);\
                                                                                  
        // Create an initial context for initialization
        rc = createOneContext(&_contexts[0],0);
        if(rc) {result=rc;  return;}
        // Initialize Point-to-Point Communication
        // This is used to build an initial "simple" mapping
        // Using rank and size
        // This returns locked, so make sure to unlock before returning
        rc = _contexts[0]->initP2P(&myrank, &mysize, &_main_lapi_handle);
        if(rc) {result=rc; _contexts[0]->unlock(); return;}

        // Initialize the mapping to be used for collectives
        __global.mapping.init(myrank, mysize);        

        // Get number of tasks for this world
        // Get list of tasks for this world (colon separated string)
        // Determine whether or not we are initializing shared memory
        char *world_procs, *world_tasks, *dyntasking;
        world_procs = getenv("MP_I_WORLD_PROCS");
        world_tasks = getenv("MP_I_WORLD_TASKS");
        dyntasking  = getenv("MP_I_DYNAMIC_TASKING");
        
        // If I am a spawned task, Do not set up a geometry, or even
        // a mapping
        // see the TODO below about building mappings for
        // dynamic tasking
        // Also, do what we can to disable the shared memory:
        // Another TODO:  remove the _mm dependency from platdevs
        if(!world_procs && dyntasking)
          {
            _mm.init (__global.heap_mm, 16);
            _platdevs.generate(_clientid, _maxctxts, _mm, true);
            _platdevs.init(_clientid,0,_client,(pami_context_t)_contexts[0],&_mm,true);
            result=rc;
            _contexts[0]->unlock();
            return;
          }

        // Initialize the shared memory manager
        pami_result_t shm_rc = initializeMemoryManager ();
        if(shm_rc == PAMI_SUCCESS)
          _disable_shm = false;
        
        // We do something slightly different for dynamic tasking
        // vs non-dynamic tasking.  In the dynamic tasking case,
        // the world geometry is only available for the "world" created
        // at init time.
        // TODO:  we need to construct a mapping that contains
        // a list of ALL the ranks available at init time, so when the
        // user adds a process, it is viable to create a geometry
        // that is larger than the initial world.  We will probably
        // want to get our process managers(poe) to give us support for that.
        if(world_procs)
          {
            // Dynamic tasking case, we need to resize the world we are using
            // for collectives...
            size_t num_world_procs = atoi(world_procs);
            std::string str = world_tasks;
            std::string del = ":";
            vector<string> vec;
            simplesplit(vec,str,del);
            _world_list = (pami_task_t*)malloc(num_world_procs*sizeof(pami_task_t));
            for( unsigned int i = 0;  i < vec.size();   i++ )
              _world_list[i]=atoi(vec[i].c_str());
            
            // Initialize world geometry object, and set the geometry ptr for the context
            _world_geometry = (LAPIGeometry*)_geometryAlloc.allocateObject();
            new(_world_geometry) LAPIGeometry((pami_client_t) this,
                                              NULL,
                                              &__global.mapping,
                                              0,
                                              num_world_procs,
                                              _world_list,
                                              &_geometry_map,
                                              0); // This tells the geometry not to build
                                                  // the optimized topologies.  we can
                                                  // generate them after building a mapping
          }
        else
          {
            // Initialize world geometry object, and set the geometry ptr for the context
            _world_geometry = (LAPIGeometry*)_geometryAlloc.allocateObject();
            _world_range.lo = 0;
            _world_range.hi = mysize-1;
            new(_world_geometry) LAPIGeometry((pami_client_t) this,
                                              NULL,
                                              &__global.mapping,
                                              0,
                                              1,
                                              &_world_range,
                                              &_geometry_map,
                                              0); // This tells the geometry not to build
                                                  // the optimized topologies.  we can
                                                  // generate them after building a mapping
          }
        _contexts[0]->setWorldGeometry(_world_geometry);

        // Initialize "Safe" Collectives, which will be used
        // To build the mappings and topologies for the optimized
        // collectives and to set up token management for collectives
        _contexts[0]->initP2PCollectives();

        // --> Ok to do collectives after here, build better mappings
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
        
        PAMI::Topology::static_init(&__global.mapping);

        // Generate the "generic" device queues
	// We do this here because some devices need a properly initialized
	// mapping to correctly build (like shmem device, for example.  shmem
	// device requires to know the "local" nodes, so a proper mapping
	// must be built).
        _platdevs.generate(_clientid, _maxctxts, _mm, _disable_shm);
	_platdevs.init(_clientid,0,_client,(pami_context_t)_contexts[0],&_mm, _disable_shm);

        // Now that we have a new mapping, we want to regenerate the topologies
        // to use the optimized geometries
        _world_geometry->regenTopo();

        // Initialize the optimized collectives
        _contexts[0]->initCollectives(&_mm, _disable_shm);

        // Return error code
        result                         = rc;
        _contexts[0]->unlock();
        return;
      }

    inline ~Client ()
      {
        if(_world_list) free(_world_list);
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
	pami_result_t rc;
	rc = __global.heap_mm->memalign((void **)&_mapcache, 0,
						sizeof(*_mapcache) * mysize);
        PAMI_assertf(rc == PAMI_SUCCESS, "memory alloc failed");
	rc = __global.heap_mm->memalign((void **)&_peers, 0, sizeof(*_peers) * mysize);
        PAMI_assertf(rc == PAMI_SUCCESS, "memory alloc failed");
	rc = __global.heap_mm->memalign((void **)&host, 0, str_len);
        PAMI_assertf(rc == PAMI_SUCCESS, "memory alloc failed");
	rc = __global.heap_mm->memalign((void **)&hosts, 0, str_len*mysize);
        PAMI_assertf(rc == PAMI_SUCCESS, "memory alloc failed");
        err = gethostname(host, str_len);
        PAMI_assertf(err == 0, "gethostname failed, errno %d", errno);

	// Do an allgather to collect the map
        pami_xfer_type_t   colltype = PAMI_XFER_ALLGATHER;
        pami_algorithm_t   alg;
        pami_metadata_t    mdata;
        pami_xfer_t        xfer;
        volatile int       flag = 1;
        _world_geometry->algorithms_info(colltype,&alg,&mdata,1,NULL,NULL,0,0);
        xfer.cb_done                        = map_cache_fn;
        xfer.cookie                         = (void*)&flag;
        xfer.algorithm                      = alg;
        xfer.cmd.xfer_allgather.sndbuf      = host;
        xfer.cmd.xfer_allgather.stype       = &PAMI_TYPE_CONTIGUOUS;
        xfer.cmd.xfer_allgather.stypecount  = str_len;
        xfer.cmd.xfer_allgather.rcvbuf      = hosts;
        xfer.cmd.xfer_allgather.rtype       = &PAMI_TYPE_CONTIGUOUS;
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
        __global.heap_mm->free(host);
        __global.heap_mm->free(hosts);

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

        // per-process setup. _Lapi_env usable after this point
        if (pthread_once(&_Per_proc_lapi_init, _lapi_perproc_setup) != 0) {
            RETURN_ERR_PAMI(PAMI_ERROR, "pthread_once failed. errno=%d\n", errno);
        }

        PAMI::Client * clientp;
        pami_result_t res;
	res = __global.heap_mm->memalign((void **)&clientp, 0, sizeof(*clientp));
        PAMI_assertf(res == PAMI_SUCCESS, "Failed to alloc client"); // or return?
        memset ((void *)clientp, 0x00, sizeof(PAMI::Client));
        new (clientp) PAMI::Client(name, configuration, num_configs, res);
        if (res != PAMI_SUCCESS)
          return res;
        *client = (pami_client_t) clientp;
        
        // Store newly created client into global structure 
        res = _pami_act_clients.AddClient(*client);
        if (res != PAMI_SUCCESS)
          RETURN_ERR_PAMI(PAMI_ERROR, "Failed to add the created client to the active client counter.\n");
        return res;
      }

    static void destroy_impl (pami_client_t client)
      {
        // Delete destroyed client from global structure 
        _pami_act_clients.RemoveClient(client);
          
        Client *c = (Client *) client;
        c->~Client();
        __global.heap_mm->free(client);
      }

    inline char * getName_impl ()
      {
        return ((LapiImpl::Client*)&_lapiClient[0])->GetName();
      }


    inline pami_result_t createOneContext(PAMI::Context **ctxt,
                                          int             index)
      {
        PAMI::Context *c = (PAMI::Context *) _contextAlloc.allocateObject();

        Memory::GenMemoryManager *mm;
        new (c) PAMI::Context(this->getClient(),      /* Client ptr       */
                              _clientid,              /* Client  id       */
                              ((LapiImpl::Client*)&_lapiClient[0])->GetName(), /* Client String    */
                              index,                  /* Context id       */
                              &_platdevs,             /* Platform Devices */
                              &_geometry_map);  
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
              _contexts[i]->initCollectives(&_mm, _disable_shm);
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
                  configuration[i].value.intval = 1;  // will change when multi-endpoint support is available
                  break;
                case PAMI_CLIENT_CONST_CONTEXTS:
                  configuration[i].value.intval = 1;
                  break;
                case PAMI_CLIENT_TASK_ID:
                  configuration[i].value.intval = __global.mapping.task();
                  break;
                case PAMI_CLIENT_NUM_TASKS:
                  configuration[i].value.intval = __global.mapping.size();
                  break;
                case PAMI_CLIENT_WTICK:
                  configuration[i].value.doubleval =__global.time.tick();
                  break;
                case PAMI_GEOMETRY_OPTIMIZE:
                  result = PAMI_UNIMPL;
                  break;
                case PAMI_CLIENT_ACTIVE_CONTEXT:
                  _active_contexts.context_num = _ncontexts;
                  for (int i = 0; i < _ncontexts; i ++)
                    _active_contexts.contexts[i] = (pami_context_t) _contexts[i];
                  configuration[i].value.chararray = (char*)&_active_contexts;
                  break;
                case PAMI_CLIENT_PROTOCOL_NAME:
                  configuration[i].value.chararray = getName_impl();
                  break;
                default:
                {
                  internal_rc_t rc;
                  lapi_state_t *lp = _contexts[0]->getLapiState();
                  LapiImpl::Context *cp = (LapiImpl::Context *)lp;
                  rc = (cp->*(cp->pConfigQuery))(configuration);
                  if(rc != SUCCESS)
                    result = PAMI_UNIMPL;
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
        internal_rc_t rc = (cp->*(cp->pConfigUpdate))(configuration);
        return PAMI_RC(rc);
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
            c->_contexts[n]->_pgas_collreg->receive_global(n,g,&reduce_result[0],1);
            c->_contexts[n]->_p2p_ccmi_collreg->receive_global(n,g,&reduce_result[1],1);
            c->_contexts[n]->_cau_collreg->receive_global(n,g,&reduce_result[2],1);
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
        uint64_t                  to_reduce_vec[16];
        uint64_t                  *to_reduce;
        uint                      to_reduce_count;
	pami_result_t rc;
        // If our new geometry is NOT NULL, we will create a new geometry
        // for this client.  This new geometry will be populated with a
        // set of algorithms.
        if(geometry != NULL)
          {
	    rc = __global.heap_mm->memalign((void **)&new_geometry, 0,
							sizeof(*new_geometry));
	    PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc new_geometry");
            new(new_geometry)LAPIGeometry((pami_client_t)this,
                                          (LAPIGeometry*)parent,
                                          &__global.mapping,
                                          id,
                                          slice_count,
                                          rank_slices,
                                          &_geometry_map);
            
            PAMI::Topology *local_master_topology  = (PAMI::Topology *)new_geometry->getTopology(PAMI::Geometry::MASTER_TOPOLOGY_INDEX);
            to_reduce_count = 3 + local_master_topology->size();
            if (to_reduce_count >16) {
	       rc = __global.heap_mm->memalign((void **)&to_reduce, 0,
               					to_reduce_count * sizeof(uint64_t));
	       PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc to_reduce");
            } else {
               to_reduce = &(to_reduce_vec[0]);
            }
            for(size_t n=0; n<_ncontexts; n++)
              {
                new_geometry->resetUEBarrier(); // Reset so pgas will select the UE barrier
		int nc = 0;
		int ncur = 0;
                _contexts[n]->_pgas_collreg->register_local(n,new_geometry,&to_reduce[0/*n*/], ncur);
		nc+= ncur;
		ncur = 0;
                _contexts[n]->_p2p_ccmi_collreg->register_local(n,new_geometry,&to_reduce[1/*n*/], ncur);
		nc+= ncur;
		ncur=0;
                _contexts[n]->_cau_collreg->register_local(n,new_geometry,&to_reduce[2/*n*/], ncur);
		//nc+= ncur;
		//ideally n should be used in the BAND reduction
              }
	    new_geometry->processUnexpBarrier(&_ueb_queue,
                                              &_ueb_allocator);
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
            LAPIClassRouteId *cr;
	    rc = __global.heap_mm->memalign((void **)&cr, 0, sizeof(LAPIClassRouteId));
	    PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc LAPIClassRouteId");
            new(cr)LAPIClassRouteId(ar_algo,
                                   new_geometry,
                                   to_reduce,
                                   to_reduce_count,
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

        if (to_reduce_count > 16) __global.heap_mm->free(to_reduce);
        return PAMI_SUCCESS;
      }

    inline pami_result_t geometry_create_topology_impl(pami_geometry_t       * geometry,
                                                       pami_configuration_t    configuration[],
                                                       size_t                  num_configs,
                                                       pami_geometry_t         parent,
                                                       unsigned                id,
                                                       pami_topology_t       * topology,
                                                       pami_context_t          context,
                                                       pami_event_function     fn,
                                                       void                  * cookie)
      {
        // todo:  implement this routine
        PAMI_abort();

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
        LAPIGeometry              *new_geometry = NULL;
        uint64_t                  to_reduce_vec[16];
        uint64_t                  *to_reduce;
        uint                      to_reduce_count;
        // If our new geometry is NOT NULL, we will create a new geometry
        // for this client.  This new geometry will be populated with a
        // set of algorithms.
        if(geometry != NULL)
          {
            new_geometry=(LAPIGeometry*) malloc(sizeof(*new_geometry));
            new(new_geometry)LAPIGeometry((pami_client_t)this,
                                          (LAPIGeometry*)parent,
                                          &__global.mapping,
                                          id,
                                          task_count,
                                          tasks,
                                          &_geometry_map);

            PAMI::Topology *local_master_topology  = (PAMI::Topology *)new_geometry->getTopology(PAMI::Geometry::MASTER_TOPOLOGY_INDEX);
            to_reduce_count = 3 + local_master_topology->size();
            if (to_reduce_count >16) {
              to_reduce = (uint64_t *)malloc(to_reduce_count * sizeof(uint64_t));
            } else {
              to_reduce = &(to_reduce_vec[0]);
            }
            for(size_t n=0; n<_ncontexts; n++)
              {
                new_geometry->resetUEBarrier(); // Reset so pgas will select the UE barrier
		int nc = 0;
		int ncur = 0;
                _contexts[n]->_pgas_collreg->register_local(n,new_geometry,&to_reduce[0], ncur);
		nc+=ncur;
		ncur=0;
                _contexts[n]->_p2p_ccmi_collreg->register_local(n,new_geometry,&to_reduce[1], ncur);
		nc+=ncur;
		ncur=0;
                _contexts[n]->_cau_collreg->register_local(n,new_geometry,&to_reduce[2], ncur);
              }
            new_geometry->processUnexpBarrier(&_ueb_queue,
                                              &_ueb_allocator);
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
                                    to_reduce_count,
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

        if (to_reduce_count > 16) free(to_reduce);
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
        LAPIGeometry* g = (LAPIGeometry*)geometry;
        g->~LAPIGeometry();
        free(g);        
        return PAMI_SUCCESS;
      }
    
    inline pami_geometry_t mapidtogeometry_impl (int comm)
      {
        pami_geometry_t g = _geometry_map[comm];
        TRACE_ERR((stderr, "<%p>%s\n", g, __PRETTY_FUNCTION__));
        return g;
      }

    inline void registerUnexpBarrier_impl (unsigned     comm,
                                           pami_quad_t &info,
                                           unsigned     peer,
                                           unsigned     algorithm)
      {
        Geometry::UnexpBarrierQueueElement *ueb =
          (Geometry::UnexpBarrierQueueElement *) _ueb_allocator.allocateObject();
        new (ueb) Geometry::UnexpBarrierQueueElement (comm, info, peer, algorithm);
        _ueb_queue.pushTail(ueb);
      }
    
  protected:

    inline pami_client_t getClient () const
      {
        return _client;
      }

  private:
    inline pami_result_t  initializeMemoryManager ()
      {
        pami_result_t rc = PAMI_ERROR;
        if(_Lapi_env.use_mpi_shm == SHM_YES)
          {
            char   shmemfile[PAMI::Memory::MMKEYSIZE];
            size_t bytes     = COLLSHM_SEGSZ +      // Shmem used by collshm
              P2PSHM_MEMSIZE;      // Shmem used by p2pshm 
            size_t pagesize  = COLLSHM_PAGESZ;

            // Convert string to lower case so the string can be found by
            // external cleanup tools
            std::string s = ((LapiImpl::Client*)&_lapiClient[0])->GetName();
            std::transform(s.begin(),s.end(),s.begin(), ::tolower);
            snprintf(shmemfile, sizeof(shmemfile) - 1, "/pami-client-%s",s.c_str());

            // Round up to the page size
            size_t size = (bytes + pagesize - 1) & ~(pagesize - 1);
            rc = _mm.init(__global.shared_mm, size, 1, 1, 0, shmemfile);
          }
        return rc;
      }

  private:
    // LAPI Client object. use this for direct access 
    char                    _lapiClient[sizeof(LapiImpl::Client)];

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

    // Geometry list Object for the initial world geometry(dynamic tasking)
    pami_task_t                                 *_world_list;

    // This is a map of geometries to geometry id's
    std::map<unsigned, pami_geometry_t>          _geometry_map;
    
    // Shared Memory Manager for this Client
    Memory::GenMemoryManager                     _mm;

    // Flag to disable shared memory
    bool                                         _disable_shm;

    // Initial LAPI handle for building geometries,
    // fencing, and other comm needed for initialization
    lapi_handle_t                                _main_lapi_handle;

    // PAMI Context Allocator
    MemoryAllocator<sizeof(PAMI::Context),16>    _contextAlloc;

    // Geometry Allocator
    MemoryAllocator<sizeof(LAPIGeometry),16>     _geometryAlloc;


    //  Unexpected Barrier allocator
    MemoryAllocator <sizeof(PAMI::Geometry::UnexpBarrierQueueElement), 16> _ueb_allocator;

    //  Unexpected Barrier match queue
    MatchQueue                                                             _ueb_queue;

    // Active contexts in the Client
    PamiActiveContexts                           _active_contexts;
  }; // end class PAMI::Client
}; // end namespace PAMI


#endif
// __pami_lapi_lapiclient_h__
