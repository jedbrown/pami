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
#include "lapifunc.h"

#define PAMI_MAPPING_CLASS PAMI::Mapping

namespace PAMI
{
#define LAPI_DIMS 1
#define LAPI_LDIMS 1
    class Mapping : public Interface::Mapping::Base<Mapping>,
                       public Interface::Mapping::Torus<Mapping, LAPI_DIMS>,
                       public Interface::Mapping::Node<Mapping, LAPI_DIMS>
    {

    public:
      inline Mapping () :
        Interface::Mapping::Base<Mapping>(),
        Interface::Mapping::Torus<Mapping, LAPI_DIMS>(),
        Interface::Mapping::Node<Mapping, LAPI_DIMS>()
        {
//          LAPI_Comm_rank(LAPI_COMM_WORLD, (int*)&_task);
//          LAPI_Comm_size(LAPI_COMM_WORLD, (int*)&_size);
        };
      inline ~Mapping () {};
    protected:
      size_t    _task;
      size_t    _size;
    public:
      inline pami_result_t init(size_t task, size_t size)
        {
          _task = task;
          _size = size;
          return PAMI_SUCCESS;
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
          assert(0);
          return PAMI_UNIMPL;
        }
      inline pami_result_t nodePeers_impl (size_t & peers)
        {
          assert(0);
          return PAMI_UNIMPL;
        }
      inline bool isPeer_impl (size_t task1, size_t task2)
        {
          assert(0);
          return PAMI_UNIMPL;
        }
      inline void nodeAddr_impl (Interface::Mapping::nodeaddr_t & address)
        {
          assert(0);
        }
      inline pami_result_t task2node_impl (size_t task, Interface::Mapping::nodeaddr_t & address)
        {
          address.global=task;
          address.local =0;
          return PAMI_SUCCESS;
        }
      inline pami_result_t node2task_impl (Interface::Mapping::nodeaddr_t & address, size_t & task)
        {
          task = address.global;
          return PAMI_SUCCESS;
        }
      inline pami_result_t node2peer_impl (Interface::Mapping::nodeaddr_t & address, size_t & peer)
        {
          assert(0);
          return PAMI_UNIMPL;
        }
      inline void torusAddr_impl (size_t (&addr)[LAPI_DIMS])
        {
          addr[0]=_task;
        }
      inline pami_result_t task2torus_impl (size_t task, size_t (&addr)[LAPI_DIMS])
        {
          addr[0] = task;
          return PAMI_SUCCESS;
        }

      inline pami_result_t torus2task_impl (size_t (&addr)[LAPI_DIMS], size_t & task)
        {
          task = addr[0];
          return PAMI_SUCCESS;
        }

      inline pami_result_t task2global (size_t task, size_t (&addr)[LAPI_DIMS+LAPI_LDIMS])
        {
          addr[0] = task;
          addr[1] = 0;
          return PAMI_SUCCESS;
        }
      inline pami_result_t global2task (size_t (&addr)[LAPI_DIMS+LAPI_LDIMS], size_t & task)
        {
          task = addr[0];
          return PAMI_SUCCESS;
        }
      inline size_t       torusgetcoord_impl (size_t dimension)
        {
          if(dimension >= LAPI_DIMS)
            abort();
          return _task;
        }
      inline size_t globalDims()
        {
          return LAPI_DIMS;
        }

      inline pami_result_t task2network (pami_task_t task, pami_coord_t *addr, pami_network type)
        {
          if (task >= _size ||
              (type != PAMI_N_TORUS_NETWORK && type != PAMI_DEFAULT_NETWORK))
              {
                return PAMI_ERROR;
              }
          addr->u.n_torus.coords[0] = task;
          addr->u.n_torus.coords[1] = 0;
          addr->network             = PAMI_N_TORUS_NETWORK;
          return PAMI_SUCCESS;
        }
      inline pami_result_t network2task_impl(const pami_coord_t *addr,
                                            pami_task_t        *task,
                                            pami_network       *type)
        {
          if (addr->network != PAMI_N_TORUS_NETWORK)
            return PAMI_ERROR;
          *task = addr->u.n_torus.coords[0];
          *type = PAMI_N_TORUS_NETWORK;
          return PAMI_SUCCESS;
        }
    }; // class Mapping
};	// namespace PAMI
#endif // __components_mapping_lapi_lapimapping_h__
