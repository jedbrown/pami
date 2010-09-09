/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef __common_bgq_Global_h__
#define __common_bgq_Global_h__
///
/// \file common/bgq/Global.h
/// \brief Blue Gene/Q Global Object
///
/// This global object is constructed before main() and is a container class
/// for all other classes that need to be constructed and initialized before
/// the application starts.
///

#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include "Platform.h"
#include "util/common.h"
#include "common/GlobalInterface.h"
#include "common/bgq/BgqPersonality.h"
#include "common/bgq/BgqMapCache.h"
#include "Mapping.h"
#include "Topology.h"
#include "common/bgq/L2AtomicFactory.h"
#include "components/memory/heap/HeapMemoryManager.h"
#include "components/memory/shmem/SharedMemoryManager.h"

#ifndef PAMI_MAX_NUM_CLIENTS
/** \todo PAMI_MAX_NUM_CLIENTS needs to be setup by pami.h */
#define PAMI_MAX_NUM_CLIENTS    4
#endif // !PAMI_MAX_NUM_CLIENTS

// This enables compilation of the comm-thread code, but it will
// not be used unless something calls PAMI_Client_add_commthread_context().
// See test/BG/bgq/commthreads/single-context.c for example use.
// However, if desired, compilation may be disabled by changing
// the following to "#undef".
#define USE_COMMTHREADS // define/undef

#undef TRACE_ERR
#define TRACE_ERR(x) //fprintf x

///
/// \brief dump hex data to stderr
///  \param s : an informational text string to print
///  \param b : the (integer) buffer to dump
///  \param n : number of integers to dump from the buffer
#undef DUMP_HEXDATA
#define DUMP_HEXDATA(s,b,n) //globalDumpHexData(s,b,n)
void globalDumpHexData(const char * pstring, const uint32_t *buffer, size_t n_ints);

#define BGQ_GLOBAL_SHMEM_SIZE(nproc,nctx)	L2A_MAX_NUMNODEL2ATOMIC(nproc,nctx) * sizeof(uint64_t); ///< extra shmem for BGQ L2 Atomics and WAC

/// \page env_vars Environment Variables
///
/// PAMI_GLOBAL_SHMEMSIZE - Size, MB, Global shmem pool
/// default: sizeof(mapcache) + Node-L2Atomics
///

