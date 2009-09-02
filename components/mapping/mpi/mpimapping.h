#ifndef __components_mapping_mpi_mpimapping_h__
#define __components_mapping_mpi_mpimapping_h__

#include "sys/xmi.h"

#include "components/mapping/BaseMapping.h"
#include "components/memory/heap/HeapMemoryManager.h"

#define XMI_MAPPING_CLASS XMI::Mapping::MPIMapping
#define XMI_MAPPING_MEMORY_CLASS XMI::Memory::HeapMemoryManager

namespace XMI
{
  namespace Mapping
  {
    class MPIMapping : public Interface::Base<MPIMapping,Memory::HeapMemoryManager>
    {
      public:
        inline MPIMapping () :
            Interface::Base<MPIMapping,Memory::HeapMemoryManager>()
        {
        };
        inline ~MPIMapping () {};
      protected:
        size_t    _task;
        size_t    _size;
        size_t    _nodes;
        size_t    _peers;
        size_t * _mapcache;
        size_t * _rankcache;
      public:
        inline xmi_result_t init_impl(Memory::HeapMemoryManager & mm)
        {
          
        }
	inline size_t task_impl()
        {
          assert(0);
          return _task;
        }
        inline size_t size_impl()
        {
          assert(0);
          return _size;
        }
        inline size_t numActiveNodesGlobal_impl ()
        {
          assert(0);
          return _nodes;
        }
        inline size_t numActiveTasksGlobal_impl ()
        {
          assert(0);
          return _size;
        }
        inline size_t numActiveTasksLocal_impl ()
        {
          assert(0);
          return _peers;
        }
        inline bool isPeer_impl (size_t task1, size_t task2)
        {

        }
    };
  };
};
#endif // __components_mapping_mpi_mpimapping_h__

