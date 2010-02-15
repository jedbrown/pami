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
#include "sys/xmi_attributes.h"
#include "algorithms/interfaces/GeometryInterface.h"
#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include <map>

namespace XMI
{
  extern std::map<unsigned, xmi_geometry_t> geometry_map;
  extern std::map<unsigned, xmi_geometry_t> cached_geometry;

  namespace Geometry
  {

    // This class manages "Algorithms", which consist of a geometry
    // object and an algorithm to go along with that object
    template <class T_Geometry>
    class Algorithm
    {
    public:
      Algorithm<T_Geometry>(){}

      inline void metadata(xmi_metadata_t   *mdata)
        {
          _factory->metadata(mdata);
        }
      inline xmi_result_t generate(xmi_xfer_t *xfer)
        {
          CCMI::Executor::Composite *exec=_factory->generate((xmi_geometry_t)_geometry,
                                                             (void*) xfer);
          if(exec)
              {
                exec->setDoneCallback(xfer->cb_done, xfer->cookie);
                exec->start();
              }
          return XMI_SUCCESS;
        }
      static xmi_geometry_t mapidtogeometry (int comm)
        {
          xmi_geometry_t g = geometry_map[comm];
          return g;
        }

      inline xmi_result_t dispatch_set(size_t                     dispatch,
                                       xmi_dispatch_callback_fn   fn,
                                       void                     * cookie,
                                       xmi_collective_hint_t      options)
        {
          _factory->setAsyncInfo(false,
                                 fn,
                                 mapidtogeometry);

          return XMI_SUCCESS;
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
      inline xmi_result_t addCollective(CCMI::Adaptor::CollectiveProtocolFactory *factory,
                                        T_Geometry                               *geometry,
                                        size_t                                    context_id)
        {
          _algo_list_store[_num_algo]._factory  = factory;
          _algo_list_store[_num_algo]._geometry = geometry;
          _algo_list[_num_algo]                 = &_algo_list_store[_num_algo];
          _num_algo++;
          return XMI_SUCCESS;
        }
      inline xmi_result_t addCollectiveCheck(CCMI::Adaptor::CollectiveProtocolFactory  *factory,
                                             T_Geometry                                *geometry,
                                             size_t                                     context_id)
        {
          _algo_list_check_store[_num_algo_check]._factory  = factory;
          _algo_list_check_store[_num_algo_check]._geometry = geometry;
          _algo_list_check[_num_algo_check]                 = &_algo_list_check_store[_num_algo_check];
          _num_algo_check++;
          return XMI_SUCCESS;
        }
      inline xmi_result_t lengths(int             *lists_lengths)
        {
          lists_lengths[0] = _num_algo;
          lists_lengths[1] = _num_algo_check;
          return XMI_SUCCESS;
        }
      int                     _num_algo;
      int                     _num_algo_check;
      Algorithm<T_Geometry>  *_algo_list[16];
      Algorithm<T_Geometry>  *_algo_list_check[16];
      Algorithm<T_Geometry>   _algo_list_store[16];
      Algorithm<T_Geometry>   _algo_list_check_store[16];
    };