namespace PAMI
{
  class Global : public Interface::Global<PAMI::Global,
			PAMI::Memory::HeapMemoryManager,
			PAMI::Memory::SharedMemoryManager>
  {
    public:

      inline Global () :
          personality (),
          mapping(personality),
          l2atomicFactory(),
          _mapcache ()
      {
        pami_coord_t ll, ur;
        pami_task_t min = 0, max = 0;
        const char   * shmemfile = "/unique-pami-global-shmem-file";
        size_t   bytes;
        size_t   pagesize  = 4096;
        char *envopts;
        envopts = getenv("PAMI_DEVICE");
        _useshmem = true;
        _useMU = true;

        time.init(personality.Kernel_Config.FreqMHz);

        /// \todo #80 #99 Remove this when the DMA supports >1 context.
        /// Hopefully this is temporary. We should always include all
        /// the devices and have run time checks. But with no MU on
        /// the FPGA, this is required now, or you have to build
        /// multiple libraries.
        /// \note 4/13/2010: Enabling *just* MU does not work. Both or
        /// shmem are the only viable options.
        if (envopts != NULL)
          {
            if (strncasecmp(envopts, "B", 1) == 0) // use BOTH
              {
                _useshmem = true;
                _useMU = true;
              }
            else if (strncasecmp(envopts, "S", 1) == 0) // SHMEM only
              {
                _useshmem = true;
                _useMU = false;
              }
            else if (strncasecmp(envopts, "M", 1) == 0) // MU only
              {
                _useshmem = false;
                _useMU = true;
              }
            else
              {
                fprintf(stderr, "Invalid device option %s\n", envopts);
              }

            if (_useshmem) TRACE_ERR((stderr, "Using shmem device\n"));

            if (_useMU) TRACE_ERR((stderr, "Using MU device\n"));
          }

        bytes = initializeMapCache(personality, NULL, ll, ur, min, max, true);

        // Round up to the page size
        size_t size = ((bytes + pagesize - 1) & ~(pagesize - 1)) + BGQ_GLOBAL_SHMEM_SIZE(64,64);

	char *env = getenv("PAMI_GLOBAL_SHMEMSIZE");
	if (env) {
		size = strtoull(env, NULL, 0) * 1024 * 1024;
	}

        TRACE_ERR((stderr, "Global() .. size = %zu\n", size));
	mm.init(&shared_mm, size, 1, 0, shmemfile);
        (void)initializeMapCache(personality, &mm, ll, ur, min, max,
              ((mm.attrs() & PAMI::Memory::PAMI_MM_NODESCOPE) != 0)); //shared initialization

        mapping.init(_mapcache, personality);
        PAMI::Topology::static_init(&mapping);
        size_t rectsize = 1;

        for (unsigned d = 0; d < mapping.globalDims(); ++d)
          {
            rectsize *= (ur.u.n_torus.coords[d] - ll.u.n_torus.coords[d] + 1);
          }

        TRACE_ERR((stderr,  "Global() mapping.size %zu, rectsize %zu,mapping.globalDims %zu, min %u, max %u\n", mapping.size(), rectsize, mapping.globalDims(), min, max));

        if (mapping.size() == rectsize)
          {
            new (&topology_global) PAMI::Topology(&ll, &ur);
          }
        else if (mapping.size() == max - min + 1)
          {
            new (&topology_global) PAMI::Topology(min, max);
          }
        else
          {
            PAMI_abortf("failed to build global-world topology %zu::%zu(%zu) / %d..%d", mapping.size(), rectsize, mapping.globalDims(), min, max); //hack
          }

        topology_global.subTopologyLocalToMe(&topology_local);
        PAMI_assertf(topology_local.size() >= 1, "Failed to create valid (non-zero) local topology\n");
//fprintf(stderr, "__global.mm size=%zd\n", mm.size());
        l2atomicFactory.init(&mm, &heap_mm, &mapping, &topology_local);

        TRACE_ERR((stderr, "Global() <<\n"));

        return;
      };



      inline ~Global ()
      {
      }

      inline bgq_mapcache_t * getMapCache ()
      {
        return &_mapcache;
      };

      inline size_t size ()
      {
        return _mapcache.size;
      }

      inline size_t local_size () //hack
      {
        return _mapcache.local_size;
      }
      inline bool useshmem()
      {
        return _useshmem;
      }
      inline bool useMU()
      {
        return _useMU;
      }
      /// \todo temp function while MU2 isn't complete
      inline bool useMU(bool flag)
      {
        bool oldFlag = _useMU;
        _useMU = flag;
        return oldFlag;
      }
    private:

      inline size_t initializeMapCache (BgqPersonality  & personality,
                                        PAMI::Memory::MemoryManager *mm,
                                        pami_coord_t &ll, pami_coord_t &ur, pami_task_t &min, pami_task_t &max, bool shared);

    public:

      BgqPersonality       personality;
      PAMI::Mapping         mapping;
      PAMI::Atomic::BGQ::L2AtomicFactory l2atomicFactory;
      PAMI::Memory::MemoryManager mm;
      PAMI::Memory::MemoryManager *_wuRegion_mm;

    private:

      bgq_mapcache_t   _mapcache;
      size_t           _size;
      bool _useshmem;
      bool _useMU;
  }; // PAMI::Global
};     // PAMI


