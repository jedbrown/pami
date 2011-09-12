/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/geometry/LapiGeometry.h
 * \brief ???
 */

#ifndef __algorithms_geometry_LapiGeometry_h__
#define __algorithms_geometry_LapiGeometry_h__

#include "Topology.h"
#include "Mapping.h"
#include "algorithms/interfaces/GeometryInterface.h"
#include "algorithms/geometry/Algorithm.h"
#include <map>
#include <list>

#undef TRACE_ERR
#define TRACE_ERR(x) //fprintf x

#undef TRACE_ERR2
#define TRACE_ERR2(x) //fprintf x

#ifndef PAMI_GEOMETRY_NUMALGOLISTS
/// \todo PAMI_GEOMETRY_NUMALGOLISTS == max number of contexts??
#define PAMI_GEOMETRY_NUMALGOLISTS 64
#endif

#include "algorithms/geometry/UnexpBarrierQueueElement.h"
#include "components/atomic/native/NativeCounter.h"
#include "components/memory/MemoryAllocator.h"

namespace PAMI
{
  namespace Geometry
  {
    class Lapi :
        public Geometry<PAMI::Geometry::Lapi>
    {
      public:

      static  Lapi * get (int ctxt, int geometryID)
        {
          assert(0);
          //assert(_ncontexts * teamID + ctxt < MAX_COMMS);
          // return _instances[_ncontexts * teamID + ctxt];

// #warning "Return Geometry map"
          Lapi * g =NULL;
          return g;
        }




        inline ~Lapi ()
        {
          PAMI_assert(_destroyed == false);
          _destroyed = true;
          freeAllocations_impl();
        }

        inline Lapi(pami_client_t                        client,
                    Geometry<PAMI::Geometry::Lapi>      *parent,
                    Mapping                             *mapping,
                    unsigned                             comm,
                    pami_task_t                          nranks,
                    pami_task_t                         *ranks,
                    std::map<unsigned, pami_geometry_t> *geometry_map,
                    int                                  gen_topo=1):
            Geometry<PAMI::Geometry::Lapi>(parent,
                                           mapping,
                                           comm,
                                           nranks,
                                           ranks),
            _kvstore(),
            _commid(comm),
            _client(client),
            _rank(mapping->task()),
            _ranks_malloc(false),
            _ranks(ranks),
            _geometry_map(geometry_map),
            _allreduce_async_mode(0),
            _allreduce_iteration(0),
            _masterRank(-1),
            _destroyed(false)
        {
          TRACE_ERR((stderr, "<%p>Lapi(ranklist)\n", this));
          // this creates the topology including all subtopologies
          new(&_topos[DEFAULT_TOPOLOGY_INDEX]) PAMI::Topology(_ranks, nranks);
          if(gen_topo)
            buildSpecialTopologies();
          
          // Initialize remaining members

          (*_geometry_map)[_commid] = this;

          _allreduce_storage[0] = _allreduce_storage[1] = NULL;
          _allreduce[0] = _allreduce[1] = NULL;
          _virtual_rank    =  (pami_task_t) _topos[DEFAULT_TOPOLOGY_INDEX].rank2Index(_rank);
          PAMI_assert(_virtual_rank != (pami_task_t)-1);

          resetUEBarrier_impl();
        }

        inline Lapi (pami_client_t                        client,
                     Geometry<PAMI::Geometry::Lapi>      *parent,
                     Mapping                             *mapping,
                     unsigned                             comm,
                     int                                  numranges,
                     pami_geometry_range_t                rangelist[],
                     std::map<unsigned, pami_geometry_t> *geometry_map,
                     int                                  gen_topo = 1):
            Geometry<PAMI::Geometry::Lapi>(parent,
                                           mapping,
                                           comm,
                                           numranges,
                                           rangelist),
            _kvstore(),
            _commid(comm),
            _client(client),
            _rank(mapping->task()),
            _ranks_malloc(false),
            _ranks(NULL),
            _geometry_map(geometry_map),
            _allreduce_async_mode(0),
            _allreduce_iteration(0),
            _masterRank(-1),
            _destroyed(false)
        {
          TRACE_ERR((stderr, "<%p>Lapi(ranges)\n", this));

#if 0
          PAMI::Topology *cur         = NULL;
          PAMI::Topology *prev        = NULL;
          PAMI::Topology *new_topo    = NULL;
          PAMI::Topology *free_topo   = NULL;

          // Currently, union topologies are not supported by the topology class
          // To reduce storage, we don't want to create a range list
          // When union topologies are supported for unions of range lists,
          // we can re-enable this code
          if (0)
            {
              PAMI::Topology *_temp_topo  = (PAMI::Topology *)malloc((numranges) * sizeof(PAMI::Topology));

              for (int i = 0; i < numranges; i++)
                new(&_temp_topo[i])Topology(rangelist[i].lo, rangelist[i].hi);

              prev = &_temp_topo[0];

              for (int i = 1; i < numranges; i++)
                {
                  cur      = &_temp_topo[i];
                  new_topo = (PAMI::Topology *)malloc(sizeof(PAMI::Topology));
                  prev->unionTopology(new_topo, cur);
                  prev = new_topo;

                  if (free_topo)
                    free(free_topo);

                  free_topo = new_topo;
                }

              free(_temp_topo);
            }
          else
#endif
            if (numranges == 1)
            {
              // this creates the topology from a (single) range
              new(&_topos[DEFAULT_TOPOLOGY_INDEX]) PAMI::Topology(rangelist[0].lo, rangelist[0].hi);
            }
            else // build a rank list from N ranges
            {
              pami_task_t nranks = 0;
              int i, j, k;

              for (i = 0; i < numranges; i++)
                nranks += (rangelist[i].hi - rangelist[i].lo + 1);

              _ranks_malloc    = true;
	      pami_result_t rc = __global.heap_mm->memalign((void **)&_ranks,0,nranks*sizeof(pami_task_t));
              PAMI_assert(rc == PAMI_SUCCESS);
              for (k = 0, i = 0; i < numranges; i++)
                {
                  int size = rangelist[i].hi - rangelist[i].lo + 1;

                  for (j = 0; j < size; j++, k++)
                    _ranks[k] = rangelist[i].lo + j;
                }

              // this creates the topology 
              new(&_topos[DEFAULT_TOPOLOGY_INDEX]) PAMI::Topology(_ranks, nranks);
            }

          if (gen_topo)
            regenTopo();

#if 0 // not sure we should convert the default... 
          // See if we can eliminate the rank list default geometry with a coord?
          if (_topos[COORDINATE_TOPOLOGY_INDEX].type() == PAMI_COORD_TOPOLOGY)
          {
            // Was a coord successful? Use it and cleanup 
            _topos[DEFAULT_TOPOLOGY_INDEX] = _topos[COORDINATE_TOPOLOGY_INDEX];
            // Empty the special list topo
            new(&_topos[LIST_TOPOLOGY_INDEX]) PAMI::Topology();
            // Free rank list
            free(_ranks);
            _ranks = NULL;
            _ranks_malloc = false;
          }
#endif
          if (_topos[LIST_TOPOLOGY_INDEX].type() == PAMI_LIST_TOPOLOGY)
          {
            pami_result_t rc = PAMI_SUCCESS;
            rc = _topos[LIST_TOPOLOGY_INDEX].rankList(&_ranks);
            PAMI_assert(rc == PAMI_SUCCESS);
          }
          // Initialize remaining members

          (*_geometry_map)[_commid] = this;

          _allreduce_storage[0] = _allreduce_storage[1] = NULL;

          _virtual_rank    =  (pami_task_t) _topos[DEFAULT_TOPOLOGY_INDEX].rank2Index(_rank);
          PAMI_assert(_virtual_rank != (pami_task_t)-1);

          resetUEBarrier_impl();
        }

        inline Lapi (pami_client_t                    client,
                     Geometry<PAMI::Geometry::Lapi>  *parent,
                     Mapping                         *mapping,
                     unsigned                         comm,
                     PAMI::Topology                  *topology,
                     std::map<unsigned, pami_geometry_t> *geometry_map):
            Geometry<PAMI::Geometry::Lapi>(parent,
                                             mapping,
                                             comm,
                                             topology),
            _kvstore(),
            _commid(comm),
            _client(client),
            _rank(mapping->task()),
            _ranks_malloc(false),
            _ranks(NULL), 
            _geometry_map(geometry_map),
            _allreduce_async_mode(0),
            _allreduce_iteration(0),
            _masterRank(-1),
            _destroyed(false)
        {
          TRACE_ERR((stderr, "<%p>Lapi(topology)\n", this));

          _topos[DEFAULT_TOPOLOGY_INDEX] = *topology;

          if (_topos[DEFAULT_TOPOLOGY_INDEX].type() == PAMI_LIST_TOPOLOGY)
            _topos[DEFAULT_TOPOLOGY_INDEX].rankList(&_ranks);

          buildSpecialTopologies();

          // Initialize remaining members

          (*_geometry_map)[_commid] = this;

          _allreduce_storage[0] = _allreduce_storage[1] = NULL;
          _allreduce[0] = _allreduce[1] = NULL;

          _virtual_rank    =  (pami_task_t) _topos[DEFAULT_TOPOLOGY_INDEX].rank2Index(_rank);
          PAMI_assert(_virtual_rank != (pami_task_t)-1);

          resetUEBarrier_impl();
        }

        inline void                      regenTopo()
        {
          buildSpecialTopologies();
        }

        void                             buildSpecialTopologies()
        {
          // build local and global topos
          _topos[DEFAULT_TOPOLOGY_INDEX].subTopologyNthGlobal(&_topos[MASTER_TOPOLOGY_INDEX], 0);
          _topos[DEFAULT_TOPOLOGY_INDEX].subTopologyLocalToMe(&_topos[LOCAL_TOPOLOGY_INDEX]);
          _topos[MASTER_TOPOLOGY_INDEX].subTopologyLocalToMe(&_topos[LOCAL_MASTER_TOPOLOGY_INDEX]);
          // Find master participant on the tree/cau network
          size_t            num_master_tasks = _topos[MASTER_TOPOLOGY_INDEX].size();
          size_t            num_local_tasks = _topos[LOCAL_TOPOLOGY_INDEX].size();

          for (size_t k = 0; k < num_master_tasks; k++)
            for (size_t j = 0; j < num_local_tasks; j++)
              if (_topos[MASTER_TOPOLOGY_INDEX].index2Rank(k) == _topos[LOCAL_TOPOLOGY_INDEX].index2Rank(j))
                {
                  _masterRank= _topos[LOCAL_TOPOLOGY_INDEX].index2Rank(j);
                  break;
                };

          // Create a coordinate topo (may be EMPTY)
          _topos[COORDINATE_TOPOLOGY_INDEX] = _topos[DEFAULT_TOPOLOGY_INDEX];

          if (_topos[COORDINATE_TOPOLOGY_INDEX].type() != PAMI_COORD_TOPOLOGY)
            _topos[COORDINATE_TOPOLOGY_INDEX].convertTopology(PAMI_COORD_TOPOLOGY);

          // If we have a rank list, set the special topology, otherwise leave it EMPTY unless needed
          _topos[LIST_TOPOLOGY_INDEX] = _topos[DEFAULT_TOPOLOGY_INDEX];

          if (_topos[LIST_TOPOLOGY_INDEX].type() != PAMI_LIST_TOPOLOGY)
            new(&_topos[LIST_TOPOLOGY_INDEX]) PAMI::Topology();
        }

        inline bool isLocalMasterParticipant_impl()
        {
          return _masterRank == _rank;
        }

        inline pami_task_t localMasterParticipant_impl()
        {
          return _masterRank;
        }

        inline pami_topology_t* getTopology_impl(topologyIndex_t topo_num)
        {
          return (pami_topology_t*)&_topos[topo_num];
        }

        inline pami_topology_t*          createTopology_impl(topologyIndex_t topo_num)
        {
          // We really only on-demand create the rank list topology.  All others are used as-is.
          if ((topo_num = LIST_TOPOLOGY_INDEX) && (_topos[LIST_TOPOLOGY_INDEX].type() != PAMI_LIST_TOPOLOGY))
            {
              PAMI_assert(!_ranks_malloc);
              _topos[LIST_TOPOLOGY_INDEX] = _topos[DEFAULT_TOPOLOGY_INDEX];
              _topos[LIST_TOPOLOGY_INDEX].convertTopology(PAMI_LIST_TOPOLOGY);
              _topos[LIST_TOPOLOGY_INDEX].rankList(&_ranks);
              _ranks_malloc = true;
            }

          return (pami_topology_t*)&_topos[topo_num];
        }

        inline void                      setAsyncAllreduceMode_impl(unsigned value)
        {
          _allreduce_async_mode = value;
        }
        inline unsigned                  getAsyncAllreduceMode_impl()
        {
          return _allreduce_async_mode;
        }
        inline unsigned                  incrementAllreduceIteration_impl()
        {
          _allreduce_iteration ^= _allreduce_async_mode; // "increment" with defined mode
          return _allreduce_iteration;
        }

        inline unsigned                  comm_impl()
        {
          return _commid;
        }

        inline pami_task_t  *ranks_impl()
        {
          return _ranks; // use createTopology() if necessary, otherwise this could be null.
        }

        inline pami_task_t *ranks_sizet_impl()
        {
          return _ranks; // use createTopology() if necessary, otherwise this could be null.
        }

        inline pami_task_t nranks_impl()
        {
          return _topos[DEFAULT_TOPOLOGY_INDEX].size();
        }

        inline pami_task_t myIdx_impl()
        {
          return _topos[DEFAULT_TOPOLOGY_INDEX].rank2Index(_rank);
        }

        inline void                      generatePermutation_impl()
        {
          return;
        }

        inline void                      freePermutation_impl()
        {
          return;
        }

        inline pami_task_t *permutation_impl()
        {
          return _ranks; // use createTopology() if necessary, otherwise this could be null.
        }

        inline void                      generatePermutation_sizet_impl()
        {
          return;
        }

        inline void                      freePermutation_sizet_impl()
        {
          PAMI_abort();
          return;
        }
        inline pami_task_t *permutation_sizet_impl()
        {
          return _ranks; // use createTopology() if necessary, otherwise this could be null.
        }

        inline bool                      isRectangle_impl()
        {
          PAMI_abort();
          return false;
        }
        inline bool                      isTorus_impl()
        {
          PAMI_abort();
          return false;
        }
        inline bool                      isTree_impl()
        {
          PAMI_abort();
          return false;
        }
        inline bool                      isGlobalContext_impl()
        {
          PAMI_abort();
          return false;
        }
        inline bool                      isGI_impl()
        {
          PAMI_abort();
          return false;
        }
        inline unsigned                  getNumColors_impl()
        {
          PAMI_abort();
          return 0;
        }
        inline unsigned                  getAllreduceIteration_impl()
        {
          return _allreduce_iteration;
        }
        inline void                      freeAllocations_impl()
        {
          int sz = _cleanupFcns.size();
          for(int i=0; i<sz; i++)
          {
            pami_event_function  fn = _cleanupFcns.front();  _cleanupFcns.pop_front();
            void                *cd = _cleanupDatas.front(); _cleanupDatas.pop_front();
            if(fn) fn(NULL, cd, PAMI_SUCCESS);
          }

          if (_ranks_malloc) __global.heap_mm->free(_ranks);

          _ranks = NULL;
          _ranks_malloc = false;
          free(_allreduce_storage[0]);
          free(_allreduce_storage[1]);
          (*_geometry_map)[_commid] = NULL;
          
          return;
        }
        inline void                      setGlobalContext_impl(bool context)
        {
          PAMI_abort();
          return;
        }
        inline void                      setNumColors_impl(unsigned numcolors)
        {
          PAMI_abort();
          return;
        }
        inline MatchQueue               &asyncCollectivePostQ_impl()
        {
          return _post;
        }
        inline MatchQueue               &asyncCollectiveUnexpQ_impl()
        {
          return _ue;
        }

        inline CCMI_EXECUTOR_TYPE        getAllreduceCompositeStorage_impl(unsigned i)
        {
          if(_allreduce_storage[i] == NULL) {
	    pami_result_t rc;
	    rc = __global.heap_mm->memalign((void **)&_allreduce_storage[i],
                                            sizeof(void *), PAMI_REQUEST_NQUADS*4);
	    PAMI_assertf(rc == PAMI_SUCCESS,
                         "Failed to alloc memory for _allreduce_storage[%d]", i);
	  }
          return _allreduce_storage[i];
        }
        inline COMPOSITE_TYPE            getAllreduceComposite_impl(unsigned i)
        {
          return _allreduce[i];
        }
        inline void                      setAllreduceComposite_impl(COMPOSITE_TYPE c)
        {
          _allreduce[_allreduce_iteration] = c;

          if (c) incrementAllreduceIteration_impl();
        }
        inline void                      setAllreduceComposite_impl(COMPOSITE_TYPE c,
                                                                    unsigned i)
        {
          _allreduce[i] = c;
        }
        inline CCMI_EXECUTOR_TYPE        getAllreduceCompositeStorage_impl()
        {
          if(_allreduce_storage[_allreduce_iteration] == NULL) {
            pami_result_t rc;
	    rc = __global.heap_mm->memalign(
              (void **)&_allreduce_storage[_allreduce_iteration],
			sizeof(void *), PAMI_REQUEST_NQUADS*4);
	    PAMI_assertf(rc == PAMI_SUCCESS,
                         "Failed to alloc memory for _allreduce_storage[%d]",
                         _allreduce_iteration);
	  }
          return _allreduce_storage[_allreduce_iteration];
        }

        inline COMPOSITE_TYPE            getAllreduceComposite_impl()
        {
          return _allreduce[_allreduce_iteration];
        }

        inline void processUnexpBarrier_impl (MatchQueue * ueb_queue,
                                              MemoryAllocator < sizeof(PAMI::Geometry::UnexpBarrierQueueElement), 16 > *ueb_allocator)
        {
          UnexpBarrierQueueElement *ueb = NULL;

          while ( (ueb = (UnexpBarrierQueueElement *)ueb_queue->findAndDelete(_commid)) != NULL )
            {
              /// \todo does NOT support multicontext keystore
              CCMI::Executor::Composite *c = (CCMI::Executor::Composite *) getKey((gkeys_t)ueb->getAlgorithm());
              c->notifyRecv (ueb->getComm(), ueb->getInfo(), NULL, NULL, NULL);
              ueb_allocator->returnObject(ueb);
            }
        }

        // These methods were originally from the PGASRT Communicator class
        inline pami_task_t size_impl(void)
        {
          return _topos[DEFAULT_TOPOLOGY_INDEX].size();
        }
        inline pami_task_t rank_impl(void)
        {
          return _rank;
        }

        inline pami_task_t virtrank_impl()
        {
          return _virtual_rank;
        }

        inline pami_task_t absrankof_impl(int rank)
        {
          return _topos[DEFAULT_TOPOLOGY_INDEX].index2Rank(rank);
        }
        inline pami_task_t virtrankof_impl (int rank)
        {
          return _topos[DEFAULT_TOPOLOGY_INDEX].rank2Index(rank);
        }
        inline pami_task_t ordinal_impl (int rank)
        {
          return _topos[DEFAULT_TOPOLOGY_INDEX].rank2Index(rank);
        }
        inline pami_task_t ordinal_impl ()
        {
          return _virtual_rank;
        }
        inline pami_endpoint_t endpoint_impl (pami_task_t ordinal)
        {
          pami_task_t task = _topos[DEFAULT_TOPOLOGY_INDEX].index2Rank(ordinal);
          pami_endpoint_t ep;
          PAMI_Endpoint_create(_client, task, 0, &ep);
          return ep;
        }
        inline void                       setKey_impl(gkeys_t key, void*value)
        {
          _kvstore[key] = value;
        }
        inline void                      *getKey_impl(gkeys_t key)
        {
          void * value = _kvstore[key];
          return value;
        }

        inline void                       setKey_impl(size_t context_id, ckeys_t key, void*value)
        {
          PAMI_assert(PAMI_GEOMETRY_NUMALGOLISTS > context_id);
          TRACE_ERR((stderr, "<%p>Lapi::setKey(%d, %p)\n", this, key, value));
          _kvcstore[context_id][key] = value;
        }

        inline void                      *getKey_impl(size_t context_id, ckeys_t key)
        {
          PAMI_assert(PAMI_GEOMETRY_NUMALGOLISTS > context_id);
          void * value = _kvcstore[context_id][key];
          TRACE_ERR((stderr, "<%p>Lapi::getKey(%d, %p)\n", this, key, value));
          return value;
        }

        inline AlgoLists<Geometry<PAMI::Geometry::Lapi> > * algorithms_get_lists(size_t context_id,
                                                                                 pami_xfer_type_t  colltype)
        {
          TRACE_ERR((stderr, "<%p>Lapi::algorithms_get_lists()\n", this));

          switch (colltype)
            {
              case PAMI_XFER_BROADCAST:
                return &_broadcasts[context_id];
                break;
              case PAMI_XFER_ALLREDUCE:
                return &_allreduces[context_id];
                break;
              case PAMI_XFER_REDUCE:
                return &_reduces[context_id];
                break;
              case PAMI_XFER_ALLGATHER:
                return &_allgathers[context_id];
                break;
              case PAMI_XFER_ALLGATHERV:
                return &_allgathervs[context_id];
                break;
              case PAMI_XFER_ALLGATHERV_INT:
                return &_allgatherv_ints[context_id];
                break;
              case PAMI_XFER_SCATTER:
                return &_scatters[context_id];
                break;
              case PAMI_XFER_SCATTERV:
                return &_scattervs[context_id];
                break;
              case PAMI_XFER_SCATTERV_INT:
                return &_scatterv_ints[context_id];
                break;
              case PAMI_XFER_GATHER:
                return &_gathers[context_id];
                break;
              case PAMI_XFER_GATHERV:
                return &_gathervs[context_id];
                break;
              case PAMI_XFER_GATHERV_INT:
                return &_gatherv_ints[context_id];
                break;
              case PAMI_XFER_BARRIER:
                return &_barriers[context_id];
                break;
              case PAMI_XFER_ALLTOALL:
                return &_alltoalls[context_id];
                break;
              case PAMI_XFER_ALLTOALLV:
                return &_alltoallvs[context_id];
                break;
              case PAMI_XFER_ALLTOALLV_INT:
                return &_alltoallv_ints[context_id];
                break;
              case PAMI_XFER_SCAN:
                return &_scans[context_id];
                break;
              case PAMI_XFER_AMBROADCAST:
                return &_ambroadcasts[context_id];
                break;
              case PAMI_XFER_AMSCATTER:
                return &_amscatters[context_id];
                break;
              case PAMI_XFER_AMGATHER:
                return &_amgathers[context_id];
                break;
              case PAMI_XFER_AMREDUCE:
                return &_amreduces[context_id];
                break;
              case PAMI_XFER_REDUCE_SCATTER:
                return &_reduce_scatters[context_id];
                break;
              default:
                PAMI_abort();
                return NULL;
                break;
            }
        }

        inline pami_result_t addCollective_impl(pami_xfer_type_t                            colltype,
                                                CCMI::Adaptor::CollectiveProtocolFactory  *factory,
                                                size_t                                     context_id)
        {
          TRACE_ERR((stderr, "<%p>Lapi::addCollective_impl()\n", this));
          AlgoLists<Geometry<PAMI::Geometry::Lapi> > * alist = algorithms_get_lists(context_id, colltype);
          alist->addCollective(factory, this, context_id);
          return PAMI_SUCCESS;
        }

        inline pami_result_t             rmCollective_impl(pami_xfer_type_t                            colltype,
                                                           CCMI::Adaptor::CollectiveProtocolFactory  *factory,
                                                           size_t                                     context_id)
        {
          TRACE_ERR((stderr, "<%p>Lapi::rmCollective_impl()\n", this));
          AlgoLists<Geometry<PAMI::Geometry::Lapi> > * alist = algorithms_get_lists(context_id, colltype);
          alist->rmCollective(factory, this, context_id);
          return PAMI_SUCCESS;
        }

        inline pami_result_t addCollectiveCheck_impl(pami_xfer_type_t                            colltype,
                                                     CCMI::Adaptor::CollectiveProtocolFactory  *factory,
                                                     size_t                                     context_id)
        {
          TRACE_ERR((stderr, "<%p>Lapi::addCollectiveCheck_impl()\n", this));
          AlgoLists<Geometry<PAMI::Geometry::Lapi> > * alist = algorithms_get_lists(context_id, colltype);
          alist->addCollectiveCheck(factory, this, context_id);
          return PAMI_SUCCESS;
        }

        inline pami_result_t             rmCollectiveCheck_impl(pami_xfer_type_t                            colltype,
                                                                CCMI::Adaptor::CollectiveProtocolFactory  *factory,
                                                                size_t                                     context_id)
        {
          TRACE_ERR((stderr, "<%p>Lapi::rmCollectiveCheck_impl()\n", this));
          AlgoLists<Geometry<PAMI::Geometry::Lapi> > * alist = algorithms_get_lists(context_id, colltype);
          alist->rmCollectiveCheck(factory, this, context_id);
          return PAMI_SUCCESS;
        }

        pami_result_t algorithms_num_impl(pami_xfer_type_t  colltype,
                                          size_t             *lengths,
                                          size_t           context_id)
        {
          TRACE_ERR((stderr, "<%p>Lapi::algorithms_num_impl()\n", this));
          AlgoLists<Geometry<PAMI::Geometry::Lapi> > * alist = algorithms_get_lists(context_id, colltype);
          alist->lengths(lengths);
          return PAMI_SUCCESS;
        }

        inline pami_result_t algorithms_info_impl (pami_xfer_type_t   colltype,
                                                   pami_algorithm_t  *algs0,
                                                   pami_metadata_t   *mdata0,
                                                   size_t               num0,
                                                   pami_algorithm_t  *algs1,
                                                   pami_metadata_t   *mdata1,
                                                   size_t             num1,
                                                   size_t            context_id)
        {
          TRACE_ERR((stderr, "<%p>Lapi::algorithms_info_impl(), algs0=%p, num0=%u, mdata0=%p, algs1=%p, num1=%u, mdata1=%p\n", this, algs0, num0, mdata0, algs1, num1, mdata1));
          AlgoLists<Geometry<PAMI::Geometry::Lapi> > * alist = algorithms_get_lists(context_id, colltype);

          for (size_t i = 0; i < num0; i++)
            {
              TRACE_ERR((stderr, "<%p> alist->_algo_list[%u]=%p, mdata0[%u]=%p\n", this, i, alist->_algo_list[i], i, mdata0 ? &mdata0[i] : NULL));

              if (algs0)
                algs0[i]   = (pami_algorithm_t) alist->_algo_list[i];

              if (mdata0)
                alist->_algo_list[i]->metadata(&mdata0[i]);
            }

          for (size_t i = 0; i < num1; i++)
            {
              TRACE_ERR((stderr, "<%p> alist->_algo_list_check[%u]=%p, mdata1[%u]=%p\n", this, i, alist->_algo_list_check[i], i, mdata1 ? &mdata1[i] : NULL));

              if (algs1)
                algs1[i] = (pami_algorithm_t) alist->_algo_list_check[i];

              if (mdata1)
                alist->_algo_list_check[i]->metadata(&mdata1[i]);
            }

          return PAMI_SUCCESS;
        }

        pami_result_t default_barrier(pami_event_function       cb_done,
                                      void                   * cookie,
                                      size_t                   ctxt_id,
                                      pami_context_t            context)
        {
          TRACE_ERR((stderr, "<%p>Lapi::default_barrier()\n", this));
          pami_xfer_t cmd;
          cmd.cb_done = cb_done;
          cmd.cookie = cookie;
          _barriers[ctxt_id]._algo_list[0]->setContext(context);
          return _barriers[ctxt_id]._algo_list[0]->generate(&cmd);
        }

        pami_result_t                    ue_barrier_impl(pami_event_function     cb_done,
                                 void                   *cookie,
                                 size_t                  ctxt_id,
                                 pami_context_t          context)
        {
          TRACE_ERR((stderr, "<%p>Lapi::ue_barrier()\n", this));
          PAMI_assert (_ue_barrier._factory != NULL);

          pami_xfer_t cmd;
          cmd.cb_done = cb_done;
          cmd.cookie = cookie;
          _ue_barrier.setContext(context);
          return _ue_barrier.generate(&cmd);
        }

      void resetUEBarrier_impl()
        {
          _ue_barrier._factory  = (CCMI::Adaptor::CollectiveProtocolFactory*)NULL;
          _ue_barrier._geometry = (PAMI::Geometry::Lapi*)NULL;
        }

      pami_result_t setUEBarrier_impl(CCMI::Adaptor::CollectiveProtocolFactory *f)
        {
          _ue_barrier._factory  =f;
          _ue_barrier._geometry =this;
          return PAMI_SUCCESS;
        }

        pami_client_t getClient_impl()
        {
          return _client;
        }

      
      void setCleanupCallback(pami_event_function fcn, void *data)
        {
          _cleanupFcns.push_back(fcn);
          _cleanupDatas.push_back(data);
        }

      
      private:
        AlgoLists<Geometry<PAMI::Geometry::Lapi> >  _allreduces[PAMI_GEOMETRY_NUMALGOLISTS];
        AlgoLists<Geometry<PAMI::Geometry::Lapi> >  _broadcasts[PAMI_GEOMETRY_NUMALGOLISTS];
        AlgoLists<Geometry<PAMI::Geometry::Lapi> >  _reduces[PAMI_GEOMETRY_NUMALGOLISTS];
        AlgoLists<Geometry<PAMI::Geometry::Lapi> >  _allgathers[PAMI_GEOMETRY_NUMALGOLISTS];

        AlgoLists<Geometry<PAMI::Geometry::Lapi> >  _allgathervs[PAMI_GEOMETRY_NUMALGOLISTS];
        AlgoLists<Geometry<PAMI::Geometry::Lapi> >  _allgatherv_ints[PAMI_GEOMETRY_NUMALGOLISTS];
        AlgoLists<Geometry<PAMI::Geometry::Lapi> >  _scatters[PAMI_GEOMETRY_NUMALGOLISTS];
        AlgoLists<Geometry<PAMI::Geometry::Lapi> >  _scattervs[PAMI_GEOMETRY_NUMALGOLISTS];

        AlgoLists<Geometry<PAMI::Geometry::Lapi> >  _scatterv_ints[PAMI_GEOMETRY_NUMALGOLISTS];
        AlgoLists<Geometry<PAMI::Geometry::Lapi> >  _gathers[PAMI_GEOMETRY_NUMALGOLISTS];
        AlgoLists<Geometry<PAMI::Geometry::Lapi> >  _gathervs[PAMI_GEOMETRY_NUMALGOLISTS];
        AlgoLists<Geometry<PAMI::Geometry::Lapi> >  _gatherv_ints[PAMI_GEOMETRY_NUMALGOLISTS];

        AlgoLists<Geometry<PAMI::Geometry::Lapi> >  _alltoalls[PAMI_GEOMETRY_NUMALGOLISTS];
        AlgoLists<Geometry<PAMI::Geometry::Lapi> >  _alltoallvs[PAMI_GEOMETRY_NUMALGOLISTS];
        AlgoLists<Geometry<PAMI::Geometry::Lapi> >  _alltoallv_ints[PAMI_GEOMETRY_NUMALGOLISTS];
        AlgoLists<Geometry<PAMI::Geometry::Lapi> >  _reduce_scatters[PAMI_GEOMETRY_NUMALGOLISTS];
        AlgoLists<Geometry<PAMI::Geometry::Lapi> >  _ambroadcasts[PAMI_GEOMETRY_NUMALGOLISTS];

        AlgoLists<Geometry<PAMI::Geometry::Lapi> >  _amscatters[PAMI_GEOMETRY_NUMALGOLISTS];
        AlgoLists<Geometry<PAMI::Geometry::Lapi> >  _amgathers[PAMI_GEOMETRY_NUMALGOLISTS];
        AlgoLists<Geometry<PAMI::Geometry::Lapi> >  _amreduces[PAMI_GEOMETRY_NUMALGOLISTS];
        AlgoLists<Geometry<PAMI::Geometry::Lapi> >  _scans[PAMI_GEOMETRY_NUMALGOLISTS];

        AlgoLists<Geometry<PAMI::Geometry::Lapi> >  _barriers[PAMI_GEOMETRY_NUMALGOLISTS];
        Algorithm<PAMI::Geometry::Lapi>             _ue_barrier;

        std::map <int, void*>                       _kvstore;                              // global/geometry key store
        std::map <int, void*>                       _kvcstore[PAMI_GEOMETRY_NUMALGOLISTS]; // per context key store
        int                                         _commid;
        pami_client_t                               _client;
        pami_task_t                                 _rank;
        MatchQueue                                  _ue;
        MatchQueue                                  _post;
        bool                                        _ranks_malloc;
        pami_task_t                                *_ranks;
        std::map<unsigned, pami_geometry_t>        *_geometry_map;
        void                                       *_allreduce_storage[2];
        void                                       *_allreduce[2];
        unsigned                                    _allreduce_async_mode;
        unsigned                                    _allreduce_iteration;
        PAMI::Topology                              _topos[MAX_NUM_TOPOLOGIES];
        pami_task_t                                 _virtual_rank;
        pami_task_t                                 _masterRank;
        std::list<pami_event_function>              _cleanupFcns;
        std::list<void*>                            _cleanupDatas;
        bool                                        _destroyed;
    }; // class Geometry
  };  // namespace Geometry
}; // namespace PAMI


#endif
