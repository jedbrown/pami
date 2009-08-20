/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file adaptor/geometry/Geometry.h
 * \brief Geometry object
 * This object is a message layer approximation of communicators
 *  - Stores ranks and number of ranks in this particular group
 *  - Stores an ID which must be the same on all the ranks in the group
 *  - Has flags for tree, GI, rectangle, and torus
 *  - Has flag for global context - needed in MPI to determine if this is
 *    comm_world or a comm dupe of comm_world in multi-threaded environments
 *  - Keeps a barrier for this group of nodes
 */


#ifndef   __ccmi_geometry__h__
#define   __ccmi_geometry__h__

#include "interface/ccmi_internal.h"
#include "util/ccmi_util.h"
#include "util/ccmi_debug.h"
#include "interface/CollectiveMapping.h"
#include "algorithms/executor/Executor.h"
#include <new>

#ifdef ADAPTOR_MPI
#define ADAPTOR_NO_TORUS
#endif

#ifdef ADAPTOR_LAPI
#define ADAPTOR_NO_TORUS
#endif


#ifndef ADAPTOR_NO_TORUS
#include "algorithms/schedule/Rectangle.h"
#endif
#include "Adaptor.h"     // build needs to find this
#include "algorithms/composite/Composite.h"
#include "util/queue/Queue.h"
#include "util/queue/MatchQueue.h"


namespace CCMI
{
  namespace Adaptor
  {
    extern unsigned          _ccmi_cached_geometry_comm;
    extern CCMI_Geometry_t * _ccmi_cached_geometry;

    class Geometry
    {
    public:
      enum
      {
        colorsArray = 6
      }; /// May need an executor per color

      inline void setAsyncAllreduceMode(unsigned value)
      {
        // 1 is async on (two iterations of the composite saved)
        // 0 is async off (only one iteration of the composite saved)
        _allreduce_async_mode = value;
      }
      inline unsigned getAsyncAllreduceMode()
      {
        // 1 is async on (two iterations of the composite saved)
        // 0 is async off (only one iteration of the composite saved)
        return _allreduce_async_mode;
      }

    protected:

      /// \brief Unique ID which is the same on all ranks in the group
      unsigned                         _commid;

      /// \brief Number of ranks in this geometry
      unsigned                         _numranks;
      /// \brief Array of the ranks
      unsigned                         * _ranks;
      /// \brief is this group a rectangle solid?
      bool                             _isRectangle;
      /// \brief is tree available?
      bool                             _isTree;
      /// \brief is GI available?
      bool                             _isGI;
      /// \brief is this the global context (ie, comm_world)?
      bool                             _isGlobalContext;
      /// \brief is this some part of a torus?
      bool                             _isTorus; // generic for binom
#ifndef ADAPTOR_NO_TORUS
      /// \brief The rectangle schedule, if this is a rectangular group
      CCMI::Schedule::Rectangle      _rectangle;
      /// \brief The rectangle schedule, if this is a rectangular group
      CCMI::Schedule::Rectangle      _rectangle_mesh;
#endif
      /// \brief Need to store a barrier in the geometry
      CCMI::Executor::Executor       * _barrier_exec;
      /// \brief Need to store a barrier in the geometry
      CCMI::Executor::Executor       * _local_barrier_exec;

      /// \brief [All]reduce composite/executors stored in the geometry for
      /// async callbacks
      CCMI::Executor::Composite      * _allreduce[2];

      /// \brief Need to store executors in the geometry for async callbacks
      CCMI::Executor::Executor       * _collective_exec[colorsArray];

      /// \brief Persistent storage for [all]reduce composites.
      /// Saved for the life of the geometry and freed when the
      // geometry is destroyed.
      CCMI_Executor_t                  * _allreduce_storage[2];

      unsigned                         _allreduce_iteration;
      unsigned                         _allreduce_async_mode;
      unsigned                         * _permutation;
      int                              _myidx;
      unsigned                         _numcolors;

      //Queue               _asyncBcastPostQueue;
      //Queue               _asyncBcastUnexpQueue;

      CCMI::MatchQueue            *_asyncBcastPostQueue;
      CCMI::MatchQueue            *_asyncBcastUnexpQueue;

//      static unsigned           _ccmi_cached_geometry_comm; /// \todo is this the right place for the cache?
//      static CCMI_Geometry_t  * _ccmi_cached_geometry;
      inline unsigned incrementAllreduceIteration()
      {
        _allreduce_iteration ^= _allreduce_async_mode; // "increment" with defined mode
        // fprintf(stderr, "Geometry::incrementAllreduceIteration() %#X\n",_allreduce_iteration);
        return _allreduce_iteration;
      }


