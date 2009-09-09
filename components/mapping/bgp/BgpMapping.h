/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/mapping/bgp/BgpMapping.h
/// \brief ???
///
#ifndef __components_mapping_bgp_bgpmapping_h__
#define __components_mapping_bgp_bgpmapping_h__

#include "sys/xmi.h"

#include "../BaseMapping.h"
#include "../TorusMapping.h"
#include "../NodeMapping.h"

#include "components/sysdep/bgp/BgpPersonality.h"
#include "components/memory/shmem/SharedMemoryManager.h"

extern XMI::SysDep::BgpPersonality __global_personality;

#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif

namespace XMI
{
  namespace Mapping
  {
    //template <class T_Memory>
    //class BgpMapping : public Interface::Base<BgpMapping<T_Memory>,T_Memory>,
    //                   public Interface::Torus<BgpMapping<T_Memory>,4>
    class BgpMapping : public Interface::Base<BgpMapping,Memory::SharedMemoryManager>,
                       public Interface::Torus<BgpMapping,4>,
                       public Interface::Node<BgpMapping>
    {
      protected:

        typedef struct nodeaddr
        {
          union
          {
            kernel_coords_t coords;
            size_t          global;
          };
          size_t            local;
        } nodeaddr_t;
    
    
      public:
        inline BgpMapping () :
//            Interface::Base<BgpMapping<T_Memory>,T_Memory>(),
//            Interface::Torus<BgpMapping<T_Memory>,4>(),
            Interface::Base<BgpMapping,Memory::SharedMemoryManager>(),
            Interface::Torus<BgpMapping,4>(),
            Interface::Node<BgpMapping> (),
            _x (__global_personality.xCoord()),
            _y (__global_personality.yCoord()),
            _z (__global_personality.zCoord()),
            _t (__global_personality.tCoord())
        {
          _nodeaddr.coords.x = _x;
          _nodeaddr.coords.y = _y;
          _nodeaddr.coords.z = _z;
          _nodeaddr.coords.t =  0;
          _nodeaddr.local    = _t;
          TRACE_ERR((stderr,"BgpMapping::BgpMapping() .. torus: (%zd %zd %zd %zd), node: (%zd %zd)\n", _x, _y, _z, _t, _nodeaddr.global, _nodeaddr.local));
        };

        inline ~BgpMapping () {};

      protected:
        size_t _task;
        size_t _size;
        size_t _nodes;
        size_t _peers;
        size_t _x;
        size_t _y;
        size_t _z;
        size_t _t;
        
        nodeaddr_t _nodeaddr;
        
        size_t _numActiveRanksLocal;
        size_t _numActiveRanksGlobal;
        size_t _numActiveNodesGlobal;
        size_t _fullSize;
        
        size_t * _mapcache;
        size_t * _rankcache;
        
      public:

        /////////////////////////////////////////////////////////////////////////
        //
        // XMI::Mapping::Interface::Base interface implementation
        //
        /////////////////////////////////////////////////////////////////////////

        ///
        /// \brief Initialize the mapping
        /// \see XMI::Mapping::Interface::Base::init()
        ///
        inline xmi_result_t init_impl(Memory::SharedMemoryManager & mm);
        //inline xmi_result_t init_impl(T_Memory & mm);

        ///
        /// \brief Return the BGP global task for this process
        /// \see XMI::Mapping::Interface::Base::task()
        ///
	inline size_t task_impl()
        {
          return _task;
        }

        ///
        /// \brief Returns the number of global tasks
        /// \see XMI::Mapping::Interface::Base::size()
        ///
        inline size_t size_impl()
        {
          return _size;
        }

        ///
        /// \brief Number of physical active nodes in the partition.
        /// \see XMI::Mapping::Interface::Base::numActiveNodesGlobal()
        ///
        inline size_t numActiveNodesGlobal_impl ()
        {
          return _nodes;
        }

        ///
        /// \brief Number of physical active tasks in the partition.
        /// \see XMI::Mapping::Interface::Base::numActiveRanksGlobal()
        ///
        inline size_t numActiveTasksGlobal_impl ()
        {
          return _size;
        }

        ///
        /// \brief Number of physical active tasks in the local node.
        /// \see XMI::Mapping::Interface::Base::numActiveRanksLocal()
        ///
        inline size_t numActiveTasksLocal_impl ()
        {
          return _peers;
        }

        ///
        /// \brief Determines if two global tasks are located on the same physical node.
        /// \see XMI::Mapping::Interface::Base::isPeer()
        ///
        inline bool isPeer_impl (size_t task1, size_t task2)
        {
          unsigned xyzt1 = _mapcache[task1];
          unsigned xyzt2 = _mapcache[task2];
          return ((xyzt1>>8)==(xyzt2>>8));
        }
#if 0
        inline DCMF_Result network2task_impl (const DCMF_NetworkCoord_t  * addr,
                                              size_t                     * task,
                                              DCMF_Network               * type) const
        {
#error implement this
        }

        inline DCMF_Result task2network_impl (size_t                task,
                                              DCMF_NetworkCoord_t * addr,
                                              DCMF_Network          type) const
        {
#error implement this
        }
#endif
        /////////////////////////////////////////////////////////////////////////
        //
        // XMI::Mapping::Interface::Torus interface implementation
        //
        /////////////////////////////////////////////////////////////////////////
        
        ///
        /// \brief Return the BGP torus x coordinate (dimension 0) for this task
        ///
        /// \see XMI::Mapping::Interface::Torus::torusCoord()
        ///
        inline size_t x ()
        {
          return __global_personality.xCoord();
        }
        inline size_t y ()
        {
          return __global_personality.yCoord();
        }
        inline size_t z ()
        {
          return __global_personality.zCoord();
        }
        inline size_t t ()
        {
          return __global_personality.tCoord();
        }
        
        inline size_t xSize ()
        {
          return __global_personality.xSize();
        }
        
        inline size_t ySize ()
        {
          return __global_personality.ySize();
        }
        
        inline size_t zSize ()
        {
          return __global_personality.zSize();
        }
        
        inline size_t tSize ()
        {
          return __global_personality.tSize();
        }
        
        
#if 0

        ///
        /// \brief Return the BGP torus y coordinate (dimension 1) for this task
        ///
        /// \see XMI::Mapping::Interface::Torus::torusCoord()
        ///
        template <>
        inline size_t torusCoord_impl<1> () const
        {
          return __global_personality.yCoord();
        }

        ///
        /// \brief Return the BGP torus z coordinate (dimension 2) for this task
        ///
        /// \see XMI::Mapping::Interface::Torus::torusCoord()
        ///
        template <>
        inline size_t torusCoord_impl<2> () const
        {
          return __global_personality.zCoord();
        }

        ///
        /// \brief Return the BGP torus t coordinate (dimension 3) for this task
        ///
        /// \see XMI::Mapping::Interface::Torus::torusCoord()
        ///
        template <>
        inline size_t torusCoord_impl<3> () const
        {
          return __global_personality.tCoord();
        }

        ///
        /// \brief Return the size of the BGP torus x dimension
        ///
        /// \see XMI::Mapping::Interface::Torus::torusSize()
        ///
        template <>
        inline size_t torusSize_impl<0> () const
        {
          return __global_personality.xSize();
        }

        ///
        /// \brief Return the size of the BGP torus y dimension
        ///
        /// \see XMI::Mapping::Interface::Torus::torusSize()
        ///
        template <>
        inline size_t torusSize_impl<1> () const
        {
          return __global_personality.ySize();
        }

        ///
        /// \brief Return the size of the BGP torus z dimension
        ///
        /// \see XMI::Mapping::Interface::Torus::torusSize()
        ///
        template <>
        inline size_t torusSize_impl<2> () const
        {
          return __global_personality.zSize();
        }

        ///
        /// \brief Return the size of the BGP torus t dimension
        /// \see XMI::Mapping::Interface::Torus::torusSize()
        ///
        template <>
        inline size_t torusSize_impl<3> () const
        {
          return __global_personality.tSize();
        }
#endif
#if 0
        ///
        /// \brief Get the number of BGP torus dimensions
        /// \see XMI::Mapping::Interface::Torus::torusDims()
        ///
        inline const size_t torusDims_impl() const
        {
          return 4;
        }
#endif
        ///
        /// \brief Get the BGP torus address for this task
        /// \see XMI::Mapping::Interface::Torus::torusAddr()
        ///
        //template <>
        inline void torusAddr_impl (size_t (&addr)[4])
        //inline void torusAddr_impl (Address & addr) const
        {
          addr[0] = _x;
          addr[1] = _y;
          addr[2] = _z;
          addr[3] = _t;
        }

        ///
        /// \brief Get the BGP torus address for a task
        /// \see XMI::Mapping::Interface::Torus::task2torus()
        ///
        /// \todo Error path
        ///
        //template <>
        inline xmi_result_t task2torus_impl (size_t task, size_t (&addr)[4])
        {
          unsigned xyzt = _mapcache[task];
          addr[0] = (xyzt & 0xFF000000) >> 24;
          addr[1] = (xyzt & 0xFF0000) >> 16;
          addr[2] = (xyzt & 0xFF00) >> 8;
          addr[3] = (xyzt & 0xFF);
          return XMI_SUCCESS;
        }

        ///
        /// \brief Get the global task for a BGP torus address
        /// \see XMI::Mapping::Interface::Torus::torus2task()
        ///
        /// \todo Error path
        ///
        //template <>
        inline xmi_result_t torus2task_impl (size_t (&addr)[4], size_t & task)
        {
          size_t xSize = __global_personality.xSize();
          size_t ySize = __global_personality.ySize();
          size_t zSize = __global_personality.zSize();
          size_t tSize = __global_personality.tSize();
        
          if ((addr[0] >= xSize) ||
              (addr[1] >= ySize) ||
              (addr[2] >= zSize) ||
              (addr[3] >= tSize))
          {
            return XMI_INVAL;
          }

          size_t estimated_task =
            addr[3] * (xSize * ySize * zSize) +
            addr[2] * (xSize * ySize) +
            addr[1] * (xSize) +
            addr[0];

          // convert to 'unlikely_if'
          if (_rankcache [estimated_task] == (unsigned)-1)
          {
            return XMI_ERROR;
          }

          task = _rankcache [estimated_task];
          return XMI_SUCCESS;
        };
    
    
        /////////////////////////////////////////////////////////////////////////
        //
        // XMI::Mapping::Interface::Node interface implementation
        //
        /////////////////////////////////////////////////////////////////////////

        /// \brief Get the number of possible tasks on a node
        /// \return Dimension size
        inline xmi_result_t nodeSize_impl (size_t global, size_t & size)
        {
          TRACE_ERR((stderr,"BgpMapping::nodeSize_impl(%zd) >>\n", global));
#warning implement this!
          TRACE_ERR((stderr,"BgpMapping::nodeSize_impl(%zd) <<\n", global));
          return XMI_UNIMPL;
        };

        /// \brief Get the node address for the local task
        inline void nodeAddr_impl (size_t & global, size_t & local)
        {
          TRACE_ERR((stderr,"BgpMapping::nodeAddr_impl() >>\n"));
          global = _nodeaddr.global;
          local  = _nodeaddr.local;
          TRACE_ERR((stderr,"BgpMapping::nodeAddr_impl(%zd, %zd) <<\n", global, local));
        };

        /// \brief Get the node address for a specific task
        inline xmi_result_t task2node_impl (size_t task, size_t & global, size_t & local)
        {
          TRACE_ERR((stderr,"BgpMapping::task2node_impl(%zd) >>\n", task));
          global = _mapcache[task] & 0xffffff00;
          local  = _mapcache[task] & 0x000000ff;
          TRACE_ERR((stderr,"BgpMapping::task2node_impl(%zd, %zd, %zd) <<\n", task, global, local));
          return XMI_UNIMPL;
        };

        /// \brief Get the task associated with a specific node address
        inline xmi_result_t node2task_impl (size_t global, size_t local, size_t & task)
        {
          TRACE_ERR((stderr,"BgpMapping::node2task_impl(%zd, %zd) >>\n", global, local));
          size_t estimated_task = (global << 8) | local;
          task = _rankcache [estimated_task];
          TRACE_ERR((stderr,"BgpMapping::node2task_impl(%zd, %zd, %zd) <<\n", global, local, task));
          return XMI_UNIMPL;
        };

    
    
    
    };
  };
};

