/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/geometry/common/commongeometry.h
 * \brief ???
 */

#ifndef   __xmi_commongeometry__h__
#define   __xmi_commongeometry__h__

//#include "config.h"

#include "components/mapping/mpi/mpimapping.h"
#define XMI_GEOMETRY_CLASS XMI::Geometry::Common<XMI_MAPPING_CLASS>
#define GEOMETRY_STORAGE XMI_REQUEST_NQUADS*4


#include "components/geometry/Geometry.h"
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
                    unsigned               *ranks,
                    unsigned                nranks,
                    unsigned                comm,
                    unsigned                numcolors,
                    bool                    globalcontext):
        Geometry<XMI::Geometry::Common<T_Mapping>, T_Mapping>(mapping,ranks,nranks,comm,
                                        numcolors,globalcontext)
        {
        }
      inline Common (T_Mapping           *mapping,
                     unsigned             comm,
                     int                  numranges,
                     xmi_geometry_range_t rangelist[]):
	Geometry<XMI::Geometry::Common<T_Mapping>, T_Mapping>(mapping, comm, numranges, rangelist),
	_kvstore(),
        _commid(comm),
        _nranks(0)
        {
          this->_rank      = mapping->task();
          this->_rangelist = rangelist;
          this->_numranges = numranges;
          this->_size      = 0;
          int i;
          for(i=0; i<numranges; i++)            
            this->_size+=(this->_rangelist[i].hi-this->_rangelist[i].lo+1);

          for(i=0; i<numranges; i++)
	    _nranks+=rangelist[i].hi-rangelist[i].lo+1;

          _ranks       = (unsigned int *)malloc(_nranks*sizeof(*_ranks));
          _ranks_sizet = (size_t *)malloc(_nranks*sizeof(*_ranks_sizet));
          int k = 0;
          for(i=0; i<numranges; i++)
              {
                int range = rangelist[i].hi-rangelist[i].lo+1;
		int j     = 0;
		for(j=0;j<range;j++,k++)
                    {
                      _ranks[k] = rangelist[i].lo + j;
                      _ranks_sizet[k] = rangelist[i].lo + j;
                      if(_ranks[k] == (unsigned)this->_rank)
                        _myidx = k;
                    }
              }

          geometry_map[_commid]=this;
          updateCachedGeometry(this, _commid);

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
      inline unsigned                 *ranks_impl()
        {
          return _ranks;
        }
      inline size_t                 *ranks_sizet_impl()
        {
          return _ranks_sizet;
        }
      inline unsigned                  nranks_impl()
        {
          return _nranks;
        }
      inline int                       myIdx_impl()
        {
          return _myidx;
        }
      inline void                      generatePermutation_impl()
        {
	  return;
        }

      inline void                      freePermutation_impl()
        {
	  return;
        }
      inline unsigned                 *permutation_impl()
        {
          return _ranks;
        }
      inline void                      generatePermutation_sizet_impl()
        {
          assert(0);
	  return;
        }

      inline void                      freePermutation_sizet_impl()
        {
          assert(0);
	  return;
        }
      inline size_t                 *permutation_sizet_impl()
        {
          assert(0);
	  return NULL;
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
          assert(0);
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
      inline int                        size_impl       (void)
        {
          return _size;
        }
      inline int                        rank_impl       (void)
        {
          return _rank;
        }
      inline int                        absrankof_impl  (int rank)
        {
          int rankLeft=rank;
          for(int i=0; i<_numranges; i++)
              {
                int rangeSz  = _rangelist[i].hi-_rangelist[i].lo+1;
                rankLeft    -= rangeSz;
                if(rankLeft <= 0)
                    {
                      int offset = rangeSz+rankLeft;
                      return _rangelist[i].lo+offset;
                    }
              }
          return -1;
        }
      inline int                        virtrankof_impl (int rank)
        {
          int      sz=0;
	  unsigned r = rank;
          for(int i=0; i<_numranges; i++)
              {
                if(r>=_rangelist[i].lo && r<=_rangelist[i].hi)
                  return sz + (r - _rangelist[i].lo);
                sz+=(_rangelist[i].hi-_rangelist[i].lo+1);
              }
          assert(0);
          return -1;
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
      xmi_geometry_range_t *_rangelist;
      int                   _rank;
      int                   _size;
      MatchQueue            _ue;
      MatchQueue            _post;
      unsigned             *_ranks;
      size_t               *_ranks_sizet;
      int                   _nranks;
      void                 *_allreduce_storage[2];
      void                 *_allreduce[2];
      unsigned              _allreduce_async_mode;
      unsigned              _allreduce_iteration;
      int                   _myidx;
    }; // class Geometry
  };  // namespace Geometry
}; // namespace XMI


#endif
