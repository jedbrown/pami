/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef __components_atomic_bgq_WaitRsvMutex_h__
#define __components_atomic_bgq_WaitRsvMutex_h__

/**
 * \file components/atomic/bgq/WaitRsvMutex.h
 * \brief BGQ WaitRsv Atomics implementation of Node- or Process-scoped Mutexes
 *
 * This uses PPC lwarx/stwcx instructions to implement the mutex, and
 * uses ppc_waitrsv() to "spin" on the lock.
 *
 *
 */
#include "Global.h"
#include "Memory.h"
#include "components/memory/MemoryManager.h"
#include "components/atomic/Mutex.h"
#include "hwi/include/bqc/A2_inlines.h"

namespace PAMI {
namespace Mutex {
namespace BGQ {
        class WaitRsvInPlaceMutex : public PAMI::Atomic::Interface::InPlaceMutex<WaitRsvInPlaceMutex> {
        public:
                WaitRsvInPlaceMutex() { }
                inline void init_impl() {
                }
                void acquire_impl() {
			do {
				while (LoadReserved(&_mutex) != 0) {
					ppc_waitrsv();
				}
			} while (!StoreConditional(&_mutex, 1));
                }
                void release_impl() {
			Memory::sync();
                        _mutex = 0;
                }
                bool tryAcquire_impl() {
			if (LoadReserved(&_mutex) != 0 ||
					!StoreConditional(&_mutex, 1)) {
				return false;
			}
			return true;
                }
                bool isLocked_impl() {
                        return (_mutex > 0) ? true : false;
                }
                void *returnLock_impl() { return &_mutex; }

		static bool checkCtorMm(PAMI::Memory::MemoryManager *mm) {
			return true;
		}
		static bool checkDataMm(PAMI::Memory::MemoryManager *mm) {
			return true;
		}
        protected:
                uint64_t _mutex;
        }; // class WaitRsvInPlaceMutex

	// The user might as well do this, then only InPlace types exist.
	//
        class WaitRsvIndirMutex : public PAMI::Atomic::Interface::IndirMutex<L2IndirMutex> {
        public:
                WaitRsvIndirMutex() { _mutex = NULL; }
                inline void init_impl(PAMI::Memory::MemoryManager *mm, const char *key) {
			PAMI_assert_debugf(!_mutex, "Re-init or object is in shmem");
                        pami_result_t rc;
                        rc = mm->memalign((void **)&_mutex, 0, sizeof(*_mutex), key);
                        PAMI_assertf(rc == PAMI_SUCCESS, "Failed to allocate WaitRsvIndirMutex");
			_mutex->init();
                }
                void acquire_impl() {
			_mutex->acquire();
                }
                void release_impl() {
			_mutex->release();
                }
                bool tryAcquire_impl() {
                        return _mutex->tryAcquire();
                }
                bool isLocked_impl() {
                        return _mutex->isLocked();
                }
                void *returnLock_impl() { return _mutex->returnLock(); }

		static bool checkCtorMm(PAMI::Memory::MemoryManager *mm) {
			// This is an indirect object, cannot instantiate in shared memory.
			return (WaitRsvInPlaceMutex::checkCtorMm(mm) &&
				(mm->attrs() & PAMI::Memory::PAMI_MM_NODESCOPE) == 0);
		}
		static bool checkDataMm(PAMI::Memory::MemoryManager *mm) {
			return WaitRsvInPlaceMutex::checkDataMm(mm);
		}
        protected:
                WaitRsvInPlaceMutex *_mutex;
        }; // class WaitRsvIndirMutex

}; // BGQ namespace
}; // Mutex namespace
}; // PAMI namespace

#endif // __components_atomic_bgq_WaitRsvMutex_h__
