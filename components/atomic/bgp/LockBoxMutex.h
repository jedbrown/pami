/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef __components_atomic_bgp_LockBoxMutex_h__
#define __components_atomic_bgp_LockBoxMutex_h__

/**
 * \file components/atomic/bgp/LockBoxMutex.h
 * \brief BGP implementation of Node- or Process-scoped Mutexes
 *
 * LockBox is the only implementation, currently. Additional implementations
 * would be new classes, all inheriting from "Mutex".
 *
 *
 */
#include "Global.h"
#include "common/bgp/LockBoxFactory.h"
#include "components/atomic/Mutex.h"
#include <spi/bgp_SPI.h>
#include <bpcore/bgp_atomic_ops.h>

#ifndef __defined__tsc__
#define __defined__tsc__
static inline unsigned long __tsc() {
        unsigned long tsc;
        asm volatile ("mfspr %0,%1" : "=r" (tsc) : "i" (SPRN_TBRL));
        return tsc;
}
#endif /* ! __defined__tsc__ */

namespace PAMI {
namespace Mutex {
namespace BGP {
        //
        // These classes are used internally ONLY. See following classes for users
        //

        class LockBoxMutex : public PAMI::Atomic::Interface::IndirMutex<LockBoxMutex> {
        public:
                LockBoxMutex() { _addr = NULL; }
                ~LockBoxMutex() {}
                inline void init_impl(PAMI::Memory::MemoryManager *mm, const char *key) {
			PAMI_assert_debugf(!_addr, "Re-init or object is in shmem");
                        __global.lockboxFactory.lbx_alloc(&this->_addr, 1,
				key ? PAMI::Atomic::BGP::LBX_NODE_SCOPE : PAMI::Atomic::BGP::LBX_PROC_SCOPE);
                }
                void acquire_impl() {
                        LockBox_MutexLock((LockBox_Mutex_t)_addr);
			mem_sync();
                }
                void release_impl() {
                        LockBox_MutexUnlock((LockBox_Mutex_t)_addr);
                }
                bool tryAcquire_impl() {
                        return (LockBox_MutexTryLock((LockBox_Mutex_t)_addr) == 0) ? true : false;
                }
                bool isLocked_impl() {
                        return (LockBox_Query((LockBox_Counter_t)_addr) > 0) ? true : false;
                }
                void *returnLock_impl() { return _addr; }
        protected:
                void *_addr;
        }; // class LockBoxMutex


        class FairLockBoxMutex : public PAMI::Atomic::Interface::IndirMutex<FairLockBoxMutex> {
        public:
                FairLockBoxMutex() { _addr = NULL; }
                ~FairLockBoxMutex() {}
                inline void init_impl(PAMI::Memory::MemoryManager *mm, const char *key) {
			PAMI_assert_debugf(!_addr, "Re-init or object is in shmem");
                        __global.lockboxFactory.lbx_alloc(&this->_addr, 1,
				key ? PAMI::Atomic::BGP::LBX_NODE_SCOPE : PAMI::Atomic::BGP::LBX_PROC_SCOPE);
                }
                void acquire_impl() {
                        // LockBox_MutexLock((LockBox_Mutex_t)_addr);
                        //
                        // There is some unknown problem with lockbox mutexes whereby
                        // some contenders get "shut-out" and effectively never get
                        // the mutex. The following back-off scheme seems to eliminate
                        // the problem. If we don't get the lock on the first try,
                        // we delay a short time to keep from swamping the lockbox.
                        // We continue trying and delaying, however if we detect that
                        // someone else got the lock instead of us (we see the lock value
                        // decrease - or not increase), then we reduce our delay amount.
                        // This means that if we wait too long, we effectively have no
                        // delay, while other contenders that got the lock recently
                        // will be delaying by the full amount and thus be at a greater
                        // disadvantage compared to us. This imbalance of traffic seems
                        // to be enough to "break the spell" and allow us to get the lock.
                        //
                        unsigned long lbv, lst = 0;
                        unsigned long td = 127;
                        while ((lbv = LockBox_FetchAndInc((LockBox_Counter_t)_addr)) != 0) {
                                if (td && lbv <= lst) --td;
                                lst = lbv;
                                if (td) {
                                        unsigned long t0 = __tsc();
                                        while (__tsc() - t0 < td);
                                }
                        }
                }
                void release_impl() {
                        LockBox_MutexUnlock((LockBox_Mutex_t)_addr);
                }
                bool tryAcquire_impl() {
                        return (LockBox_MutexTryLock((LockBox_Mutex_t)_addr) == 0) ? true : false;
                }
                bool isLocked_impl() {
                        return (LockBox_Query((LockBox_Counter_t)_addr) > 0) ? true : false;
                }
                void *returnLock_impl() { return _addr; }
        protected:
                void *_addr;
        }; // class _FairLockBoxMutex

}; // BGP namespace
}; // Mutex namespace
}; // PAMI namespace

#endif // __components_atomic_bgp_lockboxmutex_h__
