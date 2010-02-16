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

#ifndef TRACE_ERR
#define TRACE_ERR(x)//  fprintf x
#endif

namespace XMI
{
    class Global : public Interface::Global<XMI::Global>
    {
      public:

        inline Global () :
          personality (),
	  mapping(personality),
          _memptr (NULL),
          _memsize (0),
          _mapcache ()
        {
	  xmi_coord_t ll, ur;
	  size_t min=0, max=0;
          const char   * shmemfile = "/unique-xmi-global-shmem-file";
          //size_t   bytes     = 1024*1024;
          size_t   bytes     = 32*1024;
          size_t   pagesize  = 4096;

          // Round up to the page size
          size_t size = (bytes + pagesize - 1) & ~(pagesize - 1);

          int fd, rc;
          size_t n = size;

	  // CAUTION! The following sequence MUST ensure that "rc" is "-1" iff failure.
          TRACE_ERR((stderr, "Global() .. size = %zd\n", size));
          rc = shm_open (shmemfile, O_CREAT | O_RDWR, 0600);
          TRACE_ERR((stderr, "Global() .. after shm_open, fd = %d\n", fd));
          if ( rc != -1 )
          {
	    fd = rc;
            rc = ftruncate( fd, n );
            TRACE_ERR((stderr, "Global() .. after ftruncate(%d,%zd), rc = %d\n", fd,n,rc));
            if ( rc != -1 )
            {
              void * ptr = mmap( NULL, n, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
              TRACE_ERR((stderr, "Global() .. after mmap, ptr = %p, MAP_FAILED = %p\n", ptr, MAP_FAILED));
              if ( ptr != MAP_FAILED )
              {
                _memptr  = ptr;
                _memsize = n;

                TRACE_ERR((stderr, "Global() .. _memptr = %p, _memsize = %zd\n", _memptr, _memsize));
                size_t bytes_used =
                  initializeMapCache (personality, ll, ur, min, max);

                // Round up to the page size
                size = (bytes_used + pagesize - 1) & ~(pagesize - 1);

                // Truncate to this size.
                rc = ftruncate( fd, size );
                TRACE_ERR((stderr, "Global() .. after second ftruncate(%d,%zd), rc = %d\n", fd,n,rc));
	      } else { rc = -1; }
            }
          }

          if (rc == -1) {
          	// There was a failure obtaining the shared memory segment, most
          	// likely because the application is running in SMP mode. Allocate
          	// memory from the heap instead.
          	//
          	// TODO - verify the run mode is actually SMP.
          	posix_memalign ((void **)&_memptr, 16, bytes);
          	memset (_memptr, 0, bytes);
          	_memsize = bytes;
          	TRACE_ERR((stderr, "Global() .. FAILED, fake shmem on the heap, _memptr = %p, _memsize = %zd\n", _memptr, _memsize));
          	//size_t bytes_used = initializeMapCache (personality, ll, ur, min, max);
	  }

	  mapping.init(_mapcache, personality);
	  XMI::Topology::static_init(&mapping);
	  size_t rectsize = 1;
	  for (unsigned d = 0; d < mapping.globalDims(); ++d) {
		rectsize *= (ur.u.n_torus.coords[d] - ll.u.n_torus.coords[d] + 1);
	  }
	  if (mapping.size() == rectsize) {
		new (&topology_global) XMI::Topology(&ll, &ur);
	  } else if (mapping.size() == max - min + 1) {
		new (&topology_global) XMI::Topology(min, max);
	  } else {
		//XMI_abortf("failed to build global-world topology %zd::%zd(%zd) / %zd..%zd", mapping.size(), rectsize, mapping.globalDims(), min, max); //hack
	  }
	  topology_global.subTopologyLocalToMe(&topology_local);

          TRACE_ERR((stderr, "Global() <<\n"));

          return;
        };



        inline ~Global () {};

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
     private:

        inline size_t initializeMapCache (BgqPersonality  & personality,
				xmi_coord_t &ll, xmi_coord_t &ur, size_t &min, size_t &max);

      public:

        BgqPersonality       personality;
	XMI::Mapping         mapping;

      private:

        void           * _memptr;
        size_t           _memsize;
        bgq_mapcache_t   _mapcache;
        size_t           _size;
    }; // XMI::Global
};     // XMI

size_t XMI::Global::initializeMapCache (BgqPersonality  & personality,
				xmi_coord_t &ll, xmi_coord_t &ur, size_t &min, size_t &max)
{
  void            * ptr      = _memptr;
  //size_t            bytes    = _memsize;
  bgq_mapcache_t  * mapcache = &_mapcache;

  TRACE_ERR( (stderr, "XMI::Global::initializeMapCache() >> ptr = %p, bytes = %zd, mapcache = %p\n", ptr, _memsize, mapcache));
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
    volatile size_t maxRank;       // Largest valid rank
    volatile size_t minRank;       // Smallest valid rank
    volatile xmi_coord_t activeLLCorner;
    volatile xmi_coord_t activeURCorner;
  } cacheAnchors_t;

