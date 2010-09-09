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
#if MM_FREE
	struct mm_alloc {
		shmhdr_t *hdr;
		size_t total_bytes;
		char key[MMKEYSIZE];
	};
#endif
      public:

	/// \brief This class is a wrapper for the shm_open/ftruncate/mmap sequence
	///
	/// Unlike general MemoryManagers, it does not actually contain any memory
	/// instead it provides an interface into the OS's shared memory pool.
	///
        inline SharedMemoryManager () :
          MemoryManager ()
        {
		_attrs = PAMI_MM_SHARED;
        }

        inline ~SharedMemoryManager ()
        {
		// if this only happens at program exit, just unlink all keys...
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
		pami_result_t rc;
		shmhdr_t *hdr;
		void *ptr;
		size_t rawbytes = bytes + sizeof(*hdr);
		bool first;
	if (_attrs == PAMI_MM_SHARED) {
		// should we keep track of each shm_open, so that we can
		// later shm_unlink?

		// first try to create the file exclusively, if that
		// succeeds then we know we should initialize it.
		// However, we still need to ensure others do not
		// start using the memory until we complete the init.
		// Use a semaphore that is initialized to 0, all but
		// the first (based on O_EXCL) will wait on the semaphore.
		// 

		// maybe use GCC atomics on the shared memory chunk, in order to
		// synchronize init, and in free will need to ensure memory gets zeroed.
		int lrc;
//#ifdef _POSIX_SHM_OPEN // This is NOT a valid selector for POSIX shm_open!!!
#if 1
		int fd;
		sem_t *sem = sem_open(key, O_CREAT, 0600, 0);
		if (sem == NULL) return PAMI_ERROR;

		lrc = shm_open (key, O_CREAT | O_EXCL | O_RDWR, 0600);
		first = (lrc != -1); // must be the first...

		if (!first) lrc = shm_open (key, O_CREAT | O_RDWR, 0600);
		if (lrc != -1)
		{
			fd = lrc;
			lrc = ftruncate( fd, rawbytes );
		}
		if ( lrc != -1 )
		{
			ptr = mmap( NULL, rawbytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
			if ( ptr != MAP_FAILED )
			{
				goto syncup;
			}
		}
		sem_close(sem);
		sem_unlink(key);
		// need to destroy file?
		// shm_unlink(key); // should not unlink until all close?
		// (but, must ensure O_EXCL works so can't unlink here... ?

#else // ! _POSIX_SHM_OPEN
#warning Need to write SysV Shmem equivalent
		// need to turn string into unique integer..
		// yuk
		int n = strlen(key);
		int x = n / sizeof(key_t);
		int y = n % sizeof(key_t);
		key_t kkey = 0, *k = (key_t *)key;
		while (x--) {
			kkey ^= *k++;
		}
		char *ck = (char *)k;
		while (y--) {
			kkey ^= *ck++;
		}
		// yukky hash code

		int id = shmget(kkey, rawbytes, IPC_CREAT | IPC_EXCL | 0600);
		if (id == -1) {
			id = shmget(kkey, 0, 0);
			if (id == -1) {
				return PAMI_ERROR;
			}
			// slave - must wait for "master" to init...
		}
		ptr = shmat(id, 0, 0);
		if (ptr == NULL || ptr == (void *)-1) {
			shmctl(id, IPC_RMID, NULL);
			return PAMI_ERROR;
		}
#endif // ! _POSIX_SHM_OPEN
		//
		// assuming failed because we're SMP mode, but really should do a better
		// job of error analysis.

		// switch to only ever try private/local allocs
		_attrs = PAMI_MM_PRIVATE;
	}
	// not shared, we're first, last, all...

		rc = heap_mm->memalign(memptr, alignment, bytes, key, init_fn, cookie);

		if (rc != PAMI_SUCCESS) return PAMI_ERROR;
		memset(*memptr, 0, bytes);
		if (init_fn)
		{
			init_fn(*memptr, bytes, key, PAMI_MM_PRIVATE, cookie);
		}
		return PAMI_SUCCESS;
syncup:
		hdr = (shmhdr_t *)ptr;
		ptr = (void *)(hdr + 1);
		if (first)
		{
			init_fn(ptr, bytes, key, PAMI_MM_SHARED, cookie);
			__sync_fetch_and_add(&hdr->ref_count, 1);
			hdr->init_done = 1;
		}
		else
		{
			__sync_fetch_and_add(&hdr->ref_count, 1);
			while (!hdr->init_done); // any better way than spinning?
		}
#if MM_FREE
		_allocs[i].total_bytes = rawbytes;
		_allocs[i].hdr = hdr;
#ifdef _POSIX_SHM_OPEN
		strncpy(_allocs[i].key, key, MMKEYSIZE);
#else // ! _POSIX_SHM_OPEN
		*((int *)_allocs[i].key) = id;
#endif // ! _POSIX_SHM_OPEN
#endif
		*memptr = ptr;
		return PAMI_SUCCESS;
	}

#if MM_FREE
	// during destruction/free...
	// Note, this isn't right or probably anywhere near it...
	for (i = 0; i < _nallocs; ++i) {
		if (!_allocs[i].hdr) continue;
		size_t c = __sync_fetch_and_add(&_allocs[i].hdr->ref_count, -1);
		if (c != 1) continue; // not last one, don't remove...
		// only one participant should run this...
#ifdef _POSIX_SHM_OPEN
		shm_unlink(_allocs[i].key);
#else // ! _POSIX_SHM_OPEN
		shmctl(*((int *)_allocs[i].key), IPC_RMID, NULL);
#endif // ! _POSIX_SHM_OPEN
		// ensure next user of this memory gets zeros!
		memset((void *)_allocs[i].hdr, 0, _allocs[i].total_bytes);
	}
#endif

    protected:
    }; // class SharedMemoryManager
  }; // namespace Memory
}; // namespace PAMI
#undef TRACE_ERR
#endif // __pami_components_memory_shmem_sharedmemorymanager_h__
