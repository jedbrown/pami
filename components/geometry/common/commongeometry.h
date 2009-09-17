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
          assert(0);
	  return 0;
        }
      inline void                      setAsyncAllreduceMode_impl(unsigned value)
        {
          assert(0);
	  return;
        }
      inline unsigned                  getAsyncAllreduceMode_impl()
        {
          assert(0);
	  return 0;
        }
      inline unsigned                  incrementAllreduceIteration_impl()
        {
          assert(0);
	  return 0;
        }
      inline unsigned                  comm_impl()
        {
          assert(0);
	  return 0;
        }
      inline unsigned                 *ranks_impl()
        {
          assert(0);
	  return NULL;
        }
      inline unsigned                  nranks_impl()
        {
          assert(0);
	  return 0;
        }
      inline int                       myIdx_impl()
        {
          assert(0);
	  return 0;
        }
      inline void                      generatePermutation_impl()
        {
          assert(0);
	  return;
        }
      inline void                      freePermutation_impl()
        {
          assert(0);
	  return;
        }
      inline unsigned                 *permutation_impl()
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
          assert(0);
	  return 0;
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

#endif
      inline void            *getBarrierExecutor_impl()
        {
          assert(0);
          return NULL;
        }

      static inline Common   *getCachedGeometry_impl(unsigned comm)
        {
          assert(0);
          return NULL;
        }
      static inline void               updateCachedGeometry_impl(Common *geometry,
                                                                 unsigned comm)        
        {
          assert(0);
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
      MatchQueue            _ue;
      MatchQueue            _post;


    }; // class Geometry
  };  // namespace Geometry
}; // namespace XMI


#endif
