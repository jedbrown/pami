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
    class SharedMemoryManager<class T_Global> : public MemoryManager
    {
      protected:
	friend class PAMI::Interface::Global<T_Global>;

	/// \brief This class is a wrapper for the shm_open/ftruncate/mmap sequence
	///
	/// Unlike general MemoryManagers, it does not actually contain any memory
	/// instead it provides an interface into the OS's shared memory pool.
	///
        inline SharedMemoryManager<T_Global> (MemoryManager *heapmm) :
          MemoryManager ()
        {
		_attrs = PAMI_MM_SHARED;
		// allocate some memory for tracking allocations...
		_mm = heapmm;
        }

        inline ~SharedMemoryManager<T_Global> ()
        {
		// if this only happens at program exit, just unlink all keys...
        }

      public:

	inline void init (MemoryManager *mm, size_t bytes, size_t alignment,
			unsigned attrs, char *key)
	{
		PAMI_abortf("SharedMemoryManager cannot be init()");
	}
	inline void init (void * addr, size_t bytes, size_t alignment, unsigned attrs)
	{
		PAMI_abortf("SharedMemoryManager cannot be init()");
	}

	/// \todo How to enforce alignment?
	inline pami_result_t memalign (void ** memptr, size_t alignment, size_t bytes,
			char *key, _mm_init_fn *init_fn, void *cookie)
	{
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
		sem_t *sem = sem_open(key, O_CREAT, 0600, 0);
		if (sem == NULL) return PAMI_ERROR;

		rc = shm_open (key, O_CREAT | O_EXCL | O_RDWR, 0600);
		bool first = (rc != -1); // must be the first...

		if (!first) rc = shm_open (key, O_CREAT | O_RDWR, 0600);
		if (rc != -1)
		{
			fd = rc;
			rc = ftruncate( fd, bytes );
		}
		if ( rc != -1 )
		{
			void * ptr = mmap( NULL, bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
			if ( ptr != MAP_FAILED )
			{
				if (first)
				{
					init_fn(ptr, bytes, key, PAMI_MM_SHARED, cookie);
					sem_post(sem); // wake up next...
				}
				else
				{
					sem_wait(sem);
					sem_post(sem); // wake up next... if any
				}
				*memptr = ptr;
				return PAMI_SUCCESS;
			}
		}
		sem_close(sem);
		sem_unlink(key);
		// need to destroy file?
		// shm_unlink(key); // should not unlink until all close?
		// (but, must ensure O_EXCL works so can't unlink here... ?
		//
		// assuming failed because we're SMP mode, but really should do a better
		// job of error analysis.

		// switch to only ever try prival/local allocs
		_attrs = PAMI_MM_PRIVATE;
	}

		rc = _mm->memalign(memptr, alignment, bytes, key, init_fn, cookie);

		if (rc != PAMI_SUCCESS) return PAMI_ERROR;
		if (init_fn)
		{
			init_fn(*memptr, bytes, key, PAMI_MM_PRIVATE, cookie);
		//
		// else? or always? memset(*memptr, 0, bytes);
		}
		return PAMI_SUCCESS;
	}

    protected:
	MemoryManager *_mm;
    }; // class SharedMemoryManager
  }; // namespace Memory
}; // namespace PAMI
#undef TRACE_ERR
#endif // __pami_components_memory_shmem_sharedmemorymanager_h__
