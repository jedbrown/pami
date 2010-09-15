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
#include "components/memory/shmem/SharedMemoryManager.h"

namespace PAMI
{
  namespace Memory
  {
    class HeapMemoryManager : public MemoryManager
    {
    public:

      inline HeapMemoryManager () :
        MemoryManager ()
        {
		COMPILE_TIME_ASSERT(sizeof(HeapMemoryManager) <= sizeof(SharedMemoryManager));
		_attrs = PAMI_MM_PROCSCOPE;
        }

      inline ~HeapMemoryManager ()
	{
		// this is only called from process exit,
		// so no need to actually free - the memory
		// is all being reclaimed by the OS.
	}

	inline pami_result_t init (MemoryManager *mm, size_t bytes, size_t alignment,
			unsigned attrs = 0, const char *key = NULL,
			MM_INIT_FN *init_fn = NULL, void *cookie = NULL)
	{
		PAMI_abortf("HeapMemoryManager cannot be init()");
		return PAMI_ERROR;
	}

	inline pami_result_t memalign (void ** memptr, size_t alignment, size_t bytes,
			const char *key = NULL,
			MM_INIT_FN *init_fn = NULL, void *cookie = NULL)
	{
		if (alignment < _alignment) alignment = _alignment;
#ifdef USE_MEMALIGN
		int rc = posix_memalign (memptr, alignment, bytes);
		if (rc != 0) {
			return PAMI_ERROR;
		}
#else
		// todo: actually do requested alignment...
		*memptr = malloc(bytes);
		if (!*memptr) return PAMI_ERROR;
#endif
		if (init_fn)
		{
			init_fn(*memptr, bytes, key, _attrs, cookie);
		//
		// else? or always? memset(*memptr, 0, bytes);
		}
		return PAMI_SUCCESS;
        }

	inline void free(void *mem)
	{
		free(mem);
	}

	inline size_t available(size_t alignment) {
		// how to tell how much is available???
		return (size_t)-1;
	}

    protected:
    };
  };
};

#endif // __pami_components_memory_heap_heapmemorymanager_h__
