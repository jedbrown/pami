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
        }
      
      inline xmi_result_t memalign_impl (void   ** memptr,
                                         size_t    alignment,
                                         size_t    bytes)
        {
#ifdef USE_MEMALIGN
          posix_memalign (memptr, alignment, bytes);
#else
          *memptr = malloc(bytes);
#endif          
        }
      
    protected:
    };
  };
};

#endif // __xmi_components_memory_shmem_sharedmemorymanager_h__


