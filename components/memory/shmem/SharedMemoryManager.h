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
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>

#ifndef TRACE_ERR
#define TRACE_ERR(x)  //fprintf x
#endif

namespace PAMI {
namespace Memory {

class SharedMemoryManager : public MemoryManager {
private:
#ifdef MM_DEBUG // set/unset in MemoryManager.h
	inline void MM_RESET_STATS() {
		_num_allocs = 0;
		_num_frees = 0;
		_loc_bytes = 0;
		_rep_bytes = 0;
		_fre_bytes = 0;
	}
	inline void MM_DUMP_STATS() {
		fprintf(stderr, "%s: %zd allocs, %zd frees, "
			"local %zd, repeat %zd, freed %zd\n",
			getName(), _num_allocs, _num_frees,
			_loc_bytes, _rep_bytes, _fre_bytes);
	}
#endif // MM_DEBUG
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
	inline SharedMemoryManager(size_t jobid, MemoryManager *mm) :
	MemoryManager(),
	_meta(),
	_jobid(jobid) {
		// This could be better decided based on number of processes
		// per node, but can't get that from __global because of
		// circular dependencies. Instead, Global could check and
		// simply not construct SharedMemoryManager in shared_mm
		// (construct HeapMemoryManager twice, in heap_mm and shared_mm).
		_attrs = PAMI_MM_NODESCOPE;
		_meta.init(mm, "/pami-shmemmgr");
		_enabled = true;
#ifdef MM_DEBUG
		_debug = setup_debug();
		MM_RESET_STATS();
#endif // MM_DEBUG
	}

	~SharedMemoryManager() {
#ifdef MM_DEBUG
		if (_debug) {
			MM_DUMP_STATS();
			if (_debug > 1) _meta.dump(getName());
		}
#endif // MM_DEBUG
		// if this only happens at program exit, just unlink all keys...

		// freeAll(); // needs to be done before/in _meta.~MemoryManagerMeta which
		// frees all the meta data buffers!

		// could free up all the meta data, but it is in heap and
		// about to be freed by _exit().
		// _meta.~MemoryManagerMeta<MemoryManagerOSShmAlloc>(); // already called!?
	}

	inline const char *getName() { return "/SharedMemoryManager"; }

	inline pami_result_t init(MemoryManager *mm, size_t bytes,
				  size_t alignment, size_t new_align,
				  unsigned attrs = 0, const char *key = NULL,
				  MM_INIT_FN *init_fn = NULL, void *cookie = NULL) {
		PAMI_abortf("SharedMemoryManager cannot be init()");
		return PAMI_ERROR;
	}

	inline pami_result_t reset(bool force = false) {
#ifdef MM_DEBUG
		if (_debug) {
			MM_DUMP_STATS();
			MM_RESET_STATS();
		}
#endif // MM_DEBUG
		pami_result_t rc = PAMI_SUCCESS;
		// does the OS ensure that memory is zeroed when re-used???
		if (force) { rc = _meta.reset(); } // acquires lock...
		return rc;
	}

