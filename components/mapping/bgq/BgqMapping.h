/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/mapping/bgq/BgqMapping.h
/// \brief ???
///
#ifndef __components_mapping_bgq_bgqmapping_h__
#define __components_mapping_bgq_bgqmapping_h__

#include <hwi/include/bqc/MU_PacketCommon.h>

#include "sys/xmi.h"

#include "components/mapping/BaseMapping.h"
#include "components/mapping/TorusMapping.h"
#include "components/mapping/NodeMapping.h"

#include "components/sysdep/bgq/BgqGlobal.h"

#include "util/common.h"

#define XMI_MAPPING_CLASS XMI::Mapping::BgqMapping

#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif

namespace XMI
{
  namespace Mapping
  {
#define BGQ_DIMS 7
    class BgqMapping : public Interface::Base<BgqMapping>,
        public Interface::Torus<BgqMapping, BGQ_DIMS>,
        public Interface::Node<BgqMapping>
    {
      public:

        inline BgqMapping () :
            Interface::Base<BgqMapping>(),
            Interface::Torus<BgqMapping, BGQ_DIMS>(),
            Interface::Node<BgqMapping> (),
            _a (__global.personality.aCoord()),
            _b (__global.personality.bCoord()),
            _c (__global.personality.cCoord()),
            _d (__global.personality.dCoord()),
            _e (__global.personality.eCoord()),
            _p (__global.personality.pCoord()),
            _t (__global.personality.tCoord())
        {
          SysDep::bgq_mapcache_t * cache = __global.getMapCache();

          _mapcache.torus.task2coords = cache->torus.task2coords;
          _mapcache.torus.coords2task = cache->torus.coords2task;
          _mapcache.node.local2peer   = cache->node.local2peer;
          _mapcache.node.peer2task    = cache->node.peer2task;

          _coords.thread   = _t;
          _coords.a        = _a;
          _coords.b        = _b;
          _coords.c        = _c;
          _coords.d        = _d;
          _coords.e        = _e;
          _coords.reserved =  0;
          _coords.core     = _p;

          coord2node (_t, _a, _b, _c, _d, _e, _p,
                      _nodeaddr.global, _nodeaddr.local);

          _size = __global.size();;

          TRACE_ERR((stderr, "BgqMapping::BgqMapping() .. coords: (%zd %zd %zd %zd %zd %zd), node: (%zd %zd)\n", _a, _b, _c, _d, _e, _t, _p, _nodeaddr.global, _nodeaddr.local));
        };

        inline ~BgqMapping () {};

      protected:
        size_t _task;
        size_t _size;
        size_t _nodes;
        size_t _peers;

        size_t _a;
        size_t _b;
        size_t _c;
        size_t _d;
        size_t _e;
        size_t _p;
        size_t _t;

        SysDep::bgq_coords_t  _coords;
        Interface::nodeaddr_t _nodeaddr;

        //size_t _numActiveRanksLocal;
        //size_t _numActiveRanksGlobal;
        //size_t _numActiveNodesGlobal;
        //size_t _fullSize;

        SysDep::bgq_mapcache_t _mapcache;
        //SysDep::bgq_coords_t * _mapcache;
        //uint32_t             * _rankcache;

        inline void coord2node (size_t   a,
                                size_t   b,
                                size_t   c,
                                size_t   d,
                                size_t   e,
                                size_t   p,
                                size_t   t,
                                size_t & global,
                                size_t & local)
        {
          global = e | (d << 1) | (c << 7) | (b << 13) | (a << 19);
          local  = p | (t << 4);
        };

        inline void node2coord (size_t & a,
                                size_t & b,
                                size_t & c,
                                size_t & d,
                                size_t & e,
                                size_t & p,
                                size_t & t,
                                size_t   global,
                                size_t   local)
        {
          e = global         & 0x00000001;
          d = (global >> 1)  & 0x0000003f;
          c = (global >> 7)  & 0x0000003f;
          b = (global >> 13) & 0x0000003f;
          a = global >> 19;

          p = local & 0x0000000f;
          t = local >> 4;
        };

      public:
        
        ///
        /// \brief Retrieve pointer to a mu destination structure for this task
        ///
        /// The intent here is to provide access to a mu destination structure
        /// initialized to the coordinates of the local node.
        ///
        inline MUHWI_Destination_t * getMuDestinationSelf ()
        {
          return (MUHWI_Destination_t *) &_coords;
        };



        /////////////////////////////////////////////////////////////////////////
        //
        // XMI::Mapping::Interface::Base interface implementation
        //
        /////////////////////////////////////////////////////////////////////////

        ///
        /// \brief Initialize the mapping
        /// \see XMI::Mapping::Interface::Base::init()
        ///
        inline xmi_result_t init_impl();

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

        /////////////////////////////////////////////////////////////////////////
        //
        // XMI::Mapping::Interface::Torus interface implementation
        //
        /////////////////////////////////////////////////////////////////////////

        ///
        /// \brief Get the BGQ torus address for this task
        /// \see XMI::Mapping::Interface::Torus::torusAddr()
        ///
        //template <>
        inline void torusAddr_impl (size_t (&addr)[BGQ_DIMS])
        {
          addr[0] = _a;
          addr[1] = _b;
          addr[2] = _c;
          addr[3] = _d;
          addr[4] = _e;
          addr[5] = _p;
          addr[7] = _t;
        }

        ///
        /// \brief Get the BGQ torus address for a task
        /// \see XMI::Mapping::Interface::Torus::task2torus()
        ///
        /// \todo Error path
        ///
        inline xmi_result_t task2torus_impl (size_t task, size_t (&addr)[BGQ_DIMS])
        {
          uint32_t abcdept = _mapcache.torus.task2coords[task].raw;

          addr[0] = (abcdept >> 24) & 0x00000003f; // 'a' coordinate
          addr[1] = (abcdept >> 18) & 0x00000003f; // 'b' coordinate
          addr[2] = (abcdept >> 12) & 0x00000003f; // 'c' coordinate
          addr[3] = (abcdept >>  6) & 0x00000003f; // 'd' coordinate
          addr[4] = (abcdept >>  5) & 0x000000001; // 'e' coordinate
          addr[5] = (abcdept)       & 0x00000000f; // 'p' coordinate
          addr[6] = (abcdept >> 30) & 0x000000003; // 't' coordinate

          return XMI_SUCCESS;
        }

        ///
        /// \brief Get the global task for a BGQ torus address
        /// \see XMI::Mapping::Interface::Torus::torus2task()
        ///
        /// \todo Error path
        ///
        inline xmi_result_t torus2task_impl (size_t (&addr)[BGQ_DIMS], size_t & task)
        {
          TRACE_ERR((stderr, "BgqMapping::torus2task_impl({%zd, %zd, %zd, %zd, %zd, %zd, %zd}, ...) >>\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6]));

          size_t aSize = __global.personality.aSize();
          size_t bSize = __global.personality.bSize();
          size_t cSize = __global.personality.cSize();
          size_t dSize = __global.personality.dSize();
          size_t eSize = __global.personality.eSize();
          size_t tSize = __global.personality.pSize();
          size_t pSize = __global.personality.tSize();

          // Verify that the bgq address is valid.
          // TODO convert to "unlikely if"
          if ((addr[0] >= aSize) ||
              (addr[1] >= bSize) ||
              (addr[2] >= cSize) ||
              (addr[3] >= dSize) ||
              (addr[4] >= eSize) ||
              (addr[5] >= pSize) ||
              (addr[6] >= tSize))
            {
              return XMI_INVAL;
            }

          // Estimate the task id based on the bgq coordinates.
          size_t hash =
            addr[6] * (aSize * bSize * cSize * dSize * eSize * pSize) +
            addr[5] * (aSize * bSize * cSize * dSize * eSize) +
            addr[4] * (aSize * bSize * cSize * dSize) +
            addr[3] * (aSize * bSize * cSize) +
            addr[2] * (aSize * bSize) +
            addr[1] * (aSize) +
            addr[0];

          // Verify that the estimated task is mapped.
          // TODO convert to 'unlikely_if'
          if (_mapcache.torus.coords2task[hash] == (unsigned) - 1)
            {
              return XMI_ERROR;
            }

          task = _mapcache.torus.coords2task[hash];

          TRACE_ERR((stderr, "BgqMapping::torus2task_impl({%zd, %zd, %zd, %zd, %zd, %zd, %zd}, %zd) <<\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], task));

          return XMI_SUCCESS;
        };


        /////////////////////////////////////////////////////////////////////////
        //
        // XMI::Mapping::Interface::Node interface implementation
        //
        /////////////////////////////////////////////////////////////////////////

        /// \see XMI::Mapping::Interface::Node::nodeTasks()
        inline xmi_result_t nodeTasks_impl (size_t global, size_t & tasks)
        {
          TRACE_ERR((stderr, "BgqMapping::nodeTasks_impl(%zd) >>\n", global));
#warning implement this!
          TRACE_ERR((stderr, "BgqMapping::nodeTasks_impl(%zd) <<\n", global));
          return XMI_UNIMPL;
        };

        /// \see XMI::Mapping::Interface::Node::nodePeers()
        inline xmi_result_t nodePeers_impl (size_t & peers)
        {
          peers = _peers;
          return XMI_SUCCESS;
        };

        /// \see XMI::Mapping::Interface::Node::isPeer()
        inline bool isPeer_impl (size_t task1, size_t task2)
        {
          uint32_t coord1 = _mapcache.torus.task2coords[task1].raw;
          uint32_t coord2 = _mapcache.torus.task2coords[task2].raw;

          return (bool) ((coord1 & 0x3fffffe0) == (coord2 & 0x3fffffe0));
        }

        /// \see XMI::Mapping::Interface::Node::nodeAddr()
        inline void nodeAddr_impl (Interface::nodeaddr_t & address)
        {
          TRACE_ERR((stderr, "BgqMapping::nodeAddr_impl() >>\n"));
          // Can this just be:  address = _nodeaddr; ???
          address.global = _nodeaddr.global;
          address.local  = _nodeaddr.local;
          TRACE_ERR((stderr, "BgqMapping::nodeAddr_impl(%zd, %zd) <<\n", address.global, address.local));
        };

        /// \see XMI::Mapping::Interface::Node::task2node()
        inline xmi_result_t task2node_impl (size_t task, Interface::nodeaddr_t & address)
        {
          TRACE_ERR((stderr, "BgqMapping::task2node_impl(%zd) >>\n", task));

          uint32_t coords = _mapcache.torus.task2coords[task].raw;

          // global coordinate is just the a,b,c,d,e torus coords.
          address.global = (coords >> 5) & 0x01ffffff;

          // local coordinate is the thread id (t) in the most significant
          // position followed by the core id (p) in the least significant
          // position.
          address.local = (coords & 0x0000000f) |
                          ((coords >> 22) & 0x000000300);

          TRACE_ERR((stderr, "BgqMapping::task2node_impl(%zd, {%zd, %zd}) <<\n", task, address.global, address.local));
          return XMI_SUCCESS;
        };

        /// \see XMI::Mapping::Interface::Node::node2task()
        inline xmi_result_t node2task_impl (Interface::nodeaddr_t address, size_t & task)
        {
          TRACE_ERR((stderr, "BgqMapping::node2task_impl({%zd, %zd}, ...) >>\n", address.global, address.local));

          size_t aSize = __global.personality.aSize();
          size_t bSize = __global.personality.bSize();
          size_t cSize = __global.personality.cSize();
          size_t dSize = __global.personality.dSize();
          size_t eSize = __global.personality.eSize();
          size_t tSize = __global.personality.pSize();
          size_t pSize = __global.personality.tSize();

          // global coordinate is just the a,b,c,d,e torus coords.
          size_t aCoord =  address.global >> 19;
          size_t bCoord = (address.global >> 13) & 0x0000003f;
          size_t cCoord = (address.global >>  7) & 0x0000003f;
          size_t dCoord = (address.global >>  1) & 0x0000003f;
          size_t eCoord =  address.global        & 0x00000001;

          // local coordinate is the thread id (t) in the most significant
          // position followed by the core id (p) in the least significant
          // position.
          size_t tCoord = address.local >> 4;
          size_t pCoord = address.local & 0x0000000f;

          // Verify that the bgq address is valid.
          // TODO convert to "unlikely if"
          if ((aCoord >= aSize) ||
              (bCoord >= bSize) ||
              (cCoord >= cSize) ||
              (dCoord >= dSize) ||
              (eCoord >= eSize) ||
              (pCoord >= pSize) ||
              (tCoord >= tSize))
            {
              return XMI_INVAL;
            }

          // Estimate the task id based on the bgq coordinates.
          size_t hash =
            tCoord * (aSize * bSize * cSize * dSize * eSize * pSize) +
            pCoord * (aSize * bSize * cSize * dSize * eSize) +
            eCoord * (aSize * bSize * cSize * dSize) +
            dCoord * (aSize * bSize * cSize) +
            cCoord * (aSize * bSize) +
            bCoord * (aSize) +
            aCoord;

          // Verify that the estimated task is mapped.
          // TODO convert to 'unlikely_if'
          if (_mapcache.torus.coords2task[hash] == (unsigned) - 1)
            {
              return XMI_ERROR;
            }

          task = _mapcache.torus.coords2task[hash];

          TRACE_ERR((stderr, "BgqMapping::node2task_impl({%zd, %zd}, %zd) <<\n", address.global, address.local, task));
          return XMI_SUCCESS;
        };

        inline xmi_result_t node2peer_impl (XMI::Mapping::Interface::nodeaddr_t & address, size_t & peer)
        {
          TRACE_ERR((stderr, "BgqMapping::node2peer_impl({%zd, %zd}, ...) >>\n", address.global, address.local));

          size_t tSize = __global.personality.pSize();
          size_t pSize = __global.personality.tSize();

          // local coordinate is the thread id (t) in the most significant
          // position followed by the core id (p) in the least significant
          // position.
          size_t tCoord = address.local >> 4;
          size_t pCoord = address.local & 0x0000000f;

          // Verify that the local node address is valid.
          // TODO convert to "unlikely if"
          if ((pCoord >= pSize) || (tCoord >= tSize))
            {
              return XMI_INVAL;
            }

          // Estimate the task id based on the bgq coordinates.
          size_t hash = tCoord * pSize + pCoord;

          // Verify that the address hash is valid.
          // TODO convert to 'unlikely_if'
          if (_mapcache.node.local2peer[hash] == (unsigned) - 1)
            {
              return XMI_ERROR;
            }

          peer = _mapcache.node.local2peer[hash];

          TRACE_ERR((stderr, "BgqMapping::node2peer_impl({%zd, %zd}, %zd) <<\n", address.global, address.local, peer));
          return XMI_SUCCESS;
        };

      private:

        static inline void compile_time_assert ()
        {
          // Make sure that the coords structure and the MUHWI_Destination_t
          // structure match
          COMPILE_TIME_ASSERT(sizeof(SysDep::bgq_coords_t) == sizeof(MUHWI_Destination_t));
        };
    };
  };
};

#if 0
static size_t XMI::Mapping::BgqMapping::initializeMapCache (SysDep::BgqPersonality  & personality,
                                                            void                    * ptr,
                                                            size_t                    bytes,
                                                            bgq_coords_t           ** mapcache,
                                                            uint32_t               ** rankcache)
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
      uint64_t exit;
    } atomic;
    volatile size_t *rankCachePtr; // Pointer to rank cache
    volatile size_t numActiveRanksLocal; // Number of ranks on our physical node.
    volatile size_t numActiveRanksGlobal;// Number of ranks in the partition.
    volatile size_t numActiveNodesGlobal;// Number of nodes in the partition.
    volatile size_t maxRank;       // Largest valid rank
    volatile size_t minRank;       // Smallest valid rank
  } cacheAnchors_t;

