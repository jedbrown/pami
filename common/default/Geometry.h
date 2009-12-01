/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file common/default/Geometry.h
 * \brief ???
 */

#ifndef __common_default_Geometry_h__
#define __common_default_Geometry_h__

//#include "config.h"

//#include "components/mapping/mpi/mpimapping.h"
#define XMI_GEOMETRY_CLASS XMI::Geometry::Common<XMI_MAPPING_CLASS>
#define GEOMETRY_STORAGE XMI_REQUEST_NQUADS*4

#include "sys/xmi_attributes.h"
#include "common/GeometryInterface.h"
#include <map>

namespace XMI
{
  extern std::map<unsigned, xmi_geometry_t> geometry_map;
  extern std::map<unsigned, xmi_geometry_t> cached_geometry;
  namespace Geometry
  {
    template <class T_Mapping>
      class Common : public Geometry<XMI::Geometry::Common<XMI_MAPPING_CLASS>, T_Mapping>
    {
    public:
      inline Common(T_Mapping              *mapping,
                    xmi_task_t             *ranks,
                    xmi_task_t              nranks,
                    unsigned                comm,
                    unsigned                numcolors,
                    bool                    globalcontext):
      Geometry<XMI::Geometry::Common<T_Mapping>, T_Mapping>(mapping,ranks,nranks,comm,
                                                            numcolors,globalcontext)
      {
        xmi_ca_unset_all(&attributes);
      }
      inline Common (T_Mapping           *mapping,
                     unsigned             comm,
                     int                  numranges,
                     xmi_geometry_range_t rangelist[]):
	Geometry<XMI::Geometry::Common<T_Mapping>, T_Mapping>(mapping, comm, numranges, rangelist),
	_kvstore(),
        _commid(comm)
        {
          int i, j, k, size;
          xmi_task_t nranks;
          xmi_task_t *ranks;

          xmi_ca_unset_all(&attributes);

          size = 0;
          nranks = 0;
          _mytopo = 0;
          _rank = mapping->task();
          _numtopos = (numranges == 1)? 1 : numranges + 1;
          
          _topos = new XMI::Topology[numranges];

          for (i = 0; i < numranges; i++)
            nranks += (rangelist[i].hi - rangelist[i].lo + 1);

          ranks = (xmi_task_t*)malloc(nranks * sizeof(xmi_task_t));

          for (k = 0, i = 0; i < numranges; i++)
          {
            size = rangelist[i].hi - rangelist[i].lo + 1;
            
            for (j = 0; j < size; j++, k++)
            {
              ranks[k] = rangelist[i].lo + j;
              if (ranks[k] == (xmi_task_t) _rank)
                _virtual_rank = k;
            }
          }

          // this creates the topology including all subtopologies
          new(&_topos[0]) XMI::Topology(ranks, nranks);
          _topos[0].rankList(&_ranks);
          
          // now build up the individual subtopologies
          for (i = 1; i < numranges; i++)
          {
            new(&_topos[i]) XMI::Topology(rangelist[i].lo, rangelist[i].hi);
            size = rangelist[i].hi - rangelist[i].lo + 1;
            
            for (j = 0; j < size; j++)
            {
              ranks[j] = rangelist[i].lo + j;
              if (ranks[j] == (xmi_task_t) _rank)
                _mytopo = i;
            }
          }

          geometry_map[_commid]=this;
          updateCachedGeometry(this, _commid);
          free(ranks);

          // now we should set the attributes of the topologies or geometry
          // i guess we should have attributes per topo and per geometry
#warning need to do the following per topology maybe
          if (_topos[0].isRectSeg())
            xmi_ca_set(&attributes, XMI_GEOMETRY_RECT);
          if (_topos[0].isGlobal())
            xmi_ca_set(&attributes, XMI_GEOMETRY_GLOBAL);
          if (XMI_ISPOF2(_topos[0].size()))
            xmi_ca_set(&attributes, XMI_GEOMETRY_POF2);
          if (!XMI_ISEVEN(_topos[0].size()))
            xmi_ca_set(&attributes, XMI_GEOMETRY_ODD);
        }

      inline XMI::Topology* getTopology_impl(int topo_num)
      {
        return _topos[topo_num];
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
#if 0
      inline RECTANGLE_TYPE            rectangle_impl()
      {
      }
      inline RECTANGLE_TYPE            rectangle_mesh_impl()
      {
      }
      inline void                      setBarrierExecutor_impl(EXECUTOR_TYPE bar)
      {
      }
      inline EXECUTOR_TYPE             getLocalBarrierExecutor_impl()
      {
      }
      inline void                      setLocalBarrierExecutor_impl(EXECUTOR_TYPE bar)
      {
      }

      inline EXECUTOR_TYPE             getCollectiveExecutor_impl(unsigned color=0)
      {
      }
      inline void                      setCollectiveExecutor_impl(EXECUTOR_TYPE exe,
                                                                  unsigned color=0)
      {
      }

      inline void            *getBarrierExecutor_impl()
      {
        assert(0);
        return NULL;
      }
#endif

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
        size_t first, last;
        for(i = 0; i < _numtopos; i++)
        {
          _topos[i].rankRange(&first, &last);
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

    private:
      std::map <int, void*> _kvstore;
      int                   _commid;
      int                   _numranges;
      xmi_task_t            _rank;
      MatchQueue            _ue;
      MatchQueue            _post;
      xmi_task_t            *_ranks;
      void                  *_allreduce_storage[2];
      void                  *_allreduce[2];
      unsigned              _allreduce_async_mode;
      unsigned              _allreduce_iteration;
      XMI::Topology         *_topos;
      int                   _numtopos;
      int                   _mytopo;
      xmi_task_t            _virtual_rank;
      xmi_ca_t              attributes;
    }; // class Geometry
  };  // namespace Geometry
}; // namespace XMI


#endif