//template <class T_Memory>
//xmi_result_t XMI::Mapping::BgpMapping<T_Memory>::init_impl (T_Memory & mm)
xmi_result_t XMI::Mapping::BgpMapping::init_impl (XMI::Memory::SharedMemoryManager & mm)
{
  // This structure anchors pointers to the map cache and rank cache.
  // It is created in the static portion of shared memory in this constructor,
  // but exists there only for the duration of this constructor.  It
  // communicates mapping initialization information to the other tasks running
  // on this physical node.
  typedef struct cacheAnchors
  {
    volatile size_t * mapCachePtr; // Pointer to map cache
    volatile size_t *rankCachePtr; // Pointer to rank cache
    volatile size_t done[4];       // Indicators as to when each of the t coordinates
                                   // on our physical node are done extracting the
                                   // cache pointers from this structure in shared
                                   // memory (0 = not done, 1 = done.
    volatile size_t numActiveRanksLocal; // Number of ranks on our physical node.
    volatile size_t numActiveRanksGlobal;// Number of ranks in the partition.
    volatile size_t numActiveNodesGlobal;// Number of nodes in the partition.
    volatile size_t maxRank;       // Largest valid rank
    volatile size_t minRank;       // Smallest valid rank
#if 0
    volatile CM_Coord_t activeLLCorner;
    volatile CM_Coord_t activeURCorner;
#endif
  } cacheAnchors_t;

  volatile cacheAnchors_t * cacheAnchorsPtr;
  bool meMaster = false;
  size_t tt, num_t = 0, rank, size;
  int err;

  //_processor_id = rts_get_processor_id();
  //_pers = & __global_personality;
  //_myNetworkCoord.network = CM_TORUS_NETWORK;
  //unsigned ix = 0;
  //_myNetworkCoord.n_torus.coords[ix++] = x();
  //_myNetworkCoord.n_torus.coords[ix++] = y();
  //_myNetworkCoord.n_torus.coords[ix++] = z();
  //_numGlobalDims = ix;
  //_myNetworkCoord.n_torus.coords[ix++] = t();
  //_numDims = ix;

  //DCMF_assert_debug(_numDims <= CM_MAX_DIMS);
  size_t tCoord = t ();
  size_t tsize  = tSize ();

  _numActiveRanksLocal = 0;
  _numActiveRanksGlobal= 0;
  _numActiveNodesGlobal= 0;

  // Calculate the number of potential ranks in this partition.
  _fullSize = xSize() * ySize() * zSize() * tsize;

  // Use the static portion of the shared memory area (guaranteed to be zeros
  // at this point when it is in shared memory) to anchor pointers to
  // the caches.  Only the master rank will initialize these caches and set the
  // pointers into this structure.  When the non-master ranks on this physical
  // node see the non-zero pointers, they can begin to use them.
  xmi_result_t result = mm.memalign((void **) &cacheAnchorsPtr, 16, sizeof(cacheAnchors_t));
#warning fixme - shared memory allocation will FAIL in SMP mode - blocksome

  // Determine if we are the master rank on our physical node.  Do this by
  // finding the lowest t coordinate on our node, and if it is us, then we
  // are the master.
  for (tt=0; tt<tsize; tt++)
  {
    // See if there is a rank on our xyz with this t.
    int rc = Kernel_Coord2Rank( x(), y(), z(), tt, &rank, &size);
    if (rc == 0) // Found a t.
    {
      // If this t is the first one we found, and it is us, then we are the
      // master.
      if ( (num_t == 0) && (tt == tCoord) ) 
      {
        meMaster = true;
        cacheAnchorsPtr->done[tt] = 1; // Indicate this t is done so we
                                       // (the master) don't wait on it
                                       // later.
      }

      // If this t is us, save the rank and size in the mapping object.
      if ( tt == tCoord )
      {
        _task = rank; // Save our task rank in the mapping object.
        _size = size; // Save the size in the mapping object.
      }

      // Count the t's on our physical node.
      num_t++;
    }
    // If there is no rank on this t, indicate that this t is done so we
    // don't wait on it later.
    else
    {
      cacheAnchorsPtr->done[tt] = 1;
    }
  }

  // Note:  All nodes allocate the map and rank caches.  When in DUAL or VN modes,
  //        the allocator will return the same address in shared memory,
  //        effectively allocating the physically same buffer.  When in SMP mode,
  //        there is only one node that will allocate out of the heap.

  // Allocate the map cache from shared memory (in DUAL or VN modes) or from
  // heap (in SMP mode).
#warning fixme - shared memory allocation will FAIL in SMP mode - blocksome
  //_mapcache = (size_t *) mm.scratchpad_dynamic_area_memalign (16, sizeof(kernel_coords_t) * _fullSize );
  result = mm.memalign((void **) &_mapcache, 16, sizeof(kernel_coords_t) * _fullSize);

  // Allocate the rank cache from shared memory (in DUAL or VN modes) or from
  // heap (in SMP mode).
#warning fixme - shared memory allocation will FAIL in SMP mode - blocksome
  //_rankcache = (unsigned *) mm.scratchpad_dynamic_area_memalign ( 16, sizeof(size_t) * _fullSize);
  result = mm.memalign((void **) &_rankcache, 16, sizeof(size_t) * _fullSize);

  size_t max_rank = 0, min_rank = (size_t)-1;
#if 0
  CM_Coord_t ll, ur;
#endif



  // If we are the master, then initialize the caches.
  // Then, set the cache pointers into the shared memory area for the other
  // ranks on this node to see, and wait for them to see it.
  if ( meMaster )
  {
    uint16_t rarray [72][32][32]; /* Full 72 rack partition */
    memset(rarray, 0, sizeof(rarray));

    memset(_rankcache, -1, sizeof(size_t) * _fullSize);

    /* Fill in the _mapcache array in a single syscall.
     * It is indexed by rank, dimensioned to be the full size of the
     * partition (ignoring -np), and filled in with the xyzt
     * coordinates of each rank packed into a single 4 byte int.
     * Non-active ranks (-np) have x, y, z, and t equal to 255, such
     * that the entire 4 byte int is -1.
     */
    int rc = Kernel_Ranks2Coords((kernel_coords_t *)_mapcache, _fullSize);

    /* The above system call is new in V1R3M0.  If it works, obtain info
     * from the returned _mapcache.
     */
    if (rc==0)
    {
#if 0
      ll.network = ur.network = CM_TORUS_NETWORK;
      ll.n_torus.coords[0] = ur.n_torus.coords[0] = x();
      ll.n_torus.coords[1] = ur.n_torus.coords[1] = y();
      ll.n_torus.coords[2] = ur.n_torus.coords[2] = z();
      ll.n_torus.coords[3] = ur.n_torus.coords[3] = t();
#endif

      /* Obtain the following information from the _mapcache:
       * 1. Number of active ranks in the partition.
       * 2. Number of active compute nodes in the partition.
       * 3. _rankcache (the reverse of _mapcache).  It is indexed by
       *    coordinates and contains the rank.
       * 4. Number of active ranks on each compute node.
       */
      size_t i;
      for (i = 0; i < _fullSize; i++)
      {
        if ( (int)_mapcache[i] != -1 )
        {
          size_t x, y, z, t;
          kernel_coords_t mapCacheElement = *(kernel_coords_t*)&_mapcache[i];
          x = mapCacheElement.x;
          y = mapCacheElement.y;
          z = mapCacheElement.z;
          t = mapCacheElement.t;
          
          // Increment the rank count on this node.
          rarray[x][y][z]++;
          
          // Increment the number of global ranks.
          cacheAnchorsPtr->numActiveRanksGlobal++;
          
          // If the rank count on this node is '1', this is the first
          // rank encountered on this node. Increment the number of active nodes.
          if (rarray[x][y][z] == 1) cacheAnchorsPtr->numActiveNodesGlobal++;
          
          size_t estimated_rank =
            t * (xSize() * ySize() * zSize()) +
            z * (xSize() * ySize()) +
            y * (xSize()) + x;
            
          _rankcache[estimated_rank] = i;
           
          // because of "for (i..." this will give us MAX after loop.
          max_rank = i;
          if (min_rank == (size_t)-1) min_rank = i;
#if 0
          if (x < ll.n_torus.coords[0]) ll.n_torus.coords[0] = x;
          if (y < ll.n_torus.coords[1]) ll.n_torus.coords[1] = y;
          if (z < ll.n_torus.coords[2]) ll.n_torus.coords[2] = z;
          if (t < ll.n_torus.coords[3]) ll.n_torus.coords[3] = t;

          if (x > ur.n_torus.coords[0]) ur.n_torus.coords[0] = x;
          if (y > ur.n_torus.coords[1]) ur.n_torus.coords[1] = y;
          if (z > ur.n_torus.coords[2]) ur.n_torus.coords[2] = z;
          if (t > ur.n_torus.coords[3]) ur.n_torus.coords[3] = t;
#endif
        }
      }
      
      cacheAnchorsPtr->maxRank = max_rank;
      cacheAnchorsPtr->minRank = min_rank;
#if 0
      // why can't this just be assigned???
      memcpy((void *)&cacheAnchorsPtr->activeLLCorner, &ll, sizeof(ll));
      memcpy((void *)&cacheAnchorsPtr->activeURCorner, &ur, sizeof(ur));
#endif
    }
    /* If the system call fails, assume the kernel is older and does not
     * have this system call.  Use the original system call, one call per
     * rank (which is slower than the single new system call) to obtain
     * the information necessary to fill in the _mapcache, etc.
     */
    else
    {
      size_t i;
      for (i = 0; i < _fullSize; i++)
      {
        unsigned x, y, z, t;
        err = Kernel_Rank2Coord ((int)i, &x, &y, &z, &t);
        if (err==0)
        {
          _mapcache[i] = ((x&0xFF)<<24)|((y&0xFF)<<16)|((z&0xFF)<<8)|(t&0xFF);
          rarray[x][y][z]++;
          cacheAnchorsPtr->numActiveRanksGlobal++;
          if(rarray[x][y][z]==1)
            cacheAnchorsPtr->numActiveNodesGlobal++;
          int estimated_rank =
            t * (xSize() * ySize() * zSize()) +
            z * (xSize() * ySize()) +
            y * (xSize()) +
            x;
          _rankcache[estimated_rank] = i;
        }
        else
        {
          _mapcache[i] = (unsigned)-1;
        }
      }
       cacheAnchorsPtr->numActiveRanksLocal=rarray[x()][y()][z()];
    }

    // Now that the map and rank caches have been initialized, 
    // store their pointers into the shared memory cache pointer area so the
    // other nodes see these pointers.
    cacheAnchorsPtr->mapCachePtr  = _mapcache;
    cacheAnchorsPtr->rankCachePtr = _rankcache;

    // Copy the rank counts into the mapping object.    
    _numActiveRanksLocal  = cacheAnchorsPtr->numActiveRanksLocal;
    _numActiveRanksGlobal = cacheAnchorsPtr->numActiveRanksGlobal;
    _numActiveNodesGlobal = cacheAnchorsPtr->numActiveNodesGlobal;

    _bgp_mbar();  // Ensure that stores to memory are in the memory.

    // Wait until the other t's on our physical node have seen the cache
    // pointers.
    for (tt=0; tt<tsize; tt++)
    {
      while ( cacheAnchorsPtr->done[tt] == 0 )
      {
	_bgp_msync();
      }
    }
    
    // Now that all nodes have seen the cache pointers, zero out the cache
    // anchor structure for others who expect this area to be zero.
    memset ((void*)cacheAnchorsPtr, 0x00, sizeof(cacheAnchors_t));

  } // End: Allocate an initialize the map and rank caches.


  // We are not the master t on our physical node.  Wait for the master t to
  // initialize the caches.  Then grab the pointers and rank
  // counts, and then indicate we have seen them.
  else
  {
    while ( cacheAnchorsPtr->mapCachePtr == NULL )
    {
      _bgp_msync();
    }
    _mapcache = (unsigned*)(cacheAnchorsPtr->mapCachePtr);

    while ( cacheAnchorsPtr->rankCachePtr == NULL )
    {
      _bgp_msync();
    } 
    _rankcache = (unsigned*)(cacheAnchorsPtr->rankCachePtr);
    
    _numActiveRanksLocal  = cacheAnchorsPtr->numActiveRanksLocal;
    _numActiveRanksGlobal = cacheAnchorsPtr->numActiveRanksGlobal;
    _numActiveNodesGlobal = cacheAnchorsPtr->numActiveNodesGlobal;
    max_rank = cacheAnchorsPtr->maxRank;
    min_rank = cacheAnchorsPtr->minRank;
    
#if 0
    // why can't these just be assigned???
    memcpy(&ll, (void *)&cacheAnchorsPtr->activeLLCorner, sizeof(ll));
    memcpy(&ur, (void *)&cacheAnchorsPtr->activeURCorner, sizeof(ur));
#endif
    _bgp_mbar();
   
    cacheAnchorsPtr->done[tCoord] = 1;  // Indicate we have seen the info.
  }
  
  
#if 0
  // This is other gunk .. used by collectives?
  size_t rectsize = (ur.n_torus.coords[0] - ll.n_torus.coords[0] + 1) *
                    (ur.n_torus.coords[1] - ll.n_torus.coords[1] + 1) *
                    (ur.n_torus.coords[2] - ll.n_torus.coords[2] + 1) *
                    (ur.n_torus.coords[3] - ll.n_torus.coords[3] + 1);
  if (_numActiveRanksGlobal == rectsize) {
    _g_topology_world = new (&__topology_w) LL::Topology(&ll, &ur);
  } else if (max_rank - min_rank + 1 == _numActiveRanksGlobal) {
    // does this ever happen for "COMM_WORLD"?
    _g_topology_world = new (&__topology_w) LL::Topology(min_rank, max_rank);
  } else {
    // wait for COMM_WORLD so we don't allocate yet-another ranks list?
    // actually, COMM_WORLD should use our rank list...
    // does this ever happen for "COMM_WORLD"? 
    // _g_topology_world = new (&__topology_w) LL::Topology(ranks, nranks);
    fprintf(stderr, "failed to build comm-world topology\n");
  }
  _g_topology_world->subTopologyLocalToMe(&__topology_l);
  _g_topology_local = &__topology_l;

  _localranks = (unsigned *)malloc(sizeof(*_localranks) * NUM_CORES);
  DCMF_assert_debug(_localranks);
  unsigned index = 0;
  for (unsigned i = 0; i < tSize(); i++) {
      unsigned peerrank = (unsigned)-1;
      int rc = torus2rank(x(), y(), z(), i, peerrank);
      if ((rc == CM_SUCCESS) && (peerrank != (unsigned)-1)) {
          _localranks[index++] = peerrank;
      }
  }
#endif

  return XMI_SUCCESS;
};
#undef TRACE_ERR
#endif // __components_mapping_bgp_bgpmapping_h__