  volatile cacheAnchors_t * cacheAnchorsPtr;

  size_t myRank, mySize;

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

  cacheAnchorsPtr = (volatile cacheAnchors_t *) ptr;

  // Notify all other tasks on the node that this task has entered the
  // map cache initialization function.  If the value returned is zero
  // then this task is the first one in and is designated the "master".
  // All other tasks will wait until the master completes the
  // initialization.
  uint64_t participant =
    Fetch_and_Add ((uint64_t *) & (cacheAnchorsPtr->atomic.enter), 1);

  myRank = personality.rank();
#warning set 'mySize' ?

  // Calculate the number of potential ranks in this partition.
  size_t fullSize = aSize * bSize * cSize * dSize * eSize * pSize * tSize;

  _mapcache  = (bgq_coords_t *) (cacheAnchorsPtr + 1);
  _rankcache = (uint32_t *) (_mapcache + fullSize);

  size_t max_rank = 0, min_rank = (size_t) - 1;

  // If we are the master (participant 0), then initialize the caches.
  // Then, set the cache pointers into the shared memory area for the other
  // ranks on this node to see, and wait for them to see it.
  if (participant == 0)
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
#endif

xmi_result_t XMI::Mapping::BgqMapping::init_impl ()
{
  //_mapcache  = __global.getMapCache();
  //_rankcache = __global.getRankCache();
  

#if 0
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
  //_pers = & __global.personality;
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

  // Determine if we are the master rank on our physical node.  Do this by
  // finding the lowest t coordinate on our node, and if it is us, then we
  // are the master.
  for (tt = 0; tt < tsize; tt++)
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
  result = mm.memalign((void **) & _mapcache, 16, sizeof(kernel_coords_t) * _fullSize);

  // Allocate the rank cache from shared memory (in DUAL or VN modes) or from
  // heap (in SMP mode).
#warning fixme - shared memory allocation will FAIL in SMP mode - blocksome
  //_rankcache = (unsigned *) mm.scratchpad_dynamic_area_memalign ( 16, sizeof(size_t) * _fullSize);
  result = mm.memalign((void **) & _rankcache, 16, sizeof(size_t) * _fullSize);

  size_t max_rank = 0, min_rank = (size_t) - 1;
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
      if (rc == 0)
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
                  kernel_coords_t mapCacheElement = *(kernel_coords_t*) & _mapcache[i];
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

                  if (min_rank == (size_t) - 1) min_rank = i;

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

              if (err == 0)
                {
                  _mapcache[i] = ((x & 0xFF) << 24) | ((y & 0xFF) << 16) | ((z & 0xFF) << 8) | (t & 0xFF);
                  rarray[x][y][z]++;
                  cacheAnchorsPtr->numActiveRanksGlobal++;

                  if (rarray[x][y][z] == 1)
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
                  _mapcache[i] = (unsigned) - 1;
                }
            }

