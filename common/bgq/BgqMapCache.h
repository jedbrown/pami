/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef __common_bgq_BgqMapCache_h__
#define __common_bgq_BgqMapCache_h__
///
/// \file common/bgq/BgqMapCache.h
/// \brief Blue Gene/Q Map- and Rank-Cache Object
///

#include <hwi/include/bqc/A2_inlines.h>

#include "common/bgq/BgqPersonality.h"

/// \brief Creates valid index into _rankcache[].
///
/// It is imperative that all who create an index into _rankcache[]
/// do it in the identical way. Thus all should use this macro.
///
#define ESTIMATED_TASK(a,b,c,d,e,t,p,aSize,bSize,cSize,dSize,eSize,tSize,pSize) \
	((((((t * pSize + p) * eSize + e) * dSize + d) * cSize + c)* bSize + b) * aSize + a)

namespace XMI
{
    ///
    /// \brief Blue Gene/Q coordinate structure
    ///
    /// This structure takes 32-bits on any 32/64 bit system. The a, b, c,
    /// and d fields are the same size and in the same location as the MU
    /// descriptor structure. The thread/core fields are sized for 16 cores
    /// with 4 hardware threads each, though the reserved bit can be stolen
    /// for the 17th core if it needs a rank. The e dimension is sized to the
    /// current node-layout maximum, though the MU hardware supports the full
    /// 6 bits.
    ///
    /// \see MUHWI_Destination_t
    ///
    typedef union bgq_coords
    {
      struct
      {
        uint32_t thread   : 2; ///< Hardware thread id, 4 threads per core
        uint32_t a        : 6; ///< Torus 'a' dimension
        uint32_t b        : 6; ///< Torus 'b' dimension
        uint32_t c        : 6; ///< Torus 'c' dimension
        uint32_t d        : 6; ///< Torus 'd' dimension
        uint32_t e        : 1; ///< Torus 'e' dimension, two nodes per node card
        uint32_t reserved : 1; ///< Reserved - possibly to identify the 17th core
        uint32_t core     : 4; ///< Core id, 16 application cores per node
      };
      uint32_t   raw;          ///< Raw memory storage
    } bgq_coords_t;

    typedef struct
    {
      struct
      {
        bgq_coords_t * task2coords;
        uint32_t     * coords2task;
      } torus;
      struct
      {
        size_t       * local2peer;
        size_t       * peer2task;
      } node;
      size_t	size;
    } bgq_mapcache_t;

    class BgqMapCache {
      public:

        inline BgqMapCache () {};

        inline ~BgqMapCache () {};