    class Common :
      public Geometry<XMI::Geometry::Common>
    {
    public:
      inline Common(Mapping                *mapping,
                    xmi_task_t             *ranks,
                    xmi_task_t              nranks,
                    unsigned                comm,
                    unsigned                numcolors,
                    bool                    globalcontext):
        Geometry<XMI::Geometry::Common>(mapping,
                                        ranks,
                                        nranks,
                                        comm,
                                        numcolors,
                                        globalcontext)
        {
          xmi_ca_unset_all(&_attributes);
        }
      inline Common (Geometry<XMI::Geometry::Common> *parent,
                     Mapping                         *mapping,
                     unsigned                         comm,
                     int                              numranges,
                     xmi_geometry_range_t             rangelist[]):
	Geometry<XMI::Geometry::Common>(parent,
                                        mapping,
                                        comm,
                                        numranges,
                                        rangelist),
	_kvstore(),
        _commid(comm)
        {
          int i, j, k, size;
          xmi_task_t nranks;

          xmi_ca_unset_all(&_attributes);

          size = 0;
          nranks = 0;
          _mytopo = 0;
          _rank = mapping->task();
          _numtopos =  numranges + 1;

          _topos = new XMI::Topology[_numtopos];

          for (i = 0; i < numranges; i++)
            nranks += (rangelist[i].hi - rangelist[i].lo + 1);

          _ranks = (xmi_task_t*)malloc(nranks * sizeof(xmi_task_t));

          for (k = 0, i = 0; i < numranges; i++)
              {
                size = rangelist[i].hi - rangelist[i].lo + 1;

                for (j = 0; j < size; j++, k++)
                    {
                      _ranks[k] = rangelist[i].lo + j;
                      if (_ranks[k] == (xmi_task_t) _rank)
                        _virtual_rank = k;
                    }
              }

          // this creates the topology including all subtopologies
          new(&_topos[0]) XMI::Topology(_ranks, nranks);

          // now build up the individual subtopologies
          for (i = 1; i < _numtopos; i++)
              {
                new(&_topos[i]) XMI::Topology(rangelist[i-1].lo, rangelist[i-1].hi);
                size = rangelist[i-1].hi - rangelist[i-1].lo + 1;

                for (j = 0; j < size; j++)
                    {
                      if ((rangelist[i-1].lo + j) == (xmi_task_t) _rank)
                        _mytopo = i;
                    }
              }

          geometry_map[_commid]=this;
          updateCachedGeometry(this, _commid);

          // now we should set the attributes of the topologies or geometry
          // i guess we should have attributes per topo and per geometry
          // \todo need to do the following per topology maybe
          if (_topos[0].isRectSeg())
            xmi_ca_set(&_attributes, XMI_GEOMETRY_RECT);
          // \todo isGlobal is not yet implemented
	  //          if (_topos[0].isGlobal())
	  //            xmi_ca_set(&attributes, XMI_GEOMETRY_GLOBAL);
          if (XMI_ISPOF2(_topos[0].size()))
            xmi_ca_set(&_attributes, XMI_GEOMETRY_POF2);
          if (!XMI_ISEVEN(_topos[0].size()))
            xmi_ca_set(&_attributes, XMI_GEOMETRY_ODD);

        }

      inline xmi_topology_t* getTopology_impl(int topo_num)
        {
          return (xmi_topology_t *)(&_topos[topo_num]);
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
      inline xmi_task_t  *ranks_impl()
        {
          return _ranks;
        }
      inline xmi_task_t *ranks_sizet_impl()
        {
          return _ranks;
        }
      inline xmi_task_t nranks_impl()
        {
          return _topos[0].size();
        }
      inline xmi_task_t myIdx_impl()
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
      inline xmi_task_t *permutation_impl()
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
      inline xmi_task_t *permutation_sizet_impl()
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
            _allreduce_storage[i] =  malloc (XMI_REQUEST_NQUADS*4);
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
            _allreduce_storage[_allreduce_iteration] = malloc (XMI_REQUEST_NQUADS*4);
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
      inline xmi_task_t size_impl(void)
        {
          return _topos[0].size();
        }
      inline xmi_task_t rank_impl(void)
        {
          return _rank;
        }

      inline xmi_task_t virtrank_impl()
        {
          return _virtual_rank;
        }

      inline xmi_task_t absrankof_impl(int rank)
        {
          // the following was written assuming range topologies, must change
          // implementation to account for other topologies
          int i, range_size, rank_left = rank;
          xmi_task_t first, last;
          for(i = 1; i < _numtopos; i++)
              {
                xmi_result_t result = _topos[i].rankRange(&first, &last);
                XMI_assert(result == XMI_SUCCESS);
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
      inline xmi_task_t virtrankof_impl (int rank)
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

      inline AlgoLists<Geometry<XMI::Geometry::Common> > * algorithms_get_lists(size_t context_id,
                                                                                xmi_xfer_type_t  colltype)
        {
          switch(colltype)
              {
                  case XMI_XFER_BROADCAST:
                    return &_broadcasts[context_id];
                    break;
                  case XMI_XFER_ALLREDUCE:
                    return &_allreduces[context_id];
                    break;
                  case XMI_XFER_REDUCE:
                    return &_reduces[context_id];
                    break;
                  case XMI_XFER_ALLGATHER:
                    return &_allgathers[context_id];
                    break;
                  case XMI_XFER_ALLGATHERV:
                    return &_allgathervs[context_id];
                    break;
                  case XMI_XFER_ALLGATHERV_INT:
                    return &_allgatherv_ints[context_id];
                    break;
                  case XMI_XFER_SCATTER:
                    return &_scatters[context_id];
                    break;
                  case XMI_XFER_SCATTERV:
                    return &_scattervs[context_id];
                    break;
                  case XMI_XFER_SCATTERV_INT:
                    return &_scatterv_ints[context_id];
                    break;
                  case XMI_XFER_BARRIER:
                    return &_barriers[context_id];
                    break;
                  case XMI_XFER_ALLTOALL:
                    return &_alltoalls[context_id];
                    break;
                  case XMI_XFER_ALLTOALLV:
                    return &_alltoallvs[context_id];
                    break;
                  case XMI_XFER_ALLTOALLV_INT:
                    return &_alltoallv_ints[context_id];
                    break;
                  case XMI_XFER_SCAN:
                    return &_scans[context_id];
                    break;
                  case XMI_XFER_AMBROADCAST:
                    return &_ambroadcasts[context_id];
                    break;
                  case XMI_XFER_AMSCATTER:
                    return &_amscatters[context_id];
                    break;
                  case XMI_XFER_AMGATHER:
                    return &_amgathers[context_id];
                    break;
                  case XMI_XFER_AMREDUCE:
                    return &_amreduces[context_id];
                    break;
                  default:
                    XMI_abort();
                    return NULL;
                    break;
              }
        }

      inline xmi_result_t addCollective_impl(xmi_xfer_type_t                            colltype,
                                             CCMI::Adaptor::CollectiveProtocolFactory  *factory,
                                             size_t                                     context_id)
        {
          AlgoLists<Geometry<XMI::Geometry::Common> > * alist = algorithms_get_lists(context_id, colltype);
          alist->addCollective(factory, this, context_id);
          return XMI_SUCCESS;
        }

      inline xmi_result_t addCollectiveCheck_impl(xmi_xfer_type_t                            colltype,
                                                  CCMI::Adaptor::CollectiveProtocolFactory  *factory,
                                                  size_t                                     context_id)
        {
          AlgoLists<Geometry<XMI::Geometry::Common> > * alist = algorithms_get_lists(context_id, colltype);
          alist->addCollectiveCheck(factory, this, context_id);
          return XMI_SUCCESS;
        }

      xmi_result_t algorithms_num_impl(xmi_xfer_type_t  colltype,
                                       int             *lengths,
                                       size_t           context_id)
        {
          AlgoLists<Geometry<XMI::Geometry::Common> > * alist = algorithms_get_lists(context_id, colltype);
          alist->lengths(lengths);
          return XMI_SUCCESS;
        }

      inline xmi_result_t algorithms_info_impl (xmi_xfer_type_t   colltype,
                                                xmi_algorithm_t  *algs0,
                                                xmi_metadata_t   *mdata0,
                                                int               num0,
                                                xmi_algorithm_t  *algs1,
                                                xmi_metadata_t   *mdata1,
                                                int               num1,
                                                size_t            context_id)
        {
          AlgoLists<Geometry<XMI::Geometry::Common> > * alist = algorithms_get_lists(context_id, colltype);
          for(int i=0; i<num0; i++)
              {
                if(algs0)
                  algs0[i]   =(xmi_algorithm_t) alist->_algo_list[i];
                if(mdata0)
                  alist->_algo_list[i]->metadata(&mdata0[i]);
              }
          for(int i=0; i<num1; i++)
              {
                if(algs1)
                  algs1[i] =(xmi_algorithm_t) alist->_algo_list_check[i];
                if(mdata1)
                  alist->_algo_list_check[i]->metadata(&mdata1[i]);
              }
          return XMI_SUCCESS;
        }

      xmi_result_t default_barrier(xmi_event_function       cb_done,
                                   void                   * cookie,
                                   size_t                   ctxt_id,
                                   xmi_context_t            context)
        {
          xmi_xfer_t cmd;
          cmd.cb_done=cb_done;
          cmd.cookie =cookie;
          return _barriers[ctxt_id]._algo_list[0]->generate(&cmd);
        }


    private:
      AlgoLists<Geometry<XMI::Geometry::Common> >  _allreduces[64];
      AlgoLists<Geometry<XMI::Geometry::Common> >  _broadcasts[64];
      AlgoLists<Geometry<XMI::Geometry::Common> >  _reduces[64];
      AlgoLists<Geometry<XMI::Geometry::Common> >  _allgathers[64];

      AlgoLists<Geometry<XMI::Geometry::Common> >  _allgathervs[64];
      AlgoLists<Geometry<XMI::Geometry::Common> >  _allgatherv_ints[64];
      AlgoLists<Geometry<XMI::Geometry::Common> >  _scatters[64];
      AlgoLists<Geometry<XMI::Geometry::Common> >  _scattervs[64];

      AlgoLists<Geometry<XMI::Geometry::Common> >  _scatterv_ints[64];
      AlgoLists<Geometry<XMI::Geometry::Common> >  _gathers[64];
      AlgoLists<Geometry<XMI::Geometry::Common> >  _gathervs[64];
      AlgoLists<Geometry<XMI::Geometry::Common> >  _gatherv_ints[64];

      AlgoLists<Geometry<XMI::Geometry::Common> >  _alltoalls[64];
      AlgoLists<Geometry<XMI::Geometry::Common> >  _alltoallvs[64];
      AlgoLists<Geometry<XMI::Geometry::Common> >  _alltoallv_ints[64];
      AlgoLists<Geometry<XMI::Geometry::Common> >  _ambroadcasts[64];

      AlgoLists<Geometry<XMI::Geometry::Common> >  _amscatters[64];
      AlgoLists<Geometry<XMI::Geometry::Common> >  _amgathers[64];
      AlgoLists<Geometry<XMI::Geometry::Common> >  _amreduces[64];
      AlgoLists<Geometry<XMI::Geometry::Common> >  _scans[64];

      AlgoLists<Geometry<XMI::Geometry::Common> >  _barriers[64];

      std::map <int, void*>                        _kvstore;
      int                                          _commid;
      int                                          _numranges;
      xmi_task_t                                   _rank;
      MatchQueue                                   _ue;
      MatchQueue                                   _post;
      xmi_task_t                                  *_ranks;
      void                                        *_allreduce_storage[2];
      void                                        *_allreduce[2];
      unsigned                                     _allreduce_async_mode;
      unsigned                                     _allreduce_iteration;
      XMI::Topology                               *_topos;
      int                                          _numtopos;
      int                                          _mytopo;
      xmi_task_t                                   _virtual_rank;
      xmi_ca_t                                     _attributes;
    }; // class Geometry
  };  // namespace Geometry
}; // namespace XMI


#endif
