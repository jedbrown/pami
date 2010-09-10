/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/memory/shmem/SharedMemoryManager.h
/// \brief Shared memory manager class
///
/// The class defined in this file uses C++ templates. C++ templates
/// require all source code to be #include'd from a header file.
///
#ifndef __components_memory_shmem_SharedMemoryManager_h__
#define __components_memory_shmem_SharedMemoryManager_h__

#include "components/memory/MemoryManager.h"

#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>

#ifndef TRACE_ERR
#define TRACE_ERR(x)  //fprintf x
#endif

#define _POSIX_SHM_OPEN // This is NOT a valid selector for Linux!

namespace PAMI
{
  namespace Memory
  {
    class SharedMemoryManager : public MemoryManager
    {
      public:

	/// \brief This class is a wrapper for the shm_open/ftruncate/mmap sequence
	///
	/// Unlike general MemoryManagers, it does not actually contain any memory
	/// instead it provides an interface into the OS's shared memory pool.
	///
	/// Because shm segments persist after the job exits (on some platforms)
	/// this will need to keep track of all allocations so that the destructor
	/// can unlink them all. This only happens when the ref_count reaches zero.
	///
	/// Support for free() is not provided yet, but allocations must still be
	/// tracked for the afore mentioned reason.
	///
        inline SharedMemoryManager () :
          MemoryManager ()
        {
		COMPILE_TIME_ASSERT(sizeof(SharedMemoryManager) <= sizeof(MemoryManager));
		// This could be better decided based on number of processes
		// per node, but can't get that from __global because of
		// circular dependencies. Instead, Global could check and
		// simply not construct SharedMemoryManager in shared_mm
		// (construct HeapMemoryManager twice, in heap_mm and shared_mm).
		_attrs = PAMI_MM_NODESCOPE;
        }

        inline ~SharedMemoryManager ()
        {
		// if this only happens at program exit, just unlink all keys...
		freeAll();
        }


	inline pami_result_t init (MemoryManager *mm, size_t bytes, size_t alignment,
			unsigned attrs = 0, const char *key = NULL,
			MM_INIT_FN *init_fn = NULL, void *cookie = NULL)
	{
		PAMI_abortf("SharedMemoryManager cannot be init()");
		return PAMI_ERROR;
	}

