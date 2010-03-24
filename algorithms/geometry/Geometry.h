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
#include "sys/pami_attributes.h"
#include "algorithms/interfaces/GeometryInterface.h"
#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include <map>

namespace PAMI
{
  extern std::map<unsigned, pami_geometry_t> geometry_map;
  extern std::map<unsigned, pami_geometry_t> cached_geometry;

  namespace Geometry
  {

    // This class manages "Algorithms", which consist of a geometry
    // object and an algorithm to go along with that object
    template <class T_Geometry>
    class Algorithm
    {
    public:
      Algorithm<T_Geometry>(){}

      inline void metadata(pami_metadata_t   *mdata)
        {
          _factory->metadata(mdata);
        }
      inline pami_result_t generate(pami_xfer_t *xfer)
        {
          CCMI::Executor::Composite *exec=_factory->generate((pami_geometry_t)_geometry,
                                                             (void*) xfer);
          if(exec)
              {
                exec->setDoneCallback(xfer->cb_done, xfer->cookie);
                exec->start();
              }
          return PAMI_SUCCESS;
        }
      static pami_geometry_t mapidtogeometry (int comm)
        {
          pami_geometry_t g = geometry_map[comm];
          return g;
        }

      inline pami_result_t dispatch_set(size_t                     dispatch,
                                       pami_dispatch_callback_fn   fn,
                                       void                     * cookie,
                                       pami_collective_hint_t      options)
        {
          _factory->setAsyncInfo(false,
                                 fn,
                                 mapidtogeometry);

          return PAMI_SUCCESS;
        }


      CCMI::Adaptor::CollectiveProtocolFactory                    *_factory;
      T_Geometry                                                  *_geometry;
    };

    // This class manages lists of algorithms
    template <class T_Geometry>
    class AlgoLists
    {
    public:
      AlgoLists():
        _num_algo(0),
        _num_algo_check(0)
        {
          // Clear the algorithm list
          memset(&_algo_list[0], 0, sizeof(_algo_list));
          memset(&_algo_list[0], 0, sizeof(_algo_list_check));
          memset(&_algo_list_store[0], 0, sizeof(_algo_list_store));
          memset(&_algo_list_check_store[0], 0, sizeof(_algo_list_check_store));
        }
      inline pami_result_t addCollective(CCMI::Adaptor::CollectiveProtocolFactory *factory,
                                        T_Geometry                               *geometry,
                                        size_t                                    context_id)
        {
          _algo_list_store[_num_algo]._factory  = factory;
          _algo_list_store[_num_algo]._geometry = geometry;
          _algo_list[_num_algo]                 = &_algo_list_store[_num_algo];
          _num_algo++;
          return PAMI_SUCCESS;
        }
      inline pami_result_t addCollectiveCheck(CCMI::Adaptor::CollectiveProtocolFactory  *factory,
                                             T_Geometry                                *geometry,
                                             size_t                                     context_id)
        {
          _algo_list_check_store[_num_algo_check]._factory  = factory;
          _algo_list_check_store[_num_algo_check]._geometry = geometry;
          _algo_list_check[_num_algo_check]                 = &_algo_list_check_store[_num_algo_check];
          _num_algo_check++;
          return PAMI_SUCCESS;
        }
      inline pami_result_t lengths(int             *lists_lengths)
        {
          lists_lengths[0] = _num_algo;
          lists_lengths[1] = _num_algo_check;
          return PAMI_SUCCESS;
        }
      int                     _num_algo;
      int                     _num_algo_check;
      Algorithm<T_Geometry>  *_algo_list[16];
      Algorithm<T_Geometry>  *_algo_list_check[16];
      Algorithm<T_Geometry>   _algo_list_store[16];
      Algorithm<T_Geometry>   _algo_list_check_store[16];
    };

