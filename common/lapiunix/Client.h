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

inline void
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

static inline int find_first_bit(uint64_t in)
{
  int index;
  if (in == 0)
    return (0);
  for (index = 1; !(in&1); index++)
    in = (uint64_t)in >> 1;
  return index;
}

namespace PAMI
{

  class Client : public Interface::Client<PAMI::Client>
  {
  public:

    /* Glue code to PAMI::LapiImpl::Client */
    static bool CheckpointEvent(void* cookie)
      {
          Client *client = (Client *) cookie;
          return client->Checkpoint();
      }

    static bool RestartEvent(void* cookie)
      {
          Client *client = (Client *) cookie;
          return client->Restart();
      }

    static bool ResumeEvent(void* cookie)
      {
          Client *client = (Client *) cookie;
          return client->Resume();
      }

    inline bool Checkpoint()
      {
        bool rc = false;
        ITRC(IT_INITTERM, "PAMI::Clinet 0x%p: Checkpoint() enters\n", this);
        if (_Lapi_env.MP_infolevel > 2)
          fprintf(stderr, "PAMI::Clinet 0x%p: Checkpoint() enters\n", this);
        /* loop through all geometries */
        std::map<unsigned, pami_geometry_t>::iterator g_it = _geometry_map.begin();
        while(g_it != _geometry_map.end()) {
          LAPIGeometry *g = (LAPIGeometry*)(g_it->second);
          if (g) {
            rc = g->Checkpoint();
            if (!rc) {
              /* Checkpoint failed. */ 
              /*
               * TODO: Resume all previously checkpointed geometry.
               *       Is that doable?
               */
              if (_Lapi_env.MP_infolevel > 2)
                fprintf(stderr, "PAMI::Client 0x%p: FAILED to Checkpoint "
                        "LAPIGeometry 0x%p\n", this, g);
              return false;
            }
          }
          g_it++;
        }
        ITRC(IT_INITTERM, "PAMI::Clinet 0x%p: Checkpoint() exits\n", this);
        if (_Lapi_env.MP_infolevel > 2)
          fprintf(stderr, "PAMI::Clinet 0x%p: Checkpoint() exits\n", this);

        return rc;
      }

    inline bool Restart()
      {                                                        
        bool rc = false;
        ITRC(IT_INITTERM, "PAMI::Clinet 0x%p: Restart() enters\n", this);
        if (_Lapi_env.MP_infolevel > 2)
          fprintf(stderr, "PAMI::Clinet 0x%p: Restart() enters\n", this);
        /* loop through all geometries */
        std::map<unsigned, pami_geometry_t>::iterator g_it = _geometry_map.begin();
        while(g_it != _geometry_map.end()) {
          LAPIGeometry *g = (LAPIGeometry*)(g_it->second);
          if (g) {
            rc = g->Restart();
            if (!rc) {
              ITRC(IT_INITTERM,
                "PAMI::Clinet 0x%p: FAILED to Restart LAPIGeometry 0x%p\n", this, g);
              if (_Lapi_env.MP_infolevel > 2)
                fprintf(stderr,
                  "PAMI::Clinet 0x%p: FAILED to Restart LAPIGeometry 0x%p\n", this, g);
              /*
               * Restart failed we have no choice but return error to upper level.
               * Upper level should hangle this error (maybe print message and
               * terminate the job)
               */
              return false;
            }
          }
          g_it++;
        }

        ITRC(IT_INITTERM, "PAMI::Clinet 0x%p: Restart() exits\n", this);   
        if (_Lapi_env.MP_infolevel > 2)
          fprintf(stderr, "PAMI::Clinet 0x%p: Restart() exits\n", this);   
        return rc;
      }

    inline bool Resume()
      {
        bool rc = false;
        ITRC(IT_INITTERM, "PAMI::Clinet 0x%p: Resume() enters\n", this);
        if (_Lapi_env.MP_infolevel > 2)
          fprintf(stderr, "PAMI::Clinet 0x%p: Resume() enters\n", this);

        /* loop through all geometries */
        std::map<unsigned, pami_geometry_t>::iterator g_it = _geometry_map.begin();
        while(g_it != _geometry_map.end()) {
          LAPIGeometry *g = (LAPIGeometry*)(g_it->second);
          if (g) {
            rc = g->Resume();
            if (!rc) {                                         
              ITRC(IT_INITTERM,
                "PAMI::Clinet 0x%p: FAILED to Resume LAPIGeometry 0x%p\n", this, g);
              if (_Lapi_env.MP_infolevel > 2)
                fprintf(stderr,
                  "PAMI::Clinet 0x%p: FAILED to Resume LAPIGeometry 0x%p\n", this, g);

              /*
               * Resume failed we have no choice but return error to upper level.
               * Upper level should hangle this error (maybe print message and
               * terminate the job)                                      
               */
              return false;
            }
          }
          g_it++;
        }

        ITRC(IT_INITTERM, "PAMI::Clinet 0x%p: Resume() exits\n", this);
        if (_Lapi_env.MP_infolevel > 2)
          fprintf(stderr, "PAMI::Clinet 0x%p: Resume() exits\n", this);

        return rc;
      }

