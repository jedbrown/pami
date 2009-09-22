/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef __xmi_components_sysdep_bgq_bgqglobal_h__
#define __xmi_components_sysdep_bgq_bgqglobal_h__
///
/// \file components/sysdep/bgq/BgqGlobal.h
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

#include "util/common.h"
#include "components/sysdep/bgq/BgqPersonality.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x)  //fprintf x
#endif

namespace XMI
{
  namespace SysDep
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
    } bgq_mapcache_t;

    class BgqGlobal
    {
      public:

        inline BgqGlobal () :
          personality (),
          _memptr (NULL),
          _memsize (0),
          _mapcache ()
        {
          const char   * shmemfile = "/unique-xmi-global-shmem-file";
          size_t   bytes     = 1024*1024;
          size_t   pagesize  = 4096;

          // Round up to the page size
          size_t size = (bytes + pagesize - 1) & ~(pagesize - 1);

          int fd, rc;
          size_t n = size;

          TRACE_ERR((stderr, "BgqGlobal() .. size = %zd\n", size));
          fd = shm_open (shmemfile, O_CREAT | O_RDWR, 0600);
          TRACE_ERR((stderr, "BgqGlobal() .. after shm_open, fd = %d\n", fd));
          if ( fd != -1 )
          {
            rc = ftruncate( fd, n );
            TRACE_ERR((stderr, "BgqGlobal() .. after ftruncate(%d,%zd), rc = %d\n", fd,n,rc));
            if ( rc != -1 )
            {
              void * ptr = mmap( NULL, n, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
              TRACE_ERR((stderr, "BgqGlobal() .. after mmap, ptr = %p, MAP_FAILED = %p\n", ptr, MAP_FAILED));
              if ( ptr != MAP_FAILED )
              {
                _memptr  = ptr;
                _memsize = n;
                
                TRACE_ERR((stderr, "BgqGlobal() .. _memptr = %p, _memsize = %zd\n", _memptr, _memsize));
                size_t bytes_used =
                  initializeMapCache (personality);
                                      
                // Round up to the page size
                size = (bytes_used + pagesize - 1) & ~(pagesize - 1);
                
                // Truncate to this size.
                rc = ftruncate( fd, size );
                TRACE_ERR((stderr, "BgqGlobal() .. after second ftruncate(%d,%zd), rc = %d\n", fd,n,rc));
                if (rc != -1) return;
              }
            }
          }

          // There was a failure obtaining the shared memory segment, most
          // likely because the application is running in SMP mode. Allocate
          // memory from the heap instead.
          //
          // TODO - verify the run mode is actually SMP.
          posix_memalign ((void **)&_memptr, 16, bytes);
          memset (_memptr, 0, bytes);
          _memsize = bytes;
          TRACE_ERR((stderr, "BgqGlobal() .. FAILED, fake shmem on the heap, _memptr = %p, _memsize = %zd\n", _memptr, _memsize));
          size_t bytes_used = initializeMapCache (personality);

          TRACE_ERR((stderr, "BgqGlobal() <<\n"));

          return;
        };



        inline ~BgqGlobal () {};

        inline bgq_mapcache_t * getMapCache ()
        {
          return &_mapcache;
        };
        
        inline size_t size ()
        {
          return _size;
        }
        
     private:

        inline size_t initializeMapCache (BgqPersonality  & personality);

      public:

        BgqPersonality       personality;

      private:

        void           * _memptr;
        size_t           _memsize;
        bgq_mapcache_t   _mapcache;
        size_t           _size;
    }; // XMI::SysDep::BgqGlobal
  };   // XMI::SysDep
};     // XMI

