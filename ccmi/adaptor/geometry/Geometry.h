
#ifndef   __ccmi_geometry__h__
#define   __ccmi_geometry__h__

#include "../ccmi_util.h"
#include "../ccmi_internal.h"
#include "../Mapping.h"
#include "../../interfaces/Executor.h"
#include "../pgasp2p/Adaptor.h"
#include "../../interfaces/Composite.h"
#include "../../queue/Queue.h"
#include "../../queue/MatchQueue.h"

namespace CCMI
{
  namespace Adaptor 
  {
    class Geometry
    {
    protected:    
      /// \brief Unique ID which is the same on all ranks in the group
      unsigned                         _commid;

      /// \brief Number of ranks in this geometry
      unsigned                         _numranks;
      /// \brief Array of the ranks
      unsigned                         * _ranks;

      /// \brief is this the global context (ie, comm_world)?
      bool                             _isGlobalContext;
      /// \brief is this some part of a torus?
      bool                             _isTorus; // generic for binom

      /// \brief Need to store a barrier in the geometry
      CCMI::Executor::Executor       * _barrier_exec;
      /// \brief Need to store a barrier in the geometry
      CCMI::Executor::Executor       * _local_barrier_exec;

      /// \brief [All]reduce composite/executors stored in the geometry for
      /// async callbacks
      CCMI::Executor::Composite      * _allreduce;

      /// \brief Persistent storage for [all]reduce composites.    
      /// Saved for the life of the geometry and freed when the
      // geometry is destroyed.  
      CCMI_Executor_t                  * _allreduce_storage;

      unsigned                         * _permutation;
      int                              _myidx;      
      
      CCMI::MatchQueue            *_asyncBcastPostQueue;
      CCMI::MatchQueue            *_asyncBcastUnexpQueue;

      static unsigned           _ccmi_cached_geometry_comm;
      static CCMI_Geometry_t  * _ccmi_cached_geometry;

    public:

      /// \brief constructor
      Geometry(
              CCMI::Mapping *mapping,
              unsigned *ranks,
              unsigned nranks,
              unsigned comm,
              bool     globalcontext);

      /// --------------------
      /// --- Query Functions
      /// -------------------

      inline unsigned   comm()
      {
        return  _commid;
      }
      inline unsigned   *ranks()
      {
        return  _ranks;
      }
      inline unsigned   nranks()
      {
        return  _numranks;
      }
      inline int        myIdx ()
      {
        return  _myidx;
      }

      void generatePermutation ();
      void freePermutation ();

      inline unsigned   *permutation() 
      {
        if(!_permutation)
          generatePermutation();

        return _permutation;
      }

      inline bool       isGlobalContext()
      {
        return _isGlobalContext;
      }

      inline CCMI::Executor::Executor * getBarrierExecutor ()
      {
        return _barrier_exec;
      }

      inline void setBarrierExecutor (CCMI::Executor::Executor *bar)
      {
        _barrier_exec = bar;
      }

      inline CCMI::Executor::Executor * getLocalBarrierExecutor ()
      {
        return _local_barrier_exec;
      }

      inline void setLocalBarrierExecutor (CCMI::Executor::Executor *bar)
      {
        _local_barrier_exec = bar;
      }

      inline CCMI_Executor_t  *getAllreduceCompositeStorage () 
      {
        if(_allreduce_storage == NULL)
          _allreduce_storage = (CCMI_Executor_t *) CCMI_Alloc (sizeof (CCMI_Executor_t));
        return _allreduce_storage; 
      }

      inline CCMI::Executor::Composite *getAllreduceComposite()
      {
        // returns current iteration and increments the iteration value
        //fprintf(stderr, "Geometry::getAllreduceComposite(%#X)\n",_allreduce_iteration);
        return _allreduce;
      }

      inline void  setAllreduceComposite(CCMI::Executor::Composite *c)
      {
        //fprintf(stderr, "Geometry::setAllreduceComposite(%#X) %#.8X\n",_allreduce_iteration,(int)c);
        _allreduce = c;
      }
      inline void freeAllocations ()
      {
        //int i;
	//            fprintf(stderr,"Geometry::freeAllocations(), _allreduce %#X, _allreduce_storage %#X\n",
	//                    (int)_allreduce, (int)_allreduce_storage);	

	if(_asyncBcastPostQueue)
	  free (_asyncBcastPostQueue);
	_asyncBcastPostQueue = NULL;
	
	if(_asyncBcastUnexpQueue)
	  free (_asyncBcastUnexpQueue);
	_asyncBcastUnexpQueue = NULL;
          if(_allreduce)
          {
            _allreduce->~Composite();
            _allreduce = NULL;
          }
          if(_allreduce_storage)
          {
            CCMI_Free (_allreduce_storage);
            _allreduce_storage = NULL;
          }
      }

      inline void setGlobalContext(bool context)
      {
        _isGlobalContext = context;
      }

      inline CCMI::MatchQueue & asyncBcastPostQ()
      {

        if(_asyncBcastPostQueue == NULL)
        {
          void *buf = CCMI_Alloc (sizeof(CCMI::MatchQueue));
          _asyncBcastPostQueue = new (buf) CCMI::MatchQueue ();
        }

        return *_asyncBcastPostQueue;
      }

      inline CCMI::MatchQueue & asyncBcastUnexpQ()
      {

        if(_asyncBcastUnexpQueue == NULL)
        {
          void *buf = CCMI_Alloc (sizeof(CCMI::MatchQueue));
          _asyncBcastUnexpQueue = new (buf) CCMI::MatchQueue();
        }

        return *_asyncBcastUnexpQueue;
      }

      static inline CCMI_Geometry_t *getCachedGeometry (unsigned comm)
      {
        if(comm == _ccmi_cached_geometry_comm)
          return _ccmi_cached_geometry;
        return NULL;
      }

      static inline void updateCachedGeometry (CCMI_Geometry_t *geometry, unsigned comm)
      {
        _ccmi_cached_geometry_comm = comm;
        _ccmi_cached_geometry = geometry;
      }

      static inline void _compile_time_assert_ ()
      {
        // Compile time assert
        COMPILE_TIME_ASSERT((sizeof(CCMI_Executor_t) * 2) <= sizeof(CCMI_CollectiveRequest_t));
      }
      
    }; // class Geometry
  };  //Adaptor
}; // namespace CCMI


#endif
