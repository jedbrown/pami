/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/mapping/lapiunix/lapiunixmapping.h
 * \brief ???
 */

#ifndef __components_mapping_lapi_lapimapping_h__
#define __components_mapping_lapi_lapimapping_h__

#include "sys/xmi.h"
#include "Platform.h"
#include "common/BaseMappingInterface.h"
#include "common/TorusMappingInterface.h"
#include "common/NodeMappingInterface.h"
#include "components/memory/heap/HeapMemoryManager.h"
#include <lapi.h>

#define XMI_MAPPING_CLASS XMI::Mapping

namespace XMI
{
#define LAPI_DIMS 1
    class Mapping : public Interface::Mapping::Base<Mapping, XMI::Memory::HeapMemoryManager>,
                       public Interface::Mapping::Torus<Mapping, LAPI_DIMS>,
                       public Interface::Mapping::Node<Mapping, LAPI_DIMS>
    {

    public:
      inline Mapping () : 
        Interface::Mapping::Base<Mapping,XMI::Memory::HeapMemoryManager >(),
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
      inline xmi_result_t init(size_t &min_rank, size_t &max_rank)
        {
          min_rank = 0;
          max_rank = _size-1;

          return XMI_SUCCESS;
        }
      inline size_t task_impl()
        {
          return _task;
        }
      inline size_t size_impl()
        {
          return _size;
        }
      inline xmi_result_t nodeTasks_impl (size_t global, size_t & tasks)
        {
          assert(0);
          return XMI_UNIMPL;
        }
      inline xmi_result_t nodePeers_impl (size_t & peers)
        {
          assert(0);
          return XMI_UNIMPL;
        }
      inline bool isPeer_impl (size_t task1, size_t task2)
        {
          assert(0);
          return XMI_UNIMPL;
        }
      inline void nodeAddr_impl (Interface::Mapping::nodeaddr_t & address)
        {
          assert(0);
        }
      inline xmi_result_t task2node_impl (size_t task, Interface::Mapping::nodeaddr_t & address)
        {
          address.global=task;
          address.local =0;
          return XMI_SUCCESS;
        }
      inline xmi_result_t node2task_impl (Interface::Mapping::nodeaddr_t & address, size_t & task)
        {
          task = address.global;
          return XMI_SUCCESS;
        }
      inline xmi_result_t node2peer_impl (Interface::Mapping::nodeaddr_t & address, size_t & peer)
        {
          assert(0);
          return XMI_UNIMPL;
        }
      inline void torusAddr_impl (size_t (&addr)[LAPI_DIMS])
        {
          addr[0]=_task;
        }
      inline xmi_result_t task2torus_impl (size_t task, size_t (&addr)[LAPI_DIMS])
        {
          addr[0] = task;
          return XMI_SUCCESS;
        }

      inline xmi_result_t torus2task_impl (size_t (&addr)[LAPI_DIMS], size_t & task)
        {
          task = addr[0];
          return XMI_SUCCESS;
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
    }; // class Mapping
};	// namespace XMI
#endif // __components_mapping_lapi_lapimapping_h__
