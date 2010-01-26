/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file common/bgp/Mapping.h
/// \brief ???
///
#ifndef __common_bgp_Mapping_h__
#define __common_bgp_Mapping_h__

#include "sys/xmi.h"

#include "Platform.h"
#include "common/BaseMappingInterface.h"
#include "common/TorusMappingInterface.h"
#include "common/NodeMappingInterface.h"

#include "common/bgp/BgpPersonality.h"
#include "common/bgp/BgpMapCache.h"

#define XMI_MAPPING_CLASS XMI::Mapping

#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif

#define XMI_BGP_NETWORK_DIMS	3
#define XMI_BGP_LOCAL_DIMS	1

/// \brief how to get the global part of an estimated task
#define ESTIMATED_TASK_GLOBAL(x,y,z,t,xSize,ySize,zSize,tSize)	\
	  ESTIMATED_TASK(x,y,z,0,xSize,ySize,zSize,1)

/// \brief how to get the local part of an estimated task
#define ESTIMATED_TASK_LOCAL(x,y,z,t,xSize,ySize,zSize,tSize)	\
	  (t)

/// \brief how to get the estimated task back from global+local
///
/// This is closely tied to ESTIMATED_TASK_GLOBAL and ESTIMATED_TASK_LOCAL!
#define ESTIMATED_TASK_NODE(global,local,xSize,ySize,zSize,tSize)	\
	  ((local * xSize * ySize * zSize) + global)

namespace XMI
{
    class Mapping : public Interface::Mapping::Base<Mapping>,
                       public Interface::Mapping::Torus<Mapping,XMI_BGP_NETWORK_DIMS>,
                       public Interface::Mapping::Node<Mapping,XMI_BGP_LOCAL_DIMS>
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
        inline Mapping (XMI::BgpPersonality &pers) :
            Interface::Mapping::Base<Mapping>(),
            Interface::Mapping::Torus<Mapping,XMI_BGP_NETWORK_DIMS>(),
            Interface::Mapping::Node<Mapping,XMI_BGP_LOCAL_DIMS> (),
	    _personality(pers),
            _task (0),
            _size (0),
            _nodes (0),
            _peers (0),
            _x (_personality.xCoord()),
            _y (_personality.yCoord()),
            _z (_personality.zCoord()),
            _t (_personality.tCoord()),
            _mapcache (NULL),
            _rankcache (NULL)
        {
        };

        inline ~Mapping () {};

      protected:
	XMI::BgpPersonality &_personality;
        xmi_task_t _task;
        size_t _size;
        size_t _nodes;
        size_t _peers;
        size_t _x;
        size_t _y;
        size_t _z;
        size_t _t;

        XMI::Interface::Mapping::nodeaddr_t _nodeaddr;

        size_t * _mapcache;
        size_t * _rankcache;
        size_t   _peercache[NUM_CORES];
        unsigned _localranks[NUM_CORES];

      public:

        /////////////////////////////////////////////////////////////////////////
        //
        // XMI::Interface::Mapping::Base interface implementation
        //
        /////////////////////////////////////////////////////////////////////////

        ///
        /// \brief Initialize the mapping
        ///
	inline xmi_result_t init(XMI::BgpMapCache &mapcache,
				XMI::BgpPersonality &personality);

        ///
        /// \brief Return the BGP global task for this process
        /// \see XMI::Interface::Mapping::Base::task()
        ///
        inline xmi_task_t task_impl()
        {
          return _task;
        }

        ///
        /// \brief Returns the number of global tasks
        /// \see XMI::Interface::Mapping::Base::size()
        ///
        inline size_t size_impl()
        {
          return _size;
        }
#if 0
        ///
        /// \brief Number of physical active nodes in the partition.
        /// \see XMI::Interface::Mapping::Base::numActiveNodesGlobal()
        ///
        inline size_t numActiveNodesGlobal_impl ()
        {
          return _nodes;
        }

