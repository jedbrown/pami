/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file common/lapiunix/Mapping.h
 * \brief ???
 */

#ifndef __common_lapiunix_Mapping_h__
#define __common_lapiunix_Mapping_h__

#include <pami.h>
#include "Platform.h"
#include "common/BaseMappingInterface.h"
#include "common/TorusMappingInterface.h"
#include "common/NodeMappingInterface.h"
#include "util/common.h"

#define PAMI_MAPPING_CLASS PAMI::Mapping

// todo: this should go in a platform specific
// or runtime specific file
// it has been defined in pami_ext.h, but that is the wrong place for it
#ifdef  PAMI_MAX_DIMS
#undef  PAMI_MAX_DIMS
#endif

#define PAMI_MAX_DIMS 2

namespace PAMI
{
#define LAPI_TDIMS 1
#define LAPI_LDIMS 1
  class Mapping : public Interface::Mapping::Base<Mapping>,
                  public Interface::Mapping::Torus<Mapping, LAPI_TDIMS>,
                  public Interface::Mapping::Node<Mapping, LAPI_LDIMS>
  {
  public:
    inline Mapping () :
      Interface::Mapping::Base<Mapping>(),
      Interface::Mapping::Torus<Mapping, LAPI_TDIMS>(),
      Interface::Mapping::Node<Mapping, LAPI_TDIMS>(),
      _mapcache(NULL)
      {
      };
    inline ~Mapping () {};

    inline pami_result_t init(size_t task, size_t size)
      {
        _task = task;
        _size = size;
        return PAMI_SUCCESS;
      }

    inline pami_result_t set_mapcache(uint32_t *mapcache,
                                      size_t   *peers,
                                      size_t    npeers)
      {
        _mapcache = mapcache;
        _peers    = peers;
        _npeers   = npeers;
      }

    inline size_t task_impl()
      {
        return _task;
      }
    inline size_t size_impl()
      {
        return _size;
      }
    inline pami_result_t nodeTasks_impl (size_t global, size_t & tasks)
      {
        PAMI_abort();
        return PAMI_UNIMPL;
      }
    inline pami_result_t nodePeers_impl (size_t & peers)
      {
        peers = _npeers;
        return PAMI_SUCCESS;
      }
    inline bool isPeer_impl (size_t task1, size_t task2)
      {
        /** \todo isPeer does not support PAMI_MAPPING_TSIZE */
        // this really needs to be global, but if we only check locally it
        // will at least work for Topology.
        size_t peer;
        Interface::Mapping::nodeaddr_t addr;
        task2node_impl(task1, addr);
        pami_result_t err1 = node2peer_impl(addr, peer);
        task2node_impl(task2, addr);
        pami_result_t err2 = node2peer_impl(addr, peer);
        return (err1 == PAMI_SUCCESS && err2 == PAMI_SUCCESS);
      }
    inline void nodeAddr_impl (Interface::Mapping::nodeaddr_t & address)
      {
        pami_result_t err = task2node_impl(_task, address);
        PAMI_assertf(err == PAMI_SUCCESS, "Internal error, my task does not exist");
      }
    inline pami_result_t task2node_impl (size_t task, Interface::Mapping::nodeaddr_t & address)
      {
        uint32_t x = _mapcache[task];
        address.global = x >> 16;
        address.local = x & 0x0000ffff;
        return PAMI_SUCCESS;
      }
    inline pami_result_t node2task_impl (Interface::Mapping::nodeaddr_t & address, size_t & task)
      {
        uint32_t x = (address.global << 16) | address.local;
        // since we expect this to be small, searching is probably the easiest way
        size_t r;
        for (r = 0; r < _size && _mapcache[r] != x; ++r);
        if (r < _size) {
          task = r;
          return PAMI_SUCCESS;
        }
        // never happens?
        return PAMI_ERROR;
      }
    inline pami_result_t node2peer_impl (Interface::Mapping::nodeaddr_t & address, size_t & peer)
      {
        size_t x, r = address.local;
        for (x = 0; x < _npeers && r != _peers[x]; ++x);
        if (x < _npeers) {
          peer = x;
          return PAMI_SUCCESS;
        }
        return PAMI_ERROR;
      }
    inline void torusAddr_impl (size_t (&addr)[LAPI_TDIMS])
      {
        task2torus_impl(_task, addr);
      }
    inline pami_result_t task2torus_impl (size_t task, size_t (&addr)[LAPI_TDIMS])
      {
        if (task >= _size) {
          return PAMI_ERROR;
        }
        uint32_t x = _mapcache[task];
        addr[0] = x >> 16;
        return PAMI_SUCCESS;
      }

    inline pami_result_t task2global (size_t task, size_t (&addr)[LAPI_TDIMS+LAPI_LDIMS])
      {
        if (task >= _size) {
          return PAMI_ERROR;
        }
        uint32_t x = _mapcache[task];
        addr[0] = x >> 16;
        addr[1] = x & 0x0000ffff;
        return PAMI_SUCCESS;
      }

    inline pami_result_t torus2task_impl (size_t (&addr)[LAPI_TDIMS], size_t & task)
      {
        PAMI_abort();
        return PAMI_ERROR;
      }

    inline pami_result_t global2task (size_t (&addr)[LAPI_TDIMS+LAPI_LDIMS], size_t & task)
      {
        uint32_t x = (addr[0] << 16) | addr[1];
        // since we expect this to be small, searching is probably the easiest way
        size_t r;
        for (r = 0; r < _size && _mapcache[r] != x; ++r);
        if (r < _size) {
          task = r;
          return PAMI_SUCCESS;
        }
        // never happens?
        return PAMI_ERROR;
      }
    inline size_t       torusgetcoord_impl (size_t dimension)
      {
        if(dimension >= LAPI_TDIMS + LAPI_LDIMS)
          abort();
        return _task;
      }
    inline size_t globalDims()
      {
        return LAPI_TDIMS + LAPI_LDIMS;
      }

    inline pami_result_t task2network (pami_task_t task, pami_coord_t *addr, pami_network type)
      {
        if (task >= _size ||
            (type != PAMI_N_TORUS_NETWORK && type != PAMI_DEFAULT_NETWORK)) {
          return PAMI_ERROR;
        }
        uint32_t x = _mapcache[task];
        addr->u.n_torus.coords[0] = x >> 16;
        addr->u.n_torus.coords[1] = x & 0x0000ffff;
        addr->network = PAMI_N_TORUS_NETWORK;
        return PAMI_SUCCESS;
      }
    inline pami_result_t network2task_impl(const pami_coord_t *addr,
                                           pami_task_t        *task,
                                           pami_network       *type)
      {
        if (addr->network != PAMI_N_TORUS_NETWORK) {
          return PAMI_ERROR;
        }

        uint32_t x = (addr->u.n_torus.coords[0] << 16) | addr->u.n_torus.coords[1];
        // since we expect this to be small, searching is probably the easiest way
        size_t r;
        for (r = 0; r < _size && _mapcache[r] != x; ++r);
        if (r < _size) {
          *type = PAMI_N_TORUS_NETWORK;
          *task = r;
          return PAMI_SUCCESS;
        }
        // never happens?
        return PAMI_ERROR;
      }

  private:
    uint32_t *_mapcache;
    size_t   *_peers;
    size_t    _task;
    size_t    _size;
    size_t    _npeers;

  }; // class Mapping
};	// namespace PAMI
#endif // __components_mapping_lapi_lapimapping_h__