	/// \todo How to enforce alignment?
	inline pami_result_t memalign(void **memptr, size_t alignment, size_t bytes,
				      const char *key = NULL,
				      MM_INIT_FN *init_fn = NULL, void *cookie = NULL) {
		//fprintf(stderr,">> SharedMemoryManager::memalign(), this = %p\n", this);
		PAMI_assert_debugf(_attrs == PAMI_MM_NODESCOPE, "SharedMemoryManager not shared");
		// May need to enforce uniquness at a higher level than just this
		// PAMI job. May need to acquire a unique prefix from, say, Mapping
		// that ensures the underlying OS shmem segment will not conflict
		// with any other jobs that might be running.
		//
		char nkey[MMKEYSIZE];
		if (key && key[0]) {
			if (*key == '/') { ++key; } // or... allow "relative" vs. "absolute" keys?
			snprintf(nkey, sizeof(nkey), "/job%zd-%s", _jobid, key);
		} else {
			// this should be unique... or not? memptr might not be...
			snprintf(nkey, sizeof(nkey), "/job%zd-pid%d-%lx", _jobid,
				 getpid(), (unsigned long)memptr);
			if (key) {
				// callers wants to know the unique key we chose...
				strcpy((char *)key, nkey);
			}
		}
		// ... use 'nkey' here-after...
		//
		//fprintf(stderr,"   SharedMemoryManager::memalign(), key = '%s', nkey = '%s'\n", key, nkey);


		if (alignment < _alignment) { alignment = _alignment; }
		void *ptr = NULL;
		bool first = false;
		// note, inital (worst-case) padding now set, when
		// actual pointer assigned below, padding is updated.

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
		MemoryManagerOSShmAlloc *alloc;
		int lrc, fd = -1;
		size_t max = MemoryManagerOSShmAlloc::maxSize(bytes, alignment);

		lrc = shm_open(nkey, O_CREAT | O_EXCL | O_RDWR, 0600);
		first = (lrc != -1); // must be the first...
		//fprintf(stderr,"   SharedMemoryManager::memalign(), lrc = %d, first = %d\n", lrc, first);
		if (first) {
			fd = lrc;
			lrc = ftruncate(fd, max); // this zeroes memory...
			if (lrc == -1) {
				close(fd);
				shm_unlink(nkey); // yes?
				return PAMI_ERROR;
			}
		} else {
			lrc = shm_open(nkey, O_RDWR, 0);
			if (lrc == -1) {
				return PAMI_ERROR;
			}
			fd = lrc;

                        // wait for the shm size to be increased before mmap
                        struct stat st;
                        do {
                            lrc = fstat(fd, &st);
                            if (lrc == -1)
                                return PAMI_ERROR;
                        } while ((size_t)st.st_size != max);
		}
		ptr = mmap(NULL, max, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		close(fd); // no longer needed
		if (ptr == NULL || ptr == MAP_FAILED) {
			// segment is not mapped...
			if (first) { shm_unlink(nkey); } // yes?
			return PAMI_ERROR;
		}
		// shared segment acquired and mapped, now sync and init.
		_meta.acquire(); // only makes this thread-safe, not proc-safe.
		alloc = _meta.findFree(ptr, bytes, alignment, nkey);
		if (alloc == NULL) {
			munmap(ptr, max);
			_meta.release();
			return PAMI_ERROR;
		}

		_meta.release();

		//fprintf(stderr,"memalign(), init_fn = %p, first = %d, nkey = '%s'\n", init_fn, first, nkey);

		if (init_fn) {
			if (first) {
				init_fn(alloc->userMem(), alloc->userSize(),
					alloc->key(), _attrs, cookie);
				alloc->initDone();
			} else {
				alloc->waitDone();
			}
		}
		*memptr = alloc->userMem();
#ifdef MM_DEBUG
		if (_debug) {
			++_num_allocs;
			size_t alloc_bytes = alloc->rawSize();
			if (first) {
				_loc_bytes += alloc_bytes;
			} else {
				_rep_bytes += alloc_bytes;
			}
		}
#endif // MM_DEBUG
		return PAMI_SUCCESS;
	}

	static void last_free(MemoryManagerOSShmAlloc *m) {
		shm_unlink(m->key());
	}

	inline void free(void *mem) {
		_meta.acquire();
		MemoryManagerOSShmAlloc *m = _meta.find(mem);
		if (m) {
#ifdef MM_DEBUG
			if (_debug) {
				++_num_frees;
				_fre_bytes += m->rawSize();
			}
#endif // MM_DEBUG
			munmap(m->rawMem(), m->rawSize());
			_meta.free(m, last_free);
		}
		_meta.release();
	}

	inline size_t available(size_t alignment) {
		// how to tell how much is available???
		// BGQ: getenv(BG_SHAREDMEMSIZE), minus something...
		return (size_t)-1;
	}

	inline void dump(const char *str) {
		if (str) {
			fprintf(stderr, "%s: %s %x\n", str, getName(), _attrs);
		} else {
			fprintf(stderr, "%s %x\n", getName(), _attrs);
		}
#ifdef MM_DEBUG
		if (_debug) {
			MM_DUMP_STATS();
		}
#endif // MM_DEBUG
		_meta.dump(getName());
	}

protected:

	MemoryManagerMeta<MemoryManagerOSShmAlloc> _meta;
	size_t _jobid;
#ifdef MM_DEBUG
	long _debug;
	size_t _num_allocs;
	size_t _num_frees;
	size_t _loc_bytes;
	size_t _rep_bytes;
	size_t _fre_bytes;
#endif // MM_DEBUG

}; // class SharedMemoryManager

}; // namespace Memory
}; // namespace PAMI
#undef TRACE_ERR
#endif // __pami_components_memory_shmem_sharedmemorymanager_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=java --indent=force-tab=8 --indent-preprocessor
// astyle options --indent-col1-comments --max-instatement-indent=79
// astyle options --min-conditional-indent=2 --pad-oper --unpad-paren
// astyle options --pad-header --add-brackets --keep-one-line-blocks
// astyle options --keep-one-line-statements --align-pointer=name --lineend=linux
//