// If 'mm' is NULL, compute total memory needed for mapcache and return (doing nothing else).
size_t PAMI::Global::initializeMapCache (BgqPersonality  & personality,
                                         PAMI::Memory::MemoryManager *mm,
                                         pami_coord_t &ll, pami_coord_t &ur, pami_task_t &min, pami_task_t &max, bool shared)
{
  bgq_mapcache_t  * mapcache = &_mapcache;

  TRACE_ERR( (stderr, "Global::initializeMapCache() >> mm = %p, mapcache = %p\n", mm, mapcache));
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
      uint64_t exit;
    } atomic;
    volatile size_t *rankCachePtr; // Pointer to rank cache
    volatile size_t numActiveRanksLocal; // Number of ranks on our physical node.
    volatile size_t numActiveRanksGlobal;// Number of ranks in the partition.
    volatile size_t numActiveNodesGlobal;// Number of nodes in the partition.
    volatile pami_task_t maxRank;       // Largest valid rank
    volatile pami_task_t minRank;       // Smallest valid rank
    volatile pami_coord_t activeLLCorner;
    volatile pami_coord_t activeURCorner;
    volatile size_t       lowestTCoordOnMyNode;
  } cacheAnchors_t;

  //size_t myRank;



  size_t aCoord = personality.aCoord ();
  size_t bCoord = personality.bCoord ();
  size_t cCoord = personality.cCoord ();
  size_t dCoord = personality.dCoord ();
  size_t eCoord = personality.eCoord ();
  size_t tCoord = personality.tCoord ();

  size_t aSize  = personality.aSize ();
  size_t bSize  = personality.bSize ();
  size_t cSize  = personality.cSize ();
  size_t dSize  = personality.dSize ();
  size_t eSize  = personality.eSize ();
  size_t tSize  = personality.tSize ();

  TRACE_ERR( (stderr, "Global::initializeMapCache() .. myCoords{%zu %zu %zu %zu %zu %zu} size{%zu %zu %zu %zu %zu %zu}\n", aCoord, bCoord, cCoord, dCoord, eCoord, tCoord, aSize, bSize, cSize, dSize, eSize, tSize));

  // Calculate the number of potential tasks in this partition.
  size_t fullSize = aSize * bSize * cSize * dSize * eSize * tSize;

  // Calculate the number of potential tasks on a node in this partition.
  size_t peerSize = tSize;

  if (!mm)
    {
      size_t mapsize = sizeof(cacheAnchors_t) +
                       fullSize * sizeof(*mapcache->torus.task2coords) +
                       fullSize * sizeof(*mapcache->torus.coords2task) +
                       peerSize * sizeof(*mapcache->node.local2peer) +
                       peerSize * sizeof(*mapcache->node.peer2task);
      TRACE_ERR( (stderr, "Global::initializeMapCache() << mapsize = %zu\n", mapsize));
      return mapsize;
    }

  volatile cacheAnchors_t *cacheAnchorsPtr;
  mm->memalign((void **)&cacheAnchorsPtr, 16, sizeof(*cacheAnchorsPtr));
  PAMI_assertf(cacheAnchorsPtr, "Failed to get memory for cacheAnchorsPtr");

  TRACE_ERR( (stderr, "Global::initializeMapCache() .. mapcache = %p, size = %zu, cacheAnchorsPtr = %p, sizeof(cacheAnchors_t) = %zu, fullSize = %zu, peerSize = %zu\n", mapcache, mm->size(), cacheAnchorsPtr, sizeof(cacheAnchors_t), fullSize, peerSize));

  if (shared)
    {
      DUMP_HEXDATA("Shared memory", (const uint32_t *)cacheAnchorsPtr, 16);
    }

  // Notify all other tasks on the node that this task has entered the
  // map cache initialization function.  If the value returned is zero
  // then this task is the first one in and is designated the "master".
  // All other tasks will wait until the master completes the
  // initialization.
  uint64_t participant = Fetch_and_Add((uint64_t *) & (cacheAnchorsPtr->atomic.enter), 1); /// \todo this isn't working on mambo

  //myRank = personality.rank();

  TRACE_ERR( (stderr, "Global::initializeMapCache() .. participant=%ld\n", participant));

  mm->memalign((void **)&mapcache->torus.task2coords, 16, fullSize * sizeof(*mapcache->torus.task2coords));
  PAMI_assertf(cacheAnchorsPtr, "Failed to get memory for task2coords");
  mm->memalign((void **)&mapcache->torus.coords2task, 16, fullSize * sizeof(*mapcache->torus.coords2task));
  PAMI_assertf(cacheAnchorsPtr, "Failed to get memory for coords2task");
  TRACE_ERR( (stderr, "Global::initializeMapCache() .. mapcache->torus.task2coords = %p mapcache->torus.coords2task = %p\n", mapcache->torus.task2coords, mapcache->torus.coords2task));
  TRACE_ERR( (stderr, "Global::initializeMapCache() .. mapcache->node.local2peer = %p mapcache->torus.coords2task = %p\n", mapcache->node.local2peer, mapcache->torus.coords2task));

  mm->memalign((void **)&mapcache->node.local2peer, 16, peerSize * sizeof(*mapcache->node.local2peer));
  PAMI_assertf(cacheAnchorsPtr, "Failed to get memory for local2peer");
  mm->memalign((void **)&mapcache->node.peer2task, 16, peerSize * sizeof(*mapcache->node.peer2task));
  PAMI_assertf(cacheAnchorsPtr, "Failed to get memory for peer2task");
  TRACE_ERR( (stderr, "Global::initializeMapCache() .. mapcache->node.local2peer = %p mapcache->node.peer2task = %p\n", mapcache->node.local2peer, mapcache->node.peer2task));

  pami_task_t max_rank = 0, min_rank = (pami_task_t) - 1;
  pami_coord_t _ll, _ur;

  // If we are the master (participant 0), then initialize the caches.
  // Then, set the cache pointers into the shared memory area for the other
  // ranks on this node to see, and wait for them to see it.
  if ((participant == 0) || !shared)
    {
      // Allocate storage for an array to be used in the loop below to track
      // the number of physical nodes in the partition.  The loop goes through
      // each rank, gets that rank's physical node coordinates, and sets a bit
      // in the array corresponding to that node, indicating that there is
      // a rank on that node.  The loop monitors the 0 to 1 transition of a
      // bit, and increments numActiveNodesGlobal when it sees the first
      // rank on the node.  After the loop, the storage for the array is
      // freed.
      uint64_t numNodes   = aSize * bSize * cSize * dSize * eSize;
      // Calculate number of array slots needed...
      uint64_t narraySize = (numNodes + 63) >> 6; // Divide by 64 bits.
      uint64_t *narray = (uint64_t*)malloc(narraySize * sizeof(uint64_t));
      PAMI_assert(narray != NULL);
      memset(narray, 0, narraySize*sizeof(uint64_t));

      // Initialize the task and peer mappings to -1 (== "not mapped")
      memset (mapcache->torus.coords2task, (uint32_t) - 1, sizeof(uint32_t) * fullSize);
      memset (mapcache->node.peer2task, (uint8_t) - 1, sizeof(size_t) * peerSize);

      size_t a = 0;
      size_t b = 0;
      size_t c = 0;
      size_t d = 0;
      size_t e = 0;
      size_t t = 0;

      /* Fill in the _mapcache array in a single syscall.
       * It is indexed by rank, dimensioned to be the full size of the
       * partition (ignoring -np), and filled in with the xyzt
       * coordinates of each rank packed into a single 4 byte int.
       * Non-active ranks (-np) have x, y, z, and t equal to 255, such
       * that the entire 4 byte int is -1.
       */
      int rc = 0;

      uint64_t numentries;

      rc = Kernel_RanksToCoords(fullSize * sizeof(*mapcache->torus.task2coords), (BG_CoordinateMapping_t *)mapcache->torus.task2coords, &numentries);
      TRACE_ERR( (stderr, "Global::initializeMapCache() .. fullSize = %zu, numentries %zu\n", fullSize, numentries));

      //BG_CoordinateMapping_t* map = (BG_CoordinateMapping_t *)mapcache->torus.task2coords;

      //for (uint64_t x = 0; x < numentries; x++) fprintf(stderr, "Global::initializeMapCache() index=%zu: a=%d, b=%d, c=%d, d=%d, e=%d, t=%d\n", x, map[x].a, map[x].b, map[x].c, map[x].d, map[x].e, map[x].t);

      // If the syscall works, obtain info from the returned _mapcache.
      if (rc == 0)
        {
          /* Obtain the following information from the _mapcache:
           * 1. Number of active ranks in the partition.
           * 2. Number of active compute nodes in the partition.
           * 3. _rankcache (the reverse of _mapcache).  It is indexed by
           *    coordinates and contains the rank.
           * 4. Number of active ranks on each compute node.
	   * 5. Lowest T coordinate on our node.
           */
          size_t i;

          _ll.network = _ur.network = PAMI_N_TORUS_NETWORK;
          _ll.u.n_torus.coords[0] = _ur.u.n_torus.coords[0] = personality.aCoord();
          _ll.u.n_torus.coords[1] = _ur.u.n_torus.coords[1] = personality.bCoord();
          _ll.u.n_torus.coords[2] = _ur.u.n_torus.coords[2] = personality.cCoord();
          _ll.u.n_torus.coords[3] = _ur.u.n_torus.coords[3] = personality.dCoord();
          _ll.u.n_torus.coords[4] = _ur.u.n_torus.coords[4] = personality.eCoord();
          _ll.u.n_torus.coords[5] = _ur.u.n_torus.coords[5] = tCoord;

          size_t numActiveRanksGlobal = fullSize;
          size_t numActiveNodesGlobal = 0;
	  size_t lowestTCoordOnMyNode = 999999999;

          max_rank = fullSize - 1;
          min_rank = 0;

          for (i = 0; i < fullSize; i++)
            {
              a = mapcache->torus.task2coords[i].mapped.a;
              b = mapcache->torus.task2coords[i].mapped.b;
              c = mapcache->torus.task2coords[i].mapped.c;
              d = mapcache->torus.task2coords[i].mapped.d;
              e = mapcache->torus.task2coords[i].mapped.e;
              t = mapcache->torus.task2coords[i].mapped.t;
              TRACE_ERR( (stderr, "Global::initializeMapCache() task = %zu, estimated task = %zu, coords{%zu,%zu,%zu,%zu,%zu,%zu}\n", i, ESTIMATED_TASK(a, b, c, d, e, t, aSize, bSize, cSize, dSize, eSize, tSize), a, b, c, d, e, t));

	      // If this task is local to our node
	      // 1. Track the lowest T coord on our node
	      // 2. Track whether this task is local to our node.  This is stored in
	      //    the "reserved" bit of the coords structure.
	      //
	      if ( (a==aCoord) && (b==bCoord) && (c==cCoord) && (d==dCoord) && (e==eCoord) )
		{
		  if ( t < lowestTCoordOnMyNode )
		    lowestTCoordOnMyNode = t;

		  mapcache->torus.task2coords[i].mapped.reserved = 1;
		}

              // Set the bit corresponding to the physical node of this rank,
              // indicating that we have found a rank on that node.
              // Increment numActiveNodesGlobal when the bit goes from 0 to 1.
              uint64_t tmpIndex      = ESTIMATED_TASK(a, b, c, d, e, 0, aSize, bSize, cSize, dSize, eSize, tSize) / tSize;
              uint64_t narrayIndex   = tmpIndex >> 6;     // Divide by 64 to get narray index.
              uint64_t bitNumber     = tmpIndex & (64 - 1); // Mask off high bits to get bit number.
              uint64_t bitNumberMask = _BN(bitNumber);

              if ((narray[narrayIndex] & bitNumberMask) == 0)
                {
                  numActiveNodesGlobal++;
                  narray[narrayIndex] |= bitNumberMask;
                  TRACE_ERR( (stderr, "Global::initializeMapCache() bitNumberMask = %#.16lX, narray[%#.16lX]=%#.16lX\n", bitNumberMask, narrayIndex, narray[narrayIndex]));
                }

              uint32_t addr_hash = ESTIMATED_TASK(a, b, c, d, e, t, aSize, bSize, cSize, dSize, eSize, tSize);
              mapcache->torus.coords2task[addr_hash] = i;
              TRACE_ERR((stderr, "Global::initializeMapCache() .. coords2task[%d]=%#lX\n", addr_hash, i));


              if (a < _ll.u.n_torus.coords[0]) _ll.u.n_torus.coords[0] = a;

              if (b < _ll.u.n_torus.coords[1]) _ll.u.n_torus.coords[1] = b;

              if (c < _ll.u.n_torus.coords[2]) _ll.u.n_torus.coords[2] = c;

              if (d < _ll.u.n_torus.coords[3]) _ll.u.n_torus.coords[3] = d;

              if (e < _ll.u.n_torus.coords[4]) _ll.u.n_torus.coords[4] = e;

              if (t < _ll.u.n_torus.coords[5]) _ll.u.n_torus.coords[5] = t;

              if (a > _ur.u.n_torus.coords[0]) _ur.u.n_torus.coords[0] = a;

              if (b > _ur.u.n_torus.coords[1]) _ur.u.n_torus.coords[1] = b;

              if (c > _ur.u.n_torus.coords[2]) _ur.u.n_torus.coords[2] = c;

              if (d > _ur.u.n_torus.coords[3]) _ur.u.n_torus.coords[3] = d;

              if (e > _ur.u.n_torus.coords[4]) _ur.u.n_torus.coords[4] = e;

              if (t > _ur.u.n_torus.coords[5]) _ur.u.n_torus.coords[5] = t;

            }

	  // Record information in the common cacheAnchors structure.
          cacheAnchorsPtr->numActiveRanksGlobal = numActiveRanksGlobal;
          cacheAnchorsPtr->numActiveNodesGlobal = numActiveNodesGlobal;
	  cacheAnchorsPtr->lowestTCoordOnMyNode  = lowestTCoordOnMyNode;

          free(narray);
          narray = NULL;
          cacheAnchorsPtr->maxRank = max_rank;
          cacheAnchorsPtr->minRank = min_rank;
          memcpy((void *)&cacheAnchorsPtr->activeLLCorner, &_ll, sizeof(_ll));
          memcpy((void *)&cacheAnchorsPtr->activeURCorner, &_ur, sizeof(_ur));
          TRACE_ERR((stderr, "Global::initializeMapCache() numActiveRanksGlobal %zu,numActiveNodesGlobal %zu,max_rank %u, min_rank %u, lowestTCoordOnMyNode %zu\n", numActiveRanksGlobal, numActiveNodesGlobal, max_rank, min_rank, lowestTCoordOnMyNode ));
        }
      else PAMI_abortf("Kernel_RanksToCoords(%zd, %p, %p) rc = %d\n", fullSize * sizeof(*mapcache->torus.task2coords), mapcache->torus.task2coords, &numentries, rc);

      // Initialize the node task2peer and peer2task caches.
      uint32_t hash;
      size_t peer = 0;
      size_t numRanks = 0;

      for (t = 0; t < tSize; t++)
        {
          hash = ESTIMATED_TASK(a, b, c, d, e, t, aSize, bSize, cSize, dSize, eSize, tSize);
          mapcache->node.peer2task[peer] = mapcache->torus.coords2task[hash];

          hash = ESTIMATED_TASK(0, 0, 0, 0, 0, t, 1, 1, 1, 1, 1, tSize);
          mapcache->node.local2peer[hash] = peer++;

          numRanks++; // increment local variable

          TRACE_ERR((stderr, "Global::initializeMapCache() .. peer2task[%zu]=%zu, local2peer[%d]=%zu\n", peer - 1, mapcache->node.peer2task[peer-1], hash, peer - 1));
        }

      cacheAnchorsPtr->numActiveRanksLocal = numRanks; // update global from local variable

      mbar();  // Ensure that stores to memory are in the memory.

      // Notify the other processes on this node that the master has
      // completed the initialization.
      Fetch_and_Add ((uint64_t *)&(cacheAnchorsPtr->atomic.exit), 1);

      memcpy((void *)&ll, &_ll, sizeof(ll));
      memcpy((void *)&ur, &_ur, sizeof(ur));

    } // End: Allocate an initialize the map and rank caches.


  // We are not the master t on our physical node.  Wait for the master t to
  // initialize the caches.  Then grab the pointers and rank counts
  else
    {
      while (cacheAnchorsPtr->atomic.exit == 0);

      max_rank = cacheAnchorsPtr->maxRank;
      min_rank = cacheAnchorsPtr->minRank;

      memcpy(&ll, (void *)&cacheAnchorsPtr->activeLLCorner, sizeof(ll));
      memcpy(&ur, (void *)&cacheAnchorsPtr->activeURCorner, sizeof(ur));

    }

  min = min_rank;
  max = max_rank;

  mapcache->size = cacheAnchorsPtr->numActiveRanksGlobal;
  mapcache->local_size = cacheAnchorsPtr->numActiveRanksLocal;
  mapcache->lowestTCoordOnMyNode = cacheAnchorsPtr->lowestTCoordOnMyNode;
  mapcache->numActiveNodesGlobal= cacheAnchorsPtr->numActiveNodesGlobal;

  TRACE_ERR( (stderr, "Global::initializeMapCache() .. size: %zu local_size:%zu lowestTCoordOnMyNode %zu\n", mapcache->size, mapcache->local_size,mapcache->lowestTCoordOnMyNode ));

  return 0;
};