        ///
        /// \brief Number of physical active tasks in the partition.
        /// \see XMI::Interface::Mapping::Base::numActiveRanksGlobal()
        ///
        inline size_t numActiveTasksGlobal_impl ()
        {
          return _size;
        }
#endif
        ///
        /// \brief Determines if two global tasks are located on the same physical node.
        /// \see XMI::Interface::Mapping::Base::isPeer()
        ///
        inline bool isPeer_impl (size_t task1, size_t task2)
        {
          unsigned xyzt1 = _mapcache[task1];
          unsigned xyzt2 = _mapcache[task2];
          return ((xyzt1 >> 8) == (xyzt2 >> 8));
        }
        inline xmi_result_t network2task_impl (const xmi_coord_t  * addr,
                                              xmi_task_t                 * task,
                                              xmi_network               * type)
        {
		size_t xSize = _personality.xSize();
		size_t ySize = _personality.ySize();
		size_t zSize = _personality.zSize();
		size_t tSize = _personality.tSize();
		if (addr->network != XMI_DEFAULT_NETWORK &&
			addr->network != XMI_N_TORUS_NETWORK) {
			return XMI_INVAL;
		}
		size_t x = addr->u.n_torus.coords[0];
		size_t y = addr->u.n_torus.coords[1];
		size_t z = addr->u.n_torus.coords[2];
		size_t t = addr->u.n_torus.coords[3];

		if ((x >= xSize) || (y >= ySize) ||
			  (z >= zSize) || (t >= tSize)) {
			return XMI_INVAL;
		}


		size_t estimated_task = ESTIMATED_TASK(x,y,z,t,xSize,ySize,zSize,tSize);
                
		// convert to 'unlikely_if'
		if (_rankcache [estimated_task] == (unsigned)-1) {
			return XMI_ERROR;
		}

		*type = XMI_N_TORUS_NETWORK;
		*task = _rankcache [estimated_task];
		return XMI_SUCCESS;
        }

        inline xmi_result_t task2network_impl (xmi_task_t            task,
                                               xmi_coord_t * addr,
                                               xmi_network          type)
        {
          unsigned xyzt = _mapcache[task];
          addr->network = XMI_N_TORUS_NETWORK;
          addr->u.n_torus.coords[0] = (xyzt >> 24) & 0x0ff;
          addr->u.n_torus.coords[1] = (xyzt >> 16) & 0x0ff;
          addr->u.n_torus.coords[2] = (xyzt >> 8) & 0x0ff;
          addr->u.n_torus.coords[3] = (xyzt >> 0) & 0x0ff;
          return XMI_SUCCESS;
        }
        /////////////////////////////////////////////////////////////////////////
        //
        // XMI::Interface::Mapping::Torus interface implementation
        //
        /////////////////////////////////////////////////////////////////////////

        ///
        /// \brief Return the BGP torus x coordinate (dimension 0) for this task
        ///
        /// \see XMI::Interface::Mapping::Torus::torusCoord()
        ///
        inline size_t x ()
        {
          return _personality.xCoord();
        }
        inline size_t y ()
        {
          return _personality.yCoord();
        }
        inline size_t z ()
        {
          return _personality.zCoord();
        }
        inline size_t t ()
        {
          return _personality.tCoord();
        }

        inline size_t xSize ()
        {
          return _personality.xSize();
        }

        inline size_t ySize ()
        {
          return _personality.ySize();
        }

        inline size_t zSize ()
        {
          return _personality.zSize();
        }

        inline size_t tSize ()
        {
          return _personality.tSize();
        }


#if 0

        ///
        /// \brief Return the size of the BGP torus x dimension
        ///
        /// \see XMI::Interface::Mapping::Torus::torusSize()
        ///
        template <>
        inline size_t torusCoord_impl<0> () const
        {
          return _personality.xCoord();
        }

        ///
        /// \brief Return the BGP torus y coordinate (dimension 1) for this task
        ///
        /// \see XMI::Interface::Mapping::Torus::torusCoord()
        ///
        template <>
        inline size_t torusCoord_impl<1> () const
        {
          return _personality.yCoord();
        }

        ///
        /// \brief Return the BGP torus z coordinate (dimension 2) for this task
        ///
        /// \see XMI::Interface::Mapping::Torus::torusCoord()
        ///
        template <>
        inline size_t torusCoord_impl<2> () const
        {
          return _personality.zCoord();
        }

        ///
        /// \brief Return the BGP torus t coordinate (dimension 3) for this task
        ///
        /// \see XMI::Interface::Mapping::Torus::torusCoord()
        ///
        template <>
        inline size_t torusCoord_impl<3> () const
        {
          return _personality.tCoord();
        }

        ///
        /// \brief Return the size of the BGP torus x dimension
        ///
        /// \see XMI::Interface::Mapping::Torus::torusSize()
        ///
        template <>
        inline size_t torusSize_impl<0> () const
        {
          return _personality.xSize();
        }

