#ifndef __xmi_components_memory_shmem_sharedmemorymanager_h__
#define __xmi_components_memory_shmem_sharedmemorymanager_h__

#include "components/memory/MemoryManager.h"


namespace XMI
{
  namespace Memory
  {
    class HeapMemoryManager : public Interface::MemoryManager<HeapMemoryManager>
    {
    public:
      inline HeapMemoryManager () :
        Interface::MemoryManager<HeapMemoryManager> ()
        {
          assert(0);
        }
      
      inline xmi_result_t memalign_impl (void   ** memptr,
                                         size_t    alignment,
                                         size_t    bytes)
        {
          assert(0);
        }
      
    protected:
    };
  };
};

#endif // __xmi_components_memory_shmem_sharedmemorymanager_h__


