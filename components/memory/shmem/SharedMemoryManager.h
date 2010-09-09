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
	struct shmhdr_t {
		size_t ref_count;
		size_t init_done;
	};

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
		if (alignment < _alignment) alignment = _alignment;
		pami_result_t rc;
		MemoryManagerChunk *alloc = NULL;
		void *ptr = NULL;
		bool first = false;
		int fd = -1;
		rc = heap_mm->memalign((void **)&alloc, sizeof(void *), sizeof(*alloc));
		if (rc != PAMI_SUCCESS) {
			return rc;
		}
		new (alloc) MemoryManagerChunk(key, sizeof(void *), bytes);
		size_t rawbytes = alloc->safeSize();
		if (_attrs == PAMI_MM_NODESCOPE) {
			// should we keep track of each shm_open, so that we can
			// later shm_unlink?

			// first try to create the file exclusively, if that
			// succeeds then we know we should initialize it.
			// However, we still need to ensure others do not
			// start using the memory until we complete the init.
			// Use a "counter mutex" that is initialized to 0, all but
			// the first (based on O_EXCL) will wait on it.
			// 

			// maybe use GCC atomics on the shared memory chunk, in order to
			// synchronize init, and in free will need to ensure memory gets zeroed.
			int lrc;
//#ifdef _POSIX_SHM_OPEN // This is NOT a valid selector for POSIX shm_open!!!
#if 1
			lrc = shm_open (alloc->key(), O_CREAT | O_EXCL | O_RDWR, 0600);
			first = (lrc != -1); // must be the first...

			if (!first) lrc = shm_open (alloc->key(), O_CREAT | O_RDWR, 0600);
			if (lrc != -1)
			{
				fd = lrc;
				lrc = ftruncate( fd, rawbytes );
				if ( lrc != -1 )
				{
					ptr = mmap( NULL, rawbytes, PROT_READ | PROT_WRITE,
								MAP_SHARED, fd, 0);
					if ( ptr == MAP_FAILED )
					{
						ptr = NULL;
					}
				}
				if (ptr == NULL)
				{
					close(fd);
					shm_unlink(alloc->key());
				}
			}
#else // ! _POSIX_SHM_OPEN
#warning Need to write SysV Shmem equivalent
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
			if (lrc != -1)
			{
				fd = lrc;
				ptr = shmat(fd, 0, 0);
				if (ptr == NULL || ptr == (void *)-1) {
					shmctl(fd, IPC_RMID, NULL);
					ptr = NULL;
				} else {
					fd = -1;
				}
			}
#endif // ! _POSIX_SHM_OPEN
		}
		if (ptr == NULL) // then (fd == -1) also...
		{
			// assuming failed because in SMP mode, but should do a better
			// job of error analysis.

			// switch to only ever try private/local allocs
			_attrs = PAMI_MM_PROCSCOPE;
			// not shared, we're first, last, all...
			first = true;

			rc = heap_mm->memalign(&ptr, alignment, bytes);

			if (rc != PAMI_SUCCESS) {
				heap_mm->free(alloc);
				return PAMI_ERROR;
			}
			memset(ptr, 0, bytes); // simulate shmem zeroing
		}
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
		_mmhdr.push(alloc);
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

    protected:

	inline void _free(MemoryManagerChunk *m) {
#ifdef _POSIX_SHM_OPEN
		if (m->fd() != -1) {
			close(m->fd());
		}
#endif // _POSIX_SHM_OPEN
		if (m->rmRef() == 1) {
			if (m->fd() != -1) {
#ifdef _POSIX_SHM_OPEN
				shm_unlink(m->key());
#else // ! _POSIX_SHM_OPEN
				shmctl(m->fd(), IPC_RMID, NULL);
#endif // ! _POSIX_SHM_OPEN
			} else {
				heap_mm->free(m->rawAddress());
			}
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

	MemoryManagerHeader _mmhdr; // _mutex not used/needed?
    }; // class SharedMemoryManager
  }; // namespace Memory
}; // namespace PAMI
#undef TRACE_ERR
#endif // __pami_components_memory_shmem_sharedmemorymanager_h__
