/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef __components_atomic_bgq_L2Barrier_h__
#define __components_atomic_bgq_L2Barrier_h__

/**
 * \file components/atomic/bgq/L2Barrier.h
 *
 * \brief BG/Q L2 Atomics implementation of a Barriers
 *
 * These objects must NOT be instantiated in shared memory!
 * Each process may get a different virtual address, so *_counters
 * would get stomped.
 */
#include "components/atomic/Barrier.h"
#include "Global.h"
#include <spi/include/l2/atomic.h>

namespace PAMI {
namespace Barrier {
namespace BGQ {
        /**
         * \brief Base structure for proc-scoped L2 atomics barrier
         *
         * This houses 5 local memory counters which are used to implement
         * a barrier.
         */
        struct L2_Barrier_ctrs {
                uint64_t ctrl_lock;
                uint64_t lock[2];
                uint64_t status[2];
        };

        class _L2_Barrier_s {
        public:
                _L2_Barrier_s() :
		_counters(NULL)
		{ }

		pami_result_t __init(PAMI::Memory::MemoryManager *mm, const char *key) {
			PAMI_assert_debugf(!_counters, "Re-init or object is in shmem");
			return mm->memalign((void **)&_counters, sizeof(uint64_t),
							sizeof(*_counter), key);
		}

                inline uint64_t *controlPtr() { return &_counters->ctrl_lock; }
                inline uint64_t *lockPtr(int n) { return &_counters->lock[n]; }
                inline uint64_t *statusPtr(int n) { return &_counters->status[n]; }

        private:
                L2_Barrier_ctrs *_counters;
        public:
                uint8_t _master;    /**< master participant */
                uint8_t _coreshift; /**< convert core to process for comparing to master */
                uint8_t _nparties;  /**< number of participants */
        };

/*
 * This class cannot be used directly. The super class must allocate the
 * particular type of lockbox based on desired scope.
 */
class _L2Barrier {
public:
        _L2Barrier() { }
        ~_L2Barrier() { }

        inline void init_impl(Memory::MemoryManager *mm, const char *key, size_t z, bool m) {
                PAMI_abortf("_L2Barrier class must be subclass");
        }

        inline pami_result_t enter_impl() {
                pollInit_impl();
                while (poll_impl() != PAMI::Atomic::Interface::Done);
                return PAMI_SUCCESS;
        }

        inline void enterPoll_impl(PAMI::Atomic::Interface::pollFcn fcn, void *arg) {
                pollInit_impl();
                while (poll_impl() != PAMI::Atomic::Interface::Done) {
                        fcn(arg);
                }
        }

        inline void pollInit_impl() {
                uint64_t lockup;
                mem_sync();
                lockup = L2_AtomicLoad(_barrier.controlPtr());
                L2_AtomicLoadIncrement(_barrier.lockPtr(lockup));
                _data = (void*)lockup;
                _status = PAMI::Atomic::Interface::Entered;
        }

        inline PAMI::Atomic::Interface::barrierPollStatus poll_impl() {
                PAMI_assert(_status == PAMI::Atomic::Interface::Entered);
                uint64_t lockup, value;
                lockup = (uint64_t)_data;
                if (L2_AtomicLoad(_barrier.lockPtr(lockup)) < _barrier._nparties) {
                        return PAMI::Atomic::Interface::Entered;
                }

                // All cores have participated in the barrier
                // We need all cores to block until checkin
                // to clear the lock atomically
                L2_AtomicLoadIncrement(_barrier.lockPtr(lockup));
                do {
                        value = L2_AtomicLoad(_barrier.lockPtr(lockup));
                } while (value > 0 && value < (unsigned)(2 * _barrier._nparties));

                if ((Kernel_PhysicalProcessorID() >> _barrier._coreshift) == _barrier._master) {
                        if (lockup) {
                                L2_AtomicLoadDecrement(_barrier.controlPtr());
                        } else {
                                L2_AtomicLoadIncrement(_barrier.controlPtr());
                        }
                        L2_AtomicLoadClear(_barrier.statusPtr(lockup));
                        L2_AtomicLoadClear(_barrier.lockPtr(lockup));
                } else {
                        // wait until master releases the barrier by clearing the lock
                        while (L2_AtomicLoad(_barrier.lockPtr(lockup)) > 0);
                }
                _status = PAMI::Atomic::Interface::Initialized;
                return PAMI::Atomic::Interface::Done;
        }
        // With 5 lockboxes used... which one should be returned?
        inline void *returnBarrier_impl() { return _barrier.controlPtr(); }
protected:
        _L2_Barrier_s _barrier;
        void *_data;
        PAMI::Atomic::Interface::barrierPollStatus _status;
}; // class _L2Barrier

class L2NodeCoreBarrier :
                public PAMI::Atomic::Interface::IndirBarrier<L2NodeCoreBarrier>,
                public _L2Barrier {
public:
        L2NodeCoreBarrier() {}
        ~L2NodeCoreBarrier() {}
        inline void init_impl(PAMI::Memory::MemoryManager *mm, const char *key, size_t z, bool m) {
                // For core-granularity, everything is
                // a core number. Assume the master core
                // is the lowest-numbered core in the
                // process.

		pami_result_t rc = _barrier.__init(mm, key);
		PAMI_assertf(rc == PAMI_SUCCESS, "Failed to get L2 Atomic NodeCoreBarrier");

                // PAMI_assert(m .iff. me == masterProc());
                _barrier._master = __global.l2atomicFactory.masterProc() << __global.l2atomicFactory.coreShift();
                _barrier._coreshift = 0;
                _barrier._nparties = __global.l2atomicFactory.numCore();
                _status = PAMI::Atomic::Interface::Initialized;
        }
}; // class L2NodeCoreBarrier

class L2NodeProcBarrier :
                public PAMI::Atomic::Interface::IndirBarrier<L2NodeProcBarrier>,
                public _L2Barrier {
public:
        L2NodeProcBarrier() {}
        ~L2NodeProcBarrier() {}
        inline void init_impl(PAMI::Memory::MemoryManager *mm, const char *key, size_t z, bool m) {
                // For proc-granularity, must convert
                // between core id and process id,
                // and only one core per process will
                // participate.

		pami_result_t rc = _barrier.__init(mm, key);
		PAMI_assertf(rc == PAMI_SUCCESS, "Failed to get L2 Atomic NodeProcBarrier");

                // PAMI_assert(m .iff. me == masterProc());
                _barrier._master = __global.l2atomicFactory.coreXlat(__global.l2atomicFactory.masterProc()) >> __global.l2atomicFactory.coreShift();
                _barrier._coreshift = __global.l2atomicFactory.coreShift();
                _barrier._nparties = __global.l2atomicFactory.numProc();
                _status = PAMI::Atomic::Interface::Initialized;
        }
}; // class L2NodeProcBarrier

}; // BGQ namespace
}; // Barrier namespace
}; // PAMI namespace

#endif // __components_atomic_bgq_L2Barrier_h__
