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

#include "Topology.h"
#include "Mapping.h"
#include "algorithms/geometry/Attributes.h"
#include "algorithms/interfaces/GeometryInterface.h"
#include "algorithms/geometry/Algorithm.h"
#include <map>

#undef TRACE_ERR
#define TRACE_ERR(x) //fprintf x

#undef TRACE_ERR2
#define TRACE_ERR2(x) //fprintf x

#ifndef PAMI_GEOMETRY_NUMALGOLISTS
/// \todo PAMI_GEOMETRY_NUMALGOLISTS == max number of contexts??
#define PAMI_GEOMETRY_NUMALGOLISTS 64
#endif

#include "algorithms/geometry/UnexpBarrierQueueElement.h"
#include "components/atomic/gcc/GccCounter.h"
#include "components/memory/MemoryAllocator.h"
typedef PAMI::Counter::GccProcCounter GeomCompCtr;

#define MASTER_TOPOLOGY_INDEX       -1  // index into _topos[] for master/global sub-topology
#define LOCAL_TOPOLOGY_INDEX        -2  // index into _topos[] for local sub-topology
#define LOCAL_MASTER_TOPOLOGY_INDEX -3  // index into _topos[] for local master sub-topology

namespace PAMI
{
  namespace Geometry
  {
    class Common :
      public Geometry<PAMI::Geometry::Common>
    {
    public:
      inline Common(pami_client_t           client,
        Mapping                *mapping,
                    pami_task_t             *ranks,
                    pami_task_t              nranks,
                    unsigned                comm,
                    unsigned                numcolors,
                    bool                    globalcontext):
        Geometry<PAMI::Geometry::Common>(mapping,
                                        ranks,
                                        nranks,
                                        comm,
                                        numcolors,
                                        globalcontext),
  _client(client)
        {
          TRACE_ERR((stderr, "<%p>Common()\n", this));
        }
      inline Common (pami_client_t                    client,
                     Geometry<PAMI::Geometry::Common> *parent,
                     Mapping                         *mapping,
                     unsigned                         comm,
                     int                              numranges,
                     pami_geometry_range_t             rangelist[]):
        Geometry<PAMI::Geometry::Common>(parent,
                                        mapping,
                                        comm,
                                        numranges,
                                        rangelist),
        _kvstore(),
        _commid(comm),
        _client(client),
        _masterRank(-1)
        {
          TRACE_ERR((stderr, "<%p>Common(parent)\n", this));
          int i, j, k, size;
          pami_task_t nranks;


          size = 0;
          nranks = 0;
          _mytopo = 0;
          _rank = mapping->task();
          _numtopos =  numranges + 1;

          _topos = new PAMI::Topology[_numtopos + 3]; // storing numtopos + local + global + local_master
          _topos = &_topos[3]; // skip local & global storage & local_master

          for (i = 0; i < numranges; i++)
            nranks += (rangelist[i].hi - rangelist[i].lo + 1);

          _ranks = (pami_task_t*)malloc(nranks * sizeof(pami_task_t));

          for (k = 0, i = 0; i < numranges; i++)
              {
                size = rangelist[i].hi - rangelist[i].lo + 1;

                for (j = 0; j < size; j++, k++)
                    {
                      _ranks[k] = rangelist[i].lo + j;
                      if (_ranks[k] == (pami_task_t) _rank)
                        _virtual_rank = k;
                    }
              }

          // this creates the topology including all subtopologies
          new(&_topos[0]) PAMI::Topology(_ranks, nranks);

          // now build up the individual subtopologies
          for (i = 1; i < _numtopos; i++)
              {
                new(&_topos[i]) PAMI::Topology(rangelist[i-1].lo, rangelist[i-1].hi);
                size = rangelist[i-1].hi - rangelist[i-1].lo + 1;

                for (j = 0; j < size; j++)
                    {
                      if ((rangelist[i-1].lo + j) == (pami_task_t) _rank)
                        _mytopo = i;
                    }
              }

          // build local and global topos
          _global_all_topo   = &_topos[0];
          _local_master_topo = &_topos[-1];
          _local_topo        = &_topos[-2];
          _my_master_topo    = &_topos[-3];
          _global_all_topo->subTopologyLocalMaster(_local_master_topo);
          _global_all_topo->subTopologyLocalToMe(_local_topo);
          _local_master_topo->subTopologyLocalToMe(_my_master_topo);

          // Find master participant on the tree/cau network
          size_t            num_master_tasks = _local_master_topo->size();
          size_t            num_local_tasks = _local_topo->size();
          for(size_t k=0; k<num_master_tasks; k++)
            for(size_t j=0; j<num_local_tasks; j++)
              if(_local_master_topo->index2Rank(k) == _local_topo->index2Rank(j))
              {
                _masterRank = _local_topo->index2Rank(j);
                break;
              };

          PAMI::geometry_map[_commid]=this;
          updateCachedGeometry(this, _commid);
        }
      inline Common (pami_client_t                    client,
                     Geometry<PAMI::Geometry::Common> *parent,
                     Mapping                         *mapping,
                     unsigned                         comm,
                     PAMI::Topology                  *topology):
        Geometry<PAMI::Geometry::Common>(parent,
                                        mapping,
                                        comm,
                                        topology),
        _kvstore(),
        _commid(comm),
        _client(client),
        _masterRank(-1)
        {
          TRACE_ERR((stderr, "<%p>Common(parent)\n", this));
          _mytopo = 0;
          _rank = mapping->task();
          _numtopos =  1;

          _topos = new PAMI::Topology[_numtopos + 3]; // storing numtopos + local + global + local_master
          _topos = &_topos[3]; // skip local & global storage & local_master

          // this creates the topology including all subtopologies
          new (&_topos[0]) PAMI::Topology(topology);

          // build local and global topos
          _global_all_topo   = &_topos[0];
          _local_master_topo = &_topos[-1];
          _local_topo        = &_topos[-2];
          _my_master_topo    = &_topos[-3];
          _global_all_topo->subTopologyLocalMaster(_local_master_topo);
          _global_all_topo->subTopologyLocalToMe(_local_topo);
          _local_master_topo->subTopologyLocalToMe(_my_master_topo);
          // Find master participant on the tree/cau network
          size_t            num_master_tasks = _local_master_topo->size();
          size_t            num_local_tasks = _local_topo->size();
          for(size_t k=0; k<num_master_tasks; k++)
            for(size_t j=0; j<num_local_tasks; j++)
              if(_local_master_topo->index2Rank(k) == _local_topo->index2Rank(j))
              {
                _masterRank = _local_topo->index2Rank(j);
                break;
              };

          PAMI::geometry_map[_commid]=this;
          updateCachedGeometry(this, _commid);
        }

       /// \brief Convenience callback used by geometry completion sub-events
       ///
       /// Each sub-event must invoke this when finished, and the final (user) completion
       /// will be invoked once all have checked in.
       ///
       /// \param[in] ctx       The context on which competion is ocurring
       /// \param[in] cookie    The geometry object
       /// \param[in] result    Status of this completion (error or success)
       ///
       static void _done_cb(pami_context_t ctx, void *cookie, pami_result_t result)
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
       inline void setCompletion(pami_event_function fn, void *cookie)
        {
                _cb_done = (pami_callback_t){fn, cookie};
                _cb_result = PAMI_SUCCESS;
        }

       /// \brief Add one completion event to geometry
       ///
       /// This is called each time some work has been started on which
       /// geometry create completion depends (must wait).
       ///
       inline void addCompletion()
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
       inline void rmCompletion(pami_context_t ctx, pami_result_t result)
        {
                if (result != PAMI_SUCCESS) _cb_result = result;
                if (_comp.fetch_and_dec() == 1) {
                        if (_cb_done.function) {
                                _cb_done.function(ctx, _cb_done.clientdata, _cb_result);
                        }
                }
        }

       inline bool isLocalMasterParticipant_impl()
         {
           return _masterRank == _rank;
         }

       inline pami_task_t localMasterParticipant_impl()
         {
           return _masterRank;
         }

      inline pami_topology_t* getTopology_impl(int topo_num)
        {
          return (pami_topology_t *)(&_topos[topo_num]);
        }

      inline pami_topology_t* getLocalTopology_impl()
        {
          return (pami_topology_t*)_local_topo;
        }

      inline pami_topology_t* getLocalMasterTopology_impl()
        {
          return (pami_topology_t*)_local_master_topo;
        }

      inline int myTopologyId_impl()
        {
          return _mytopo;
        }
      inline int getNumTopos_impl()
        {
          return _numtopos;
        }
      inline int                       getColorsArray_impl()
        {
          PAMI_abort();
          return 0;
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
      inline pami_topology_t* getMyMasterTopology_impl()
        {
          return (pami_topology_t*)_my_master_topo;
        }
      inline unsigned                  comm_impl()
        {
          return _commid;
        }
      inline pami_task_t  *ranks_impl()
        {
          return _ranks;
        }
      inline pami_task_t *ranks_sizet_impl()
        {
          return _ranks;
        }
      inline pami_task_t nranks_impl()
        {
          return _topos[0].size();
        }
      inline pami_task_t myIdx_impl()
        {
          return _topos[0].rank2Index(_rank);
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
          return _ranks;
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
          return _ranks;
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
          delete [] _topos;
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
          if(_allreduce_storage[i] == NULL)
            _allreduce_storage[i] =  malloc (PAMI_REQUEST_NQUADS*4);
          return _allreduce_storage[i];
        }
      inline COMPOSITE_TYPE            getAllreduceComposite_impl(unsigned i)
        {
          return _allreduce[i];
        }
      inline void                      setAllreduceComposite_impl(COMPOSITE_TYPE c)
        {
          _allreduce[_allreduce_iteration] = c;
          if(c) incrementAllreduceIteration_impl();
        }
      inline void                      setAllreduceComposite_impl(COMPOSITE_TYPE c,
                                                                  unsigned i)
        {
          _allreduce[i] = c;
        }
      inline CCMI_EXECUTOR_TYPE        getAllreduceCompositeStorage_impl()
        {
          if(_allreduce_storage[_allreduce_iteration] == NULL)
            _allreduce_storage[_allreduce_iteration] = malloc (PAMI_REQUEST_NQUADS*4);
          return _allreduce_storage[_allreduce_iteration];
        }

      inline COMPOSITE_TYPE            getAllreduceComposite_impl()
        {
          return _allreduce[_allreduce_iteration];
        }


      static inline Common   *getCachedGeometry_impl(unsigned comm)
        {
          return (Common*)PAMI::cached_geometry[comm];
        }
      static inline void               updateCachedGeometry_impl(Common *geometry,
                                                                 unsigned comm)
      {
  PAMI_assert(geometry!=NULL);
  PAMI::cached_geometry[comm]=(void*)geometry;
      }

      static inline void registerUnexpBarrier_impl (unsigned comm, pami_quad_t &info,
                unsigned peer, unsigned algorithm)
      {
  UnexpBarrierQueueElement *ueb = (UnexpBarrierQueueElement *) _ueb_allocator.allocateObject();

  new (ueb) UnexpBarrierQueueElement (comm, info, peer, algorithm);
  _ueb_queue.pushTail(ueb);
      }

      inline void processUnexpBarrier_impl () {
  UnexpBarrierQueueElement *ueb = NULL;
  while ( (ueb = (UnexpBarrierQueueElement *)_ueb_queue.findAndDelete(_commid)) != NULL ) {
    CCMI::Executor::Composite *c = (CCMI::Executor::Composite *) getKey((gkeys_t)ueb->getAlgorithm());/// \todo does NOT support multicontext keystore
    c->notifyRecv (ueb->getComm(), ueb->getInfo(), NULL, NULL, NULL);
    _ueb_allocator.returnObject(ueb);
  }
      }

      // These methods were originally from the PGASRT Communicator class
      inline pami_task_t size_impl(void)
        {
          return _topos[0].size();
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
          // the following was written assuming range topologies, must change
          // implementation to account for other topologies
          int i, range_size, rank_left = rank;
          pami_task_t first, last;
          for(i = 1; i < _numtopos; i++)
              {
                pami_result_t result;
                result = _topos[i].rankRange(&first, &last);
                PAMI_assert(result == PAMI_SUCCESS);
                range_size = _topos[i].size();
                rank_left -= range_size;
                if(rank_left <= 0)
                    {
                      if(rank_left == 0)
                        continue;

                      int offset = range_size + rank_left;
                      return first + offset;
                    }
              }
          return -1;
        }
      inline pami_task_t virtrankof_impl (int rank)
        {
          return _topos[0].rank2Index(rank);
        }
      inline void                       setKey_impl(gkeys_t key, void*value)
        {
          TRACE_ERR((stderr, "<%p>Common::setKey(%d, %p)\n", this, key, value));
          _kvstore[key]=value;
        }
      inline void                      *getKey_impl(gkeys_t key)
        {
          void * value = _kvstore[key];
          TRACE_ERR((stderr, "<%p>Common::getKey(%d, %p)\n", this, key, value));
          return value;
        }
      inline void                       setKey_impl(size_t context_id, ckeys_t key, void*value)
        {
          PAMI_assert(PAMI_GEOMETRY_NUMALGOLISTS > context_id);
          TRACE_ERR((stderr, "<%p>Common::setKey(%d, %p)\n", this, key, value));
          _kvcstore[context_id][key]=value;
        }
      inline void                      *getKey_impl(size_t context_id, ckeys_t key)
        {
          PAMI_assert(PAMI_GEOMETRY_NUMALGOLISTS > context_id);
          void * value = _kvcstore[context_id][key];
          TRACE_ERR((stderr, "<%p>Common::getKey(%d, %p)\n", this, key, value));
          return value;
        }

      inline AlgoLists<Geometry<PAMI::Geometry::Common> > * algorithms_get_lists(size_t context_id,
                                                                                pami_xfer_type_t  colltype)
        {
          TRACE_ERR((stderr, "<%p>Common::algorithms_get_lists(%zu, %u)\n", this, context_id, colltype));
          PAMI_assert(PAMI_GEOMETRY_NUMALGOLISTS > context_id);
          switch(colltype)
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
                    PAMI_abortf("colltype %u\n",colltype);
                    return NULL;
                    break;
              }
        }

      inline pami_result_t addCollective_impl(pami_xfer_type_t                            colltype,
                                             CCMI::Adaptor::CollectiveProtocolFactory  *factory,
                                             size_t                                     context_id)
        {
          TRACE_ERR((stderr, "<%p>Common::addCollective_impl()\n", this));
          AlgoLists<Geometry<PAMI::Geometry::Common> > * alist = algorithms_get_lists(context_id, colltype);
          alist->addCollective(factory, this, context_id);
          return PAMI_SUCCESS;
        }

      inline pami_result_t rmCollective_impl(pami_xfer_type_t                            colltype,
                                             CCMI::Adaptor::CollectiveProtocolFactory  *factory,
                                             size_t                                     context_id)
        {
          TRACE_ERR((stderr, "<%p>Common::rmCollective_impl()\n", this));
          AlgoLists<Geometry<PAMI::Geometry::Common> > * alist = algorithms_get_lists(context_id, colltype);
          alist->rmCollective(factory, this, context_id);
          return PAMI_SUCCESS;
        }

      inline pami_result_t addCollectiveCheck_impl(pami_xfer_type_t                            colltype,
                                                  CCMI::Adaptor::CollectiveProtocolFactory  *factory,
                                                  size_t                                     context_id)
        {
          TRACE_ERR((stderr, "<%p>Common::addCollectiveCheck_impl()\n", this));
          AlgoLists<Geometry<PAMI::Geometry::Common> > * alist = algorithms_get_lists(context_id, colltype);
          alist->addCollectiveCheck(factory, this, context_id);
          return PAMI_SUCCESS;
        }

      pami_result_t algorithms_num_impl(pami_xfer_type_t  colltype,
                                       size_t             *lengths,
                                       size_t           context_id)
        {
          TRACE_ERR((stderr, "<%p>Common::algorithms_num_impl()\n", this));
          AlgoLists<Geometry<PAMI::Geometry::Common> > * alist = algorithms_get_lists(context_id, colltype);
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
          TRACE_ERR((stderr, "<%p>Common::algorithms_info_impl(), algs0=%p, num0=%zu, mdata0=%p, algs1=%p, num1=%zu, mdata1=%p\n", this, algs0,num0,mdata0,algs1,num1,mdata1));
          AlgoLists<Geometry<PAMI::Geometry::Common> > * alist = algorithms_get_lists(context_id, colltype);
          for(size_t i=0; i<num0; i++)
              {
            TRACE_ERR((stderr, "<%p> alist->_algo_list[%zu]=%p, mdata0[%zu]=%p\n", this, i, alist->_algo_list[i],i,mdata0?&mdata0[i]:NULL));
                if(algs0)
                  algs0[i]   =(pami_algorithm_t) alist->_algo_list[i];
                if(mdata0)
                  alist->_algo_list[i]->metadata(&mdata0[i]);
              }
          for(size_t i=0; i<num1; i++)
              {
            TRACE_ERR((stderr, "<%p> alist->_algo_list_check[%zu]=%p, mdata1[%zu]=%p\n", this, i, alist->_algo_list_check[i],i,mdata1?&mdata1[i]:NULL));
                if(algs1)
                  algs1[i] =(pami_algorithm_t) alist->_algo_list_check[i];
                if(mdata1)
                  alist->_algo_list_check[i]->metadata(&mdata1[i]);
              }
          return PAMI_SUCCESS;
        }

      pami_result_t default_barrier(pami_event_function       cb_done,
                                   void                   * cookie,
                                   size_t                   ctxt_id,
                                   pami_context_t            context)
        {
          TRACE_ERR((stderr, "<%p>Common::default_barrier()\n", this));
          pami_xfer_t cmd;
          cmd.cb_done=cb_done;
          cmd.cookie =cookie;
          return _barriers[ctxt_id]._algo_list[0]->generate(&cmd);
        }

      pami_result_t ue_barrier(pami_event_function     cb_done,
                               void                   *cookie,
                               size_t                  ctxt_id,
                               pami_context_t          context)
        {
          TRACE_ERR((stderr, "<%p>Common::ue_barrier()\n", this));
          pami_xfer_t cmd;
          cmd.cb_done=cb_done;
          cmd.cookie =cookie;
          return _ue_barrier.generate(&cmd);
        }

      pami_client_t getClient_impl()
        {
          return _client;
        }

      void setUEBarrier(CCMI::Adaptor::CollectiveProtocolFactory *f)
        {
          _ue_barrier._factory  =f;
          _ue_barrier._geometry =this;
        }


    private:
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _allreduces[PAMI_GEOMETRY_NUMALGOLISTS];
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _broadcasts[PAMI_GEOMETRY_NUMALGOLISTS];
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _reduces[PAMI_GEOMETRY_NUMALGOLISTS];
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _allgathers[PAMI_GEOMETRY_NUMALGOLISTS];

      AlgoLists<Geometry<PAMI::Geometry::Common> >  _allgathervs[PAMI_GEOMETRY_NUMALGOLISTS];
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _allgatherv_ints[PAMI_GEOMETRY_NUMALGOLISTS];
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _scatters[PAMI_GEOMETRY_NUMALGOLISTS];
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _scattervs[PAMI_GEOMETRY_NUMALGOLISTS];

      AlgoLists<Geometry<PAMI::Geometry::Common> >  _scatterv_ints[PAMI_GEOMETRY_NUMALGOLISTS];
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _gathers[PAMI_GEOMETRY_NUMALGOLISTS];
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _gathervs[PAMI_GEOMETRY_NUMALGOLISTS];
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _gatherv_ints[PAMI_GEOMETRY_NUMALGOLISTS];

      AlgoLists<Geometry<PAMI::Geometry::Common> >  _alltoalls[PAMI_GEOMETRY_NUMALGOLISTS];
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _alltoallvs[PAMI_GEOMETRY_NUMALGOLISTS];
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _alltoallv_ints[PAMI_GEOMETRY_NUMALGOLISTS];
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _reduce_scatters[PAMI_GEOMETRY_NUMALGOLISTS];
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _ambroadcasts[PAMI_GEOMETRY_NUMALGOLISTS];

      AlgoLists<Geometry<PAMI::Geometry::Common> >  _amscatters[PAMI_GEOMETRY_NUMALGOLISTS];
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _amgathers[PAMI_GEOMETRY_NUMALGOLISTS];
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _amreduces[PAMI_GEOMETRY_NUMALGOLISTS];
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _scans[PAMI_GEOMETRY_NUMALGOLISTS];

      AlgoLists<Geometry<PAMI::Geometry::Common> >  _barriers[PAMI_GEOMETRY_NUMALGOLISTS];
      Algorithm<PAMI::Geometry::Common>             _ue_barrier;

      std::map <int, void*>                         _kvstore;                              // global/geometry key store
      std::map <int, void*>                         _kvcstore[PAMI_GEOMETRY_NUMALGOLISTS]; // per context key store
      int                                           _commid;
      pami_client_t                                 _client;
      int                                           _numranges;
      pami_task_t                                   _rank;
      MatchQueue                                    _ue;
      MatchQueue                                    _post;
      pami_task_t                                  *_ranks;
      void                                         *_allreduce_storage[2];
      void                                         *_allreduce[2];
      unsigned                                      _allreduce_async_mode;
      unsigned                                      _allreduce_iteration;
      PAMI::Topology                               *_topos;
      int                                           _numtopos;
      int                                           _mytopo;
      pami_task_t                                   _virtual_rank;
      pami_callback_t                               _cb_done;
      pami_result_t                                 _cb_result;
      GeomCompCtr                                   _comp;
      pami_task_t                                   _masterRank;
      PAMI::Topology                               *_global_all_topo;
      PAMI::Topology                               *_local_master_topo;
      PAMI::Topology                               *_local_topo;
      PAMI::Topology                               *_my_master_topo;

    }; // class Geometry
  };  // namespace Geometry
}; // namespace PAMI


#endif
