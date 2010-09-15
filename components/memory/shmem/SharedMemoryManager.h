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

#ifdef _POSIX_SHM_OPEN

#define PAMI_MM_SHM_OPEN_EXCL(m) shm_open(m->key(), O_CREAT | O_EXCL | O_RDWR, 0600)
#define PAMI_MM_SHM_OPEN(m)	shm_open(m->key(), O_RDWR, 0)
#define PAMI_MM_SHM_MAP(m) ({ (ftruncate(m->fd(), m->size()) == -1) ? NULL : mmap(NULL, m->size(), PROT_READ | PROT_WRITE, MAP_SHARED, m->fd(), 0); })
#define PAMI_MM_SHM_UNMAP(m)	{close(m->fd()); munmap(m->mem(), m->size());}
#define PAMI_MM_SHM_DELETE(m)	{shm_unlink(m->key());}

#else // ! _POSIX_SHM_OPEN

#warning Need to write/validate SysV Shmem equivalent
#define PAMI_MM_SHM_OPEN_EXCL(m) shmget(m->vkey(), m->size(), IPC_CREAT | IPC_EXCL | 0600)
#define PAMI_MM_SHM_OPEN(m)	shmget(m->vkey(), 0, 0)
#define PAMI_MM_SHM_MAP(m)	shmat(m->fd(), 0, 0)
#define PAMI_MM_SHM_UNMAP(m)	{shmdt(m->mem());}
#define PAMI_MM_SHM_DELETE(m)	{shmctl(m->fd(), IPC_RMID, NULL);}

#endif // ! _POSIX_SHM_OPEN

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
        inline SharedMemoryManager (MemoryManager *mm) :
          MemoryManager (),
	  _meta()
        {
		// This could be better decided based on number of processes
		// per node, but can't get that from __global because of
		// circular dependencies. Instead, Global could check and
		// simply not construct SharedMemoryManager in shared_mm
		// (construct HeapMemoryManager twice, in heap_mm and shared_mm).
		_attrs = PAMI_MM_NODESCOPE;
		_meta.init(mm, "/pami-shmemmgr");
		_enabled = true;
        }

        inline ~SharedMemoryManager ()
        {
		// if this only happens at program exit, just unlink all keys...
		freeAll();
		_meta.~MemoryManagerMeta<MemoryManagerOSAlloc>();
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
		void *ptr = NULL;
		bool first = false;
		_meta.acquire();
		MemoryManagerOSAlloc *alloc = _meta.findFree();
		if (alloc == NULL) {
			_meta.release();
			return PAMI_ERROR;
		}
		alloc->key(key);
		alloc->userSize(bytes, alignment);
		// note, inital (worst-case) padding now set, when
		// actual pointer assigned below, padding is updated.
#ifdef _POSIX_SHM_OPEN
#else // ! _POSIX_SHM_OPEN
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
		alloc->vkey(kkey);
#endif // ! _POSIX_SHM_OPEN
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

		lrc = PAMI_MM_SHM_OPEN_EXCL(alloc);
		first = (lrc != -1); // must be the first...

		if (!first) lrc = PAMI_MM_SHM_OPEN(alloc);
		if (lrc == -1)
		{
			alloc->free();
			_meta.release();
			return PAMI_ERROR;
		}
		alloc->fd(lrc); // PAMI_MM_SHM_MAP (et al.) requires this first.
		ptr = PAMI_MM_SHM_MAP(alloc);
		if (ptr == NULL || ptr == MAP_FAILED)
		{
			PAMI_MM_SHM_UNMAP(alloc);
			PAMI_MM_SHM_DELETE(alloc);
			alloc->free();
			_meta.release();
			return PAMI_ERROR;
		}
		// shared segment acquired, now sync and init.

		alloc->mem(ptr, alignment); // required for addRef(), userMem(), etc...
		alloc->addRef();
		if (init_fn) {
			if (first) {
				init_fn(alloc->userMem(), alloc->userSize(),
						alloc->key(), _attrs, cookie);
				alloc->initDone();
			} else {
				// note: sync may not be needed if _meta.acquire/release
				// already prevents the race condition.
				alloc->waitDone();
			}
		}
		*memptr = alloc->userMem();
		_meta.release();
		return PAMI_SUCCESS;
	}

	inline void free(void *mem) {
		_meta.acquire();
		MemoryManagerOSAlloc *m = _meta.find(mem);
		if (m) {
			__free(m);
		}
		_meta.release();
	}

	inline size_t available(size_t alignment) {
		// how to tell how much is available???
		// BGQ: getenv(BG_SHAREDMEMSIZE), minus something...
		return (size_t)-1;
	}

    protected:

	// lock held by caller.
	inline void __free(MemoryManagerOSAlloc *m) {
		PAMI_MM_SHM_UNMAP(m);
		if (m->rmRef() == 1) {
			// zero memory so that next use is zeroed?
			// memset(m->rawAddress(), 0, m->size());
			PAMI_MM_SHM_DELETE(m);
			m->free();
		}
	}

	// lock held by caller.
	static void _free(MemoryManagerOSAlloc *m, void *cookie) {
		SharedMemoryManager *thus = (SharedMemoryManager *)cookie;
		thus->__free(m);
	}

	inline void freeAll() {
		_meta.acquire();
		_meta.forAllActive(_free);
		_meta.release();
	}

	MemoryManagerMeta<MemoryManagerOSAlloc> _meta;

    }; // class SharedMemoryManager
  }; // namespace Memory
}; // namespace PAMI
#undef TRACE_ERR
#endif // __pami_components_memory_shmem_sharedmemorymanager_h__