        inline size_t init (XMI::BgqPersonality & personality,
                            void                   * ptr,
                            size_t                   bytes
                            )
        {
#ifdef OBSOLETE
          // This structure anchors pointers to the map cache and rank cache.
          // It is created in the static portion of shared memory in this
          // constructor, but exists there only for the duration of this
          // constructor.  It communicates mapping initialization information
          // to the other tasks running on this physical node.
          typedef struct cacheAnchors
          {
            struct
            {
              uint64_t enter;
            } atomic;
            volatile size_t * mapCachePtr; // Pointer to map cache
            volatile size_t *rankCachePtr; // Pointer to rank cache
            volatile size_t numActiveRanksLocal; // Number of ranks on our physical node.
            volatile size_t numActiveRanksGlobal;// Number of ranks in the partition.
            volatile size_t numActiveNodesGlobal;// Number of nodes in the partition.
            volatile size_t maxRank;       // Largest valid rank
            volatile size_t minRank;       // Smallest valid rank
          } cacheAnchors_t;

          size_t myRank, mySize;

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
          size_t pCoord = personality.pCoord ();
          size_t tCoord = personality.tCoord ();
          size_t psize  = personality.pSize ();
          size_t tsize  = personality.tSize ();

          cacheAnchorsPtr = (volatile cacheAnchors_t *) ptr;

          // Notify all other tasks on the node that this task has entered the
          // map cache initialization function.  If the value returned is zero
          // then this task is the first one in and is designated the "master".
          // All other tasks will wait until the master completes the
          // initialization.
          uint64_t participant =
            Fetch_and_Add ((uint64_t *)&(cacheAnchorsPtr->atomic.enter), 1);

          if (participant == 0) meMaster = true;

          myRank = personality.rank();
          // Calculate the number of potential ranks in this partition.
          size_t fullSize = personality.aSize() *
                            personality.bSize() *
                            personality.cSize() *
                            personality.dSize() *
                            personality.eSize() *
                            personality.pSize() *
                            personality.tSize();

          _mapcache  = (bgq_coords_t *) (cacheAnchorsPtr + 1);
          _rankcache = (uint32_t *) (_mapcache + fullSize);

          size_t max_rank = 0, min_rank = (size_t) - 1;

          // If we are the master, then initialize the caches.
          // Then, set the cache pointers into the shared memory area for the other
          // ranks on this node to see, and wait for them to see it.
          if ( meMaster )
            {
              // Sized to the addressable MU range.
              uint16_t rarray [64][64][64][64][2];
              memset(rarray, 0, sizeof(rarray));

              memset(_rankcache, -1, sizeof(uint32_t) * fullSize);

              size_t aSize = __global.personality.aSize();
              size_t bSize = __global.personality.bSize();
              size_t cSize = __global.personality.cSize();
              size_t dSize = __global.personality.dSize();
              size_t eSize = __global.personality.eSize();
              size_t tSize = __global.personality.pSize();
              size_t pSize = __global.personality.tSize();

              /* Fill in the _mapcache array in a single syscall.
               * It is indexed by rank, dimensioned to be the full size of the
               * partition (ignoring -np), and filled in with the xyzt
               * coordinates of each rank packed into a single 4 byte int.
               * Non-active ranks (-np) have x, y, z, and t equal to 255, such
               * that the entire 4 byte int is -1.
               */
#warning Need ranks2coords syscall!
//              int rc = Kernel_Ranks2Coords((kernel_coords_t *)_mapcache, fullSize);

              /* The above system call is new in V1R3M0.  If it works, obtain info
               * from the returned _mapcache.
               */
//              if (rc == 0)
                {
                  /* Obtain the following information from the _mapcache:
                   * 1. Number of active ranks in the partition.
                   * 2. Number of active compute nodes in the partition.
                   * 3. _rankcache (the reverse of _mapcache).  It is indexed by
                   *    coordinates and contains the rank.
                   * 4. Number of active ranks on each compute node.
                   */
                  size_t i;

                  for (i = 0; i < fullSize; i++)
                    {
                      if ( (int)_mapcache[i].raw != -1 )
                        {
                          size_t x, y, z, t;
                          bgq_coords_t mapCacheElement = *(bgq_coords_t*) & _mapcache[i];
                          a = mapCacheElement.a;
                          b = mapCacheElement.b;
                          c = mapCacheElement.c;
                          d = mapCacheElement.d;
                          e = mapCacheElement.e;
                          p = mapCacheElement.core;
                          t = mapCacheElement.thread;

                          // Increment the rank count on this node.
                          rarray[a][b][c][d][e]++;

                          // Increment the number of global ranks.
                          cacheAnchorsPtr->numActiveRanksGlobal++;

                          // If the rank count on this node is '1', this is the first
                          // rank encountered on this node. Increment the number of active nodes.
                          if (rarray[a][b][c][d][e] == 1)
                            cacheAnchorsPtr->numActiveNodesGlobal++;

                          size_t estimated_task =
				ESTIMATED_TASK(a,b,c,d,e,t,p,
					aSize,bSize,cSize,dSize,eSize,tSize,pSize);

                          _rankcache[estimated_rank] = i;

                          // because of "for (i..." this will give us MAX after loop.
                          max_rank = i;

                          if (min_rank == (size_t) - 1) min_rank = i;
                       }
                    }

                  cacheAnchorsPtr->maxRank = max_rank;
                  cacheAnchorsPtr->minRank = min_rank;
                }
              // Now that the map and rank caches have been initialized,
              // store their pointers into the shared memory cache pointer area so the
              // other nodes see these pointers.
              cacheAnchorsPtr->mapCachePtr  = _mapcache;
              cacheAnchorsPtr->rankCachePtr = _rankcache;

              // Copy the rank counts into the mapping object.
              //_numActiveRanksLocal  = cacheAnchorsPtr->numActiveRanksLocal;
              //_numActiveRanksGlobal = cacheAnchorsPtr->numActiveRanksGlobal;
              //_numActiveNodesGlobal = cacheAnchorsPtr->numActiveNodesGlobal;

              mbar();  // Ensure that stores to memory are in the memory.

              // Notify the other processes on this node that the master has
              // completed the initialization.
              Fetch_and_Add ((uint64_t *)&(cacheAnchorsPtr->atomic.exit), 1);

            } // End: Allocate an initialize the map and rank caches.


          // We are not the master t on our physical node.  Wait for the master t to
          // initialize the caches.  Then grab the pointers and rank
          // counts, and then indicate we have seen them.
          else
            {
              while (cacheAnchorsPtr->atomic.exit == 0);




              _mapcache = (unsigned*)(cacheAnchorsPtr->mapCachePtr);


              _rankcache = (unsigned*)(cacheAnchorsPtr->rankCachePtr);

              //_numActiveRanksLocal  = cacheAnchorsPtr->numActiveRanksLocal;
              //_numActiveRanksGlobal = cacheAnchorsPtr->numActiveRanksGlobal;
              //_numActiveNodesGlobal = cacheAnchorsPtr->numActiveNodesGlobal;
              max_rank = cacheAnchorsPtr->maxRank;
              min_rank = cacheAnchorsPtr->minRank;

              mbar();

              //cacheAnchorsPtr->done[personality.tCoord()] = 1;  // Indicate we have seen the info.
              Fetch_and_Add ((uint64_t *)&(cacheAnchorsPtr->atomic.exit), 1);
            }

          return sizeof(cacheAnchors_t) +
                 (sizeof(bgq_coords_t) + sizeof(uint32_t)) * fullSize;
#endif // OBSOLETE
	  return 0;
        };

#ifdef OBSOLETE
        inline size_t * getMapCache ()
        {
          return _mapcache;
        }

        inline size_t * getRankCache ()
        {
          return _rankcache;
        };

      private:

        size_t * _mapcache;
        size_t * _rankcache;
#endif // OBSOLETE

    }; // XMI::BgqMapCache
};     // XMI

#endif // __xmi_components_mapping_bgq_bgqmapcache_h__
