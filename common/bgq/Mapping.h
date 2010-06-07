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

#include <pami.h>
#include "Platform.h"

#include "common/BaseMappingInterface.h"
#include "common/TorusMappingInterface.h"
#include "common/NodeMappingInterface.h"

#include "common/bgq/BgqPersonality.h"
#include "common/bgq/BgqMapCache.h"

#include "util/common.h"

#define PAMI_MAPPING_CLASS PAMI::Mapping

#undef TRACE_ERR
#ifndef TRACE_ERR
  #define TRACE_ERR(x) //fprintf x
#endif

namespace PAMI
{
#define BGQ_TDIMS 5
#define BGQ_LDIMS 1
  class Mapping :
  public Interface::Mapping::Base<Mapping>,
  public Interface::Mapping::Torus<Mapping, BGQ_TDIMS>,
  public Interface::Mapping::Node<Mapping,BGQ_LDIMS>
  {
  public:

    inline Mapping (PAMI::BgqPersonality &pers) :
    Interface::Mapping::Base<Mapping>(),
    Interface::Mapping::Torus<Mapping, BGQ_TDIMS>(),
    Interface::Mapping::Node<Mapping,BGQ_LDIMS> (),
    _pers(pers),
    _task((size_t)-1),
    _a (_pers.aCoord()),
    _b (_pers.bCoord()),
    _c (_pers.cCoord()),
    _d (_pers.dCoord()),
    _e (_pers.eCoord()),
    _t (_pers.tCoord())
    {
      // ----------------------------------------------------------------
      // Compile-time assertions
      // ----------------------------------------------------------------

      // Make sure that the coords structure and the MUHWI_Destination_t
      // structure match
      COMPILE_TIME_ASSERT(sizeof(bgq_coords_t) == sizeof(MUHWI_Destination_t));

      _coords.mapped.t        = _t;
      _coords.mapped.a        = _a;
      _coords.mapped.b        = _b;
      _coords.mapped.c        = _c;
      _coords.mapped.d        = _d;
      _coords.mapped.e        = _e;
      _coords.mapped.reserved =  0;

      coord2node (_a, _b, _c, _d, _e, _t,      //fix?
                  _nodeaddr.global, _nodeaddr.local);
      TRACE_MAMBO((stderr, "Mapping() coords(a,b,c,d,e,t):(%zu %zu %zu %zu %zu %zu), node: (%#lX %#lX)\n", _a, _b, _c, _d, _e, _t, _nodeaddr.global, _nodeaddr.local));

    };

    inline ~Mapping ()
    {
    };

  protected:
    PAMI::BgqPersonality &_pers;
    size_t _task;
//    size_t _nodes;
//    size_t _peers;

    size_t _a;
    size_t _b;
    size_t _c;
    size_t _d;
    size_t _e;
    size_t _t;

    bgq_coords_t  _coords;
    Interface::Mapping::nodeaddr_t _nodeaddr;

    bgq_mapcache_t _mapcache;


    inline void coord2node (size_t   a,
                            size_t   b,
                            size_t   c,
                            size_t   d,
                            size_t   e,
                            size_t   t,
                            size_t & global,
                            size_t & local)
    {
      global = e | (d << 1) | (c << 7) | (b << 13) | (a << 19);
      local  = t;
      TRACE_ERR((stderr, "Mapping() coords2node((%zu %zu %zu %zu %zu %zu),(%#lX %#lX)\n", a, b, c, d, e, t, global, local));
    };

    inline void node2coord (size_t & a,
                            size_t & b,
                            size_t & c,
                            size_t & d,
                            size_t & e,
                            size_t & t,
                            size_t   global,
                            size_t   local)
    {
      e = global         & 0x00000001;
      d = (global >> 1)  & 0x0000003f;
      c = (global >> 7)  & 0x0000003f;
      b = (global >> 13) & 0x0000003f;
      a = global >> 19;

      t = local;

      TRACE_ERR((stderr, "Mapping() node2coord((%zu %zu %zu %zu %zu %zu),(%#lX %#lX)\n", a, b, c, d, e, t, global, local));

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
      return(MUHWI_Destination_t *) &_coords;
    };

    ///
    /// \brief Retrieve mu destination structure for a specific task
    ///
    /// The intent here is to provide access to a mu destination structure
    /// initialized to the coordinates of the destination node.
    ///
    /// \note Does not provide process, core, or hardware thread addressing.
    ///
    inline void getMuDestinationTask (size_t task, MUHWI_Destination_t &dest, size_t &tcoord)
    {
      uint32_t raw      = _mapcache.torus.task2coords[task].raw;
      tcoord            = (size_t) raw & 0x0000003f; // 't' coordinate
      *(uint32_t*)&dest = (raw & 0xffffffc0);
    };

    /////////////////////////////////////////////////////////////////////////
    //
    // PAMI::Interface::Mapping::Base interface implementation
    //
    /////////////////////////////////////////////////////////////////////////

    ///
    /// \brief Initialize the mapping
    /// \see PAMI::Interface::Mapping::Base::init()
    ///
    inline pami_result_t init(bgq_mapcache_t &mapcache,
                             PAMI::BgqPersonality &personality);

    ///
    /// \brief Return the BGP global task for this process
    /// \see PAMI::Interface::Mapping::Base::task()
    ///
    inline size_t task_impl()
    {
      return _task;
    }

    ///
    /// \brief Returns the number of global tasks
    /// \see PAMI::Interface::Mapping::Base::size()
    ///
    inline size_t size_impl()
    {
      return _mapcache.size;
    }

    ///
    /// \brief Returns the number of global dimensions
    /// \see PAMI::Interface::Mapping::Base::globalDims()
    ///
    inline size_t globalDims_impl()
    {
      return BGQ_TDIMS + BGQ_LDIMS;
    }

    /////////////////////////////////////////////////////////////////////////
    //
    // PAMI::Interface::Mapping::Torus interface implementation
    //
    /////////////////////////////////////////////////////////////////////////

    ///
    /// \brief Get the specified torus dimension for this task
    /// \see PAMI::Interface::Mapping::Torus::torusgetcoord()
    ///
    inline size_t       torusgetcoord_impl (size_t dimension)
    {
      if(dimension >= BGQ_TDIMS + BGQ_LDIMS)
        PAMI_abortf("dimension %zu requested\n",dimension);
      size_t addr[BGQ_TDIMS + BGQ_LDIMS];
      torusAddr_impl(addr);
      return addr[dimension];
    }
    ///
    /// \brief Get the BGQ torus address for this task
    /// \see PAMI::Interface::Mapping::Torus::torusAddr()
    ///
    //template <>
    inline void torusAddr_impl (size_t (&addr)[BGQ_TDIMS + BGQ_LDIMS])
    {
      addr[0] = _a;
      addr[1] = _b;
      addr[2] = _c;
      addr[3] = _d;
      addr[4] = _e;
      addr[5] = _t;
      TRACE_ERR((stderr, "Mapping::torusAddr_impl(%zu, %zu, %zu, %zu, %zu, %zu}\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]));
    }

    ///
    /// \brief Get the BGQ torus address for a task
    /// \see PAMI::Interface::Mapping::Torus::task2torus()
    ///
    /// \todo Error path
    ///
    inline pami_result_t task2torus_impl (size_t task, size_t (&addr)[BGQ_TDIMS])
    {
      uint32_t abcdet = _mapcache.torus.task2coords[task].raw;

      addr[0] = (abcdet >> 24) & 0x00000003f; // 'a' coordinate
      addr[1] = (abcdet >> 18) & 0x00000003f; // 'b' coordinate
      addr[2] = (abcdet >> 12) & 0x00000003f; // 'c' coordinate
      addr[3] = (abcdet >>  6) & 0x00000003f; // 'd' coordinate
      addr[4] = (abcdet >> 31); // 'e' coordinate

      TRACE_ERR((stderr, "Mapping::task2torus(%zu, {%zu, %zu, %zu, %zu, %zu}) <<\n", task, addr[0], addr[1], addr[2], addr[3], addr[4]));
      return PAMI_SUCCESS;
    }

    ///
    /// \brief Get the BGQ torus address for a task
    /// \see PAMI::Interface::Mapping::Torus::task2torus()
    ///
    /// \todo Error path
    ///
    inline pami_result_t task2global (size_t task, size_t (&addr)[BGQ_TDIMS + BGQ_LDIMS])
    {
      uint32_t abcdept = _mapcache.torus.task2coords[task].raw;

      addr[0] = (abcdept >> 24) & 0x00000003f; // 'a' coordinate
      addr[1] = (abcdept >> 18) & 0x00000003f; // 'b' coordinate
      addr[2] = (abcdept >> 12) & 0x00000003f; // 'c' coordinate
      addr[3] = (abcdept >>  6) & 0x00000003f; // 'd' coordinate
      addr[4] = (abcdept >> 31)              ; // 'e' coordinate
      addr[5] = (abcdept)       & 0x00000003f; // 't' coordinate

      TRACE_ERR((stderr, "Mapping::task2global(%zu, {%zu, %zu, %zu, %zu, %zu, %zu}) <<\n", task, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]));
      return PAMI_SUCCESS;
    }

    ///
    /// \brief Get the global task for a BGQ torus address
    /// \see PAMI::Interface::Mapping::Torus::torus2task()
    ///
    /// \todo Error path
    ///
    /// \todo why is there task2global but no global2task?  stub it in for Rectangle.h
    inline pami_result_t global2task (size_t (&addr)[BGQ_TDIMS + BGQ_LDIMS], size_t & task)
    {
      return torus2task_impl (addr,task);
    }
    inline pami_result_t torus2task_impl (size_t (&addr)[BGQ_TDIMS + BGQ_LDIMS], size_t & task)
    {

      size_t aSize = _pers.aSize();
      size_t bSize = _pers.bSize();
      size_t cSize = _pers.cSize();
      size_t dSize = _pers.dSize();
      size_t eSize = _pers.eSize();
      size_t tSize = _pers.tSize();

      // Verify that the bgq address is valid.
      if (unlikely((addr[0] >= aSize) ||
                   (addr[1] >= bSize) ||
                   (addr[2] >= cSize) ||
                   (addr[3] >= dSize) ||
                   (addr[4] >= eSize) ||
                   (addr[5] >= tSize)))
      {
        return PAMI_INVAL;
      }

      // Estimate the task id based on the bgq coordinates.
      size_t hash = ESTIMATED_TASK(addr[0],addr[1],addr[2],addr[3],addr[4],addr[5],
                                   aSize,bSize,cSize,dSize,eSize,tSize);

      // Verify that the estimated task is mapped.
      if (unlikely(_mapcache.torus.coords2task[hash] == (unsigned) - 1))
      {
        return PAMI_ERROR;
      }

      task = _mapcache.torus.coords2task[hash];

      TRACE_ERR((stderr, "Mapping::torus2task_impl({%zu, %zu, %zu, %zu, %zu, %zu}, %zu) <<\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], task));

      return PAMI_SUCCESS;
    };

    ///
    /// \brief Get the BGQ network address for a task
    /// \see PAMI::Interface::Mapping::Torus::task2network()
    ///
    /// \todo Error path
    ///
    inline pami_result_t task2network_impl (pami_task_t task,
                                           pami_coord_t *addr,
                                           pami_network type)
    {
      TRACE_ERR((stderr,"task2network %d\n",task));
      uint32_t abcdet = _mapcache.torus.task2coords[task].raw;
      addr->network = PAMI_N_TORUS_NETWORK;
      addr->u.n_torus.coords[0] = (abcdet >> 24) & 0x00000003f; // 'a' coordinate
      addr->u.n_torus.coords[1] = (abcdet >> 18) & 0x00000003f; // 'b' coordinate
      addr->u.n_torus.coords[2] = (abcdet >> 12) & 0x00000003f; // 'c' coordinate
      addr->u.n_torus.coords[3] = (abcdet >>  6) & 0x00000003f; // 'd' coordinate
      addr->u.n_torus.coords[4] = (abcdet >> 31)              ; // 'e' coordinate
      addr->u.n_torus.coords[5] = (abcdet)       & 0x00000003f; // 't' coordinate
      TRACE_ERR((stderr, "Mapping::task2network_impl(%d, {%zu, %zu, %zu, %zu, %zu, %zu}, %d) <<\n", task, addr->u.n_torus.coords[0], addr->u.n_torus.coords[1], addr->u.n_torus.coords[2], addr->u.n_torus.coords[3], addr->u.n_torus.coords[4], addr->u.n_torus.coords[5], addr->network));
      return PAMI_SUCCESS;
    }

    ///
    /// \brief Get the BGQ network address for a task
    /// \see PAMI::Interface::Mapping::Torus::network2task()
    ///
    /// \todo Error path
    ///
    inline pami_result_t network2task_impl (const pami_coord_t *addr,
                                           pami_task_t *task,
                                           pami_network *type)
    {
      size_t aSize = _pers.aSize();
      size_t bSize = _pers.bSize();
      size_t cSize = _pers.cSize();
      size_t dSize = _pers.dSize();
      size_t eSize = _pers.eSize();
      size_t tSize = _pers.tSize();
      size_t a = addr->u.n_torus.coords[0];
      size_t b = addr->u.n_torus.coords[1];
      size_t c = addr->u.n_torus.coords[2];
      size_t d = addr->u.n_torus.coords[3];
      size_t e = addr->u.n_torus.coords[4];
      size_t t = addr->u.n_torus.coords[6];
      // Verify that the bgq address is valid.
      // TODO convert to "unlikely if"
      if ((a >= aSize) ||
          (b >= bSize) ||
          (c >= cSize) ||
          (d >= dSize) ||
          (e >= eSize) ||
          (t >= tSize))
      {
        return PAMI_INVAL;
      }

      size_t hash = ESTIMATED_TASK(a,b,c,d,e,t,
                                   aSize,bSize,cSize,dSize,eSize,tSize);

      if (_mapcache.torus.coords2task[hash] == (uint32_t)-1)
      {
        return PAMI_ERROR;
      }
      *task = _mapcache.torus.coords2task[hash];
      *type = PAMI_N_TORUS_NETWORK;
      TRACE_ERR((stderr, "Mapping::network2task_impl({%zu, %zu, %zu, %zu, %zu, %zu}, %d, %d) <<\n", addr->u.n_torus.coords[0], addr->u.n_torus.coords[1], addr->u.n_torus.coords[2], addr->u.n_torus.coords[3], addr->u.n_torus.coords[4], addr->u.n_torus.coords[5], *task, addr->network));
      return PAMI_SUCCESS;
    }


    /////////////////////////////////////////////////////////////////////////
    //
    // PAMI::Interface::Mapping::Node interface implementation
    //
    /////////////////////////////////////////////////////////////////////////

    /// \see PAMI::Interface::Mapping::Node::nodeTasks()
    inline pami_result_t nodeTasks_impl (size_t global, size_t & tasks)
    {
      TRACE_ERR((stderr, "Mapping::nodeTasks_impl(%zu) >>\n", global));
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/// \todo #warning implement this!
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
      TRACE_ERR((stderr, "Mapping::nodeTasks_impl(%zu) <<\n", global));
      return PAMI_UNIMPL;
    };

    /// \see PAMI::Interface::Mapping::Node::nodePeers()
    inline pami_result_t nodePeers_impl (size_t & peers)
    {
      peers = _mapcache.local_size;
      return PAMI_SUCCESS;
    };

    /// \see PAMI::Interface::Mapping::Node::isPeer()
    inline bool isPeer_impl (size_t task1, size_t task2)
    {
      uint32_t coord1 = _mapcache.torus.task2coords[task1].raw;
      uint32_t coord2 = _mapcache.torus.task2coords[task2].raw;

      return ((coord1 & 0xbfffffc0) == (coord2 & 0xbfffffc0));
    }

    /// \see PAMI::Interface::Mapping::Node::nodeAddr()
    inline void nodeAddr_impl (Interface::Mapping::nodeaddr_t & address)
    {
      TRACE_ERR((stderr, "Mapping::nodeAddr_impl() >>\n"));
      // Can this just be:  address = _nodeaddr; ???
      address.global = _nodeaddr.global;
      address.local  = _nodeaddr.local;
      TRACE_ERR((stderr, "Mapping::nodeAddr_impl(%zu, %zu) <<\n", address.global, address.local));
    };

    /// \see PAMI::Interface::Mapping::Node::task2node()
    inline pami_result_t task2node_impl (size_t task, Interface::Mapping::nodeaddr_t & address)
    {
      TRACE_ERR((stderr, "Mapping::task2node_impl(%zu) >>\n", task));

      uint32_t coords = _mapcache.torus.task2coords[task].raw;
      TRACE_ERR((stderr, "Mapping::coords(%x) >>\n", coords));

      // global coordinate is just the a,b,c,d,e torus coords.
      address.global = ((coords >> 5) & 0x01ffffff) | (coords >> 31);

      // local coordinate is the thread id (t) in the most significant
      // position followed by the core id (p) in the least significant
      // position.
      address.local = (coords & 0x0000003f);

      TRACE_ERR((stderr, "Mapping::task2node_impl(%zu, {%zu, %zu}) <<\n", task, address.global, address.local));
      return PAMI_SUCCESS;
    };

    /// \see PAMI::Interface::Mapping::Node::node2task()
    inline pami_result_t node2task_impl (Interface::Mapping::nodeaddr_t address, size_t & task)
    {
      TRACE_ERR((stderr, "Mapping::node2task_impl({%zu, %zu}, ...) >>\n", address.global, address.local));

      size_t aSize = _pers.aSize();
      size_t bSize = _pers.bSize();
      size_t cSize = _pers.cSize();
      size_t dSize = _pers.dSize();
      size_t eSize = _pers.eSize();
      size_t tSize = _pers.tSize();

      // global coordinate is just the a,b,c,d,e torus coords.
      size_t aCoord =  address.global >> 19;
      size_t bCoord = (address.global >> 13) & 0x0000003f;
      size_t cCoord = (address.global >>  7) & 0x0000003f;
      size_t dCoord = (address.global >>  1) & 0x0000003f;
      size_t eCoord =  address.global        & 0x00000001;

      // local coordinate is the thread id (t) in the most significant
      // position followed by the core id (p) in the least significant
      // position.
      size_t tCoord = address.local;

      // Verify that the bgq address is valid.
      if (unlikely((aCoord >= aSize) ||
                   (bCoord >= bSize) ||
                   (cCoord >= cSize) ||
                   (dCoord >= dSize) ||
                   (eCoord >= eSize) ||
                   (tCoord >= tSize)))
      {
        return PAMI_INVAL;
      }

      // Estimate the task id based on the bgq coordinates.
      size_t hash = ESTIMATED_TASK(aCoord,bCoord,cCoord,dCoord,eCoord,tCoord,
                                   aSize,bSize,cSize,dSize,eSize,tSize);
      // Verify that the estimated task is mapped.
      if (unlikely(_mapcache.torus.coords2task[hash] == (unsigned)-1))
      {
        return PAMI_ERROR;
      }

      task = _mapcache.torus.coords2task[hash];

      TRACE_ERR((stderr, "Mapping::node2task_impl({%zu, %zu}, %zu) <<\n", address.global, address.local, task));
      return PAMI_SUCCESS;
    };

    inline pami_result_t node2peer_impl (PAMI::Interface::Mapping::nodeaddr_t & address, size_t & peer)
    {
      TRACE_ERR((stderr, "Mapping::node2peer_impl({%zu, %zu}, ...) >>\n", address.global, address.local));

      size_t tSize = _pers.tSize();
      //size_t peerSize = tSize * pSize;

      size_t tCoord = address.local;

      // Verify that the local node address is valid.
      if (unlikely(tCoord >= tSize))
      {
        return PAMI_INVAL;
      }

      // Estimate the task id based on the bgq coordinates.
      size_t hash = ESTIMATED_TASK(0,0,0,0,0,tCoord,
                                   1,1,1,1,1,tSize);

      // Verify that the address hash is valid.
      if (unlikely(_mapcache.node.local2peer[hash] == (unsigned) - 1))
      {
        return PAMI_ERROR;
      }

      peer = _mapcache.node.local2peer[hash];

      TRACE_ERR((stderr, "Mapping::node2peer_impl({%zu, %zu}, %zu) <<\n", address.global, address.local, peer));
      return PAMI_SUCCESS;
    };

    inline size_t a ()
    {
      return _a;
    }
    inline size_t b ()
    {
      return _b;
    }
    inline size_t c ()
    {
      return _c;
    }
    inline size_t d ()
    {
      return _d;
    }
    inline size_t e ()
    {
      return _e;
    }
    inline size_t t ()
    {
      return _t;
    }
  };  // class Mapping
};  // namespace PAMI

pami_result_t PAMI::Mapping::init(bgq_mapcache_t &mapcache,
                                PAMI::BgqPersonality &personality)
{
  _mapcache.torus.task2coords = mapcache.torus.task2coords;
  _mapcache.torus.coords2task = mapcache.torus.coords2task;
  _mapcache.node.local2peer   = mapcache.node.local2peer;
  _mapcache.node.peer2task    = mapcache.node.peer2task;
  _mapcache.size              = mapcache.size;
  _mapcache.local_size        = mapcache.local_size;

  size_t
    //asize =_pers.aSize(),// aSize isn't currently used in the calculation so we get warnings
    bsize =_pers.bSize(),
    csize =_pers.cSize(),
    dsize =_pers.dSize(),
    esize =_pers.eSize(),
    tsize =_pers.tSize();

  TRACE_MAMBO((stderr,"Mapping() size a/b/c/d/e/t = %zu/%zu/%zu/%zu/%zu/%zu\n", _pers.aSize(), bsize, csize, dsize, esize, tsize));

  size_t hash = ESTIMATED_TASK(_a,_b,_c,_d,_e,_t, _pers.aSize(),bsize,csize,dsize,esize,tsize); // asize isn't used but just in case...

  _task = _mapcache.torus.coords2task[hash];

  TRACE_MAMBO((stderr,"Mapping::init() task %zu, estimated task %zu, size %zu, peers %zu\n", _task, hash, _mapcache.size, _mapcache.local_size));

  return PAMI_SUCCESS;
}
#undef TRACE_ERR
#endif // __components_mapping_bgq_bgqmapping_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