  volatile cacheAnchors_t * cacheAnchorsPtr = (volatile cacheAnchors_t *) ptr;

  size_t myRank;


  size_t pCoord = personality.pCoord ();
  size_t tCoord = personality.tCoord ();

  size_t aSize  = personality.aSize ();
  size_t bSize  = personality.bSize ();
  size_t cSize  = personality.cSize ();
  size_t dSize  = personality.dSize ();
  size_t eSize  = personality.eSize ();
  size_t pSize  = personality.pSize ();
  size_t tSize  = personality.tSize ();

  TRACE_ERR( (stderr, "XMI::Global::initializeMapCache() .. p=%zd t=%zd {%zd %zd %zd %zd %zd}\n", pCoord, tCoord, aSize, bSize, cSize, dSize, eSize));

  // Notify all other tasks on the node that this task has entered the
  // map cache initialization function.  If the value returned is zero
  // then this task is the first one in and is designated the "master".
  // All other tasks will wait until the master completes the
  // initialization.
  uint64_t participant =
    Fetch_and_Add ((uint64_t *) & (cacheAnchorsPtr->atomic.enter), 1);

  myRank = personality.rank();

  TRACE_ERR( (stderr, "XMI::Global::initializeMapCache() .. myRank=%zd, participant=%ld\n", myRank, participant));
  TRACE_ERR( (stderr, "XMI::Global::initializeMapCache() .. {%zd %zd %zd %zd %zd %zd %zd}\n", aSize, bSize, cSize, dSize, eSize, pSize, tSize));


  // Calculate the number of potential tasks in this partition.
  size_t fullSize = aSize * bSize * cSize * dSize * eSize * pSize * tSize;

  // Calculate the number of potential tasks on a node in this partition.
  size_t peerSize = pSize * tSize;

 TRACE_ERR( (stderr, "XMI::Global::initializeMapCache() .. mapcache = %p, cacheAnchorsPtr = %p, sizeof(cacheAnchors_t) = %zd, fullSize = %zd, peerSize = %zd\n", mapcache, cacheAnchorsPtr, sizeof(cacheAnchors_t), fullSize, peerSize));
 TRACE_ERR( (stderr, "XMI::Global::initializeMapCache() .. mapcache->torus.task2coords = %p\n", mapcache->torus.task2coords));

  mapcache->torus.task2coords = (bgq_coords_t *) (cacheAnchorsPtr + 1);

 TRACE_ERR( (stderr, "XMI::Global::initializeMapCache() .. mapcache->torus.task2coords = %p mapcache->torus.coords2task = %p\n", mapcache->torus.task2coords, mapcache->torus.coords2task));

  mapcache->torus.coords2task = (uint32_t *) (mapcache->torus.task2coords + fullSize);

