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

#include <cstdlib>

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
#ifdef MM_DEBUG // set/unset in MemoryManager.h
		_debug = (getenv("PAMI_MM_DEBUG") != NULL);
		_num_allocs = 0;
		_num_frees = 0;
		_total_bytes = 0;
		// since currently this can't track number of bytes freed,
		// don't bother trying to track current/max bytes.
#endif // MM_DEBUG
        }

      ~HeapMemoryManager()
	{
		// this is only called from process exit,
		// so no need to actually free - the memory
		// is all being reclaimed by the OS.
#ifdef MM_DEBUG
		if (_debug) {
			fprintf(stderr, "HeapMemoryManager: "
					"%zd allocs, %zd frees, total %zdb\n",
				_num_allocs, _num_frees,
				_total_bytes);
		}
#endif // MM_DEBUG
	}

	inline const char *getName() { return "HeapMemoryManager"; }

	inline pami_result_t init (MemoryManager *mm, size_t bytes,
			size_t alignment, size_t new_align,
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
		int rc = posix_memalign(memptr, alignment, bytes);
		if (rc != 0) {
			return PAMI_ERROR;
		}
#else
		void *ptr = malloc(bytes + alignment);
		if (!ptr) return PAMI_ERROR;
		*memptr = (void *)(((uintptr_t)ptr + alignment - 1) &
						~(alignment - 1));
#endif
		memset(*memptr, 0, bytes);	// needed for 1 proc/node, when
						// simulating shared_mm...
		if (init_fn)
		{
			init_fn(*memptr, bytes, key, _attrs, cookie);
		//
		// else? or always? memset(*memptr, 0, bytes);
		}
#ifdef MM_DEBUG
		if (_debug) {
			++_num_allocs;
			_total_bytes += bytes;
		}
#endif // MM_DEBUG
		return PAMI_SUCCESS;
        }

	inline void free(void *mem)
	{
#ifdef MM_DEBUG
		if (_debug) {
			++_num_frees;
		}
#endif // MM_DEBUG
		std::free(mem);
	}

	inline size_t available(size_t alignment) {
		// how to tell how much is available???
		return (size_t)-1;
	}

	inline void dump(const char *str) {
		if (str) {
			fprintf(stderr, "%s: HeapMemoryManager %x\n", str,
					_attrs);
		} else {
			fprintf(stderr, "HeapMemoryManager %x\n",
					_attrs);
		}
	}

    protected:
#ifdef MM_DEBUG
	bool _debug;
	size_t _num_allocs;
	size_t _num_frees;
	size_t _total_bytes;
#endif // MM_DEBUG
    };
  };
};

#endif // __pami_components_memory_heap_heapmemorymanager_h__