        ///
        /// \brief Return the size of the BGP torus y dimension
        ///
        /// \see XMI::Interface::Mapping::Torus::torusSize()
        ///
        template <>
        inline size_t torusSize_impl<1> () const
        {
          return _personality.ySize();
        }

        ///
        /// \brief Return the size of the BGP torus z dimension
        ///
        /// \see XMI::Interface::Mapping::Torus::torusSize()
        ///
        template <>
        inline size_t torusSize_impl<2> () const
        {
          return _personality.zSize();
        }

        ///
        /// \brief Return the size of the BGP torus t dimension
        /// \see XMI::Interface::Mapping::Torus::torusSize()
        ///
        template <>
        inline size_t torusSize_impl<3> () const
        {
          return _personality.tSize();
        }
#endif
        ///
        /// \brief Get the number of BGP torus dimensions
        /// \see XMI::Interface::Mapping::Torus::torusDims()
        ///
        inline const size_t torusDims_impl() const
        {
          return XMI_BGP_NETWORK_DIMS;
        }
        ///
        /// \brief Get the number of BGP torus dimensions
        /// \see XMI::Interface::Mapping::Torus::torusDims()
        ///
        inline const size_t globalDims_impl() const
        {
          return XMI_BGP_NETWORK_DIMS + XMI_BGP_LOCAL_DIMS;
        }
        ///
        /// \brief Get the BGP torus address for this task
        /// \see XMI::Interface::Mapping::Torus::torusAddr()
        ///
        //template <>
        inline void torusAddr_impl (size_t (&addr)[XMI_BGP_NETWORK_DIMS + XMI_BGP_LOCAL_DIMS])
        {
          addr[0] = _x;
          addr[1] = _y;
          addr[2] = _z;
          addr[3] = _t;
        }

        ///
        /// \brief Get the BGP torus address for a task
        /// \see XMI::Interface::Mapping::Torus::task2torus()
        ///
        /// \todo Error path
        ///
        inline xmi_result_t task2torus_impl (size_t task, size_t (&addr)[XMI_BGP_NETWORK_DIMS + XMI_BGP_LOCAL_DIMS])
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
        /// \see XMI::Interface::Mapping::Torus::torus2task()
        ///
        /// \todo Error path
        ///
        inline xmi_result_t torus2task_impl (size_t (&addr)[XMI_BGP_NETWORK_DIMS + XMI_BGP_LOCAL_DIMS], size_t & task)
        {
          size_t xSize = _personality.xSize();
          size_t ySize = _personality.ySize();
          size_t zSize = _personality.zSize();
          size_t tSize = _personality.tSize();

          if ((addr[0] >= xSize) ||
              (addr[1] >= ySize) ||
              (addr[2] >= zSize) ||
              (addr[3] >= tSize))
            {
              return XMI_INVAL;
            }

          size_t estimated_task = ESTIMATED_TASK(addr[0], addr[1], addr[2], addr[3], xSize, ySize, zSize, tSize);

          if (unlikely(_rankcache [estimated_task] == (unsigned) - 1))
            {
              return XMI_ERROR;
            }

          task = _rankcache [estimated_task];
          return XMI_SUCCESS;
        }


        /////////////////////////////////////////////////////////////////////////
        //
        // XMI::Interface::Mapping::Node interface implementation
        //
        /////////////////////////////////////////////////////////////////////////

        /// \brief Get the number of possible tasks on a node
        /// \return Dimension size
        inline xmi_result_t nodeTasks_impl (size_t global, size_t & tasks)
        {
          TRACE_ERR((stderr,"Mapping::nodeTasks_impl(%zd) >>\n", global));
/// \todo #warning implement this!
// If this were urgent, it would have been done in the 75 business
// days since the warning was added.
          TRACE_ERR((stderr,"Mapping::nodeTasks_impl(%zd) <<\n", global));
          return XMI_UNIMPL;
        }

        ///
        /// \brief Number of active tasks on the local node.
        /// \see XMI::Interface::Mapping::Node::nodePeers()
        ///
        inline xmi_result_t nodePeers_impl (size_t & peers)
        {
          peers = _peers;
          return XMI_SUCCESS;
        }