    static void decrement_done_fn(pami_context_t   context,
                                  void           * cookie,
                                  pami_result_t    result)
      {
        volatile int* var = (volatile int*) cookie;
        (*var)--;
        return;
      }

    inline pami_result_t initP2PCollectives(Context               *ctxt,
					    Memory::MemoryManager *mm,
					    bool                   disable_shm)
      {
        Memory::MemoryManager *mm_ptr;
        if(disable_shm) mm_ptr = NULL;
        else            mm_ptr = mm;

        // Initalize Collective Registration
        pami_result_t rc;
        rc = __global.heap_mm->memalign((void **)&ctxt->_pgas_collreg, 0,
                                        sizeof(*ctxt->_pgas_collreg));
        PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc PGASCollreg");
        new(ctxt->_pgas_collreg) PGASCollreg(this,
                                             ctxt,
                                             _clientid,
                                             ctxt->getId(),
                                             ctxt->_protocol,
                                             ctxt->_lapi_device,
                                             ctxt->_devices->_shmem[ctxt->getId()],
					     &(ctxt->_lapi_handle),
                                             &ctxt->_dispatch_id,
                                             &_geometry_map,
                                             false);
        return PAMI_SUCCESS;
      }

    inline pami_result_t initCollectives(Context               *ctxt,
                                         Memory::MemoryManager *mm,
                                         bool                   disable_shm)
      {
        Memory::MemoryManager *mm_ptr;
        pami_result_t          rc;

        if(disable_shm) mm_ptr = NULL;
        else            mm_ptr = mm;

        // Init CAU/BSR
        ctxt->_bsr_device.setGenericDevices(ctxt->_devices->_generics);
        ctxt->_cau_device.setGenericDevices(ctxt->_devices->_generics);
        rc = __global.heap_mm->memalign((void **)&ctxt->_cau_collreg, 0,
                                        sizeof(*ctxt->_cau_collreg));
        PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc CAUCollreg");
        new(ctxt->_cau_collreg) CAUCollreg(_client,
                                           ctxt,
                                           ctxt->getId(),
                                           _clientid,
                                           *ctxt->_devices->_generics,
                                           ctxt->_bsr_device,
                                           ctxt->_cau_device,
                                           ctxt->_lapi_device,
                                           ctxt->_protocol,
                                           __global.mapping,
                                           ctxt->_lapi_handle,
                                           &ctxt->_dispatch_id,
                                           &_geometry_map,
                                           mm_ptr);
        size_t numpeers, numtasks;
        __global.mapping.nodePeers(numpeers);
        numtasks = __global.mapping.size();

        bool  use_shm    = mm_ptr?true:false;
        char *shm_method = getenv("PAMI_COLLECTIVES_SHM_DIRECT");
        if(shm_method && use_shm)
        {
          use_shm = atoi(shm_method);
        }
        // Init CCMI
        rc = __global.heap_mm->memalign((void **)&ctxt->_p2p_ccmi_collreg,
                                        0,
                                        sizeof(*ctxt->_p2p_ccmi_collreg));
        PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc P2PCCMICollreg");

        CCMI::Interfaces::NativeInterfaceFactory *ni_factory;
        CCMI::Interfaces::NativeInterfaceFactory *ni_factory_amc;
        if (use_shm) //shmem is available so use composite local/global (shmem/lapi) NI
        {
          rc = __global.heap_mm->memalign((void **)&ni_factory,
                                          0,
                                          sizeof(CompositeNIFactory));
          PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc NativeInterfaceFactory");

          new (ni_factory) CompositeNIFactory (_client, ctxt, _clientid, ctxt->getId(),
                                               ctxt->_devices->_shmem[ctxt->getId()],
                                               ctxt->_lapi_device,
                                               ctxt->_protocol);
          // NI Factory for AM Collectives
          rc = __global.heap_mm->memalign((void **)&ni_factory_amc,
                                          0,
                                          sizeof(CompositeNIFactory_AMC));
          PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc NativeInterfaceFactory");

          new (ni_factory_amc) CompositeNIFactory_AMC (_client, ctxt, _clientid, ctxt->getId(),
                                                       ctxt->_devices->_shmem[ctxt->getId()],
                                                       ctxt->_lapi_device,
                                                       ctxt->_protocol);
        }
        else // no shmem, use global (lapi) only
        {
          rc = __global.heap_mm->memalign((void **)&ni_factory,
                                          0,
                                          sizeof(LapiNIFactory));
          PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc NativeInterfaceFactory");

          new (ni_factory) LapiNIFactory (_client, ctxt, _clientid, ctxt->getId(),
                                          ctxt->_lapi_device,
                                          ctxt->_protocol);
          // NI Factory for AM Collectives
          rc = __global.heap_mm->memalign((void **)&ni_factory_amc,
                                          0,
                                          sizeof(LapiNIFactory_AMC));
          PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc NativeInterfaceFactory");

          new (ni_factory_amc) LapiNIFactory_AMC (_client, ctxt, _clientid, ctxt->getId(),
                                                  ctxt->_lapi_device,
                                                  ctxt->_protocol);
        }

        new(ctxt->_p2p_ccmi_collreg) P2PCCMICollreg(_client,
                                                    ctxt,
                                                    _clientid,
                                                    ctxt->getId(),                                                    
                                                    ctxt->_protocol,
                                                    numtasks,
                                                    numpeers,
                                                    &ctxt->_dispatch_id,
                                                    &_geometry_map,
                                                    ni_factory,
                                                    ni_factory_amc);

        // Finish PGAS
        ctxt->_pgas_collreg->setGenericDevice(&ctxt->_devices->_generics[ctxt->getId()]);
        return PAMI_SUCCESS;
      }



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
        static size_t   next_client_id   = 0;
        _clientid                        = next_client_id++;

