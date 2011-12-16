/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/geometry/Geometry.h
 * \brief ???
 */

#ifndef __algorithms_geometry_Geometry_h__
#define __algorithms_geometry_Geometry_h__

#include "Global.h"
#include "Topology.h"
#include "Mapping.h"
#include "algorithms/interfaces/GeometryInterface.h"
#include "algorithms/geometry/Algorithm.h"
#include "util/common.h"
#include <map>
#include <list>

#define DO_DEBUGg(x) //x

#undef TRACE_ERR
#define TRACE_ERR(x) //fprintf x

#undef TRACE_ERR2
#define TRACE_ERR2(x) //fprintf x

#include "algorithms/geometry/UnexpBarrierQueueElement.h"
#include "components/atomic/native/NativeCounter.h"
#include "components/memory/MemoryAllocator.h"
typedef PAMI::Counter::Native GeomCompCtr;

#include "util/trace.h"
#undef  DO_TRACE_ENTEREXIT
#define DO_TRACE_ENTEREXIT 0
#undef  DO_TRACE_DEBUG    
#define DO_TRACE_DEBUG     0

namespace PAMI
{
  namespace Geometry
  {
    static inline void*** allocKVS(size_t nrows, size_t ncols)
    {
      size_t             i;
      pami_result_t    rc;
      void          ***array;
      rc = __global.heap_mm->memalign((void**)&array,0,nrows*sizeof(void**));
      PAMI_assertf(array && (rc == PAMI_SUCCESS), "Unable to allocate KVS\n");
      rc = __global.heap_mm->memalign((void**)&array[0],0,nrows*ncols*sizeof(void*));
      PAMI_assertf(array[0] && (rc == PAMI_SUCCESS), "Unable to allocate KVS row\n");
      memset(array[0], 0, nrows*ncols*sizeof(void*));
      for (i = 0; i < nrows; i++)
        array[i]   = array[0] + i * ncols;
      return array;
    }
    static inline void freeKVS(void*** array, size_t nrows)
    {
      return;
      for(size_t i=0; i<nrows; i++)
        __global.heap_mm->free(array[i]);
      __global.heap_mm->free(array);
    }