 TRACE_ERR( (stderr, "XMI::Global::initializeMapCache() .. mapcache->node.local2peer = %p mapcache->torus.coords2task = %p\n", mapcache->node.local2peer, mapcache->torus.coords2task));

  mapcache->node.local2peer   = (size_t *) (mapcache->torus.coords2task + peerSize);

 TRACE_ERR( (stderr, "XMI::Global::initializeMapCache() .. mapcache->node.local2peer = %p mapcache->node.peer2task = %p\n", mapcache->node.local2peer, mapcache->node.peer2task));

  mapcache->node.peer2task    = (size_t *) (mapcache->node.local2peer + peerSize);

 TRACE_ERR( (stderr, "XMI::Global::initializeMapCache() .. mapcache->node.local2peer = %p mapcache->node.peer2task = %p\n", mapcache->node.local2peer, mapcache->node.peer2task));

  size_t max_rank = 0, min_rank = (size_t) - 1;
  xmi_coord_t _ll, _ur;

  // If we are the master (participant 0), then initialize the caches.
  // Then, set the cache pointers into the shared memory area for the other
  // ranks on this node to see, and wait for them to see it.
  if (participant == 0)
    {
      // Allocate storage for an array to be used in the loop below to track
      // the number of physical nodes in the partition.  The loop goes through
      // each rank, gets that rank's physical node coordinates, and sets a bit
      // in the array corresponding to that node, indicating that there is
      // a rank on that node.  The loop monitors the 0 to 1 transition of a
      // bit, and increments numActiveNodesGlobal when it sees the first
      // rank on the node.  After the loop, the storage for the array is
      // freed.
      uint64_t bcdeSize   = bSize * cSize * dSize * eSize;
      uint64_t cdeSize    = cSize * dSize * eSize;
      uint64_t deSize     = dSize * eSize;
      uint64_t numNodes   = aSize * bcdeSize;
      // Calculate number of array slots needed...
      uint64_t narraySize = (numNodes+63) >> 6; // Divide by 64 bits.
      uint8_t *narray = (uint8_t*)malloc(narraySize);
      XMI_assert(narray != NULL);
      memset(narray, 0, narraySize);

      // Initialize the task and peer mappings to -1 (== "not mapped")
      memset (mapcache->torus.coords2task, (uint32_t)-1, sizeof(uint32_t) * fullSize);
      memset (mapcache->node.peer2task, (uint8_t)-1, sizeof(size_t) * peerSize);

      size_t a = 0;
      size_t b = 0;
      size_t c = 0;
      size_t d = 0;
      size_t e = 0;
      size_t t = 0;
      size_t p = 0;

      /* Fill in the _mapcache array in a single syscall.
       * It is indexed by rank, dimensioned to be the full size of the
       * partition (ignoring -np), and filled in with the xyzt
       * coordinates of each rank packed into a single 4 byte int.
       * Non-active ranks (-np) have x, y, z, and t equal to 255, such
       * that the entire 4 byte int is -1.
       */
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// #warning Need ranks2coords syscall!
// int rc = Kernel_Ranks2Coords((bgq_coords_t *)mapcache->torus.task2coords, fullSize);
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

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
 TRACE_ERR( (stderr, "XMI::Global::initializeMapCache() .. fullSize = %zd\n", fullSize));

	_ll.network = _ur.network = XMI_N_TORUS_NETWORK;
	_ll.u.n_torus.coords[0] = _ur.u.n_torus.coords[0] = personality.aCoord();
	_ll.u.n_torus.coords[1] = _ur.u.n_torus.coords[1] = personality.bCoord();
	_ll.u.n_torus.coords[2] = _ur.u.n_torus.coords[2] = personality.cCoord();
	_ll.u.n_torus.coords[3] = _ur.u.n_torus.coords[3] = personality.dCoord();
	_ll.u.n_torus.coords[4] = _ur.u.n_torus.coords[4] = personality.eCoord();
	_ll.u.n_torus.coords[5] = _ur.u.n_torus.coords[5] = tCoord;
	_ll.u.n_torus.coords[6] = _ur.u.n_torus.coords[6] = pCoord;

        for (i = 0; i < fullSize; i++)
          {
            //if ( (int)_mapcache[i] != -1 )
            //  {
                //bgq_coords_t mapCacheElement = *((bgq_coords_t*) & _mapcache[i]);
                a = mapcache->torus.task2coords[i].a;
                b = mapcache->torus.task2coords[i].b;
                c = mapcache->torus.task2coords[i].c;
                d = mapcache->torus.task2coords[i].d;
                e = mapcache->torus.task2coords[i].e;
                p = mapcache->torus.task2coords[i].core;
                t = mapcache->torus.task2coords[i].thread;

 TRACE_ERR( (stderr, "XMI::Global::initializeMapCache() .. i = %zd, {%zd %zd %zd %zd %zd %zd %zd}\n", i, a,b,c,d,e,p,t));

                // Set the bit corresponding to the physical node of this rank,
		// indicating that we have found a rank on that node.
		// Increment numActiveNodesGlobal when the bit goes from 0 to 1.
		uint64_t tmpIndex    = (a*bcdeSize) + (b*cdeSize) + (c*deSize) + (d*eSize) + e;
		uint64_t narrayIndex = tmpIndex >> 6;     // Divide by 64 to get narray index.
		uint64_t bitNumber   = tmpIndex & (64-1); // Mask off high bits to get bit number.
		uint64_t bitNumberMask = _BN(bitNumber);

                if ( (narray[narrayIndex] & bitNumberMask) == 0)
		  {
		    cacheAnchorsPtr->numActiveNodesGlobal++;
		    narray[narrayIndex] |= bitNumberMask;
		  }

                // Increment the number of global ranks.
                cacheAnchorsPtr->numActiveRanksGlobal++;

                uint32_t addr_hash = ESTIMATED_TASK(a,b,c,d,e,t,p,aSize,bSize,cSize,dSize,eSize,tSize,pSize);
                mapcache->torus.coords2task[addr_hash] = i;

                // because of "for (i..." this will give us MAX after loop.
                max_rank = i;

                if (min_rank == (size_t) - 1) min_rank = i;

		if (a < _ll.u.n_torus.coords[0]) _ll.u.n_torus.coords[0] = a;
		if (b < _ll.u.n_torus.coords[1]) _ll.u.n_torus.coords[1] = b;
		if (c < _ll.u.n_torus.coords[2]) _ll.u.n_torus.coords[2] = c;
		if (d < _ll.u.n_torus.coords[3]) _ll.u.n_torus.coords[3] = d;
		if (e < _ll.u.n_torus.coords[4]) _ll.u.n_torus.coords[4] = e;
		if (t < _ll.u.n_torus.coords[5]) _ll.u.n_torus.coords[5] = t;
		if (p < _ll.u.n_torus.coords[6]) _ll.u.n_torus.coords[6] = p;

		if (a < _ur.u.n_torus.coords[0]) _ur.u.n_torus.coords[0] = a;
		if (b < _ur.u.n_torus.coords[1]) _ur.u.n_torus.coords[1] = b;
		if (c < _ur.u.n_torus.coords[2]) _ur.u.n_torus.coords[2] = c;
		if (d < _ur.u.n_torus.coords[3]) _ur.u.n_torus.coords[3] = d;
		if (e < _ur.u.n_torus.coords[4]) _ur.u.n_torus.coords[4] = e;
		if (t < _ur.u.n_torus.coords[5]) _ur.u.n_torus.coords[5] = t;
		if (p < _ur.u.n_torus.coords[6]) _ur.u.n_torus.coords[6] = p;
            //  }
          }

	free(narray); narray=NULL;
        cacheAnchorsPtr->maxRank = max_rank;
        cacheAnchorsPtr->minRank = min_rank;
	memcpy((void *)&cacheAnchorsPtr->activeLLCorner, &_ll, sizeof(_ll));
	memcpy((void *)&cacheAnchorsPtr->activeURCorner, &_ur, sizeof(_ur));
      }