    public:

      /// \brief constructor
#ifndef ADAPTOR_NO_TORUS

      Geometry(
              CCMI::TorusCollectiveMapping *mapping, /// \todo should not be torus
              unsigned *ranks,
              unsigned nranks,
              unsigned comm,
              unsigned numcolors,
              bool     globalcontext);
      Geometry(
              CCMI::CollectiveMapping *mapping,
              unsigned *ranks,
              unsigned nranks,
              unsigned comm,
              unsigned numcolors,
              bool     globalcontext) { XMI_abortf("Need to add support for generic Mapping"); }
#else // MPI
      Geometry(
              CCMI::CollectiveMapping *mapping,
              unsigned *ranks,
              unsigned nranks,
              unsigned comm,
              unsigned numcolors,
              bool     globalcontext);
#endif

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


      inline bool       isRectangle()
      {
        return _isRectangle;
      }
      inline bool       isTorus()
      {
        return _isTorus;
      }
      inline bool       isTree()
      {
        return _isTree;
      }
      inline bool       isGlobalContext()
      {
        return _isGlobalContext;
      }
      inline bool       isGI()
      {
        return _isGI;
      }
      inline unsigned int getNumColors()
      {
        return _numcolors;
      };

#ifndef ADAPTOR_NO_TORUS
      inline CCMI::Schedule::Rectangle *rectangle()
      {
        return &_rectangle;
      }

      inline CCMI::Schedule::Rectangle *rectangle_mesh()
      {
        return &_rectangle_mesh;
      }
#endif
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

      inline unsigned getAllreduceIteration()
      {
        return _allreduce_iteration;
      }

      inline CCMI_Executor_t  *getAllreduceCompositeStorage ()
      {
        if(_allreduce_storage[_allreduce_iteration] == NULL)
          _allreduce_storage[_allreduce_iteration] = (CCMI_Executor_t *) CCMI_Alloc (sizeof (CCMI_Executor_t));
        return _allreduce_storage[_allreduce_iteration];
      }

      inline CCMI_Executor_t  *getAllreduceCompositeStorage(unsigned i)
      {
        if(_allreduce_storage[i] == NULL)
          _allreduce_storage[i] = (CCMI_Executor_t *) CCMI_Alloc (sizeof (CCMI_Executor_t));
        return _allreduce_storage[i];
      }

      inline CCMI::Executor::Composite *getAllreduceComposite()
      {
        // returns current iteration and increments the iteration value
        //fprintf(stderr, "Geometry::getAllreduceComposite(%#X)\n",_allreduce_iteration);
        return _allreduce[_allreduce_iteration];
      }

      inline CCMI::Executor::Composite *getAllreduceComposite(unsigned i)
      {
        //fprintf(stderr, "Geometry::getAllreduceComposite(unsigned i:%#X)\n",i);
        return _allreduce[i];
      }

      inline void  setAllreduceComposite(CCMI::Executor::Composite *c)
      {
        //fprintf(stderr, "Geometry::setAllreduceComposite(%#X) %#.8X\n",_allreduce_iteration,(int)c);
        _allreduce[_allreduce_iteration] = c;
        // When setting a non-null composite, increment our iteration.
        // The assumption is the caller is using this 'c' now and we should
        // setup for the next one to be used/ctor'd.
        if(c) incrementAllreduceIteration();
      }

      inline void  setAllreduceComposite(CCMI::Executor::Composite *c, unsigned i)
      {
        //fprintf(stderr, "Geometry::setAllreduceComposite(unsigned i:%#X) %#.8X\n",i,(int)c);
        _allreduce[i] = c;
      }
      inline void freeAllocations ()
      {
  //      int i;
	//            fprintf(stderr,"Geometry::freeAllocations(), _allreduce %#X, _allreduce_storage %#X\n",
	//                    (int)_allreduce, (int)_allreduce_storage);
	if(_asyncBcastPostQueue)
	  CCMI_Free (_asyncBcastPostQueue);
	_asyncBcastPostQueue = NULL;

	if(_asyncBcastUnexpQueue)
	  CCMI_Free (_asyncBcastUnexpQueue);
	_asyncBcastUnexpQueue = NULL;
        for(int i=0; i < 2; ++i)
        {

          if(_allreduce[i])
          {
            _allreduce[i]->~Composite();
            _allreduce[i] = NULL;
          }
          if(_allreduce_storage[i])
          {
            CCMI_Free (_allreduce_storage[i]);
            _allreduce_storage[i] = NULL;
          }
      }
      }