          cacheAnchorsPtr->numActiveRanksLocal = rarray[x()][y()][z()];
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

      _bgq_mbar();  // Ensure that stores to memory are in the memory.

      // Wait until the other t's on our physical node have seen the cache
      // pointers.
      for (tt = 0; tt < tsize; tt++)
        {
          while ( cacheAnchorsPtr->done[tt] == 0 )
            {
              _bgq_msync();
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
          _bgq_msync();
        }

      _mapcache = (unsigned*)(cacheAnchorsPtr->mapCachePtr);

      while ( cacheAnchorsPtr->rankCachePtr == NULL )
        {
          _bgq_msync();
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
      _bgq_mbar();

      cacheAnchorsPtr->done[tCoord] = 1;  // Indicate we have seen the info.
    }


#if 0
  // This is other gunk .. used by collectives?
  size_t rectsize = (ur.n_torus.coords[0] - ll.n_torus.coords[0] + 1) *
                    (ur.n_torus.coords[1] - ll.n_torus.coords[1] + 1) *
                    (ur.n_torus.coords[2] - ll.n_torus.coords[2] + 1) *
                    (ur.n_torus.coords[3] - ll.n_torus.coords[3] + 1);

  if (_numActiveRanksGlobal == rectsize)
    {
      _g_topology_world = new (&__topology_w) LL::Topology(&ll, &ur);
    }
  else if (max_rank - min_rank + 1 == _numActiveRanksGlobal)
    {
      // does this ever happen for "COMM_WORLD"?
      _g_topology_world = new (&__topology_w) LL::Topology(min_rank, max_rank);
    }
  else
    {
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

  for (unsigned i = 0; i < tSize(); i++)
    {
      unsigned peerrank = (unsigned) - 1;
      int rc = torus2rank(x(), y(), z(), i, peerrank);

      if ((rc == CM_SUCCESS) && (peerrank != (unsigned) - 1))
        {
          _localranks[index++] = peerrank;
        }
    }

#endif
#endif
  return XMI_SUCCESS;
};
#undef TRACE_ERR
#endif // __components_mapping_bgq_bgqmapping_h__