      // Initialize the node task2peer and peer2task caches.
      uint32_t hash;
      size_t peer = 0;
      for (t=0; t<tSize; t++)
      {
        for (p=0; p<pSize; p++)
        {
          hash = ESTIMATED_TASK(a,b,c,d,e,t,p,aSize,bSize,cSize,dSize,eSize,tSize,pSize);

          mapcache->node.peer2task[peer] =
            mapcache->torus.coords2task[hash];

          hash = ESTIMATED_TASK(0,0,0,0,0,t,p,1,1,1,1,1,tSize,pSize);
          mapcache->node.local2peer[hash] = peer++;
		  cacheAnchorsPtr->numActiveRanksLocal++; //hack
        }
      }


      // Now that the map and rank caches have been initialized,
      // store their pointers into the shared memory cache pointer area so the
      // other nodes see these pointers.
      //cacheAnchorsPtr->mapCachePtr  = _mapcache;
      //cacheAnchorsPtr->rankCachePtr = _rankcache;

      // Copy the rank counts into the mapping object.
      //_numActiveRanksLocal  = cacheAnchorsPtr->numActiveRanksLocal;
      //_numActiveRanksGlobal = cacheAnchorsPtr->numActiveRanksGlobal;
      //_numActiveNodesGlobal = cacheAnchorsPtr->numActiveNodesGlobal;