	/// \todo How to enforce alignment?
	inline pami_result_t memalign (void ** memptr, size_t alignment, size_t bytes,
			const char *key = NULL,
			MM_INIT_FN *init_fn = NULL, void *cookie = NULL)
	{
		PAMI_assert_debugf(_attrs == PAMI_MM_NODESCOPE, "SharedMemoryManager not shared");
		if (alignment < _alignment) alignment = _alignment;
		pami_result_t rc;
		MemoryManagerChunk *alloc = NULL;
		void *ptr = NULL;
		bool first = false;
		int fd = -1;
		rc = heap_mm->memalign((void **)&alloc,
			MemoryManagerChunk::ALIGNMENT, sizeof(*alloc));
		if (rc != PAMI_SUCCESS) {
			return rc;
		}
		new (alloc) MemoryManagerChunk(key, alignment, bytes);
		size_t rawbytes = alloc->safeSize();

		// Note, Global does not construct this if the target conditions
		// would prevent it from succeeding - for example SMP-mode on BG.
		// So that simplifies the code, as any errors are just fatal.

		// should we keep track of each shm_open, so that we can
		// later shm_unlink?

		// first try to create the file exclusively, if that
		// succeeds then we know we should initialize it.
		// However, we still need to ensure others do not
		// start using the memory until we complete the init.
		// Use a "counter mutex" that is initialized to 0, all but
		// the first (based on O_EXCL) will wait on it.
		// 

		// use GCC atomics on the shared memory chunk, in order to
		// synchronize init, and in free will need to ensure memory gets zeroed.
		int lrc;
#ifdef _POSIX_SHM_OPEN // This is NOT a valid selector for POSIX shm_open!!!
		lrc = shm_open(alloc->key(), O_CREAT | O_EXCL | O_RDWR, 0600);
		first = (lrc != -1); // must be the first...

		if (!first) lrc = shm_open(alloc->key(), O_CREAT | O_RDWR, 0600);
		if (lrc == -1)
		{
			heap_mm->free(alloc);
			return PAMI_ERROR;
		}
		fd = lrc;
		lrc = ftruncate(fd, rawbytes);
		if (lrc == -1)
		{
			close(fd); // needed? possible?
			shm_unlink(alloc->key()); // right?
			heap_mm->free(alloc);
			return PAMI_ERROR;
		}
		ptr = mmap(NULL, rawbytes, PROT_READ | PROT_WRITE,
							MAP_SHARED, fd, 0);
		if (ptr == NULL || ptr == MAP_FAILED)
		{
			close(fd); // needed? possible?
			shm_unlink(alloc->key()); // right?
			heap_mm->free(alloc);
			return PAMI_ERROR;
		}
#else // ! _POSIX_SHM_OPEN
#warning Need to write/validate SysV Shmem equivalent
		key_t kkey;
		if (!key) {
			kkey = IPC_PRIVATE;
		} else {
			// need to turn string into unique integer..
			// yuk
			int n = strlen(key);
			int x = n / sizeof(key_t);
			int y = n % sizeof(key_t);
			kkey = 0;
			key_t  *k = (key_t *)key;
			while (x--) {
				kkey ^= *k++;
			}
			char *ck = (char *)k;
			while (y--) {
				kkey ^= *ck++;
			}
			// yukky hash code
		}

		lrc = shmget(kkey, rawbytes, IPC_CREAT | IPC_EXCL | 0600);
		first = (lrc != -1); // must be the first...
		if (!first) lrc = shmget(kkey, 0, 0);
		if (lrc == -1)
		{
			heap_mm->free(alloc);
			return PAMI_ERROR;
		}
		fd = lrc;
		ptr = shmat(fd, 0, 0);
		if (ptr == NULL || ptr == (void *)-1) {
			shmctl(fd, IPC_RMID, NULL);
			heap_mm->free(alloc);
			return PAMI_ERROR;
		}
#endif // ! _POSIX_SHM_OPEN
		// shared segment acquired, now sync and init.

		alloc->init(ptr, fd); // fd might be -1, indicates not-shm
		ptr = alloc->address();
		if (first)
		{
			init_fn(ptr, bytes, alloc->key(), _attrs, cookie);
			alloc->addRef();
			alloc->initDone();
		}
		else
		{
			alloc->addRef();
			while (!alloc->isInitDone()); // any better way than spinning?
		}
		*memptr = ptr;
		_mmhdr.acquire();
		_mmhdr.push(alloc);
		_mmhdr.release();
		return PAMI_SUCCESS;
	}

	inline void free(void *mem) {
		_mmhdr.acquire();
		MemoryManagerChunk *m = _mmhdr.find(mem);
		if (m) {
			_free(m);
		}
		_mmhdr.release();
	}

	inline size_t available(size_t alignment) {
		// how to tell how much is available???
		// BGQ: getenv(BG_SHAREDMEMSIZE), minus something...
		return (size_t)-1;
	}

    protected:

	// lock held by caller.
	inline void _free(MemoryManagerChunk *m) {
#ifdef _POSIX_SHM_OPEN
		close(m->fd()); // needed? possible?
#endif // _POSIX_SHM_OPEN
		if (m->rmRef() == 1) {
			// zero memory so that next use is zeroed?
			// memset(m->rawAddress(), 0, m->size());
#ifdef _POSIX_SHM_OPEN
			shm_unlink(m->key());
#else // ! _POSIX_SHM_OPEN
			shmctl(m->fd(), IPC_RMID, NULL);
#endif // ! _POSIX_SHM_OPEN
		}
		_mmhdr.dequeue(m);
	}

	inline void freeAll() {
		_mmhdr.acquire();
		MemoryManagerChunk *m = _mmhdr.head();
		MemoryManagerChunk *n;
		while (m) {
			n = m->next();
			_free(m);
			m = n;
		}
		_mmhdr.release();
	}

    }; // class SharedMemoryManager
  }; // namespace Memory
}; // namespace PAMI
#undef TRACE_ERR
#endif // __pami_components_memory_shmem_sharedmemorymanager_h__