      inline CCMI::Executor::Executor * getCollectiveExecutor (unsigned color=0)
      {
        CCMI_assert(color < colorsArray);
        return _collective_exec[color];
      }

      inline void setCollectiveExecutor (CCMI::Executor::Executor *exe, unsigned color=0)
      {
        CCMI_assert(color < colorsArray);
        _collective_exec[color] = exe;
      }

      inline void setGlobalContext(bool context)
      {
        _isGlobalContext = context;
      }

      inline void setNumColors(unsigned numcolors)
      {
        _numcolors = numcolors;
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

    }; // class Geometry
  };  //Adaptor
}; // namespace CCMI

#ifndef __bgp__
#ifndef __bogus_dcmf_hardware_definitions__
  #warning BOGUS TEMPORARY DCMF DEFINITIONS
  /** \brief The size of the processor name string (happens to be the same as the MPI version) */
#define DCMF_HARDWARE_MAX_PROCESSOR_NAME 128
  /** \brief The number of dimensions in the hardware information arrays */
#define DCMF_HARDWARE_NUM_DIMS 4

  /** \brief Hardware information data type */
  typedef struct
  {
    uint32_t lrank;  /**< Logical rank of the node in the existing mapping.                   */
    uint32_t lsize;  /**< Logical size of the partition, based on the mapping and -np value.  */
    uint32_t prank;  /**< Physical rank of the node (irrespective of mapping).                */
    uint32_t psize;  /**< Physical size of the partition (irrespective of mapping).           */

    union {
      uint32_t Size[DCMF_HARDWARE_NUM_DIMS];  /**< Array for the dimension size of the HW.    */
      struct {
        uint32_t tSize;  /**< The number of ranks possible on the node.                       */
        uint32_t zSize;  /**< The Z dimension size of the HW.                                 */
        uint32_t ySize;  /**< The Y dimension size of the HW.                                 */
        uint32_t xSize;  /**< The X dimension size of the HW.                                 */
      };
    };

    union {
      uint32_t Coord[DCMF_HARDWARE_NUM_DIMS]; /**< Array for the physical coordinates of the calling node. */
      struct {
        uint32_t tCoord; /**< The physical T rank       of the calling node.                  */
        uint32_t zCoord; /**< The physical Z coordinate of the calling node.                  */
        uint32_t yCoord; /**< The physical Y coordinate of the calling node.                  */
        uint32_t xCoord; /**< The physical X coordinate of the calling node.                  */
      };
    };

    union {
      uint32_t Torus[DCMF_HARDWARE_NUM_DIMS];  /**< Array for torus/mesh indicators.          */
      struct {
        uint32_t  tTorus; /**< Do we have a torus in the T dimension (Yes).                   */
        uint32_t  zTorus; /**< Do we have a torus in the Z dimension.                         */
        uint32_t  yTorus; /**< Do we have a torus in the Y dimension.                         */
        uint32_t  xTorus; /**< Do we have a torus in the X dimension.                         */
      };
    };

    uint32_t rankInPset; /**< Rank in the Pset (irrespective of mapping).                     */
    uint32_t sizeOfPset; /**< Number of nodes in the Pset (irrespective of mapping).          */
    uint32_t idOfPset;   /**< Rank of the Pset in the partition (irrespective of mapping).    */

    uint32_t clockMHz;   /**< Frequency of the core clock, in units of 10^6/seconds           */
    uint32_t memSize;    /**< Size of the core main memory, in units of 1024^2 Bytes          */
    char name[DCMF_HARDWARE_MAX_PROCESSOR_NAME]; /**< A unique name string for the calling node. */
  } DCMF_Hardware_t;
  inline void DCMF_Hardware (DCMF_Hardware_t *hw_info){memset(hw_info, 0x00, sizeof(DCMF_Hardware_t));}
  #define __bogus_dcmf_hardware_definitions__
#endif // !__bogus_dcmf_hardware_definitions__
#endif // !__bgp__

#endif
