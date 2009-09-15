#ifndef   __xmi_commongeometry__h__
#define   __xmi_commongeometry__h__

//#include "platform.h"

//#include "components/mapping/mpi/mpimapping.h"
#define XMI_GEOMETRY_CLASS XMI::Geometry::Common<XMI_MAPPING_CLASS>

#include "components/geometry/Geometry.h"
#include <map>

namespace XMI
{
  namespace Geometry
  {
    template <class T_Mapping>
    class Common : public Geometry<XMI::Geometry::Common<T_Mapping>, T_Mapping>
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
                     int                  numranges,
                     xmi_geometry_range_t rangelist[]):
	Geometry<XMI::Geometry::Common<T_Mapping>, T_Mapping>(mapping, numranges, rangelist),
	_kvstore()
        {
          this->_rank      = mapping->task();
          this->_rangelist = rangelist;
          this->_numranges = numranges;
          this->_size      = 0;
          for(int i=0; i<numranges; i++)
            this->_size+=(this->_rangelist[i].hi-this->_rangelist[i].lo+1);
        }

      inline int                       getColorsArray_impl()
        {
        }
      inline void                      setAsyncAllreduceMode_impl(unsigned value)
        {
        }
      inline unsigned                  getAsyncAllreduceMode_impl()
        {
        }
      inline unsigned                  incrementAllreduceIteration_impl()
        {
        }
      inline unsigned                  comm_impl()
        {
        }
      inline unsigned                 *ranks_impl()
        {
        }
      inline unsigned                  nranks_impl()
        {
        }
      inline int                       myIdx_impl()
        {
        }
      inline void                      generatePermutation_impl()
        {
        }
      inline void                      freePermutation_impl()
        {
        }
      inline unsigned                 *permutation_impl()
        {
        }
      inline bool                      isRectangle_impl()
        {
        }
      inline bool                      isTorus_impl()
        {
        }
      inline bool                      isTree_impl()
        {
        }
      inline bool                      isGlobalContext_impl()
        {
        }
      inline bool                      isGI_impl()
        {
        }
      inline unsigned                  getNumColors_impl()
        {
        }
      inline unsigned                  getAllreduceIteration_impl()
        {
        }
      inline void                      freeAllocations_impl()
        {
        }
      inline void                      setGlobalContext_impl(bool context)
        {
        }
      inline void                      setNumColors_impl(unsigned numcolors)
        {
        }
      inline MatchQueue               &asyncBcastPostQ_impl()
        {
        }
      inline MatchQueue               &asyncBcastUnexpQ_impl()
        {
        }
#if 0
      inline RECTANGLE_TYPE            rectangle_impl()
        {
        }
      inline RECTANGLE_TYPE            rectangle_mesh_impl()
        {
        }
      inline EXECUTOR_TYPE             getBarrierExecutor_impl()
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

      inline CCMI_EXECUTOR_TYPE        getAllreduceCompositeStorage_impl()
        {
        }
      inline CCMI_EXECUTOR_TYPE        getAllreduceCompositeStorage_impl(unsigned i)
        {
        }
      inline COMPOSITE_TYPE            getAllreduceComposite_impl()
        {
        }
      inline COMPOSITE_TYPE            getAllreduceComposite_impl(unsigned i)
        {
        }
      inline void                      setAllreduceComposite_impl(COMPOSITE_TYPE c)
        {
        }
      inline void                      setAllreduceComposite_impl(COMPOSITE_TYPE c,
                                                                  unsigned i)
        {
        }

      inline EXECUTOR_TYPE             getCollectiveExecutor_impl(unsigned color=0)
        {
        }
      inline void                      setCollectiveExecutor_impl(EXECUTOR_TYPE exe,
                                                                  unsigned color=0)
        {
        }

      static inline CCMI_Geometry_t   *getCachedGeometry_impl(unsigned comm)
        {
        }
      static inline void               updateCachedGeometry_impl(CCMI_Geometry_t *geometry,
                                                                 unsigned comm)
        {
        }
#endif
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
          int sz=0;
          for(int i=0; i<_numranges; i++)
              {
                if(rank>=_rangelist[i].lo && rank<=_rangelist[i].hi)
                  return sz + (rank - _rangelist[i].lo);
                sz+=(_rangelist[i].hi-_rangelist[i].lo+1);
              }
          assert(0);
        }
      inline void                       setKey(int key, void*value)
      {
	_kvstore[key]=value;
      }
      inline void                      *getKey(int key)
      {
	return _kvstore[key];
      }

    private:
      std::map <int, void*> _kvstore;
      int                   _numranges;
      xmi_geometry_range_t *_rangelist;
      int                   _rank;
      int                   _size;


    }; // class Geometry
  };  // namespace Geometry
}; // namespace XMI


#endif
