/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef __components_atomic_bgq_L2Mutex_h__
#define __components_atomic_bgq_L2Mutex_h__

/**
 * \file components/atomic/bgq/L2Mutex.h
 * \brief BGQ L2 Atomics implementation of Node- or Process-scoped Mutexes
 *
 * LockBox is the only implementation, currently. Additional implementations
 * would be new classes, all inheriting from "Mutex".
 *
 *
 */
#include "Global.h"
#include "components/memory/MemoryManager.h"
#include "components/atomic/Mutex.h"
#include <spi/include/l2/atomic.h>

namespace PAMI {
namespace Mutex {
namespace BGQ {
        //
        // These classes are used internally ONLY. See following classes for users
        //
        class L2InPlaceMutex : public PAMI::Atomic::Interface::InPlaceMutex<L2InPlaceMutex> {
        public:
                L2InPlaceMutex() { }
                inline void init_impl() {
                }
                void acquire_impl() {
                        while (L2_AtomicLoadIncrement(&_counter) != 0);
                }
                void release_impl() {
			mem_sync();
                        L2_AtomicLoadClear(&_counter);
                }
                bool tryAcquire_impl() {
                        return (L2_AtomicLoadIncrement(&_counter) == 0);
                }
                bool isLocked_impl() {
                        return (L2_AtomicLoad(&_counter) > 0) ? true : false;
                }
                void *returnLock_impl() { return &_counter; }

		static bool checkCtorMm(PAMI::Memory::MemoryManager *mm) {
			// This is an L2 Atomic object, must have L2 Atomic-mapped memory
			return ((mm->attrs() & PAMI::Memory::PAMI_MM_L2ATOMIC) != 0);
		}
		static bool checkDataMm(PAMI::Memory::MemoryManager *mm) {
			// This is an L2 Atomic object, must have L2 Atomic-mapped memory
			return ((mm->attrs() & PAMI::Memory::PAMI_MM_L2ATOMIC) != 0);
		}
        protected:
                uint64_t _counter;
        }; // class L2InPlaceMutex

        //
        // These classes are used internally ONLY. See following classes for users
        //
        class L2IndirMutex : public PAMI::Atomic::Interface::IndirMutex<L2IndirMutex> {
        public:
                L2IndirMutex() { _counter = NULL; }
                inline void init_impl(PAMI::Memory::MemoryManager *mm, const char *key) {
			PAMI_assert_debugf((mm->attrs() & PAMI::Memory::PAMI_MM_L2ATOMIC) != 0,
				"mm is not L2Atomic-capable");
			PAMI_assert_debugf(!_counter, "Re-init or object is in shmem");
                        pami_result_t rc = mm->memalign((void **)&_counter, 0, 1, key);
                        PAMI_assertf(rc == PAMI_SUCCESS, "Failed to allocate L2 Atomic Mutex");
                }
                void acquire_impl() {
                        while (L2_AtomicLoadIncrement(_counter) != 0);
                }
                void release_impl() {
			mem_sync();
                        L2_AtomicLoadClear(_counter);
                }
                bool tryAcquire_impl() {
                        return (L2_AtomicLoadIncrement(_counter) == 0);
                }
                bool isLocked_impl() {
                        return (L2_AtomicLoad(_counter) > 0) ? true : false;
                }
                void *returnLock_impl() { return _counter; }

		static bool checkCtorMm(PAMI::Memory::MemoryManager *mm) {
			// This is an indirect object, cannot instantiate in shared memory.
			return ((mm->attrs() & PAMI::Memory::PAMI_MM_NODESCOPE) == 0);
		}
		static bool checkDataMm(PAMI::Memory::MemoryManager *mm) {
			// This is an L2 Atomic object, must have L2 Atomic-mapped memory
			return ((mm->attrs() & PAMI::Memory::PAMI_MM_L2ATOMIC) != 0);
		}
        protected:
                uint64_t *_counter;
        }; // class L2IndirMutex

}; // BGQ namespace
}; // Mutex namespace
}; // PAMI namespace

#endif // __components_atomic_bgq_L2Mutex_h__
