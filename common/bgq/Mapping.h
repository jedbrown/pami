/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file common/bgq/Mapping.h
/// \brief ???
///
#ifndef __common_bgq_Mapping_h__
#define __common_bgq_Mapping_h__

#include <hwi/include/bqc/MU_PacketCommon.h>

#include "sys/xmi.h"
#include "Platform.h"

#include "common/BaseMappingInterface.h"
#include "common/TorusMappingInterface.h"
#include "common/NodeMappingInterface.h"

#include "common/bgq/BgqPersonality.h"
#include "common/bgq/BgqMapCache.h"

#include "util/common.h"

#define XMI_MAPPING_CLASS XMI::Mapping

#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif

namespace XMI
{
#define BGQ_TDIMS 5
#define BGQ_LDIMS 2
    class Mapping :
        public Interface::Mapping::Base<Mapping>,
        public Interface::Mapping::Torus<Mapping, BGQ_TDIMS>,
        public Interface::Mapping::Node<Mapping,BGQ_LDIMS>
    {
      public:

        inline Mapping (XMI::BgqPersonality &pers) :
            Interface::Mapping::Base<Mapping>(),
            Interface::Mapping::Torus<Mapping, BGQ_TDIMS>(),
            Interface::Mapping::Node<Mapping,BGQ_LDIMS> (),
	    _pers(pers),
            _a (_pers.aCoord()),
            _b (_pers.bCoord()),
            _c (_pers.cCoord()),
            _d (_pers.dCoord()),
            _e (_pers.eCoord()),
            _p (_pers.pCoord()),
            _t (_pers.tCoord())
        {
          // ----------------------------------------------------------------
          // Compile-time assertions
          // ----------------------------------------------------------------

          // Make sure that the coords structure and the MUHWI_Destination_t
          // structure match
          COMPILE_TIME_ASSERT(sizeof(bgq_coords_t) == sizeof(MUHWI_Destination_t));

          // ----------------------------------------------------------------
          // Compile-time assertions
          // ----------------------------------------------------------------

          _coords.thread   = _t;
          _coords.a        = _a;
          _coords.b        = _b;
          _coords.c        = _c;
          _coords.d        = _d;
          _coords.e        = _e;
          _coords.reserved =  0;
          _coords.core     = _p;

          //coord2node (_t, _a, _b, _c, _d, _e, _p,
          //            _nodeaddr.global, _nodeaddr.local);

          coord2node (_a, _b, _c, _d, _e, _p,_t,			//fix?
                      _nodeaddr.global, _nodeaddr.local);
          TRACE_ERR((stderr, "Mapping() .. coords: (%zd %zd %zd %zd %zd %zd), node: (%zd %zd)\n", _a, _b, _c, _d, _e, _t, _p, _nodeaddr.global, _nodeaddr.local));
        };

        inline ~Mapping () {};

      protected:
	XMI::BgqPersonality &_pers;
        size_t _task;
        size_t _nodes;
        size_t _peers;

        size_t _a;
        size_t _b;
        size_t _c;
        size_t _d;
        size_t _e;
        size_t _p;
        size_t _t;

        bgq_coords_t  _coords;
        Interface::Mapping::nodeaddr_t _nodeaddr;

        bgq_mapcache_t _mapcache;

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
        // XMI::Interface::Mapping::Base interface implementation
        //
        /////////////////////////////////////////////////////////////////////////

        ///
        /// \brief Initialize the mapping
        /// \see XMI::Interface::Mapping::Base::init()
        ///
	inline xmi_result_t init(bgq_mapcache_t &mapcache,
				XMI::BgqPersonality &personality);

        ///
        /// \brief Return the BGP global task for this process
        /// \see XMI::Interface::Mapping::Base::task()
        ///
        inline size_t task_impl()
        {
          //return _task;
		 return _p/(16/_mapcache.local_size); //hack
        }

        ///
        /// \brief Returns the number of global tasks
        /// \see XMI::Interface::Mapping::Base::size()
        ///
        inline size_t size_impl()
        {
          //return _mapcache.size;
          return _mapcache.local_size; //hack
        }

        ///
        /// \brief Returns the number of global dimensions
        /// \see XMI::Interface::Mapping::Base::globalDims()
        ///
        inline size_t globalDims_impl()
        {
          return BGQ_TDIMS + BGQ_LDIMS;
        }

        /////////////////////////////////////////////////////////////////////////
        //
        // XMI::Interface::Mapping::Torus interface implementation
        //
        /////////////////////////////////////////////////////////////////////////

        ///
        /// \brief Get the BGQ torus address for this task
        /// \see XMI::Interface::Mapping::Torus::torusAddr()
        ///
        //template <>
        inline void torusAddr_impl (size_t (&addr)[BGQ_TDIMS + BGQ_LDIMS])
        {
          addr[0] = _a;
          addr[1] = _b;
          addr[2] = _c;
          addr[3] = _d;
          addr[4] = _e;
          addr[6] = _t;
          addr[5] = _p;
        }

        ///
        /// \brief Get the BGQ torus address for a task
        /// \see XMI::Interface::Mapping::Torus::task2torus()
        ///
        /// \todo Error path
        ///
        inline xmi_result_t task2torus_impl (size_t task, size_t (&addr)[BGQ_TDIMS])
        {
          uint32_t abcdept = _mapcache.torus.task2coords[task].raw;

          addr[0] = (abcdept >> 24) & 0x00000003f; // 'a' coordinate
          addr[1] = (abcdept >> 18) & 0x00000003f; // 'b' coordinate
          addr[2] = (abcdept >> 12) & 0x00000003f; // 'c' coordinate
          addr[3] = (abcdept >>  6) & 0x00000003f; // 'd' coordinate
          addr[4] = (abcdept >>  5) & 0x000000001; // 'e' coordinate

          return XMI_SUCCESS;
        }

        ///
        /// \brief Get the BGQ torus address for a task
        /// \see XMI::Interface::Mapping::Torus::task2torus()
        ///
        /// \todo Error path
        ///
        inline xmi_result_t task2global (size_t task, size_t (&addr)[BGQ_TDIMS + BGQ_LDIMS])
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
        /// \see XMI::Interface::Mapping::Torus::torus2task()
        ///
        /// \todo Error path
        ///
        inline xmi_result_t torus2task_impl (size_t (&addr)[BGQ_TDIMS + BGQ_LDIMS], size_t & task)
        {
          TRACE_ERR((stderr, "Mapping::torus2task_impl({%zd, %zd, %zd, %zd, %zd, %zd, %zd}, ...) >>\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6]));

          size_t aSize = _pers.aSize();
          size_t bSize = _pers.bSize();
          size_t cSize = _pers.cSize();
          size_t dSize = _pers.dSize();
          size_t eSize = _pers.eSize();
          size_t tSize = _pers.pSize();
          size_t pSize = _pers.tSize();

          // Verify that the bgq address is valid.
          if (unlikely((addr[0] >= aSize) ||
                       (addr[1] >= bSize) ||
                       (addr[2] >= cSize) ||
                       (addr[3] >= dSize) ||
                       (addr[4] >= eSize) ||
                       (addr[6] >= tSize) ||
                       (addr[5] >= pSize)))
            {
              return XMI_INVAL;
            }

          // Estimate the task id based on the bgq coordinates.
          size_t hash = ESTIMATED_TASK(addr[0],addr[1],addr[2],addr[3],addr[4],addr[6],addr[5],
					aSize,bSize,cSize,dSize,eSize,tSize,pSize);

          // Verify that the estimated task is mapped.
          if (unlikely(_mapcache.torus.coords2task[hash] == (unsigned) - 1))
            {
              return XMI_ERROR;
            }

          task = _mapcache.torus.coords2task[hash];

          TRACE_ERR((stderr, "Mapping::torus2task_impl({%zd, %zd, %zd, %zd, %zd, %zd, %zd}, %zd) <<\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], task));

          return XMI_SUCCESS;
        };

        ///
        /// \brief Get the BGQ network address for a task
        /// \see XMI::Interface::Mapping::Torus::task2network()
        ///
        /// \todo Error path
        ///
        inline xmi_result_t task2network_impl (xmi_task_t task,
						xmi_coord_t *addr,
						xmi_network type)
        {
          uint32_t abcdept = _mapcache.torus.task2coords[task].raw;
	  addr->network = XMI_N_TORUS_NETWORK;
          addr->u.n_torus.coords[0] = (abcdept >> 24) & 0x00000003f; // 'a' coordinate
          addr->u.n_torus.coords[1] = (abcdept >> 18) & 0x00000003f; // 'b' coordinate
          addr->u.n_torus.coords[2] = (abcdept >> 12) & 0x00000003f; // 'c' coordinate
          addr->u.n_torus.coords[3] = (abcdept >>  6) & 0x00000003f; // 'd' coordinate
          addr->u.n_torus.coords[4] = (abcdept >>  5) & 0x000000001; // 'e' coordinate
	  addr->u.n_torus.coords[5] = (abcdept)       & 0x00000000f; // 'p' coordinate
          addr->u.n_torus.coords[6] = (abcdept >> 30) & 0x000000003; // 't' coordinate
          return XMI_SUCCESS;
        }

        ///
        /// \brief Get the BGQ network address for a task
        /// \see XMI::Interface::Mapping::Torus::network2task()
        ///
        /// \todo Error path
        ///
        inline xmi_result_t network2task_impl (const xmi_coord_t *addr,
						xmi_task_t *task,
						xmi_network *type)
        {
          size_t aSize = _pers.aSize();
          size_t bSize = _pers.bSize();
          size_t cSize = _pers.cSize();
          size_t dSize = _pers.dSize();
          size_t eSize = _pers.eSize();
          size_t tSize = _pers.pSize();
          size_t pSize = _pers.tSize();
          size_t a = addr->u.n_torus.coords[0];
	  size_t b = addr->u.n_torus.coords[1];
	  size_t c = addr->u.n_torus.coords[2];
	  size_t d = addr->u.n_torus.coords[3];
	  size_t e = addr->u.n_torus.coords[4];
	  size_t t = addr->u.n_torus.coords[6];
	  size_t p = addr->u.n_torus.coords[5];
          // Verify that the bgq address is valid.
          // TODO convert to "unlikely if"
          if ((a >= aSize) ||
              (b >= bSize) ||
              (c >= cSize) ||
              (d >= dSize) ||
              (e >= eSize) ||
              (p >= pSize) ||
              (t >= tSize))
            {
              return XMI_INVAL;
            }

          size_t hash = ESTIMATED_TASK(a,b,c,d,e,t,p,
					aSize,bSize,cSize,dSize,eSize,tSize,pSize);

	  if (_mapcache.torus.coords2task[hash] == (uint32_t)-1) {
		return XMI_ERROR;
	  }
	  *task = _mapcache.torus.coords2task[hash];
	  *type = XMI_N_TORUS_NETWORK;
          return XMI_SUCCESS;
        }


        /////////////////////////////////////////////////////////////////////////
        //
        // XMI::Interface::Mapping::Node interface implementation
        //
        /////////////////////////////////////////////////////////////////////////

        /// \see XMI::Interface::Mapping::Node::nodeTasks()
        inline xmi_result_t nodeTasks_impl (size_t global, size_t & tasks)
        {
          TRACE_ERR((stderr, "Mapping::nodeTasks_impl(%zd) >>\n", global));
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// #warning implement this!
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
          TRACE_ERR((stderr, "Mapping::nodeTasks_impl(%zd) <<\n", global));
          return XMI_UNIMPL;
        };

        /// \see XMI::Interface::Mapping::Node::nodePeers()
        inline xmi_result_t nodePeers_impl (size_t & peers)
        {
          //peers = _peers;
		 peers = _mapcache.local_size; //hack
          return XMI_SUCCESS;
        };

        /// \see XMI::Interface::Mapping::Node::isPeer()
        inline bool isPeer_impl (size_t task1, size_t task2)
        {
          uint32_t coord1 = _mapcache.torus.task2coords[task1].raw;
          uint32_t coord2 = _mapcache.torus.task2coords[task2].raw;

          return (bool) ((coord1 & 0x3fffffe0) == (coord2 & 0x3fffffe0));
        }

        /// \see XMI::Interface::Mapping::Node::nodeAddr()
        inline void nodeAddr_impl (Interface::Mapping::nodeaddr_t & address)
        {
          TRACE_ERR((stderr, "Mapping::nodeAddr_impl() >>\n"));
          // Can this just be:  address = _nodeaddr; ???
          address.global = _nodeaddr.global;
          address.local  = _nodeaddr.local;
          TRACE_ERR((stderr, "Mapping::nodeAddr_impl(%zd, %zd) <<\n", address.global, address.local));
        };

        /// \see XMI::Interface::Mapping::Node::task2node()
        inline xmi_result_t task2node_impl (size_t task, Interface::Mapping::nodeaddr_t & address)
        {
          TRACE_ERR((stderr, "Mapping::task2node_impl(%zd) >>\n", task));

          uint32_t coords = _mapcache.torus.task2coords[task].raw;
          TRACE_ERR((stderr, "Mapping::coords(%x) >>\n", coords));

          // global coordinate is just the a,b,c,d,e torus coords.
          address.global = (coords >> 5) & 0x01ffffff;

          // local coordinate is the thread id (t) in the most significant
          // position followed by the core id (p) in the least significant
          // position.
          address.local = (coords & 0x0000000f) |
                          ((coords >> 22) & 0x000000300);

          TRACE_ERR((stderr, "Mapping::task2node_impl(%zd, {%zd, %zd}) <<\n", task, address.global, address.local));
          return XMI_SUCCESS;
        };

        /// \see XMI::Interface::Mapping::Node::node2task()
        inline xmi_result_t node2task_impl (Interface::Mapping::nodeaddr_t address, size_t & task)
        {
          TRACE_ERR((stderr, "Mapping::node2task_impl({%zd, %zd}, ...) >>\n", address.global, address.local));

          size_t aSize = _pers.aSize();
          size_t bSize = _pers.bSize();
          size_t cSize = _pers.cSize();
          size_t dSize = _pers.dSize();
          size_t eSize = _pers.eSize();
          size_t tSize = _pers.pSize();
          size_t pSize = _pers.tSize();

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
          if (unlikely((aCoord >= aSize) ||
                       (bCoord >= bSize) ||
                       (cCoord >= cSize) ||
                       (dCoord >= dSize) ||
                       (eCoord >= eSize) ||
                       (pCoord >= pSize) ||
                       (tCoord >= tSize)))
            {
              return XMI_INVAL;
            }

          // Estimate the task id based on the bgq coordinates.
          size_t hash = ESTIMATED_TASK(aCoord,bCoord,cCoord,dCoord,eCoord,tCoord,pCoord,
					aSize,bSize,cSize,dSize,eSize,tSize,pSize);
          // Verify that the estimated task is mapped.
          if (unlikely(_mapcache.torus.coords2task[hash] == (unsigned) - 1))
            {
              return XMI_ERROR;
            }

          task = _mapcache.torus.coords2task[hash];

          TRACE_ERR((stderr, "Mapping::node2task_impl({%zd, %zd}, %zd) <<\n", address.global, address.local, task));
          return XMI_SUCCESS;
        };

        inline xmi_result_t node2peer_impl (XMI::Interface::Mapping::nodeaddr_t & address, size_t & peer)
        {
          TRACE_ERR((stderr, "Mapping::node2peer_impl({%zd, %zd}, ...) >>\n", address.global, address.local));

          size_t tSize = _pers.pSize();
          size_t pSize = _pers.tSize();

          // local coordinate is the thread id (t) in the most significant
          // position followed by the core id (p) in the least significant
          // position.
          size_t tCoord = address.local >> 4;
          size_t pCoord = address.local & 0x0000000f;

          // Verify that the local node address is valid.
          if (unlikely((pCoord >= pSize) || (tCoord >= tSize)))
            {
              return XMI_INVAL;
            }

          // Estimate the task id based on the bgq coordinates.
          size_t hash = ESTIMATED_TASK(0,0,0,0,0,tCoord,pCoord,
					1,1,1,1,1,tSize,pSize);

          // Verify that the address hash is valid.
          if (unlikely(_mapcache.node.local2peer[hash] == (unsigned) - 1))
            {
              return XMI_ERROR;
            }

          peer = _mapcache.node.local2peer[hash];

          TRACE_ERR((stderr, "Mapping::node2peer_impl({%zd, %zd}, %zd) <<\n", address.global, address.local, peer));
          return XMI_SUCCESS;
        };

        inline size_t a () { return _a; }
        inline size_t b () { return _b; }
        inline size_t c () { return _c; }
        inline size_t d () { return _d; }
        inline size_t e () { return _e; }
        inline size_t p () { return _p; }
        inline size_t t () { return _t; }
    };	// class Mapping
};	// namespace XMI

xmi_result_t XMI::Mapping::init(bgq_mapcache_t &mapcache,
				XMI::BgqPersonality &personality)
{
	_mapcache.torus.task2coords = mapcache.torus.task2coords;
	_mapcache.torus.coords2task = mapcache.torus.coords2task;
	_mapcache.node.local2peer   = mapcache.node.local2peer;
	_mapcache.node.peer2task    = mapcache.node.peer2task;
	_mapcache.size    = mapcache.size;
    _mapcache.local_size    = mapcache.local_size; //hack


	return XMI_SUCCESS;
}
#undef TRACE_ERR
#endif // __components_mapping_bgq_bgqmapping_h__