        // Initialize a LAPI Client Object
        LapiImpl::Client::Config config(name, configuration, num_configs);
        try {
            new(&_lapiClient[0]) LapiImpl::Client(config);
        } catch (internal_rc_t int_err) {
            result = PAMI_RC(int_err);
            return;
        } catch (std::bad_alloc) {
            result = PAMI_ENOMEM;
            return;
        }
        result = PAMI_SUCCESS;
      }

    inline pami_result_t createContext_impl (pami_configuration_t configuration[],
                                             size_t               count,
                                             pami_context_t      *contexts,
                                             size_t               ncontexts)
      {
        pami_result_t   rc = PAMI_SUCCESS;
        size_t          myrank, mysize;

        // Support more contexts
        pami_configuration_t query = { PAMI_CLIENT_NUM_CONTEXTS };
        query_impl(&query, 1);
        _maxctxts = query.value.intval;
        if (ncontexts > _maxctxts)
          RETURN_ERR_PAMI(PAMI_ERROR, "creating %d contexts while %d available\n",
              ncontexts, _maxctxts);

        // Create all contexts with P2P initialized
        for(int i=0; i< ncontexts; i++)
            {
              lapi_handle_t t_lhandle;

              rc = createOneContext(&_contexts[i],i);
              if(rc) RETURN_ERR_PAMI(PAMI_ERROR, "createOneContext failed with rc %d\n", rc);

              rc = _contexts[i]->initP2P(&myrank, &mysize, &t_lhandle);
              if(rc) RETURN_ERR_PAMI(PAMI_ERROR, "initP2P failed with rc %d\n", rc);
            }

        // Additional work after contexts are created
        try {
          ((LapiImpl::Client *)_lapiClient)->EndContextCreate();
        } catch (internal_rc_t int_err) {
          return PAMI_RC(int_err);                                                 
        } catch (std::bad_alloc) {                                                 
          return PAMI_ENOMEM;                                        
        }

        // Initialize the mapping to be used for collectives
        __global.mapping.init(myrank, mysize);

        // TODO: remove this return for collectives supporting multiple endpoints
        if (_Lapi_env.endpoints > 1) {
            _platdevs.generate(_clientid, _maxctxts, _mm, _disable_shm);
            _platdevs.init(_clientid,0,_client,(pami_context_t)_contexts[0],&_mm, _disable_shm);
            _contexts[0]->unlock();
            return initMoreContexts(configuration, count, contexts, ncontexts);
        }

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
            _contexts[0]->unlock();
            return initMoreContexts(configuration, count, contexts, ncontexts);
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
                                              1,
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
                                              1,
                                              1,
                                              &_world_range,
                                              &_geometry_map,
                                              0); // This tells the geometry not to build
                                                  // the optimized topologies.  we can
                                                  // generate them after building a mapping
          }

        // Initialize "Safe" Collectives, which will be used
        // To build the mappings and topologies for the optimized
        // collectives and to set up token management for collectives
        initP2PCollectives(_contexts[0], &_mm, _disable_shm);
        _world_geometry->resetUEBarrier();
        _contexts[0]->_pgas_collreg->analyze(_contexts[0]->getId(),_world_geometry);

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

        // gather and exchange affinity info from all tasks to enable/disable BSR
        // this is a blocking call
        // Also find the first common cau index
        bool affinity_checked = generateDeviceInfo();

        // initialize CAU and BSR devices
        _contexts[0]->initDevices(affinity_checked, _cau_uniqifier);

        // Initialize the optimized collectives
        initCollectives(_contexts[0], &_mm, _disable_shm);

        // Initialize the optimized collectives by creating the "true world geometry"
        volatile int flag = 1;
        pami_geometry_t _new_world_geometry;
        if(world_procs)
        {
          size_t num_world_procs = atoi(world_procs);
          geometry_create_tasklist_impl(&_new_world_geometry,
                                        NULL,
                                        0,
                                        _world_geometry,
                                        0,
                                        _world_list,
                                        num_world_procs,
                                        (pami_context_t)_contexts[0],
                                        decrement_done_fn,
                                        (void*)&flag);
        }
        else
          geometry_create_taskrange_impl(&_new_world_geometry,
                                         NULL,
                                         0,
                                         (pami_geometry_t)_world_geometry,
                                         0,
                                         &_world_range,
                                         1,
                                         (pami_context_t)_contexts[0],
                                         decrement_done_fn,
                                         (void*)&flag);
        while(flag)
          _contexts[0]->advance(10,rc);

        // Clean up the temporary world geometry
        _world_geometry->~Lapi();
        _geometryAlloc.returnObject(_world_geometry);
        _world_geometry = (LAPIGeometry*)_new_world_geometry;


        // Return error code
        _contexts[0]->unlock();
        return initMoreContexts(configuration, count, contexts, ncontexts);
      }

    inline void cleanup_geometry() {
        /* clean up all existing geometries */
        std::map<unsigned, pami_geometry_t>::iterator g_it = _geometry_map.begin();
        while (g_it != _geometry_map.end())
        {
          // It's ok to delete an element while traversing a map
          // if you save a tmp ptr
          // Destructor of the geometry deletes the map entry
          std::map<unsigned, pami_geometry_t>::iterator tmp = g_it++;
          LAPIGeometry* g = (LAPIGeometry*)(tmp->second);
          g->~Lapi();
        }
    }

    inline ~Client ()
      {
        cleanup_geometry();
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
        int     nz,tz;
        nrt_node_number_t *hosts;

        // local node process/rank info
	pami_result_t rc;
	rc = __global.heap_mm->memalign((void **)&_mapcache, 0,
						sizeof(*_mapcache) * mysize);
        PAMI_assertf(rc == PAMI_SUCCESS, "memory alloc failed");
	rc = __global.heap_mm->memalign((void **)&_peers, 0, sizeof(*_peers) * mysize);
        PAMI_assertf(rc == PAMI_SUCCESS, "memory alloc failed");
	rc = __global.heap_mm->memalign((void **)&hosts, 0, sizeof(nrt_node_number_t) * mysize);
        PAMI_assertf(rc == PAMI_SUCCESS, "memory alloc failed");

        // get task-node map
        LapiImpl::Context *lapi_context = (LapiImpl::Context *)_contexts[0];
        lapi_context->GetTaskNodeMap(hosts);

        nSize = 0;
        tSize = 1;
        _npeers = 0;
        for (r = 0; r < mysize; ++r) {
          // search backwards for anyone with the same hostname...
          for (q = r - 1; (int)q >= 0 && hosts[r] != hosts[q]; --q);
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
          if (hosts[myrank] == hosts[r]) {
            _peers[_npeers++] = r;
          }
        }
        __global.heap_mm->free(hosts);

        PAMI_assertf(_npeers != 0, "Peers is 0"); // or return?

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

    // a blocking call to gather and exchange affinity info among tasks
    // return true when affinity is right for BSR
    // return false otherwise
    bool generateDeviceInfo()
      {
        pami_xfer_type_t   colltype = PAMI_XFER_ALLREDUCE;
        pami_algorithm_t   alg;
        pami_metadata_t    mdata;
        pami_xfer_t        xfer;
        volatile int       flag = 1;
        pami_result_t      rc;

        // gather local affinity info through PNSD
        uint64_t deviceCheck[3];
        deviceCheck[0] = (((LapiImpl::Context*)_contexts[0])->CheckAffinityInfo())?1ULL:0ULL;
        deviceCheck[1] = ((LapiImpl::Context*)_contexts[0])->nrt[0]->table_info.cau_index_resources;
        deviceCheck[2] = _disable_shm?1ULL:0ULL;

        volatile uint64_t deviceCheckResult[3] = {0,0,0};

        _world_geometry->algorithms_info(colltype,&alg,&mdata,1,NULL,NULL,0,0);

        xfer.cb_done                        = decrement_done_fn;
        xfer.cookie                         = (void*)&flag;
        xfer.algorithm                      = alg;
        xfer.cmd.xfer_allreduce.sndbuf      = (char*)&deviceCheck[0];
        xfer.cmd.xfer_allreduce.stype       = PAMI_TYPE_UNSIGNED_LONG_LONG;
        xfer.cmd.xfer_allreduce.stypecount  = 3;
        xfer.cmd.xfer_allreduce.rcvbuf      = (char*)&deviceCheckResult[0];
        xfer.cmd.xfer_allreduce.rtype       = PAMI_TYPE_UNSIGNED_LONG_LONG;
        xfer.cmd.xfer_allreduce.rtypecount  = 3;
        xfer.cmd.xfer_allreduce.op          = PAMI_DATA_BAND;
        xfer.cmd.xfer_allreduce.commutative = 1;
        xfer.cmd.xfer_allreduce.data_cookie = NULL;

        // do an allreduce to exchange affinity info
        _contexts[0]->collective(&xfer);

        while(flag)
          _contexts[0]->advance(10,rc);

        _cau_uniqifier = find_first_bit(deviceCheckResult[1]);
        if(deviceCheckResult[2]) _disable_shm=true;
        
        ITRC(IT_CAU, "Client Affinity Discovery: [in]=0x%llx [out]=0x%llx\n",
             deviceCheck[0], deviceCheckResult[0]);
        ITRC(IT_CAU, "Client CAU Mask Discovery: [in]=0x%llx [out]=0x%llx firstbit(cau_uniqifier)=%d\n",
             deviceCheck[1], deviceCheckResult[1],_cau_uniqifier);
        ITRC(IT_CAU, "Client Disable SHM Discovery: [in]=0x%llx [out]=0x%llx\n",
             deviceCheck[2], deviceCheckResult[2]);

        return (deviceCheckResult[0] == 1);
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

        // Register checkpoint related event functions
        ((LapiImpl::Client *) clientp)->RegisterCheckpointEvents(
          CheckpointEvent, RestartEvent, ResumeEvent, clientp);

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

    inline pami_result_t createOneContext(PAMI::Context **ctxt,
                                          int             index)
      {
        PAMI::Context *c = (PAMI::Context *) _contextAlloc.allocateObject();

        Memory::GenMemoryManager *mm;
        new (c) PAMI::Context(this->getClient(),      /* Client ptr       */
                              _clientid,              /* Client  id       */
                              ((LapiImpl::Client*)&_lapiClient[0])->GetName(), /* Client String    */
                              index,                  /* Context id       */
                              &_platdevs);             /* Platform Devices */
        *ctxt = c;
        _ncontexts++;
        return PAMI_SUCCESS;
      }



    inline pami_result_t initMoreContexts (pami_configuration_t configuration[],
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
	      _platdevs.init(_clientid,i,_client,(pami_context_t)_contexts[i],&_mm, _disable_shm);

              // TODO: remove this check for collectives supporting multiple endpoints
              if (_Lapi_env.endpoints > 1) {
                  _contexts[i]->unlock();
                  contexts[i] = (pami_context_t) _contexts[i];
                  continue;
              }
              initP2PCollectives(_contexts[i],&_mm,_disable_shm);
              initCollectives(_contexts[i],&_mm, _disable_shm);
              _contexts[i]->unlock();

              // Setup return for other contexts
              contexts[i] = (pami_context_t) _contexts[i];
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
            switch ((pami_attribute_name_ext_t)configuration[i].name)
              {
                case PAMI_CLIENT_CONST_CONTEXTS:
                  configuration[i].value.intval = 1;
                  break;
                case PAMI_CLIENT_WTICK:
                  if (_Lapi_env.use_hfi) {
                      LapiImpl::Client *lapi_client = (LapiImpl::Client *)_lapiClient;
                      configuration[i].value.doubleval = lapi_client->GetWtick();
                  } else {
                      configuration[i].value.doubleval =__global.time.tick();
                  }
                  break;

                case PAMI_CLIENT_CLOCK_MHZ:
                case PAMI_CLIENT_WTIMEBASE_MHZ:
                  if (_Lapi_env.use_hfi) {
                      LapiImpl::Client *lapi_client = (LapiImpl::Client *)_lapiClient;
                      configuration[i].value.intval = lapi_client->GetWtimebaseMhz();
                  } else {
                      configuration[i].value.intval = __global.time.clockMHz();
                  }
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
                default:
                {
                  LapiImpl::Client *lapi_client = (LapiImpl::Client *)_lapiClient;
                  pami_result_t rc = lapi_client->Query(configuration[i]);
                  if(rc != PAMI_SUCCESS)
                    result = rc;
                }
              }
          }
        return result;
      }

    inline pami_result_t update_impl (pami_configuration_t configuration[],
                                      size_t               num_configs)
      {
        pami_result_t result = PAMI_SUCCESS;
        size_t i;
        for(i=0; i<num_configs; i++)
          {
            switch (configuration[i].name)
              {
                default:
                {
                  LapiImpl::Client *lapi_client = (LapiImpl::Client *)_lapiClient;
                  pami_result_t rc = lapi_client->Update(configuration[i]);
                  if(rc != PAMI_SUCCESS)
                    result = rc;
                }
              }
          }
        return result;
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


    template <class T_Geometry>
    class PostedClassRoute : public PAMI::Geometry::ClassRouteId<T_Geometry>
    {
    public:
      typedef  PAMI::Geometry::ClassRouteId<T_Geometry> ClassRouteId;
      typedef  typename ClassRouteId::cr_event_function cr_event_function;
      typedef  PAMI::Geometry::Algorithm<T_Geometry>    Algorithm;
      typedef  PAMI::Device::Generic::GenericThread     GenericThread;

      static void _allreduce_done(pami_context_t context, void *cookie, pami_result_t result)
        {
          TRACE((stderr, "%p  Allreduce is Done!\n", cookie));
          PostedClassRoute  *classroute = (PostedClassRoute *)cookie;
          classroute->_done               = true;
        }
      
      static void _cr_done(pami_context_t context, void *cookie, pami_result_t result)
        {
          TRACE((stderr, "%p Classroute is done,barrier done, freeing\n", classroute));          
          PostedClassRoute *classroute = (PostedClassRoute*)cookie;
          classroute->_user_cb_done(context,
                                    classroute->_user_cookie,
                                    PAMI_SUCCESS);
          if(classroute->_free_bitmask)
            __global.heap_mm->free(classroute->_bitmask);
          __global.heap_mm->free(cookie);
        }

      static pami_result_t _do_classroute(pami_context_t context, void *cookie)
        {
          PostedClassRoute *classroute = (PostedClassRoute*)cookie;
          Context          *ctxt       = (Context*) context;

          if(classroute->_started == false)
          {
            // Start the Allreduce
            TRACE((stderr, "%p Starting Allreduce\n", cookie));
            classroute->startAllreduce(context, _allreduce_done, classroute);
            classroute->_started = true;
          }
          if(classroute->_done == true)
          {
            TRACE((stderr, "%p Classroute is done, Dequeueing with barrier\n", classroute));
            classroute->_result_cb_done(context,
                                        classroute->_result_cookie,
                                        classroute->_bitmask,
                                        classroute->_geometry,
                                        PAMI_SUCCESS);
            LAPIGeometry   *g = (LAPIGeometry *)classroute->_geometry;
            g->default_barrier(_cr_done, classroute, ctxt->getId(), context);
            return PAMI_SUCCESS;
          }
          else
          {
            TRACE((stderr, "%p classroute allreduce busy: %d %d\n",
                    classroute, classroute->_started, classroute->_done));
            return PAMI_EAGAIN;
          }
        }
    static void cr_func(pami_context_t  context,
                        void           *cookie,
                        uint64_t       *reduce_result,
                        LAPIGeometry   *g,
                        pami_result_t   result )
      {
        TRACE((stderr, "%p CR FUNC 1\n", cookie));
        int count = 1;
        PostedClassRoute<LAPIGeometry> *classroute = (PostedClassRoute<LAPIGeometry> *)cookie;
        classroute->_context->_pgas_collreg->receive_global(0,classroute->_geometry,
                                                            &reduce_result[0],1);
        classroute->_context->_p2p_ccmi_collreg->receive_global(0,classroute->_geometry,
                                                                &reduce_result[1],1);
        classroute->_context->_cau_collreg->analyze(0,classroute->_geometry,
                                                    &reduce_result[2],&count,1);
        classroute->_context->_pgas_collreg->analyze(0,classroute->_geometry,1,&reduce_result[2]);
      }
      
    static void create_classroute(pami_context_t context, void *cookie, pami_result_t result)
      {
        // Barrier is done, start phase 1
        TRACE((stderr, "%p create_classroute\n", cookie));
        PostedClassRoute<LAPIGeometry> *classroute = (PostedClassRoute<LAPIGeometry>*)cookie;
        classroute->start();
      }
    public:
      PostedClassRoute(Context             *context,
                       Algorithm           *ar_algo,
                       T_Geometry          *geometry,
                       uint64_t            *bitmask,
                       size_t               count,
                       cr_event_function    result_cb_done,
                       void                *result_cookie,
                       pami_event_function  user_cb_done,
                       void                *user_cookie,
                       bool                 free_bitmask = false):
        ClassRouteId(ar_algo, geometry, bitmask, count,
                     result_cb_done, result_cookie, user_cb_done,
                     user_cookie),
        _context(context),
        _started(false),
        _done(false),
        _free_bitmask(free_bitmask),
        _work(_do_classroute, this)
        {
        }
      inline void start()
        {
          _context->_devices->_generics[_context->getId()].postThread(&_work);
        }
      Context        *_context;
      volatile bool   _started;
      volatile bool   _done;
      bool            _free_bitmask;
      GenericThread   _work;
    };

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
        PAMI::Context * ctxt = (PAMI::Context*) context;
        ctxt->plock();

        LAPIGeometry              *new_geometry = NULL;
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
            to_reduce_count = 3 + 3*local_master_topology->size();
            rc = __global.heap_mm->memalign((void **)&to_reduce, 0,
                                            to_reduce_count * sizeof(uint64_t));
            PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc to_reduce");
            for(size_t n=0; n<_ncontexts; n++)
              {
                new_geometry->resetUEBarrier(); // Reset so pgas will select the UE barrier
		int nc = 0;
		int ncur = 0;
                _contexts[n]->_pgas_collreg->register_local(n,new_geometry,&to_reduce[0], ncur);
		nc+= ncur;
		ncur = 0;
                _contexts[n]->_p2p_ccmi_collreg->register_local(n,new_geometry,&to_reduce[1], ncur);
		nc+= ncur;
		ncur=0;
                _contexts[n]->_cau_collreg->analyze(n,new_geometry,&to_reduce[2],&ncur, 0);
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
            PostedClassRoute<LAPIGeometry> *cr[1];
	    rc = __global.heap_mm->memalign((void **)&cr[0], 0, sizeof(PostedClassRoute<LAPIGeometry>));
	    PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc PostedClassRoute<LAPIGeometry>");
            to_reduce[0] = 0;
            to_reduce[1] = 0;
            new(cr[0])PostedClassRoute<LAPIGeometry>(ctxt,
                                                     ar_algo,
                                                     new_geometry,
                                                     to_reduce,
                                                     to_reduce_count,
                                                     PostedClassRoute<LAPIGeometry>::cr_func,
                                                     cr[0],
                                                     fn,
                                                     cookie,
                                                     true);
            TRACE((stderr, "Allocated Classroutes:  %p %p\n", cr[0], cr[1]));
            if(bargeom)
              bargeom->default_barrier(PostedClassRoute<LAPIGeometry>::create_classroute, cr[0], ctxt->getId(), context);
            else
              new_geometry->ue_barrier(PostedClassRoute<LAPIGeometry>::create_classroute, cr[0], ctxt->getId(), context);
          }
        else
          {
            if(bargeom)
              bargeom->default_barrier(fn, cookie, ctxt->getId(), context);
            else
              {
                // Null parent and null new geometry?  Why are you here?
                ctxt->punlock();
                return PAMI_INVAL;
              }
          }
        ctxt->punlock();
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
        PAMI::Context    *ctxt    = (PAMI::Context *)context;
        ctxt->plock();
        // todo:  implement this routine
        LAPIGeometry              *new_geometry = NULL;
        uint64_t                  *to_reduce;
        uint                      to_reduce_count;
        pami_result_t             rc;
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
                                          task_count,
                                          tasks,
                                          &_geometry_map);

            PAMI::Topology *local_master_topology  = (PAMI::Topology *)new_geometry->getTopology(PAMI::Geometry::MASTER_TOPOLOGY_INDEX);
            to_reduce_count = 3 + 3*local_master_topology->size();
            rc = __global.heap_mm->memalign((void **)&to_reduce, 0,
                                            to_reduce_count * sizeof(uint64_t));
            PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc to_reduce");
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
                _contexts[n]->_cau_collreg->analyze(n,new_geometry,&to_reduce[2], &ncur, 0);
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
        if(new_geometry)
          {
            pami_algorithm_t  alg;
            pami_metadata_t   md;
            pami_result_t     rc;
            new_geometry->algorithms_info(PAMI_XFER_ALLREDUCE,
                                          &alg,
                                          &md,
                                          1,
                                          NULL,
                                          NULL,
                                          0,
                                          ctxt->getId());
            Geometry::Algorithm<LAPIGeometry> *ar_algo = (Geometry::Algorithm<LAPIGeometry> *)alg;
            PostedClassRoute<LAPIGeometry> *cr[1];
            rc = __global.heap_mm->memalign((void **)&cr[0], 0, sizeof(PostedClassRoute<LAPIGeometry>));
            PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc PostedClassRoute<LAPIGeometry>");
            to_reduce[0] = 0;
            to_reduce[1] = 0;
            new(cr[0])PostedClassRoute<LAPIGeometry>(ctxt,
                                                     ar_algo,
                                                     new_geometry,
                                                     to_reduce,
                                                     to_reduce_count,
                                                     PostedClassRoute<LAPIGeometry>::cr_func,
                                                     cr[0],
                                                     fn,
                                                     cookie,
                                                     true);
            if(bargeom)
              bargeom->default_barrier(PostedClassRoute<LAPIGeometry>::create_classroute, cr[0], ctxt->getId(), context);
            else
              new_geometry->ue_barrier(PostedClassRoute<LAPIGeometry>::create_classroute, cr[0], ctxt->getId(), context);
          }
        else
          {
            if(bargeom)
              bargeom->default_barrier(fn, cookie, ctxt->getId(), context);
            else
              {
                // Null parent and null new geometry?  Why are you here?
                ctxt->punlock();
                return PAMI_INVAL;
              }
          }
        ctxt->punlock();
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

      inline pami_result_t geometry_algorithms_num_impl (pami_geometry_t geometry,
                                                        pami_xfer_type_t colltype,
                                                        size_t *lists_lengths)
        {
          LAPIGeometry *_geometry = (LAPIGeometry*) geometry;
          return _geometry->algorithms_num(colltype, lists_lengths, 0);
        }

      inline pami_result_t geometry_algorithms_info_impl (pami_geometry_t    geometry,
                                                          pami_xfer_type_t   colltype,
                                                          pami_algorithm_t  *algs0,
                                                          pami_metadata_t   *mdata0,
                                                          size_t                num0,
                                                          pami_algorithm_t  *algs1,
                                                          pami_metadata_t   *mdata1,
                                                          size_t                num1)
      {
        LAPIGeometry *_geometry = (LAPIGeometry*) geometry;
        return _geometry->algorithms_info(colltype,
                                          algs0,
                                          mdata0,
                                          num0,
                                          algs1,
                                          mdata1,
                                          num1,
                                          0);
      }

    class geomCleanup
    {
    public:
      typedef  PAMI::Device::Generic::GenericThread     GenericThread;
      geomCleanup(LAPIGeometry        *geometry,
                  pami_event_function  user_done_cb,
                  void                *user_cookie):
        _geometry(geometry),
        _user_done_cb(user_done_cb),
        _user_cookie(user_cookie),
        _work(_do_geom_destroy, this)
        {
        }
      LAPIGeometry              *_geometry;
      pami_event_function        _user_done_cb;
      void                      *_user_cookie;
      GenericThread              _work;
    };
    static pami_result_t _do_geom_destroy(pami_context_t context, void *cookie)
      {
        PAMI::Context *ctxt         = (PAMI::Context*)context;
        PAMI::Client  *clnt         = (PAMI::Client*)ctxt->getClient();
        geomCleanup   *gc           = (geomCleanup*)cookie;
        LAPIGeometry  *g            = gc->_geometry;
        int commid                  = g->comm();
        
        g->~LAPIGeometry();        
        
        if(gc->_user_done_cb)
          gc->_user_done_cb(context,gc->_user_cookie,PAMI_SUCCESS);
        __global.heap_mm->free(g);
        __global.heap_mm->free(gc);
        return PAMI_SUCCESS;
      }
    static void geometry_destroy_done_fn(pami_context_t   context,
                                         void           * cookie,
                                         pami_result_t    result)
      {
        // Post this to the generic device to not re-enter lapi dispatcher
        // when destroying CAU
        PAMI::Context *ctxt         = (PAMI::Context*)context;
        geomCleanup   *gc           = (geomCleanup*)cookie;
        ctxt->_devices->_generics[ctxt->getId()].postThread(&gc->_work);
      }

    // This code destroys a geometry.
    // For this context, the destroy of a geometry can have many race conditions
    // In particular, shared memory segments must be freed in the geometry destroy.
    // Other tasks may still be using the shared memory control structures, while
    // the leader allocator task is freeing the structure.
    // Therefore,  a barrier is necessary to ensure that all tasks are done using the strucure.

    // We cannot free the geometry in the advance loop because CAU group creation calls advance
    // and the geometry is destroyed in the geometry destructor.
    // In addition, we must guarantee that an incoming geometry create request (via unexpected barrier)
    // with the same geometry id as the geometry being destroyed cannot be looked up in the
    // geometry map object, that maps geometry id's to geometry objects.

    // Note:  This operation is **blocking**, which may be a violation of the API for geometry destroy.
    // Some clarification of the API is necessary.  The code below assumes that we can defer the geometry
    // destruction by not blocking in advance.  However, we cannot destroy the geometry because
    // the barrier uses the rank list, which is provided by the user.

    // Our options are
    // a)  copy the rank list(topology), and create a protocol that uses the new ranklist/native interfaces
    // b)  block in this call

    // We chose to block because of it's simplicity.

    // The flow is commented below:

    inline pami_result_t geometry_destroy_impl (pami_geometry_t      geometry,
                                                pami_context_t       context,
                                                pami_event_function  fn,
                                                void                *cookie)
      {
        _contexts[0]->plock();

        // Create a geometry cleanup object
        // 1)  Cleanup object has a flag, the geometry, and the allocation list
        //     for the object.  We keep this code so that we can eventually
        //     make this non-blocking if required.
        LAPIGeometry* g = (LAPIGeometry*)geometry;
        geomCleanup *gc  = (geomCleanup*)__global.heap_mm->malloc(sizeof(geomCleanup));
        new(gc)geomCleanup(g,fn,cookie);
        // Start the ue barrier, the barrier callback will decrement the counter
        // indicating the barrier is complete.  It will also enqueue
        // the cleanup object for cleanup (for non-blocking).

        // It also clears the geometry map for the current id so an incoming
        // ue barrier with the same geometry id will not reference the current
        // object.  This really happens, because advance may process multiple messages
        // and the timing is highly dependent on the exit fromt eh barrier....it can
        // be "staggered" across tasks, meaning that some task may race ahead and
        // send a subsequent geometry create packet.

        // We do not destroy the geometry object in the done callback because
        // The destroy is dependent on the dispatcher to de-regester a cau group.
        g->ue_barrier(geometry_destroy_done_fn, gc, 0, _contexts[0]);
        pami_result_t rc;
        _contexts[0]->advance(10,rc);
        
        _contexts[0]->punlock();
        return PAMI_SUCCESS;
      }

    inline pami_geometry_t mapidtogeometry_impl (int comm)
      {
        std::map<unsigned, pami_geometry_t>::iterator iter = _geometry_map.find(comm);
        if(iter == _geometry_map.end())
          return (pami_geometry_t)NULL;
        else
          return iter->second;
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

    inline double wtime_impl ()
      {
          if (_Lapi_env.use_hfi) {
              LapiImpl::Client *lapi_client = (LapiImpl::Client *)_lapiClient;
              return  lapi_client->GetWtime();
          } else {
              return __global.time.time();
          }
      }

    inline unsigned long long wtimebase_impl ()
      {
          if (_Lapi_env.use_hfi) {
              LapiImpl::Client *lapi_client = (LapiImpl::Client *)_lapiClient;
              return lapi_client->GetWtimebase();
          } else {
              return __global.time.timebase();
          }
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
        if(((LapiImpl::Context*)_contexts[0])->coll_use_shm)
          {
            char   shmemfile[PAMI::Memory::MMKEYSIZE];
            size_t bytes     =  PAMI::LAPICSMemoryManager::_size +      // Shmem used by collshm
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
    char                                         _lapiClient[sizeof(LapiImpl::Client)];

    // Array of PAMI Contexts associated with this Client
    PAMI::Context                               *_contexts[128];

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
    MatchQueue                                   _ueb_queue;

    // Active contexts in the Client
    PamiActiveContexts                           _active_contexts;

    // Unique, shared cau index to identify cau resources across jobs
    unsigned                                     _cau_uniqifier;
  }; // end class PAMI::Client
}; // end namespace PAMI

#endif
// __pami_lapi_lapiclient_h__