        /// \brief Get the node address for the local task
        inline void nodeAddr_impl (Interface::Mapping::nodeaddr_t & addr)
        {
          TRACE_ERR((stderr,"Mapping::nodeAddr_impl() >>\n"));
          addr = _nodeaddr;
          //global = _nodeaddr.global;
          //local  = _nodeaddr.local;
          TRACE_ERR((stderr,"Mapping::nodeAddr_impl(%zd, %zd) <<\n", addr.global, addr.local));
        }

        /// \brief Get the node address for a specific task
        inline xmi_result_t task2node_impl (size_t task, Interface::Mapping::nodeaddr_t & addr)
        {
          TRACE_ERR((stderr,"Mapping::task2node_impl(%zd) >>\n", task));
          //fprintf(stderr, "Mapping::task2node_impl() .. _mapcache[%zd] = 0x%08x\n", task, _mapcache[task]);
          //fprintf(stderr, "Mapping::task2node_impl() .. _mapcache[%zd] = 0x%08x &_mapcache[%zd] = %p\n", task, _mapcache[task], task, &_mapcache[task]);
	  size_t coords[XMI_BGP_NETWORK_DIMS + XMI_BGP_LOCAL_DIMS];
	  task2torus_impl(task, coords);
	  addr.global = ESTIMATED_TASK_GLOBAL(coords[0],coords[1],coords[2],coords[3],xSize(),ySize(),zSize(),tSize());
	  addr.local = ESTIMATED_TASK_LOCAL(coords[0],coords[1],coords[2],coords[3],xSize(),ySize(),zSize(),tSize());
          TRACE_ERR((stderr,"Mapping::task2node_impl(%zd, %zd, %zd) <<\n", task, addr.global, addr.local));
          return XMI_SUCCESS;
        }

        /// \brief Get the task associated with a specific node address
        inline xmi_result_t node2task_impl (Interface::Mapping::nodeaddr_t addr, size_t & task)
        {
          TRACE_ERR((stderr,"Mapping::node2task_impl(%zd, %zd) >>\n", addr.global, addr.local));
          size_t estimated_task = ESTIMATED_TASK_NODE(addr.global,addr.local,xSize(),ySize(),zSize(),tSize());
          task = _rankcache [estimated_task];
          TRACE_ERR((stderr,"Mapping::node2task_impl(%zd, %zd, %zd) <<\n", addr.global, addr.local, task));
          return XMI_SUCCESS;
        }

        /// \brief Get the peer identifier associated with a specific node address
        inline xmi_result_t node2peer_impl (Interface::Mapping::nodeaddr_t & addr, size_t & peer)
        {
          TRACE_ERR((stderr,"Mapping::node2peer_impl(%zd, %zd) >>\n", addr.global, addr.local));

          peer = _peercache[addr.local];

          TRACE_ERR((stderr,"Mapping::node2peer_impl(%zd, %zd, %zd) <<\n", addr.global, addr.local, peer));
          return XMI_SUCCESS;
        }
    };	// class Mapping
};	// namespace XMI

xmi_result_t XMI::Mapping::init(XMI::BgpMapCache &mapcache,
				XMI::BgpPersonality &personality)
{
  //fprintf (stderr, "Mapping::init_impl >>\n");
  _personality = personality;
  _mapcache  = mapcache.getMapCache();
  _rankcache = mapcache.getRankCache();
  _task = mapcache.getTask();
  _size = mapcache.getSize();

  unsigned i;

  for (i = 0; i < _size; i++) task2node (i, _nodeaddr);

  task2node (_task, _nodeaddr);

  size_t peer = 0;
  xmi_task_t task;
  xmi_network dummy;
  xmi_coord_t c;
  c.network = XMI_N_TORUS_NETWORK;
  c.u.n_torus.coords[0] = _x;
  c.u.n_torus.coords[1] = _y;
  c.u.n_torus.coords[2] = _z;
  _peers = 0;

  for (c.u.n_torus.coords[3]=0; c.u.n_torus.coords[3]<_personality.tSize(); c.u.n_torus.coords[3]++)
  {
    if (network2task(&c, &task, &dummy) == XMI_SUCCESS)
    {
          //fprintf (stderr, "BgpMapping::init_impl .. _peercache[%zd] = %zd\n", c.n_torus.coords[3], peer);
          _peercache[c.u.n_torus.coords[3]] = peer++;
          _localranks[_peers] = task;
          _peers++;
    }
  }
  //fprintf (stderr, "Mapping::init_impl <<\n");

  return XMI_SUCCESS;
};
#undef TRACE_ERR
#endif // __components_mapping_bgp_bgpmapping_h__
