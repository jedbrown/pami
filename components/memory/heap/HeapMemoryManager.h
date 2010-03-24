/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/memory/heap/HeapMemoryManager.h
 * \brief ???
 */

#ifndef __components_memory_heap_HeapMemoryManager_h__
#define __components_memory_heap_HeapMemoryManager_h__

#include "components/memory/MemoryManager.h"


namespace PAMI
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

      inline pami_result_t memalign_impl (void   ** memptr,
                                         size_t    alignment,
                                         size_t    bytes)
        {
#ifdef USE_MEMALIGN
          posix_memalign (memptr, alignment, bytes);
#else
          *memptr = malloc(bytes);
#endif
	  return PAMI_SUCCESS;
        }

    protected:
    };
  };
};

#endif // __pami_components_memory_heap_heapmemorymanager_h__
