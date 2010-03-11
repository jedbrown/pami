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
#include "SysDep.h"
#include "components/atomic/Mutex.h"
#include <spi/include/l2/atomic.h>

namespace XMI {
namespace Mutex {
namespace BGQ {
	//
	// These classes are used internally ONLY. See following classes for users
	//
	class L2ProcMutex : public Interface::Mutex<L2ProcMutex> {
	public:
		L2ProcMutex() { }
		inline void init_impl(XMI::SysDep *sd) {
		}
		void acquire_impl() {
			while (L2_AtomicLoadIncrement(&_counter) != 0);
		}
		void release_impl() {
			L2_AtomicLoadClear(&_counter);
		}
		bool tryAcquire_impl() {
			return (L2_AtomicLoadIncrement(&_counter) == 0);
		}
		bool isLocked_impl() {
			return (L2_AtomicLoad(&_counter) > 0) ? true : false;
		}
		void *returnLock_impl() { return &_counter; }
	protected:
		uint64_t _counter;
	}; // class L2ProcMutex

	class L2NodeMutex : public Interface::Mutex<L2NodeMutex> {
	public:
		L2NodeMutex() { }
		inline void init_impl(XMI::SysDep *sd) {
			xmi_result_t rc = sd->mm.memalign((void **)&_counter,
						L1D_CACHE_LINE_SIZE,
						sizeof(*_counter));
			XMI_assertf(rc == XMI_SUCCESS,
				"Failed to allocate shared memory for Node Mutes");
		}
		void acquire_impl() {
			while (L2_AtomicLoadIncrement(_counter) != 0);
		}
		void release_impl() {
			L2_AtomicLoadClear(_counter);
		}
		bool tryAcquire_impl() {
			return (L2_AtomicLoadIncrement(_counter) == 0);
		}
		bool isLocked_impl() {
			return (L2_AtomicLoad(_counter) > 0) ? true : false;
		}
		void *returnLock_impl() { return _counter; }
	protected:
		uint64_t _counter;
	}; // class L2NodeMutex

}; // BGQ namespace
}; // Mutex namespace
}; // XMI namespace

#endif // __components_atomic_bgq_L2Mutex_h__