    class Common :
    public Geometry<PAMI::Geometry::Common>
    {
    public:
      static  Common * get (int ctxt, int geometryID)
      {
        assert(0);
        //assert(_ncontexts * teamID + ctxt < MAX_COMMS);
        // return _instances[_ncontexts * teamID + ctxt];

        // #warning "Return Geometry map"
        Common * g =NULL;
        return g;
      }



      inline ~Common()
      {
        TRACE_FN_ENTER();
        freeAllocations_impl();
        TRACE_FN_EXIT();
      }

      inline Common(pami_client_t                     client,
                    Geometry<PAMI::Geometry::Common> *parent,
                    Mapping                          *mapping,
                    unsigned                          comm,
                    pami_task_t                       nranks,
                    pami_task_t                      *ranks,
                    std::map<unsigned, pami_geometry_t> *geometry_map,
                    size_t                               context_offset,
                    size_t                               ncontexts):
      Geometry<PAMI::Geometry::Common>(parent,
                                       mapping,
                                       comm,
                                       nranks,
                                       ranks),
      _allreduces(ncontexts),
      _broadcasts(ncontexts),
      _reduces(ncontexts),
      _allgathers(ncontexts),
      _allgathervs(ncontexts),
      _allgatherv_ints(ncontexts),
      _scatters(ncontexts),
      _scattervs(ncontexts),
      _scatterv_ints(ncontexts),
      _gathers(ncontexts),
      _gathervs(ncontexts),
      _gatherv_ints(ncontexts),
      _alltoalls(ncontexts),
      _alltoallvs(ncontexts),
      _alltoallv_ints(ncontexts),
      _reduce_scatters(ncontexts),
      _ambroadcasts(ncontexts),
      _amscatters(ncontexts),
      _amgathers(ncontexts),
      _amreduces(ncontexts),
      _scans(ncontexts),
      _barriers(ncontexts),
      _ue_barrier(NULL),
      _commid(comm),
      _client(client),
      _rank(mapping->task()),
      _ranks_malloc(false),
      _ranks(ranks),
      _geometry_map(geometry_map),
      _masterRank(-1),
      _checkpointed(false),
      _cb_result(PAMI_EAGAIN)
      {
        TRACE_ERR((stderr, "<%p>Common(ranklist)\n", this));
        // this creates the topology including all subtopologies
        new(&_topos[DEFAULT_TOPOLOGY_INDEX]) PAMI::Topology(_ranks, nranks);
        //fprintf(stderr, "<%u><%p> geometry our _ranks %p\n", __LINE__,this,_ranks);
        buildSpecialTopologies();

        // Initialize remaining members
        (*_geometry_map)[_commid] = this;

        for(size_t n=0; n<MAX_CONTEXTS; n++)
          {
            _allreduce[n][0] = _allreduce[n][1] = NULL;
            _allreduce_async_mode[n]      = 1;
            _allreduce_iteration[n]       = 0;
          }

        _cb_done = (pami_callback_t)
        {
          NULL, NULL
        };

        _kvcstore = allocKVS(NUM_CKEYS,ncontexts);

        size_t nctxt     = (PAMI_ALL_CONTEXTS == context_offset)?_ncontexts:1;
        size_t start_off = (PAMI_ALL_CONTEXTS == context_offset)? 0 : context_offset;
        pami_result_t rc = __global.heap_mm->memalign((void **)&_ue_barrier,
                                                      0,
                                                      nctxt*sizeof(*_ue_barrier));
        (void)rc; //unused warnings in assert case
        PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc _temp_topo");

        // Allocate per context UE queues
        rc = __global.heap_mm->memalign((void **)&_ue,
                                        0,
                                        nctxt*sizeof(MatchQueue));
        PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc _temp_topo");

        // Allocate per context Posted queues
        rc = __global.heap_mm->memalign((void **)&_post,
                                        0,
                                        nctxt*sizeof(MatchQueue));
        PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc _temp_topo");


        for(size_t n=start_off; n<nctxt; n++)
        {
          new(&_ue_barrier[n]) Algorithm<PAMI::Geometry::Common>(NULL,NULL);
          new(&_ue[n]) MatchQueue();
          new(&_post[n]) MatchQueue();
          resetUEBarrier_impl(n);
        }
      }

        /**
            Construct a geometry based on a list of endpoints
         */
      inline Common (pami_client_t                        client,
                     Geometry<PAMI::Geometry::Common>    *parent,
                     Mapping                             *mapping,
                     unsigned                             comm,
                      pami_endpoint_t                   neps,
                      pami_endpoint_t                  *eps,
                      std::map<unsigned, pami_geometry_t> *geometry_map,
                      size_t                            context_offset,
                      size_t                            ncontexts,
                      bool):
      Geometry<PAMI::Geometry::Common>(parent,
                                       mapping,
                                       comm,
                                       neps,
                                       eps),
       _allreduces(ncontexts),
      _broadcasts(ncontexts),
      _reduces(ncontexts),
      _allgathers(ncontexts),
      _allgathervs(ncontexts),
      _allgatherv_ints(ncontexts),
      _scatters(ncontexts),
      _scattervs(ncontexts),
      _scatterv_ints(ncontexts),
      _gathers(ncontexts),
      _gathervs(ncontexts),
      _gatherv_ints(ncontexts),
      _alltoalls(ncontexts),
      _alltoallvs(ncontexts),
      _alltoallv_ints(ncontexts),
      _reduce_scatters(ncontexts),
      _ambroadcasts(ncontexts),
      _amscatters(ncontexts),
      _amgathers(ncontexts),
      _amreduces(ncontexts),
      _scans(ncontexts),
      _barriers(ncontexts),
      _ue_barrier(NULL),
      _commid(comm),
      _client(client),
      _rank(mapping->task()),
      _ranks_malloc(false),
      _endpoints(eps),
      _geometry_map(geometry_map),
      _masterRank(-1),
      _checkpointed(false),
      _cb_result(PAMI_EAGAIN)
      {
        TRACE_ERR((stderr, "<%p>Common(endpointlist)\n", this));
        // this creates the topology including all subtopologies
        new(&_topos[DEFAULT_TOPOLOGY_INDEX]) PAMI::Topology(_endpoints, neps, true);
        //fprintf(stderr, "<%u><%p> geometry our _ranks %p\n", __LINE__,this, _endpoints);
        buildSpecialTopologies();

        // Initialize remaining members

        (*_geometry_map)[_commid] = this;

        for(size_t n=0; n<MAX_CONTEXTS; n++)
          {
            _allreduce[n][0] = _allreduce[n][1] = NULL;
            _allreduce_async_mode[n]      = 1;
            _allreduce_iteration[n]       = 0;
          }

        _cb_done = (pami_callback_t)
          {
          NULL, NULL
        };

        _kvcstore = allocKVS(NUM_CKEYS,ncontexts);

        size_t nctxt     = (PAMI_ALL_CONTEXTS == context_offset)?_ncontexts:1;
        size_t start_off = (PAMI_ALL_CONTEXTS == context_offset)? 0 : context_offset;
        pami_result_t rc = __global.heap_mm->memalign((void **)&_ue_barrier,
                                                      0,
                                                      nctxt*sizeof(*_ue_barrier));
        (void)rc; //unused warnings in assert case
        PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc _temp_topo");

        // Allocate per context UE queues
        rc = __global.heap_mm->memalign((void **)&_ue,
                                        0,
                                        nctxt*sizeof(MatchQueue));
        PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc _temp_topo");

        // Allocate per context Posted queues
        rc = __global.heap_mm->memalign((void **)&_post,
                                        0,
                                        nctxt*sizeof(MatchQueue));
        PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc _temp_topo");


        for(size_t n=start_off; n<nctxt; n++)
        {
          new(&_ue_barrier[n]) Algorithm<PAMI::Geometry::Common>(NULL,NULL);
          new(&_ue[n]) MatchQueue();
          new(&_post[n]) MatchQueue();
          resetUEBarrier_impl(n);
          }
        }

      inline Common (pami_client_t                        client,
                     Geometry<PAMI::Geometry::Common>    *parent,
                     Mapping                             *mapping,
                     unsigned                             comm,
                     int                                  numranges,
                     pami_geometry_range_t                rangelist[],
                     std::map<unsigned, pami_geometry_t> *geometry_map,
                     size_t                               context_offset,
                     size_t                               ncontexts):
      Geometry<PAMI::Geometry::Common>(parent,
                                       mapping,
                                       comm,
                                       numranges,
                                       rangelist),
      _allreduces(ncontexts),
      _broadcasts(ncontexts),
      _reduces(ncontexts),
      _allgathers(ncontexts),
      _allgathervs(ncontexts),
      _allgatherv_ints(ncontexts),
      _scatters(ncontexts),
      _scattervs(ncontexts),
      _scatterv_ints(ncontexts),
      _gathers(ncontexts),
      _gathervs(ncontexts),
      _gatherv_ints(ncontexts),
      _alltoalls(ncontexts),
      _alltoallvs(ncontexts),
      _alltoallv_ints(ncontexts),
      _reduce_scatters(ncontexts),
      _ambroadcasts(ncontexts),
      _amscatters(ncontexts),
      _amgathers(ncontexts),
      _amreduces(ncontexts),
      _scans(ncontexts),
      _barriers(ncontexts),
      _ue_barrier(NULL),
      _commid(comm),
      _client(client),
      _rank(mapping->task()),
      _ranks_malloc(false),
      _ranks(NULL),
      _geometry_map(geometry_map),
      _masterRank(-1),
      _checkpointed(false),
      _cb_result(PAMI_EAGAIN),
      _context_offset(context_offset),
      _ncontexts(ncontexts)
      {
        TRACE_ERR((stderr, "<%p>Common(ranges)\n", this));
        pami_result_t rc;
          if (numranges == 1)
        {
          // this creates the topology from a (single) range
          if(_context_offset==0) {
          new(&_topos[DEFAULT_TOPOLOGY_INDEX]) PAMI::Topology(rangelist[0].lo, rangelist[0].hi);
        }
          else {
            new(&_topos[DEFAULT_TOPOLOGY_INDEX]) PAMI::Topology(rangelist[0].lo, rangelist[0].hi,_context_offset, _ncontexts);
          }
        }
        else // build a rank list from N ranges
        {
          pami_task_t nranks = 0;
          int i, j, k;

          for (i = 0; i < numranges; i++)
            nranks += (rangelist[i].hi - rangelist[i].lo + 1);

          _ranks_malloc = true;
          rc = __global.heap_mm->memalign((void **)&_ranks, 0, nranks * sizeof(pami_task_t));
          //fprintf(stderr, "<%u><%p> geometry ranks_malloc true - %p\n", __LINE__,this,_ranks);
          PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc _ranks");

          for (k = 0, i = 0; i < numranges; i++)
          {
            int size = rangelist[i].hi - rangelist[i].lo + 1;

            for (j = 0; j < size; j++, k++)
              _ranks[k] = rangelist[i].lo + j;
          }

          // this creates the topology 
          if(_context_offset==0) {
          new(&_topos[DEFAULT_TOPOLOGY_INDEX]) PAMI::Topology(_ranks, nranks);
        }
          else{
            new(&_topos[DEFAULT_TOPOLOGY_INDEX]) PAMI::Topology(_ranks, nranks,_context_offset,_ncontexts);
          }
        }

        buildSpecialTopologies();

        if ((_topos[LIST_TOPOLOGY_INDEX].type() == PAMI_LIST_TOPOLOGY) && 
            (_ranks_malloc == false)) /* Don't overwrite our original _ranks */
        {
          pami_result_t rc = PAMI_SUCCESS;
          rc = _topos[LIST_TOPOLOGY_INDEX].rankList(&_ranks);
          //fprintf(stderr, "<%u><%p> geometry our _ranks %p\n", __LINE__,this,_ranks);
          PAMI_assert(rc == PAMI_SUCCESS);
        }
        // Initialize remaining members

        (*_geometry_map)[_commid] = this;

        for(size_t n=0; n<MAX_CONTEXTS; n++)
          {
            _allreduce[n][0] = _allreduce[n][1] = NULL;
            _allreduce_async_mode[n]      = 1;
            _allreduce_iteration[n]       = 0;
          }

        _cb_done = (pami_callback_t)
        {
          NULL, NULL
        };

        _kvcstore = allocKVS(NUM_CKEYS,ncontexts);

        size_t nctxt     = (PAMI_ALL_CONTEXTS == context_offset)?_ncontexts:1;
        size_t start_off = (PAMI_ALL_CONTEXTS == context_offset)? 0 : context_offset;
        rc               = __global.heap_mm->memalign((void **)&_ue_barrier,
                                                      0,
                                                      nctxt*sizeof(*_ue_barrier));
        PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc _temp_topo");


        // Allocate per context UE queues
        rc = __global.heap_mm->memalign((void **)&_ue,
                                        0,
                                        nctxt*sizeof(MatchQueue));
        PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc _temp_topo");

        // Allocate per context Posted queues
        rc = __global.heap_mm->memalign((void **)&_post,
                                        0,
                                        nctxt*sizeof(MatchQueue));
        PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc _temp_topo");


        for(size_t n=start_off; n<nctxt; n++)
        {
          new(&_ue_barrier[n]) Algorithm<PAMI::Geometry::Common>(NULL,NULL);
          new(&_ue[n]) MatchQueue();
          new(&_post[n]) MatchQueue();
          resetUEBarrier_impl(n);
        }
      }

      inline Common (pami_client_t                    client,
                     Geometry<PAMI::Geometry::Common> *parent,
                     Mapping                         *mapping,
                     unsigned                         comm,
                     PAMI::Topology                  *topology,
                     std::map<unsigned, pami_geometry_t> *geometry_map,
                     size_t                               context_offset,
                     size_t                               ncontexts):
      Geometry<PAMI::Geometry::Common>(parent,
                                       mapping,
                                       comm,
                                       topology),
      _allreduces(ncontexts),
      _broadcasts(ncontexts),
      _reduces(ncontexts),
      _allgathers(ncontexts),
      _allgathervs(ncontexts),
      _allgatherv_ints(ncontexts),
      _scatters(ncontexts),
      _scattervs(ncontexts),
      _scatterv_ints(ncontexts),
      _gathers(ncontexts),
      _gathervs(ncontexts),
      _gatherv_ints(ncontexts),
      _alltoalls(ncontexts),
      _alltoallvs(ncontexts),
      _alltoallv_ints(ncontexts),
      _reduce_scatters(ncontexts),
      _ambroadcasts(ncontexts),
      _amscatters(ncontexts),
      _amgathers(ncontexts),
      _amreduces(ncontexts),
      _scans(ncontexts),
      _barriers(ncontexts),
      _ue_barrier(NULL),
      _commid(comm),
      _client(client),
      _rank(mapping->task()),
      _ranks_malloc(false),
      _ranks(NULL), 
      _geometry_map(geometry_map),
      _masterRank(-1),
      _checkpointed(false),
      _cb_result(PAMI_EAGAIN)
      {
        TRACE_ERR((stderr, "<%p>Common(topology)\n", this));

        _topos[DEFAULT_TOPOLOGY_INDEX] = *topology;

        if (_topos[DEFAULT_TOPOLOGY_INDEX].type() == PAMI_LIST_TOPOLOGY)
        {  
          _topos[DEFAULT_TOPOLOGY_INDEX].rankList(&_ranks);
          //fprintf(stderr, "<%u><%p> geometry our _ranks %p\n", __LINE__,this,_ranks);
        }
        buildSpecialTopologies();

        // Initialize remaining members

        (*_geometry_map)[_commid] = this;

        for(size_t n=0; n<MAX_CONTEXTS; n++)
          {
            _allreduce[n][0] = _allreduce[n][1] = NULL;
            _allreduce_async_mode[n]      = 1;
            _allreduce_iteration[n]       = 0;
          }

        _cb_done = (pami_callback_t)
        {
          NULL, NULL
        };

        _kvcstore = allocKVS(NUM_CKEYS,ncontexts);

        size_t nctxt     = (PAMI_ALL_CONTEXTS == context_offset)?_ncontexts:1;
        size_t start_off = (PAMI_ALL_CONTEXTS == context_offset)? 0 : context_offset;
        pami_result_t rc = __global.heap_mm->memalign((void **)&_ue_barrier,
                                                      0,
                                                      nctxt*sizeof(*_ue_barrier));
        PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc _temp_topo");
        (void)rc; //to avoid unused warnings in non-assert

        // Allocate per context UE queues
        rc = __global.heap_mm->memalign((void **)&_ue,
                                        0,
                                        nctxt*sizeof(MatchQueue));
        PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc _temp_topo");

        // Allocate per context Posted queues
        rc = __global.heap_mm->memalign((void **)&_post,
                                        0,
                                        nctxt*sizeof(MatchQueue));
        PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc _temp_topo");


        for(size_t n=start_off; n<nctxt; n++)
        {
          new(&_ue_barrier[n]) Algorithm<PAMI::Geometry::Common>(NULL,NULL);
          new(&_ue[n]) MatchQueue();
          new(&_post[n]) MatchQueue();
          resetUEBarrier_impl(n);
        }
      }

      void                             buildSpecialTopologies()
      {
        // build local and global topos
        DO_DEBUGg(pami_task_t *list = NULL);
        DO_DEBUGg(TRACE_ERR((stderr,"(%u)buildSpecialTopologies() DEFAULT_TOPOLOGY rankList %p\n", _topos[DEFAULT_TOPOLOGY_INDEX].rankList(&list), list)));
        DO_DEBUGg(for (unsigned j = 0; j < _topos[DEFAULT_TOPOLOGY_INDEX].size(); ++j) TRACE_ERR((stderr, "buildSpecialTopologies() DEFAULT_TOPOLOGY[%u]=%zu, size %zu\n", j, (size_t)_topos[DEFAULT_TOPOLOGY_INDEX].index2Rank(j), _topos[DEFAULT_TOPOLOGY_INDEX].size())));
        _topos[DEFAULT_TOPOLOGY_INDEX].subTopologyNthGlobal(&_topos[MASTER_TOPOLOGY_INDEX], 0);
        _topos[DEFAULT_TOPOLOGY_INDEX].subTopologyLocalToMe(&_topos[LOCAL_TOPOLOGY_INDEX]);
        _topos[MASTER_TOPOLOGY_INDEX].subTopologyLocalToMe(&_topos[LOCAL_MASTER_TOPOLOGY_INDEX]);
        // Find master participant on the tree/cau network
        size_t            num_master_tasks = _topos[MASTER_TOPOLOGY_INDEX].size();
        size_t            num_local_tasks = _topos[LOCAL_TOPOLOGY_INDEX].size();

        _masterRank =(pami_task_t) -1;
        for (size_t k = 0; k < num_master_tasks; k++)
          for (size_t j = 0; j < num_local_tasks; j++)
            if (_topos[MASTER_TOPOLOGY_INDEX].index2Rank(k) == _topos[LOCAL_TOPOLOGY_INDEX].index2Rank(j))
            {
              _masterRank = _topos[LOCAL_TOPOLOGY_INDEX].index2Rank(j);
              break;
            };
        DO_DEBUGg(TRACE_ERR((stderr,"(%u)buildSpecialTopologies() MASTER_TOPOLOGY_INDEX rankList %p\n", _topos[MASTER_TOPOLOGY_INDEX].rankList(&list), list)));
        DO_DEBUGg(for (unsigned j = 0; j < _topos[MASTER_TOPOLOGY_INDEX].size(); ++j) TRACE_ERR((stderr, "buildSpecialTopologies() MASTER_TOPOLOGY[%u]=%zu, size %zu\n", j, (size_t)_topos[MASTER_TOPOLOGY_INDEX].index2Rank(j), _topos[MASTER_TOPOLOGY_INDEX].size())));
        DO_DEBUGg(TRACE_ERR((stderr,"(%u)buildSpecialTopologies() LOCAL_TOPOLOGY rankList %p\n", _topos[LOCAL_TOPOLOGY_INDEX].rankList(&list), list)));
        DO_DEBUGg(for (unsigned j = 0; j < _topos[LOCAL_TOPOLOGY_INDEX].size(); ++j) TRACE_ERR((stderr, "buildSpecialTopologies() LOCAL_TOPOLOGY[%u]=%zu, size %zu\n", j, (size_t)_topos[LOCAL_TOPOLOGY_INDEX].index2Rank(j), _topos[LOCAL_TOPOLOGY_INDEX].size())));
        TRACE_ERR((stderr, "buildSpecialTopologies() _masterRank %u\n", _masterRank));

        // Create a coordinate topo (may be EMPTY)
        _topos[COORDINATE_TOPOLOGY_INDEX] = _topos[DEFAULT_TOPOLOGY_INDEX];

        if (_topos[COORDINATE_TOPOLOGY_INDEX].type() != PAMI_COORD_TOPOLOGY)
          _topos[COORDINATE_TOPOLOGY_INDEX].convertTopology(PAMI_COORD_TOPOLOGY);

        DO_DEBUGg(TRACE_ERR((stderr,"(%u)buildSpecialTopologies() COORDINATE_TOPOLOGY rankList %p\n", _topos[COORDINATE_TOPOLOGY_INDEX].rankList(&list), list)));
        DO_DEBUGg(for (unsigned j = 0; j < _topos[COORDINATE_TOPOLOGY_INDEX].size(); ++j) TRACE_ERR((stderr, "buildSpecialTopologies() COORDINATE_TOPOLOGY[%u]=%zu, size %zu\n", j, (size_t)_topos[COORDINATE_TOPOLOGY_INDEX].index2Rank(j), _topos[COORDINATE_TOPOLOGY_INDEX].size())));

        // If we already have a rank list, set the special topology, otherwise 
        // leave it EMPTY unless needed because it will require a new rank list allocation
        if (_topos[DEFAULT_TOPOLOGY_INDEX].type() != PAMI_LIST_TOPOLOGY)
          new(&_topos[LIST_TOPOLOGY_INDEX]) PAMI::Topology();
        else // Default is a list topology, use the same ranklist storage
        {
          pami_task_t  nranks = _topos[DEFAULT_TOPOLOGY_INDEX].size();
          pami_task_t *ranks;
          _topos[DEFAULT_TOPOLOGY_INDEX].rankList(&ranks);
          new(&_topos[LIST_TOPOLOGY_INDEX]) PAMI::Topology(ranks, nranks);
        }
      }

      inline pami_topology_t*          getTopology_impl(topologyIndex_t topo_num)
      {
        return(pami_topology_t*)&_topos[topo_num];
      }

      inline void                      setAsyncAllreduceMode_impl(size_t   context_id,
                                                                  unsigned value)
      {
        _allreduce_async_mode[context_id] = value;
      }
      inline unsigned                  getAsyncAllreduceMode_impl(size_t context_id)
      {
        return _allreduce_async_mode[context_id];
      }
      inline unsigned                  incrementAllreduceIteration_impl(size_t context_id)
      {
        _allreduce_iteration[context_id] ^= _allreduce_async_mode[context_id]; // "increment" with defined mode
        return _allreduce_iteration[context_id];
      }

      inline unsigned                  comm_impl()
      {
        return _commid;
      }

      inline unsigned                  getAllreduceIteration_impl(size_t context_id)
      {
        return _allreduce_iteration[context_id];
      }
      inline void                      freeAllocations_impl()
      {
        TRACE_FN_ENTER();
        //fprintf(stderr, "<%u><%p>geometry freeallocations\n", __LINE__,this);
        int sz = _cleanupFcns.size();
        for (int i=0; i<sz; i++)
        {
          pami_event_function  fn = _cleanupFcns.front();  _cleanupFcns.pop_front();
          void                *cd = _cleanupDatas.front(); _cleanupDatas.pop_front();
          if (fn) fn(NULL, cd, PAMI_SUCCESS);
        }

        //fprintf(stderr, "<%u><%p>geometry free ranklist now %p\n", __LINE__,this,_ranks);
        if (_ranks_malloc) __global.heap_mm->free(_ranks);

        _ranks = NULL;
        _ranks_malloc = false;
        __global.heap_mm->free(_ue_barrier);
        __global.heap_mm->free(_ue);
        __global.heap_mm->free(_post);
        freeKVS(_kvcstore, NUM_CKEYS);

        (*_geometry_map)[_commid] = NULL;

        TRACE_FN_EXIT();
        return;
      }
      inline MatchQueue               &asyncCollectivePostQ_impl(size_t context_id)
      {
        return _post[context_id];
      }
      inline MatchQueue               &asyncCollectiveUnexpQ_impl(size_t context_id)
      {
        return _ue[context_id];
      }

      inline COMPOSITE_TYPE            getAllreduceComposite_impl(size_t   context_id,
                                                                  unsigned i)
      {
        return _allreduce[context_id][i];
      }
      inline void                      setAllreduceComposite_impl(size_t         context_id,
                                                                  COMPOSITE_TYPE c)
      {
        _allreduce[context_id][_allreduce_iteration[context_id]] = c;

        if (c) incrementAllreduceIteration_impl(context_id);
      }
      inline void                      setAllreduceComposite_impl(size_t         context_id,
                                                                  COMPOSITE_TYPE c,
                                                                  unsigned       i)
      {
        _allreduce[context_id][i] = c;
      }

      inline COMPOSITE_TYPE            getAllreduceComposite_impl(size_t context_id)
      {
        return _allreduce[context_id][_allreduce_iteration[context_id]];
      }

      inline void                      processUnexpBarrier_impl (MatchQueue * ueb_queue,
                                                                 MemoryAllocator < sizeof(PAMI::Geometry::UnexpBarrierQueueElement), 16 > *ueb_allocator)
      {
        UnexpBarrierQueueElement *ueb = NULL;

        while ((ueb = (UnexpBarrierQueueElement *)ueb_queue->findAndDelete(_commid)) != NULL)
        {
          CCMI::Executor::Composite *c = (CCMI::Executor::Composite *) getKey(ueb->getContextId(),
                                                                              (ckeys_t)ueb->getAlgorithm());
          c->notifyRecv (ueb->getComm(), ueb->getInfo(), NULL, NULL, NULL);
          ueb_allocator->returnObject(ueb);
        }
      }

      inline pami_task_t               size_impl(void)
      {
        return _topos[DEFAULT_TOPOLOGY_INDEX].size();
      }
      inline pami_task_t               rank_impl(void)
      {
        return _rank;
      }

      inline size_t ordinal_impl (pami_endpoint_t ep)
      {
        return _topos[DEFAULT_TOPOLOGY_INDEX].endpoint2Index(ep);
      }
      inline pami_endpoint_t endpoint_impl (size_t ordinal)
      {
        return _topos[DEFAULT_TOPOLOGY_INDEX].index2Endpoint(ordinal);
      }

      inline void  setDispatch_impl(size_t key, DispatchInfo *value)
      {
        _dispatch[key] = *value;
      }

      inline DispatchInfo  * getDispatch_impl(size_t key)
      {
        std::map<size_t, DispatchInfo>::iterator iter = _dispatch.find(key);

        if(unlikely(iter == _dispatch.end()))
        {
          return (DispatchInfo *)NULL;
        }

        return &iter->second;
      }

      inline void  * getKey_impl(size_t context_id, ckeys_t key) {
        PAMI_assert(key < NUM_CKEYS);
        PAMI_assert(context_id != -1UL);
        void * value = _kvcstore[key][context_id];
        TRACE_ERR((stderr, "<%p>Common::getCKey(k=%d, val=%p, ctxt=%ld)\n",
                   this, key, value,context_id));
        return value;
      }

      inline void                      setKey_impl(size_t context_id, ckeys_t key, void*value)
      {
        PAMI_assert(key < NUM_CKEYS);
        PAMI_assert(context_id != -1UL);
        TRACE_ERR((stderr, "<%p>Common::setCKey(k=%d, v=%p,ctxt=%ld)\n", this, key, value,context_id));
        _kvcstore[key][context_id] = value;
      }


      inline AlgoLists<Geometry<PAMI::Geometry::Common> > * algorithms_get_lists(pami_xfer_type_t  colltype)
      {
        TRACE_ERR((stderr, "<%p>Common::algorithms_get_lists(%u)\n", this, colltype));
        switch (colltype)
        {
        case PAMI_XFER_BROADCAST:
          return &_broadcasts;
          break;
        case PAMI_XFER_ALLREDUCE:
          return &_allreduces;
          break;
        case PAMI_XFER_REDUCE:
          return &_reduces;
          break;
        case PAMI_XFER_ALLGATHER:
          return &_allgathers;
          break;
        case PAMI_XFER_ALLGATHERV:
          return &_allgathervs;
          break;
        case PAMI_XFER_ALLGATHERV_INT:
          return &_allgatherv_ints;
          break;
        case PAMI_XFER_SCATTER:
          return &_scatters;
          break;
        case PAMI_XFER_SCATTERV:
          return &_scattervs;
          break;
        case PAMI_XFER_SCATTERV_INT:
          return &_scatterv_ints;
          break;
        case PAMI_XFER_GATHER:
          return &_gathers;
          break;
        case PAMI_XFER_GATHERV:
          return &_gathervs;
          break;
        case PAMI_XFER_GATHERV_INT:
          return &_gatherv_ints;
          break;
        case PAMI_XFER_BARRIER:
          return &_barriers;
          break;
        case PAMI_XFER_ALLTOALL:
          return &_alltoalls;
          break;
        case PAMI_XFER_ALLTOALLV:
          return &_alltoallvs;
          break;
        case PAMI_XFER_ALLTOALLV_INT:
          return &_alltoallv_ints;
          break;
        case PAMI_XFER_SCAN:
          return &_scans;
          break;
        case PAMI_XFER_AMBROADCAST:
          return &_ambroadcasts;
          break;
        case PAMI_XFER_AMSCATTER:
          return &_amscatters;
          break;
        case PAMI_XFER_AMGATHER:
          return &_amgathers;
          break;
        case PAMI_XFER_AMREDUCE:
          return &_amreduces;
          break;
        case PAMI_XFER_REDUCE_SCATTER:
          return &_reduce_scatters;
          break;
        default:
          PAMI_abortf("colltype %u\n", colltype);
          return NULL;
          break;
        }
      }

      inline pami_result_t             addCollective_impl(pami_xfer_type_t                            colltype,
                                                          Factory          *factory,
                                                          pami_context_t    context,
                                                          size_t                                     context_id)
      {
        TRACE_ERR((stderr, "<%p>Common::addCollective_impl()\n", this));
        AlgoLists<Geometry<PAMI::Geometry::Common> > * alist = algorithms_get_lists(colltype);
        alist->addCollective(factory, this, context, context_id);
        return PAMI_SUCCESS;
      }

      inline pami_result_t             rmCollective_impl(pami_xfer_type_t                            colltype,
                                                         Factory          *factory,
                                                         pami_context_t    context,
                                                         size_t                                     context_id)
      {
        TRACE_ERR((stderr, "<%p>Common::rmCollective_impl()\n", this));
        AlgoLists<Geometry<PAMI::Geometry::Common> > * alist = algorithms_get_lists(colltype);
        alist->rmCollective(factory, this);
        return PAMI_SUCCESS;
      }

      inline pami_result_t             rmCollectiveCheck_impl(pami_xfer_type_t                            colltype,
                                                              Factory          *factory,
                                                              pami_context_t    context,
                                                              size_t                                     context_id)
      {
        TRACE_ERR((stderr, "<%p>Common::rmCollectiveCheck_impl()\n", this));
        AlgoLists<Geometry<PAMI::Geometry::Common> > * alist = algorithms_get_lists(colltype);
        alist->rmCollectiveCheck(factory, this);
        return PAMI_SUCCESS;
      }

      inline pami_result_t             addCollectiveCheck_impl(pami_xfer_type_t                            colltype,
                                                               Factory          *factory,
                                                               pami_context_t    context,
                                                               size_t                                     context_id)
      {
        TRACE_ERR((stderr, "<%p>Common::addCollectiveCheck_impl()\n", this));
        AlgoLists<Geometry<PAMI::Geometry::Common> > * alist = algorithms_get_lists(colltype);
        alist->addCollectiveCheck(factory, this, context, context_id);
        return PAMI_SUCCESS;
      }

      pami_result_t                    algorithms_num_impl(pami_xfer_type_t  colltype,
                                                           size_t             *lengths)
      {
        TRACE_ERR((stderr, "<%p>Common::algorithms_num_impl()\n", this));
        AlgoLists<Geometry<PAMI::Geometry::Common> > * alist = algorithms_get_lists(colltype);
        alist->lengths(lengths);
        return PAMI_SUCCESS;
      }

      inline pami_result_t             algorithms_info_impl (pami_xfer_type_t   colltype,
                                                             pami_algorithm_t  *algs0,
                                                             pami_metadata_t   *mdata0,
                                                             size_t               num0,
                                                             pami_algorithm_t  *algs1,
                                                             pami_metadata_t   *mdata1,
                                                             size_t             num1)
      {
        TRACE_ERR((stderr, "<%p>Common::algorithms_info_impl(), algs0=%p, num0=%zu, mdata0=%p, algs1=%p, num1=%zu, mdata1=%p\n", this, algs0, num0, mdata0, algs1, num1, mdata1));
        AlgoLists<Geometry<PAMI::Geometry::Common> > * alist = algorithms_get_lists(colltype);

        for (size_t i = 0; i < num0; i++)
        {
          TRACE_ERR((stderr, "<%p> alist->_algo_array[%zu]=%p, mdata0[%zu]=%p\n", this, i, alist->_algo_array[i], i, mdata0 ? &mdata0[i] : NULL));

          if (algs0)
            algs0[i]   = (pami_algorithm_t) alist->_algo_array[i];

          if (mdata0)
            alist->_algo_array[i][0].metadata(&mdata0[i]);
        }

        for (size_t i = 0; i < num1; i++)
        {
          TRACE_ERR((stderr, "<%p> alist->_algo_array_check[%zu]=%p, mdata1[%zu]=%p\n", this, i, alist->_algo_array_check[i], i, mdata1 ? &mdata1[i] : NULL));

          if (algs1)
            algs1[i] = (pami_algorithm_t) alist->_algo_array_check[i];

          if (mdata1)
            alist->_algo_array_check[i][0].metadata(&mdata1[i]);
        }

        return PAMI_SUCCESS;
      }

      pami_result_t                    default_barrier(pami_event_function       cb_done,
                                                       void                   * cookie,
                                                       size_t                   ctxt_id,
                                                       pami_context_t            context)
      {
        TRACE_ERR((stderr, "<%p>Common::default_barrier()\n", this));
        pami_xfer_t cmd;
        cmd.cb_done = cb_done;
        cmd.cookie = cookie;
        return _barriers._algo_array[0][ctxt_id].generate(&cmd);
      }

      pami_result_t                    ue_barrier_impl(pami_event_function     cb_done,
                                                       void                   *cookie,
                                                       size_t                  ctxt_id,
                                                       pami_context_t          context)
      {
        TRACE_ERR((stderr, "<%p>Common::ue_barrier()\n", this));
        PAMI_assert (_ue_barrier[ctxt_id]._factory != NULL);
        pami_xfer_t cmd;
        cmd.cb_done = cb_done;
        cmd.cookie = cookie;
//        pami_metadata_t mdata;
//        _ue_barrier._factory->metadata(&mdata);
//        fprintf(stderr, "ue_barrier <%s>\n", mdata.name);
        return _ue_barrier[ctxt_id].generate(&cmd);
      }

      void resetUEBarrier_impl(size_t ctxt_id)
      {
        _ue_barrier[ctxt_id]._factory  = (Factory*)NULL;
        _ue_barrier[ctxt_id]._geometry = (PAMI::Geometry::Common*)NULL;
      }

      pami_result_t setUEBarrier_impl(Factory *f,
                                      size_t   ctxt_id)
      {
        if (_ue_barrier[ctxt_id]._factory == (Factory*)NULL)
        {
          _ue_barrier[ctxt_id]._factory  = f;
          _ue_barrier[ctxt_id]._geometry = this;
          TRACE_ERR((stderr, "<%p>Common::set(ctxt_id=%d) ue_barrier() %p, %p/%p, %p\n",
                     this,ctxt_id,
                     &_ue_barrier[ctxt_id],
                     f,
                     _ue_barrier[ctxt_id]._factory,
                     _ue_barrier[ctxt_id]._geometry));
          return PAMI_SUCCESS;
        }
        return PAMI_EAGAIN;  // can't set again unless you reset first.
      }

      pami_client_t                 getClient_impl()
      {
        return _client;
      }

      void setCleanupCallback_impl(pami_event_function fcn, void *data)
      {
        _cleanupFcns.push_back(fcn);
        _cleanupDatas.push_back(data);
      }

      typedef bool (*CheckpointCb)(void *data);

      struct CheckpointFunctions {
        CheckpointCb  checkpoint_fn;
        CheckpointCb  resume_fn;
        CheckpointCb  restart_fn;
        void         *cookie;
        CheckpointFunctions(CheckpointCb ckpt,
                            CheckpointCb resume,
                            CheckpointCb restart,
                            void *data):
          checkpoint_fn(ckpt),
          resume_fn(resume),
          restart_fn(restart),
          cookie(data) {};
      };

      void setCkptCallback(CheckpointCb ckptfcn, CheckpointCb resumefcn,
                           CheckpointCb restartfcn, void *data)
      {
        ITRC(IT_INITTERM, "LapiGeometry 0x%p setCkptCallback()\n", this);
        _ckptFcns.push_back(CheckpointFunctions(ckptfcn, resumefcn, restartfcn, data));
      }

      bool Checkpoint()
      {
        ITRC(IT_INITTERM, "LapiGeometry 0x%p _commid=%u: Checkpoint() enters ckptFncs.size()=%d\n",
             this, _commid, _ckptFcns.size());

        std::list<CheckpointFunctions>::iterator itFcn = _ckptFcns.begin();

        /* invoke all registered checkpoint callbacks */
        for (; itFcn != _ckptFcns.end(); itFcn++)
          {
            CheckpointCb cb   = itFcn->checkpoint_fn;
            void        *data = itFcn->cookie;

            bool rc = (*cb)(data);
            if (!rc)
              return false;
          }

        ITRC(IT_INITTERM, "LapiGeometry 0x%p _commid=%u: Checkpoint() exits\n", this, _commid);
        _checkpointed = true;
        return true;
      }

      bool Restart()
      {
        ITRC(IT_INITTERM, "LapiGeometry 0x%p _commid=%u: Restart() enters _ckptFcns.size()=%d\n",
             this, _commid, _ckptFcns.size());
        assert(_checkpointed);
        std::list<CheckpointFunctions>::iterator itFcn = _ckptFcns.begin();

        /* invoke all registered restart callbacks */
        for (; itFcn != _ckptFcns.end(); itFcn++)
          {
            CheckpointCb cb   = itFcn->restart_fn;
            void        *data = itFcn->cookie;

            bool rc = (*cb)(data);
            if (!rc)
              return false;
          }
        ITRC(IT_INITTERM, "LapiGeometry 0x%p _commid=%u: Restart() exits\n", this, _commid);
        _checkpointed = false;
        return true;
      }

      bool Resume()
      {
        ITRC(IT_INITTERM, "LapiGeometry 0x%p _commid=%u: Resume() enters\n", this, _commid);
        assert(_checkpointed);
        std::list<CheckpointFunctions>::iterator itFcn = _ckptFcns.begin();

        /* invoke all registered resume callbacks */
        for (; itFcn != _ckptFcns.end(); itFcn++)
          {
            CheckpointCb cb   = itFcn->resume_fn;
            void        *data = itFcn->cookie;

            bool rc = (*cb)(data);
            if (!rc)
              return false;
          }
        ITRC(IT_INITTERM, "LapiGeometry 0x%p _commid=%u: Resume() exits\n", this, _commid);
        _checkpointed = false;
        return true;
      }

    private:
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _allreduces;
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _broadcasts;
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _reduces;
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _allgathers;

      AlgoLists<Geometry<PAMI::Geometry::Common> >  _allgathervs;
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _allgatherv_ints;
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _scatters;
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _scattervs;

      AlgoLists<Geometry<PAMI::Geometry::Common> >  _scatterv_ints;
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _gathers;
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _gathervs;
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _gatherv_ints;

      AlgoLists<Geometry<PAMI::Geometry::Common> >  _alltoalls;
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _alltoallvs;
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _alltoallv_ints;
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _reduce_scatters;
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _ambroadcasts;

      AlgoLists<Geometry<PAMI::Geometry::Common> >  _amscatters;
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _amgathers;
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _amreduces;
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _scans;

      AlgoLists<Geometry<PAMI::Geometry::Common> >  _barriers;
      Algorithm<PAMI::Geometry::Common>            *_ue_barrier;

      void ***                                      _kvcstore;
      int                                           _commid;
      pami_client_t                                 _client;
      pami_task_t                                   _rank;
      MatchQueue                                   *_ue;
      MatchQueue                                   *_post;
      bool                                          _ranks_malloc;
      pami_task_t                                  *_ranks;
      pami_task_t                                  *_endpoints;
      std::map<unsigned, pami_geometry_t>          *_geometry_map;
      void                                         *_allreduce[MAX_CONTEXTS][2];
      unsigned                                      _allreduce_async_mode[MAX_CONTEXTS];
      unsigned                                      _allreduce_iteration[MAX_CONTEXTS];
      PAMI::Topology                                _topos[MAX_NUM_TOPOLOGIES];
      pami_task_t                                   _masterRank;
      bool                                          _checkpointed;
      /// Blue Gene/Q Specific members
      pami_callback_t                               _cb_done;
      pami_result_t                                 _cb_result;
      GeomCompCtr                                   _comp;
      std::list<pami_event_function>                _cleanupFcns;
      std::list<void*>                              _cleanupDatas;
      std::list<CheckpointFunctions>                _ckptFcns;
      std::map <size_t, DispatchInfo>               _dispatch;            // AM Collective dispatch functions
      size_t                                        _context_offset;
      size_t                                        _ncontexts;

    public:
      /// Blue Gene/Q Specific functions

      /// \brief Convenience callback used by geometry completion sub-events
      ///
      /// Each sub-event must invoke this when finished, and the final (user) completion
      /// will be invoked once all have checked in.
      ///
      /// \param[in] ctx       The context on which competion is ocurring
      /// \param[in] cookie    The geometry object
      /// \param[in] result    Status of this completion (error or success)
      ///
      static void                      _done_cb(pami_context_t ctx, void *cookie, pami_result_t result)
      {
        PAMI::Geometry::Common *thus = (PAMI::Geometry::Common *)cookie;
        thus->rmCompletion(ctx, result);
      }

      /// \brief Setup completion for geometry create
      ///
      /// Note: this is valid for both the parent and (new) sub-geometry,
      /// during the create. If any of the geometry event items returns an
      /// error, this callback will be invoked with that error (last error seen).
      ///
      /// \param[in] fn                The completion function to call
      /// \param[in] cookie    Opaque user data for callback
      ///
      inline void                      setCompletion(pami_event_function fn, void *cookie)
      {
        _cb_done = (pami_callback_t)
        {
          fn, cookie
        };
        _cb_result = PAMI_SUCCESS;
      }

      /// \brief Add one completion event to geometry
      ///
      /// This is called each time some work has been started on which
      /// geometry create completion depends (must wait).
      ///
      inline void                      addCompletion()
      {
        _comp.fetch_and_inc();
      }

      /// \brief Remove one completion event (cancel or "done")
      ///
      /// Note: this is valid for both the parent and (new) sub-geometry,
      /// during the create.
      ///
      /// \param[in] ctx       The context on which competion is ocurring
      /// \param[in] result    Status of this completion (error or success)
      ///
      inline void                      rmCompletion(pami_context_t ctx, pami_result_t result)
      {
        if (result != PAMI_SUCCESS) _cb_result = result;

        if (_comp.fetch_and_dec() == 1)
        {
          if (_cb_done.function)
          {
            _cb_done.function(ctx, _cb_done.clientdata, _cb_result);
          }
        }
      }

    }; // class Geometry    
  };  // namespace Geometry
}; // namespace PAMI

#undef TRACE_ERR
#undef TRACE_ERR2
#undef  DO_TRACE_ENTEREXIT
#undef  DO_TRACE_DEBUG    

#endif