size_t XMI::SysDep::BgqGlobal::initializeMapCache (BgqPersonality  & personality)
{
  void            * ptr      = _memptr;
  size_t            bytes    = _memsize;
  bgq_mapcache_t  * mapcache = &_mapcache;

  //fprintf (stderr, "XMI::SysDep::BgqGlobal::initializeMapCache() >> ptr = %p, bytes = %zd, mapcache = %p\n", ptr, bytes, mapcache);
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
  } cacheAnchors_t;

  volatile cacheAnchors_t * cacheAnchorsPtr = (volatile cacheAnchors_t *) ptr;

  size_t myRank;

  size_t tt, num_t = 0, rank, size;
  int err;

  size_t pCoord = personality.pCoord ();
  size_t tCoord = personality.tCoord ();

  size_t aSize  = personality.aSize ();
  size_t bSize  = personality.bSize ();
  size_t cSize  = personality.cSize ();
  size_t dSize  = personality.dSize ();
  size_t eSize  = personality.eSize ();
  size_t pSize  = personality.pSize ();
  size_t tSize  = personality.tSize ();

  //fprintf (stderr, "XMI::SysDep::BgqGlobal::initializeMapCache() .. p=%zd t=%zd {%zd %zd %zd %zd %zd}\n", pCoord, tCoord, aSize, bSize, cSize, dSize, eSize);

  // Notify all other tasks on the node that this task has entered the
  // map cache initialization function.  If the value returned is zero
  // then this task is the first one in and is designated the "master".
  // All other tasks will wait until the master completes the
  // initialization.
  uint64_t participant =
    Fetch_and_Add ((uint64_t *) & (cacheAnchorsPtr->atomic.enter), 1);

  myRank = personality.rank();

 //fprintf (stderr, "XMI::SysDep::BgqGlobal::initializeMapCache() .. myRank=%zd, participant=%ld\n", myRank, participant);
 //fprintf (stderr, "XMI::SysDep::BgqGlobal::initializeMapCache() .. {%zd %zd %zd %zd %zd %zd %zd}\n", aSize, bSize, cSize, dSize, eSize, pSize, tSize);


  // Calculate the number of potential tasks in this partition.
  size_t fullSize = aSize * bSize * cSize * dSize * eSize * pSize * tSize;

  // Calculate the number of potential tasks on a node in this partition.
  size_t peerSize = pSize * tSize;

 //fprintf (stderr, "XMI::SysDep::BgqGlobal::initializeMapCache() .. mapcache = %p, cacheAnchorsPtr = %p, sizeof(cacheAnchors_t) = %zd, fullSize = %zd, peerSize = %zd\n", mapcache, cacheAnchorsPtr, sizeof(cacheAnchors_t), fullSize, peerSize);
 //fprintf (stderr, "XMI::SysDep::BgqGlobal::initializeMapCache() .. mapcache->torus.task2coords = %p\n", mapcache->torus.task2coords);
 
  mapcache->torus.task2coords = (bgq_coords_t *) (cacheAnchorsPtr + 1);
  
 //fprintf (stderr, "XMI::SysDep::BgqGlobal::initializeMapCache() .. mapcache->torus.task2coords = %p mapcache->torus.coords2task = %p\n", mapcache->torus.task2coords, mapcache->torus.coords2task);
 
  mapcache->torus.coords2task = (uint32_t *) (mapcache->torus.task2coords + fullSize);
  
 //fprintf (stderr, "XMI::SysDep::BgqGlobal::initializeMapCache() .. mapcache->node.local2peer = %p mapcache->torus.coords2task = %p\n", mapcache->node.local2peer, mapcache->torus.coords2task);
 
  mapcache->node.local2peer   = (size_t *) (mapcache->torus.coords2task + peerSize);
  
 //fprintf (stderr, "XMI::SysDep::BgqGlobal::initializeMapCache() .. mapcache->node.local2peer = %p mapcache->node.peer2task = %p\n", mapcache->node.local2peer, mapcache->node.peer2task);
 
  mapcache->node.peer2task    = (size_t *) (mapcache->node.local2peer + peerSize);
  
 //fprintf (stderr, "XMI::SysDep::BgqGlobal::initializeMapCache() .. mapcache->node.local2peer = %p mapcache->node.peer2task = %p\n", mapcache->node.local2peer, mapcache->node.peer2task);

  size_t max_rank = 0, min_rank = (size_t) - 1;

  // If we are the master (participant 0), then initialize the caches.
  // Then, set the cache pointers into the shared memory area for the other
  // ranks on this node to see, and wait for them to see it.
  if (participant == 0)
    {
      // Sized to the addressable MU range.
      uint16_t rarray [64][64][64][64][2];
      memset(rarray, 0, sizeof(rarray));

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
#warning Need ranks2coords syscall!
//              int rc = Kernel_Ranks2Coords((bgq_coords_t *)mapcache->torus.task2coords, fullSize);

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
 //fprintf (stderr, "XMI::SysDep::BgqGlobal::initializeMapCache() .. fullSize = %zd\n", fullSize);

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

 //fprintf (stderr, "XMI::SysDep::BgqGlobal::initializeMapCache() .. i = %zd, {%zd %zd %zd %zd %zd %zd %zd}\n", i, a,b,c,d,e,p,t);

                // Increment the rank count on this node.
                rarray[a][b][c][d][e]++;

                // Increment the number of global ranks.
                cacheAnchorsPtr->numActiveRanksGlobal++;

                // If the rank count on this node is '1', this is the first
                // rank encountered on this node. Increment the number of active nodes.
                if (rarray[a][b][c][d][e] == 1)
                  cacheAnchorsPtr->numActiveNodesGlobal++;

                uint32_t addr_hash =
                  t * (aSize * bSize * cSize * dSize * eSize * pSize) +
                  p * (aSize * bSize * cSize * dSize * eSize) +
                  e * (aSize * bSize * cSize * dSize) +
                  d * (aSize * bSize * cSize) +
                  c * (aSize * bSize) +
                  b * (aSize) +
                  a;

                mapcache->torus.coords2task[addr_hash] = i;

                // because of "for (i..." this will give us MAX after loop.
                max_rank = i;

                if (min_rank == (size_t) - 1) min_rank = i;
            //  }
          }

        cacheAnchorsPtr->maxRank = max_rank;
        cacheAnchorsPtr->minRank = min_rank;
      }
      
      // Initialize the node task2peer and peer2task caches.
      uint32_t hash;
      size_t peer = 0;
      for (t=0; t<tSize; t++)
      {
        for (p=0; p<pSize; p++)
        {
          hash =
            t * (aSize * bSize * cSize * dSize * eSize * pSize) +
            p * (aSize * bSize * cSize * dSize * eSize) +
            e * (aSize * bSize * cSize * dSize) +
            d * (aSize * bSize * cSize) +
            c * (aSize * bSize) +
            b * (aSize) +
            a;

          mapcache->node.peer2task[peer] =
            mapcache->torus.coords2task[hash];

          hash = t * pSize + p;
          mapcache->node.local2peer[hash] = peer++;
        }
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

      mbar();

      //cacheAnchorsPtr->done[personality.tCoord()] = 1;  // Indicate we have seen the info.
      Fetch_and_Add ((uint64_t *)&(cacheAnchorsPtr->atomic.exit), 1);
    }

  _size = cacheAnchorsPtr->numActiveRanksGlobal;

  size_t mapsize = sizeof(cacheAnchors_t) +
         (sizeof(bgq_coords_t) + sizeof(uint32_t)) * fullSize +
         (sizeof(size_t)*2) * peerSize;

  //fprintf (stderr, "XMI::SysDep::BgqGlobal::initializeMapCache() << mapsize = %zd\n", mapsize);

  return mapsize;
};




extern XMI::SysDep::BgqGlobal __global;
#undef TRACE_ERR
#endif // __xmi_components_sysdep_bgq_bgqglobal_h__


