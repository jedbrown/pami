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
    static void resetFactoryCache (pami_context_t   ctxt,
                                   void           * factory,
                                   pami_result_t    result)
    {
      (void)ctxt;(void)result;
      CCMI::Adaptor::CollectiveProtocolFactory *cf =
        (CCMI::Adaptor::CollectiveProtocolFactory *) factory;
      cf->clearCache();
    }

    typedef bool (*CheckpointCb)(void *data);
    class CheckpointFunction
    {
    public:
      CheckpointFunction(CheckpointCb ckpt,
                         CheckpointCb resume,
                         CheckpointCb restart,
                         void *data):
        _checkpoint_fn(ckpt),
        _resume_fn(resume),
        _restart_fn(restart),
        _cookie(data)
      {
      };
      CheckpointCb  _checkpoint_fn;
      CheckpointCb  _resume_fn;
      CheckpointCb  _restart_fn;
      void         *_cookie;
    };

    class Common :
    public Geometry<Common>
    {
    public:
      typedef Algorithm<Geometry<Common> >        AlgorithmT;
      typedef std::map<size_t, AlgorithmT>        ContextMap;
      typedef std::map<uint32_t, ContextMap>      HashMap;
      typedef std::map<uint32_t, HashMap>         AlgoMap;
      typedef std::list<ContextMap*>              AlgoList;
      typedef std::map<unsigned, pami_geometry_t> GeometryMap;
      typedef PAMI::Counter::Native               GeomCompCtr;
      typedef std::list<pami_event_function>      CleanupFunctions;
      typedef std::list<void*>                    CleanupDatas;
      typedef std::list<CheckpointFunction>       CheckpointFunctions;
      typedef std::map <size_t, DispatchInfo>     DispatchMap;

      inline ~Common()
      {
        TRACE_FN_ENTER();
        freeAllocations_impl();
        TRACE_FN_EXIT();
      }

      inline Common(pami_client_t  client,
                    Common        *parent,
                    Mapping       *mapping,
                    unsigned       comm,
                    pami_task_t    nranks,
                    pami_task_t   *ranks,
                    GeometryMap   *geometry_map,
                    size_t         context_offset,
                    size_t         ncontexts):
      Geometry<Common>(parent,
                       mapping,
                       comm,
                       nranks,
                       ranks),
      _generation_id(0),
      _ue_barrier(NULL),
      _default_barrier(NULL),
      _commid(comm),
      _client(client),
      _rank(mapping->task()),
      _ranks_malloc(false),
      _ranks(ranks),
      _geometry_map(geometry_map),
      _checkpointed(false),
      _cb_result(PAMI_EAGAIN)
      {
        TRACE_ERR((stderr, "<%p>Common(ranklist)\n", this));
        // this creates the topology including all subtopologies
        new(&_topos[DEFAULT_TOPOLOGY_INDEX]) PAMI::Topology(_ranks, nranks);
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

        size_t nctxt     = (PAMI_ALL_CONTEXTS == context_offset)?ncontexts:1;
        size_t start_off = (PAMI_ALL_CONTEXTS == context_offset)? 0 : context_offset;
        pami_result_t rc = __global.heap_mm->memalign((void **)&_ue_barrier,
                                                      0,
                                                      nctxt*sizeof(*_ue_barrier));
        (void)rc; //unused warnings in assert case
        PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc _ue_barier");

        rc = __global.heap_mm->memalign((void **)&_default_barrier,
                                        0,
                                        nctxt*sizeof(*_default_barrier));
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
          new(&_ue_barrier[n]) AlgorithmT(NULL,NULL);
          new(&_default_barrier[n]) AlgorithmT(NULL,NULL);
          new(&_ue[n]) MatchQueue();
          new(&_post[n]) MatchQueue();
          resetUEBarrier_impl(n);
        }
      }

      /**
         Construct a geometry based on a list of endpoints
      */
      inline Common (pami_client_t    client,
                     Common          *parent,
                     Mapping         *mapping,
                     unsigned         comm,
                     pami_endpoint_t  neps,
                     pami_endpoint_t *eps,
                     GeometryMap     *geometry_map,
                     bool):
      Geometry<Common>(parent,
                       mapping,
                       comm,
                       neps,
                       eps),
      _generation_id(0),
      _ue_barrier(NULL),
      _default_barrier(NULL),
      _commid(comm),
      _client(client),
      _rank(mapping->task()),
      _ranks_malloc(false),
      _endpoints(eps),
      _geometry_map(geometry_map),
      _checkpointed(false),
      _cb_result(PAMI_EAGAIN)
      {
        TRACE_ERR((stderr, "<%p>Common(endpointlist)\n", this));
        // this creates the topology including all subtopologies
        new(&_topos[DEFAULT_TOPOLOGY_INDEX]) PAMI::Topology(_endpoints, neps, PAMI::tag_eplist());

        buildSpecialTopologies();
#if 0
        fprintf(stderr, "EP DEFAULT Topology sz=%d:\n", _topos[DEFAULT_TOPOLOGY_INDEX].size());
        for(int i=0; i< _topos[DEFAULT_TOPOLOGY_INDEX].size(); i++)
          {
            pami_endpoint_t ep    = _topos[DEFAULT_TOPOLOGY_INDEX].index2Endpoint(i);
            pami_task_t     task;
            size_t          offset;
            PAMI_ENDPOINT_INFO(ep, task, offset);
            fprintf(stderr, "   --->(ep=%d task=%d offset=%ld)\n", ep, task, offset);
          }

        fprintf(stderr, "EP LOCAL Topology sz=%d:\n", _topos[LOCAL_TOPOLOGY_INDEX].size());
        for(int i=0; i< _topos[LOCAL_TOPOLOGY_INDEX].size(); i++)
          {
            pami_endpoint_t ep    = _topos[LOCAL_TOPOLOGY_INDEX].index2Endpoint(i);
            pami_task_t     task;
            size_t          offset;
            PAMI_ENDPOINT_INFO(ep, task, offset);
            fprintf(stderr, "   --->(ep=%d task=%d offset=%ld)\n", ep, task, offset);
          }

        fprintf(stderr, "EP MASTER Topology: sz=%d\n", _topos[MASTER_TOPOLOGY_INDEX].size());
        for(int i=0; i< _topos[MASTER_TOPOLOGY_INDEX].size(); i++)
          {
            pami_endpoint_t ep    = _topos[MASTER_TOPOLOGY_INDEX].index2Endpoint(i);
            pami_task_t     task;
            size_t          offset;
            PAMI_ENDPOINT_INFO(ep, task, offset);
            fprintf(stderr, "   --->(ep=%d task=%d offset=%ld)\n", ep, task, offset);
          }
#endif



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

        _kvcstore = allocKVS(NUM_CKEYS,MAX_CONTEXTS);
        pami_result_t rc = __global.heap_mm->memalign((void **)&_ue_barrier,
                                                      0,
                                                      MAX_CONTEXTS*sizeof(*_ue_barrier));
        (void)rc; //unused warnings in assert case
        PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc _ue_barrier");

        rc = __global.heap_mm->memalign((void **)&_default_barrier,
                                        0,
                                        MAX_CONTEXTS*sizeof(*_default_barrier));
        PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc _default_barrier");

        // Allocate per context UE queues
        rc = __global.heap_mm->memalign((void **)&_ue,
                                        0,
                                        MAX_CONTEXTS*sizeof(MatchQueue));
        PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc _temp_topo");

        // Allocate per context Posted queues
        rc = __global.heap_mm->memalign((void **)&_post,
                                        0,
                                        MAX_CONTEXTS*sizeof(MatchQueue));
        PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc _temp_topo");


        for(size_t n=0; n<MAX_CONTEXTS; n++)
        {
          new(&_ue_barrier[n]) AlgorithmT(NULL,NULL);
          new(&_default_barrier[n]) AlgorithmT(NULL,NULL);
          new(&_ue[n]) MatchQueue();
          new(&_post[n]) MatchQueue();
          resetUEBarrier_impl(n);
        }
      }

      inline Common (pami_client_t          client,
                     Common                *parent,
                     Mapping               *mapping,
                     unsigned               comm,
                     int                    numranges,
                     pami_geometry_range_t  rangelist[],
                     GeometryMap           *geometry_map,
                     size_t                 context_offset,
                     size_t                 ncontexts):
      Geometry<Common>(parent,
                       mapping,
                       comm,
                       numranges,
                       rangelist),
      _generation_id(0),
      _ue_barrier(NULL),
      _default_barrier(NULL),
      _commid(comm),
      _client(client),
      _rank(mapping->task()),
      _ranks_malloc(false),
      _ranks(NULL),
      _geometry_map(geometry_map),
      _checkpointed(false),
      _cb_result(PAMI_EAGAIN)
      {
        TRACE_ERR((stderr, "<%p>Common(ranges)\n", this));
        pami_result_t rc;
          if (numranges == 1)
        {
          // this creates the topology from a (single) range
          if(context_offset==0) {
          new(&_topos[DEFAULT_TOPOLOGY_INDEX]) PAMI::Topology(rangelist[0].lo, rangelist[0].hi);
        }
          else {
            new(&_topos[DEFAULT_TOPOLOGY_INDEX]) PAMI::Topology(rangelist[0].lo, rangelist[0].hi,context_offset, ncontexts);
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
          PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc _ranks");

          for (k = 0, i = 0; i < numranges; i++)
          {
            int size = rangelist[i].hi - rangelist[i].lo + 1;

            for (j = 0; j < size; j++, k++)
              _ranks[k] = rangelist[i].lo + j;
          }

          // this creates the topology
          if(context_offset==0) {
          new(&_topos[DEFAULT_TOPOLOGY_INDEX]) PAMI::Topology(_ranks, nranks);
        }
          else{
            new(&_topos[DEFAULT_TOPOLOGY_INDEX]) PAMI::Topology(_ranks, nranks,context_offset,ncontexts);
          }
        }

        buildSpecialTopologies();

        if ((_topos[LIST_TOPOLOGY_INDEX].type() == PAMI_LIST_TOPOLOGY) &&
            (_ranks_malloc == false)) /* Don't overwrite our original _ranks */
        {
          pami_result_t rc = PAMI_SUCCESS;
          rc = _topos[LIST_TOPOLOGY_INDEX].rankList(&_ranks);
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

        size_t nctxt     = (PAMI_ALL_CONTEXTS == context_offset)?ncontexts:1;
        size_t start_off = (PAMI_ALL_CONTEXTS == context_offset)? 0 : context_offset;
        rc               = __global.heap_mm->memalign((void **)&_ue_barrier,
                                                      0,
                                                      nctxt*sizeof(*_ue_barrier));
        PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc _ue_barrier");

        rc               = __global.heap_mm->memalign((void **)&_default_barrier,
                                                      0,
                                                      nctxt*sizeof(*_default_barrier));
        PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc _default_barrier");


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
          new(&_ue_barrier[n]) AlgorithmT(NULL,NULL);
          new(&_ue_barrier[n]) AlgorithmT(NULL,NULL);
          new(&_ue[n]) MatchQueue();
          new(&_post[n]) MatchQueue();
          resetUEBarrier_impl(n);
        }
      }

      inline Common (pami_client_t   client,
                     Common         *parent,
                     Mapping        *mapping,
                     unsigned        comm,
                     PAMI::Topology *topology,
                     GeometryMap    *geometry_map,
                     size_t          context_offset,
                     size_t          ncontexts):
      Geometry<Common>(parent,
                       mapping,
                       comm,
                       topology),
      _generation_id(0),
      _ue_barrier(NULL),
      _default_barrier(NULL),
      _commid(comm),
      _client(client),
      _rank(mapping->task()),
      _ranks_malloc(false),
      _ranks(NULL),
      _geometry_map(geometry_map),
      _checkpointed(false),
      _cb_result(PAMI_EAGAIN)
      {
        TRACE_ERR((stderr, "<%p>Common(topology)\n", this));

        _topos[DEFAULT_TOPOLOGY_INDEX] = *topology;

        if (_topos[DEFAULT_TOPOLOGY_INDEX].type() == PAMI_LIST_TOPOLOGY)
        {
          _topos[DEFAULT_TOPOLOGY_INDEX].rankList(&_ranks);
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

        size_t nctxt     = (PAMI_ALL_CONTEXTS == context_offset)?ncontexts:1;
        size_t start_off = (PAMI_ALL_CONTEXTS == context_offset)? 0 : context_offset;
        pami_result_t rc = __global.heap_mm->memalign((void **)&_ue_barrier,
                                                      0,
                                                      nctxt*sizeof(*_ue_barrier));
        PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc _ue_barrier");
        (void)rc; //to avoid unused warnings in non-assert

        rc = __global.heap_mm->memalign((void **)&_default_barrier,
                                        0,
                                        nctxt*sizeof(*_default_barrier));
        PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc _default_barrier");

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
          new(&_ue_barrier[n]) AlgorithmT(NULL,NULL);
          new(&_default_barrier[n]) AlgorithmT(NULL,NULL);
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
        DO_DEBUGg(for (unsigned j = 0; j < _topos[DEFAULT_TOPOLOGY_INDEX].size(); ++j) TRACE_ERR((stderr, "buildSpecialTopologies() DEFAULT_TOPOLOGY[%u]=%zu, size %zu\n", j, (size_t)_topos[DEFAULT_TOPOLOGY_INDEX].index2Endpoint(j), _topos[DEFAULT_TOPOLOGY_INDEX].size())));
        _topos[DEFAULT_TOPOLOGY_INDEX].subTopologyNthGlobal(&_topos[MASTER_TOPOLOGY_INDEX], 0);
        _topos[DEFAULT_TOPOLOGY_INDEX].subTopologyLocalToMe(&_topos[LOCAL_TOPOLOGY_INDEX]);
        _topos[MASTER_TOPOLOGY_INDEX].subTopologyLocalToMe(&_topos[LOCAL_MASTER_TOPOLOGY_INDEX]);

        // Find master participant on the tree/cau network
        DO_DEBUGg(TRACE_ERR((stderr,"(%u)buildSpecialTopologies() MASTER_TOPOLOGY_INDEX rankList %p\n", _topos[MASTER_TOPOLOGY_INDEX].rankList(&list), list)));
        DO_DEBUGg(for (unsigned j = 0; j < _topos[MASTER_TOPOLOGY_INDEX].size(); ++j) TRACE_ERR((stderr, "buildSpecialTopologies() MASTER_TOPOLOGY[%u]=%zu, size %zu\n", j, (size_t)_topos[MASTER_TOPOLOGY_INDEX].index2Endpoint(j), _topos[MASTER_TOPOLOGY_INDEX].size())));
        DO_DEBUGg(TRACE_ERR((stderr,"(%u)buildSpecialTopologies() LOCAL_TOPOLOGY rankList %p\n", _topos[LOCAL_TOPOLOGY_INDEX].rankList(&list), list)));
        DO_DEBUGg(for (unsigned j = 0; j < _topos[LOCAL_TOPOLOGY_INDEX].size(); ++j) TRACE_ERR((stderr, "buildSpecialTopologies() LOCAL_TOPOLOGY[%u]=%zu, size %zu\n", j, (size_t)_topos[LOCAL_TOPOLOGY_INDEX].index2Endpoint(j), _topos[LOCAL_TOPOLOGY_INDEX].size())));

        // Create a coordinate topo (may be EMPTY)
        _topos[COORDINATE_TOPOLOGY_INDEX] = _topos[DEFAULT_TOPOLOGY_INDEX];

        if (_topos[COORDINATE_TOPOLOGY_INDEX].type() != PAMI_COORD_TOPOLOGY)
          _topos[COORDINATE_TOPOLOGY_INDEX].convertTopology(PAMI_COORD_TOPOLOGY);

        DO_DEBUGg(TRACE_ERR((stderr,"(%u)buildSpecialTopologies() COORDINATE_TOPOLOGY rankList %p\n", _topos[COORDINATE_TOPOLOGY_INDEX].rankList(&list), list)));
        DO_DEBUGg(for (unsigned j = 0; j < _topos[COORDINATE_TOPOLOGY_INDEX].size(); ++j) TRACE_ERR((stderr, "buildSpecialTopologies() COORDINATE_TOPOLOGY[%u]=%zu, size %zu\n", j, (size_t)_topos[COORDINATE_TOPOLOGY_INDEX].index2Endpoint(j), _topos[COORDINATE_TOPOLOGY_INDEX].size())));

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
        int sz = _cleanupFcns.size();
        for (int i=0; i<sz; i++)
        {
          pami_event_function  fn = _cleanupFcns.front();  _cleanupFcns.pop_front();
          void                *cd = _cleanupDatas.front(); _cleanupDatas.pop_front();
          if (fn) fn(NULL, cd, PAMI_SUCCESS);
        }

        if (_ranks_malloc) __global.heap_mm->free(_ranks);

        _ranks = NULL;
        _ranks_malloc = false;
        __global.heap_mm->free(_ue_barrier);
        __global.heap_mm->free(_default_barrier);
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
        DispatchMap::iterator iter = _dispatch.find(key);

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

      inline pami_result_t             addCollective_impl(pami_xfer_type_t  colltype,
                                                          Factory          *factory,
                                                          pami_context_t    context,
                                                          size_t            context_id)
      {
        (void)context;
        uint32_t hash = factory->nameHash(_generation_id++);
        Algorithm<Geometry<Common> >*elem = &_algoTable[colltype][hash][context_id];
        new(elem) Algorithm<Geometry<Common> >(factory, this);
        setCleanupCallback(resetFactoryCache, factory);
#if DO_TRACE_DEBUG
//        if(PAMI_XFER_ALLREDUCE == colltype)
        {
          pami_metadata_t m;
          factory->metadata(&m);
          fprintf(stderr,"addCollective %s\n",m.name);
          fprintf(stderr,"num algorithms %zu\n",_algoTable[colltype].size());
        }
#endif
        return PAMI_SUCCESS;
      }

      inline pami_result_t             rmCollective_impl(pami_xfer_type_t  colltype,
                                                         Factory          *factory,
                                                         pami_context_t    context,
                                                         size_t            context_id)
      {
        (void)context;
        if(!factory) return PAMI_SUCCESS;
        resetCleanupCallback(resetFactoryCache, factory);
        uint32_t hash = factory->nameHash();
        _algoTable[colltype][hash].erase(context_id);
        _algoTable[colltype].erase(hash);
#if DO_TRACE_DEBUG
//        if(PAMI_XFER_ALLREDUCE == colltype)
        {
          pami_metadata_t m;
          factory->metadata(&m);
          fprintf(stderr,"rmCollective %s\n",m.name);
          fprintf(stderr,"num algorithms %zu\n",_algoTable[colltype].size());
        }
#endif
        return PAMI_SUCCESS;
      }

      inline pami_result_t             rmCollectiveCheck_impl(pami_xfer_type_t  colltype,
                                                              Factory          *factory,
                                                              pami_context_t    context,
                                                              size_t            context_id)
      {
        (void)context;
        resetCleanupCallback(resetFactoryCache, factory);
        uint32_t hash = factory->nameHash();
        _algoTableCheck[colltype][hash].erase(context_id);
        _algoTableCheck[colltype].erase(hash);
#if DO_TRACE_DEBUG
//        if(PAMI_XFER_ALLREDUCE == colltype)
        {
          pami_metadata_t m;
          factory->metadata(&m);
          fprintf(stderr,"rmCollectiveCheck %s\n",m.name);
          fprintf(stderr,"num algorithms %zu\n",_algoTableCheck[colltype].size());
        }
#endif
        return PAMI_SUCCESS;
      }

      inline pami_result_t             addCollectiveCheck_impl(pami_xfer_type_t                            colltype,
                                                               Factory          *factory,
                                                               pami_context_t    context,
                                                               size_t                                     context_id)
      {
        (void)context;
        uint32_t hash = factory->nameHash(_generation_id++);
        Algorithm<Geometry<Common> >*elem = &_algoTableCheck[colltype][hash][context_id];
        new(elem) Algorithm<Geometry<Common> >(factory, this);
        setCleanupCallback(resetFactoryCache, factory);
#if DO_TRACE_DEBUG
//        if(PAMI_XFER_ALLREDUCE == colltype)
        {
          pami_metadata_t m;
          factory->metadata(&m);
          fprintf(stderr,"addCollectiveCheck %s\n",m.name);
          fprintf(stderr,"num algorithms %zu\n",_algoTableCheck[colltype].size());
        }
#endif
        return PAMI_SUCCESS;
      }

      pami_result_t                    algorithms_num_impl(pami_xfer_type_t  colltype,
                                                           size_t             *lengths)
      {
        lengths[0] = _algoTable[colltype].size();
        lengths[1] = _algoTableCheck[colltype].size();
        return PAMI_SUCCESS;
      }

      static bool compare(ContextMap *cm0, ContextMap *cm1)
        {
          // Sort the list based on context 0
          if(cm0 == NULL || cm1 == NULL) return true;
          AlgorithmT a0 = (*cm0)[0];
          AlgorithmT a1 = (*cm1)[0];
          if(a0._factory->getGenerationId() < a1._factory->getGenerationId())
            return true;
          else
            return false;
        }

      inline pami_result_t             algorithms_info_impl (pami_xfer_type_t   colltype,
                                                             pami_algorithm_t  *algs0,
                                                             pami_metadata_t   *mdata0,
                                                             size_t             num0,
                                                             pami_algorithm_t  *algs1,
                                                             pami_metadata_t   *mdata1,
                                                             size_t             num1)
      {
        HashMap *m = &_algoTable[colltype];
        HashMap::iterator iter;
        size_t i;

        // Sort the algorithm list in insertion order
        AlgoList v0, v1;
        for(i=0,iter=m->begin();iter!=m->end() && i<num0;iter++,i++)
          v0.push_back(&iter->second);

        m = &_algoTableCheck[colltype];
        for(i=0,iter=m->begin();iter!=m->end() && i<num1;iter++,i++)
          v1.push_back(&iter->second);

        v0.sort(compare);
        v1.sort(compare);

        AlgoList::iterator alist_iter;
        AlgoList          *al = &v0;
        for(i=0,alist_iter=al->begin();alist_iter!=al->end() && i<num0;alist_iter++,i++)
          {
            ContextMap *cm = (*alist_iter);
            if(algs0)
              algs0[i] = (pami_algorithm_t) cm;
            if(mdata0)
              {
                AlgorithmT *tmp_a = &((*cm)[0]);
                tmp_a->metadata(&mdata0[i]);
                TRACE_ERR((stderr,"sorted algorithms_info() %zu out of %zu/%zu %s\n",i,al->size(),num0,mdata0[i].name));
              }
          }
        al = &v1;
        for(i=0,alist_iter=al->begin();alist_iter!=al->end() && i<num1;alist_iter++,i++)
          {
            ContextMap *cm = (*alist_iter);
            if(algs1)
              algs1[i] = (pami_algorithm_t) cm;
            if(mdata1)
              {
                AlgorithmT *tmp_a = &((*cm)[0]);
                tmp_a->metadata(&mdata1[i]);
                TRACE_ERR((stderr,"sorted algorithms_info(check) %zu out of %zu/%zu %s\n",i,al->size(),num1,mdata1[i].name));
              }
          }
        return PAMI_SUCCESS;
      }

      pami_result_t default_barrier_impl(pami_event_function  cb_done,
                                         void                *cookie,
                                         size_t               ctxt_id,
                                         pami_context_t       context)
      {
        (void)context;
        PAMI_assert (_default_barrier[ctxt_id]._factory != NULL);
        pami_xfer_t cmd;
        cmd.cb_done = cb_done;
        cmd.cookie  = cookie;
        return _default_barrier[ctxt_id].generate(&cmd);
      }

      void resetDefaultBarrier_impl(size_t ctxt_id)
      {
        _default_barrier[ctxt_id]._factory  = (Factory*)NULL;
        _default_barrier[ctxt_id]._geometry = (Common*)NULL;
      }

      pami_result_t setDefaultBarrier_impl(Factory *f,
                                      size_t   ctxt_id)
      {
        if (_default_barrier[ctxt_id]._factory == (Factory*)NULL)
        {
          _default_barrier[ctxt_id]._factory  = f;
          _default_barrier[ctxt_id]._geometry = this;
          return PAMI_SUCCESS;
        }
        return PAMI_EAGAIN;  // can't set again unless you reset first.
      }

      pami_result_t                    ue_barrier_impl(pami_event_function     cb_done,
                                                       void                   *cookie,
                                                       size_t                  ctxt_id,
                                                       pami_context_t          context)
      {
        (void)context;
        TRACE_ERR((stderr, "<%p>Common::ue_barrier()\n", this));
        PAMI_assert (_ue_barrier[ctxt_id]._factory != NULL);
        pami_xfer_t cmd;
        cmd.cb_done = cb_done;
        cmd.cookie = cookie;
        return _ue_barrier[ctxt_id].generate(&cmd);
      }

      void resetUEBarrier_impl(size_t ctxt_id)
      {
        _ue_barrier[ctxt_id]._factory  = (Factory*)NULL;
        _ue_barrier[ctxt_id]._geometry = (Common*)NULL;
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
      void resetCleanupCallback_impl(pami_event_function fcn, void *data)
      {
        CleanupFunctions::iterator itFcn  = _cleanupFcns.begin();
        CleanupDatas::iterator     itData = _cleanupDatas.begin();
        for (; (itFcn != _cleanupFcns.end()) && (itData != _cleanupDatas.end()); itFcn++,itData++)
          {
            pami_event_function  fn = *itFcn;
            void                *cd = *itData;

            if((cd == data) && (fn == fcn))
            {
              if (fn) fn(NULL, cd, PAMI_SUCCESS);
              _cleanupFcns.erase(itFcn);
              _cleanupDatas.erase(itData);
              return;
            }
          }
      }
      void setCkptCallback(CheckpointCb ckptfcn, CheckpointCb resumefcn,
                           CheckpointCb restartfcn, void *data)
      {
        ITRC(IT_INITTERM, "LapiGeometry 0x%p setCkptCallback()\n", this);
        _ckptFcns.push_back(CheckpointFunction(ckptfcn, resumefcn, restartfcn, data));
      }

      bool Checkpoint()
      {
        ITRC(IT_INITTERM, "LapiGeometry 0x%p _commid=%u: Checkpoint() enters ckptFncs.size()=%d\n",
             this, _commid, _ckptFcns.size());
        CheckpointFunctions::iterator itFcn = _ckptFcns.begin();

        /* invoke all registered checkpoint callbacks */
        for (; itFcn != _ckptFcns.end(); itFcn++)
          {
            CheckpointCb cb   = itFcn->_checkpoint_fn;
            void        *data = itFcn->_cookie;

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
        CheckpointFunctions::iterator itFcn = _ckptFcns.begin();

        /* invoke all registered restart callbacks */
        for (; itFcn != _ckptFcns.end(); itFcn++)
          {
            CheckpointCb cb   = itFcn->_restart_fn;
            void        *data = itFcn->_cookie;

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
        CheckpointFunctions::iterator itFcn = _ckptFcns.begin();

        /* invoke all registered resume callbacks */
        for (; itFcn != _ckptFcns.end(); itFcn++)
          {
            CheckpointCb cb   = itFcn->_resume_fn;
            void        *data = itFcn->_cookie;

            bool rc = (*cb)(data);
            if (!rc)
              return false;
          }
        ITRC(IT_INITTERM, "LapiGeometry 0x%p _commid=%u: Resume() exits\n", this, _commid);
        _checkpointed = false;
        return true;
      }

    private:
      AlgoMap                _algoTable;
      AlgoMap                _algoTableCheck;
      int                    _generation_id;
      AlgorithmT            *_ue_barrier;
      AlgorithmT            *_default_barrier;
      void                ***_kvcstore;
      int                    _commid;
      pami_client_t          _client;
      pami_task_t            _rank;
      MatchQueue            *_ue;
      MatchQueue            *_post;
      bool                   _ranks_malloc;
      pami_task_t           *_ranks;
      pami_task_t           *_endpoints;
      GeometryMap           *_geometry_map;
      void                  *_allreduce[MAX_CONTEXTS][2];
      unsigned               _allreduce_async_mode[MAX_CONTEXTS];
      unsigned               _allreduce_iteration[MAX_CONTEXTS];
      PAMI::Topology         _topos[MAX_NUM_TOPOLOGIES];
      bool                   _checkpointed;
      pami_callback_t        _cb_done;
      pami_result_t          _cb_result;
      GeomCompCtr            _comp;
      CleanupFunctions       _cleanupFcns;
      CleanupDatas           _cleanupDatas;
      CheckpointFunctions    _ckptFcns;
      DispatchMap            _dispatch;
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
        Common *thus = (Common *)cookie;
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
