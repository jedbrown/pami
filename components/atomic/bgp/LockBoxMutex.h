/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef __components_atomic_bgp_lockboxmutex_h__
#define __components_atomic_bgp_lockboxmutex_h__

/**
 * \file components/atomic/bgp/LockBoxMutex.h
 * \brief BGP implementation of Node- or Process-scoped Mutexes
 *
 * LockBox is the only implementation, currently. Additional implementations
 * would be new classes, all inheriting from "Mutex".
 *
 *
 */
#include "components/sysdep/SysDep.h"
#include "components/sysdep/bgp/LockBoxFactory.h"
#include "components/atomic/Mutex.h"
#include <spi/bgp_SPI.h>
#include <bpcore/bgp_atomic_ops.h>

#ifndef __defined__tsc__
static inline unsigned long __tsc() {
	unsigned long tsc;
	asm volatile ("mfspr %0,%1" : "=r" (tsc) : "i" (SPRN_TBRL));
	return tsc;
}
#endif /* ! __defined__tsc__ */

namespace XMI {
namespace Atomic {
	class _LockBoxMutex {
	public:
		_LockBoxMutex() { _addr = NULL; }
		inline void init_impl() {
			XMI_abortf("_LockBoxMutex must be a subclass");
		}
		void acquire_impl() {
			LockBox_MutexLock((LockBox_Mutex_t)_addr);
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
	}; // class _LockBoxMutex

	class _FairLockBoxMutex {
	public:
		_FairLockBoxMutex() { _addr = NULL; }
		inline void init_impl() {
			XMI_abortf("_FairLockBoxMutex must be a subclass");
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

	class LockBoxNodeMutex : public _LockBoxMutex {
	public:
		LockBoxNodeMutex() : _LockBoxMutex() {}
		~LockBoxNodeMutex() {}
		inline void init_impl() {
			XMI::Atomic::BGP::LockBoxFactory::lbx_alloc(&_addr, 1, XMI::Atomic::BGP::LBX_NODE_SCOPE);
		}
	}; // class LockBoxNodeMutex

	class LockBoxProcMutex : public _LockBoxMutex {
	public:
		LockBoxProcMutex() : _LockBoxMutex() {}
		~LockBoxProcMutex() {}
		inline void init_impl() {
			XMI::Atomic::BGP::LockBoxFactory::lbx_alloc(&_addr, 1, XMI::Atomic::BGP::LBX_PROC_SCOPE);
		}
	}; // class LockBoxProcMutex

	class FairLockBoxNodeMutex : public _FairLockBoxMutex {
	public:
		FairLockBoxNodeMutex() : _FairLockBoxMutex() {}
		~FairLockBoxNodeMutex() {}
		inline void init_impl() {
			XMI::Atomic::BGP::LockBoxFactory::lbx_alloc(&_addr, 1, XMI::Atomic::BGP::LBX_NODE_SCOPE);
		}
	}; // class FairLockBoxNodeMutex

	class FairLockBoxProcMutex : public _FairLockBoxMutex {
	public:
		FairLockBoxProcMutex() : _FairLockBoxMutex() {}
		~FairLockBoxProcMutex() {}
		inline void init_impl() {
			XMI::Atomic::BGP::LockBoxFactory::lbx_alloc(&_addr, 1, XMI::Atomic::BGP::LBX_PROC_SCOPE);
		}
	}; // class FairLockBoxProcMutex

}; // Atomic namespace
}; // XMI namespace

#endif // __components_atomic_bgp_lockboxmutex_h__