      mbar();  // Ensure that stores to memory are in the memory.

      // Notify the other processes on this node that the master has
      // completed the initialization.
      Fetch_and_Add ((uint64_t *)&(cacheAnchorsPtr->atomic.exit), 1);

	memcpy((void *)&ll, &_ll, sizeof(ll));
	memcpy((void *)&ur, &_ur, sizeof(ur));

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




      //_mapcache = (unsigned*)(cacheAnchorsPtr->mapCachePtr);


      //_rankcache = (unsigned*)(cacheAnchorsPtr->rankCachePtr);

      //_numActiveRanksLocal  = cacheAnchorsPtr->numActiveRanksLocal;
      //_numActiveRanksGlobal = cacheAnchorsPtr->numActiveRanksGlobal;
      //_numActiveNodesGlobal = cacheAnchorsPtr->numActiveNodesGlobal;
      max_rank = cacheAnchorsPtr->maxRank;
      min_rank = cacheAnchorsPtr->minRank;

      min = min_rank;
      max = max_rank;
      memcpy(&ll, (void *)&cacheAnchorsPtr->activeLLCorner, sizeof(ll));
      memcpy(&ur, (void *)&cacheAnchorsPtr->activeURCorner, sizeof(ur));

      mbar();

      //cacheAnchorsPtr->done[personality.tCoord()] = 1;  // Indicate we have seen the info.
      Fetch_and_Add ((uint64_t *)&(cacheAnchorsPtr->atomic.exit), 1);
    }

  mapcache->size = cacheAnchorsPtr->numActiveRanksGlobal;
  mapcache->local_size = cacheAnchorsPtr->numActiveRanksLocal; //hack
  TRACE_ERR( (stderr,"local_size:%zu\n", mapcache->local_size));


  size_t mapsize = sizeof(cacheAnchors_t) +
         (sizeof(bgq_coords_t) + sizeof(uint32_t)) * fullSize +
         (sizeof(size_t)*2) * peerSize;

  TRACE_ERR( (stderr, "XMI::Global::initializeMapCache() << mapsize = %zd\n", mapsize));

  return mapsize;
};




extern XMI::Global __global;
#undef TRACE_ERR
#endif // __xmi_components_sysdep_bgq_bgqglobal_h__