    class Common :
      public Geometry<PAMI::Geometry::Common>
    {
    public:
      inline Common(Mapping                *mapping,
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
                                        globalcontext)
        {
          pami_ca_unset_all(&_attributes);
        }
      inline Common (Geometry<PAMI::Geometry::Common> *parent,
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
        _commid(comm)
        {
          int i, j, k, size;
          pami_task_t nranks;

          pami_ca_unset_all(&_attributes);

          size = 0;
          nranks = 0;
          _mytopo = 0;
          _rank = mapping->task();
          _numtopos =  numranges + 1;

          _topos = new PAMI::Topology[_numtopos];

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

          geometry_map[_commid]=this;
          updateCachedGeometry(this, _commid);

          // now we should set the attributes of the topologies or geometry
          // i guess we should have attributes per topo and per geometry
          // \todo need to do the following per topology maybe
          if (_topos[0].isRectSeg())
            pami_ca_set(&_attributes, PAMI_GEOMETRY_RECT);
          // \todo isGlobal is not yet implemented
          //          if (_topos[0].isGlobal())
          //            pami_ca_set(&attributes, PAMI_GEOMETRY_GLOBAL);
          if (PAMI_ISPOF2(_topos[0].size()))
            pami_ca_set(&_attributes, PAMI_GEOMETRY_POF2);
          if (!PAMI_ISEVEN(_topos[0].size()))
            pami_ca_set(&_attributes, PAMI_GEOMETRY_ODD);

        }

      inline pami_topology_t* getTopology_impl(int topo_num)
        {
          return (pami_topology_t *)(&_topos[topo_num]);
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
          assert(0);
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
          assert(0);
          return;
        }
      inline pami_task_t *permutation_sizet_impl()
        {
          return _ranks;
        }

      inline bool                      isRectangle_impl()
        {
          assert(0);
          return false;
        }
      inline bool                      isTorus_impl()
        {
          assert(0);
          return false;
        }
      inline bool                      isTree_impl()
        {
          assert(0);
          return false;
        }
      inline bool                      isGlobalContext_impl()
        {
          assert(0);
          return false;
        }
      inline bool                      isGI_impl()
        {
          assert(0);
          return false;
        }
      inline unsigned                  getNumColors_impl()
        {
          assert(0);
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
          assert(0);
          return;
        }
      inline void                      setNumColors_impl(unsigned numcolors)
        {
          assert(0);
          return;
        }
      inline MatchQueue               &asyncBcastPostQ_impl()
        {
          assert(0);
          return _post;
        }
      inline MatchQueue               &asyncBcastUnexpQ_impl()
        {
          assert(0);
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
          return (Common*)cached_geometry[comm];
        }
      static inline void               updateCachedGeometry_impl(Common *geometry,
                                                                 unsigned comm)
        {
          assert(geometry!=NULL);
          cached_geometry[comm]=(void*)geometry;
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
                pami_result_t result = _topos[i].rankRange(&first, &last);
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
      inline void                       setKey_impl(keys_t key, void*value)
        {
          _kvstore[key]=value;
        }
      inline void                      *getKey_impl(keys_t key)
        {
          void * value = _kvstore[key];
          return value;
        }

      inline AlgoLists<Geometry<PAMI::Geometry::Common> > * algorithms_get_lists(size_t context_id,
                                                                                pami_xfer_type_t  colltype)
        {
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
          AlgoLists<Geometry<PAMI::Geometry::Common> > * alist = algorithms_get_lists(context_id, colltype);
          alist->addCollective(factory, this, context_id);
          return PAMI_SUCCESS;
        }

      inline pami_result_t addCollectiveCheck_impl(pami_xfer_type_t                            colltype,
                                                  CCMI::Adaptor::CollectiveProtocolFactory  *factory,
                                                  size_t                                     context_id)
        {
          AlgoLists<Geometry<PAMI::Geometry::Common> > * alist = algorithms_get_lists(context_id, colltype);
          alist->addCollectiveCheck(factory, this, context_id);
          return PAMI_SUCCESS;
        }

      pami_result_t algorithms_num_impl(pami_xfer_type_t  colltype,
                                       int             *lengths,
                                       size_t           context_id)
        {
          AlgoLists<Geometry<PAMI::Geometry::Common> > * alist = algorithms_get_lists(context_id, colltype);
          alist->lengths(lengths);
          return PAMI_SUCCESS;
        }

      inline pami_result_t algorithms_info_impl (pami_xfer_type_t   colltype,
                                                pami_algorithm_t  *algs0,
                                                pami_metadata_t   *mdata0,
                                                int               num0,
                                                pami_algorithm_t  *algs1,
                                                pami_metadata_t   *mdata1,
                                                int               num1,
                                                size_t            context_id)
        {
          AlgoLists<Geometry<PAMI::Geometry::Common> > * alist = algorithms_get_lists(context_id, colltype);
          for(int i=0; i<num0; i++)
              {
                if(algs0)
                  algs0[i]   =(pami_algorithm_t) alist->_algo_list[i];
                if(mdata0)
                  alist->_algo_list[i]->metadata(&mdata0[i]);
              }
          for(int i=0; i<num1; i++)
              {
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
          pami_xfer_t cmd;
          cmd.cb_done=cb_done;
          cmd.cookie =cookie;
          return _barriers[ctxt_id]._algo_list[0]->generate(&cmd);
        }


    private:
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _allreduces[64];
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _broadcasts[64];
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _reduces[64];
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _allgathers[64];

      AlgoLists<Geometry<PAMI::Geometry::Common> >  _allgathervs[64];
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _allgatherv_ints[64];
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _scatters[64];
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _scattervs[64];

      AlgoLists<Geometry<PAMI::Geometry::Common> >  _scatterv_ints[64];
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _gathers[64];
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _gathervs[64];
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _gatherv_ints[64];

      AlgoLists<Geometry<PAMI::Geometry::Common> >  _alltoalls[64];
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _alltoallvs[64];
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _alltoallv_ints[64];
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _ambroadcasts[64];

      AlgoLists<Geometry<PAMI::Geometry::Common> >  _amscatters[64];
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _amgathers[64];
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _amreduces[64];
      AlgoLists<Geometry<PAMI::Geometry::Common> >  _scans[64];

      AlgoLists<Geometry<PAMI::Geometry::Common> >  _barriers[64];

      std::map <int, void*>                        _kvstore;
      int                                          _commid;
      int                                          _numranges;
      pami_task_t                                   _rank;
      MatchQueue                                   _ue;
      MatchQueue                                   _post;
      pami_task_t                                  *_ranks;
      void                                        *_allreduce_storage[2];
      void                                        *_allreduce[2];
      unsigned                                     _allreduce_async_mode;
      unsigned                                     _allreduce_iteration;
      PAMI::Topology                               *_topos;
      int                                          _numtopos;
      int                                          _mytopo;
      pami_task_t                                   _virtual_rank;
      pami_ca_t                                     _attributes;
    }; // class Geometry
  };  // namespace Geometry
}; // namespace PAMI


#endif
