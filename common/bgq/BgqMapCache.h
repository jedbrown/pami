/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef __xmi_components_mapping_bgq_bgqmapcache_h__
#define __xmi_components_mapping_bgq_bgqmapcache_h__
///
/// \file common/bgq/BgqMapCache.h
/// \brief Blue Gene/Q Map- and Rank-Cache Object
///

#include <hwi/include/bqc/A2_inlines.h>

#include "common/bgq/BgqPersonality.h"


namespace XMI
{
    class BgqMapCache
    {
      public:

        inline BgqMapCache () {};

        inline ~BgqMapCache () {};

        inline size_t init (SysDep::BgqPersonality & personality,
                            void                   * ptr,
                            size_t                   bytes
                            )
        {

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

#if 0
          _numActiveRanksLocal = 0;
          _numActiveRanksGlobal = 0;
          _numActiveNodesGlobal = 0;

          // Calculate the number of potential ranks in this partition.
          _fullSize = xSize() * ySize() * zSize() * tsize;

          // Use the static portion of the shared memory area (guaranteed to be zeros
          // at this point when it is in shared memory) to anchor pointers to
          // the caches.  Only the master rank will initialize these caches and set the
          // pointers into this structure.  When the non-master ranks on this physical
          // node see the non-zero pointers, they can begin to use them.
          xmi_result_t result = mm.memalign((void **) & cacheAnchorsPtr, 16, sizeof(cacheAnchors_t));
#warning fixme - shared memory allocation will FAIL in SMP mode - blocksome
#endif
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
#warning set 'mySize' ?
#if 0
          // Determine if we are the master rank on our physical node.  Do this
          // by finding the lowest t coordinate on our node, and if it is us,
          // then we are the master.
          for (tt = 0; tt < tsize; tt++)
            {
              // See if there is a rank on our xyz with this t.
              int rc = Kernel_Coord2Rank (personality.xCoord(),
                                          personality.yCoord(),
                                          personality.zCoord(),
                                          tt, &rank, &size);

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
                      myRank = rank; // Save our task rank in the mapping object.
                      mySize = size; // Save the size in the mapping object.
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
#endif
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
                      if ( (int)_mapcache[i] != -1 )
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
                            t * (aSize * bSize * cSize * dSize * eSize * pSize) +
                            p * (aSize * bSize * cSize * dSize * eSize) +
                            e * (aSize * bSize * cSize * dSize) +
                            d * (aSize * bSize * cSize) +
                            c * (aSize * bSize) +
                            b * (aSize) +
                            a;

                          _rankcache[estimated_rank] = i;

                          // because of "for (i..." this will give us MAX after loop.
                          max_rank = i;

                          if (min_rank == (size_t) - 1) min_rank = i;
                       }
                    }

                  cacheAnchorsPtr->maxRank = max_rank;
                  cacheAnchorsPtr->minRank = min_rank;
                }
#if 0
              /* If the system call fails, assume the kernel is older and does not
               * have this system call.  Use the original system call, one call per
               * rank (which is slower than the single new system call) to obtain
               * the information necessary to fill in the _mapcache, etc.
               */
              else
                {
                  size_t i;

                  for (i = 0; i < fullSize; i++)
                    {
                      unsigned x, y, z, t;
                      err = Kernel_Rank2Coord ((int)i, &x, &y, &z, &t);

                      if (err == 0)
                        {
                          _mapcache[i] = ((x & 0xFF) << 24) | ((y & 0xFF) << 16) | ((z & 0xFF) << 8) | (t & 0xFF);
                          rarray[x][y][z]++;
                          cacheAnchorsPtr->numActiveRanksGlobal++;

                          if (rarray[x][y][z] == 1)
                            cacheAnchorsPtr->numActiveNodesGlobal++;

                          int estimated_rank =
                            t * (personality.xSize() * personality.ySize() * personality.zSize()) +
                            z * (personality.xSize() * personality.ySize()) +
                            y * (personality.xSize()) +
                            x;
                          _rankcache[estimated_rank] = i;
                        }
                      else
                        {
                          _mapcache[i] = (unsigned) - 1;
                        }
                    }

                  cacheAnchorsPtr->numActiveRanksLocal =
                    rarray[personality.xCoord()][personality.yCoord()][personality.zCoord()];
                }
#endif
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


# if 0
              // Wait until the other t's on our physical node have seen the cache
              // pointers.
              for (tt = 0; tt < tsize; tt++)
                {
                  while ( cacheAnchorsPtr->done[tt] == 0 )
                    {
                      _ppc_msync();
                    }
                }

              // Now that all nodes have seen the cache pointers, zero out the cache
              // anchor structure for others who expect this area to be zero.
              memset ((void*)cacheAnchorsPtr, 0x00, sizeof(cacheAnchors_t));
#endif
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
        };

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

    }; // XMI::BgqMapCache
};     // XMI

#endif // __xmi_components_mapping_bgq_bgqmapcache_h__