inline void globalDumpHexData(const char * pstring, const uint32_t *buffer, size_t n_ints)
{
  fprintf(stderr, "dumphex:%s:%p/%p:%zu:\n\n", pstring, &buffer, buffer, sizeof(unsigned)*n_ints);
  unsigned nChunks = n_ints / 8;

  if (!buffer || !n_ints) return;

  for (unsigned i = 0; i < nChunks; i++)
    {
      fprintf(stderr,
              "<%p>: %8.8X %8.8X %8.8X %8.8X %8.8X %8.8X %8.8X %8.8X \n",
              buffer + (i*8),
              *(buffer + (i*8) + 0),
              *(buffer + (i*8) + 1),
              *(buffer + (i*8) + 2),
              *(buffer + (i*8) + 3),
              *(buffer + (i*8) + 4),
              *(buffer + (i*8) + 5),
              *(buffer + (i*8) + 6),
              *(buffer + (i*8) + 7)
             );
    }

  if (n_ints % 8)
    {
      unsigned lastChunk = nChunks * 8;
      fprintf(stderr,
              "<%p>: %8.8X %8.8X %8.8X %8.8X %8.8X %8.8X %8.8X %8.8X \n",
              buffer + lastChunk,
              lastChunk + 0 < n_ints ? *(buffer + lastChunk + 0) : 0xDEADDEAD,
              lastChunk + 1 < n_ints ? *(buffer + lastChunk + 1) : 0xDEADDEAD,
              lastChunk + 2 < n_ints ? *(buffer + lastChunk + 2) : 0xDEADDEAD,
              lastChunk + 3 < n_ints ? *(buffer + lastChunk + 3) : 0xDEADDEAD,
              lastChunk + 4 < n_ints ? *(buffer + lastChunk + 4) : 0xDEADDEAD,
              lastChunk + 5 < n_ints ? *(buffer + lastChunk + 5) : 0xDEADDEAD,
              lastChunk + 6 < n_ints ? *(buffer + lastChunk + 6) : 0xDEADDEAD,
              lastChunk + 7 < n_ints ? *(buffer + lastChunk + 7) : 0xDEADDEAD
             );
      lastChunk = 0; // gets rid of an annoying warning when not tracing the buffer
    }
}



extern PAMI::Global __global;
#undef TRACE_ERR
#endif // __pami_components_sysdep_bgq_bgqglobal_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
