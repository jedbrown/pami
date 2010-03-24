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

namespace PAMI {
namespace Mutex {
namespace BGQ {
	//
	// These classes are used internally ONLY. See following classes for users
	//
	template <class T_Mutex>
	class _L2Mutex : public PAMI::Atomic::Interface::Mutex<T_Mutex> {
	public:
		_L2Mutex() { }
		inline void __init(PAMI::Memory::MemoryManager *mm,
					PAMI::Atomic::BGQ::l2x_scope_t scope) {
			pami_result_t rc = __global.l2atomicFactory.l2x_alloc((void **)&_counter,
									1, scope);
			PAMI_assertf(rc == PAMI_SUCCESS, "Failed to allocate L2 Atomic Mutex");
			// if need to reset, must coordinate!
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
		uint64_t *_counter;
	}; // class _L2Mutex

	//
	// These classes are used internally ONLY. See following classes for users
	//
	class L2ProcMutex : public _L2Mutex<L2ProcMutex> {
	public:
		L2ProcMutex() { }
		inline void init_impl(PAMI::Memory::MemoryManager *mm) {
			__init(mm, PAMI::Atomic::BGQ::L2A_PROC_SCOPE);
		}
	protected:
	}; // class L2ProcMutex

	class L2NodeMutex : public _L2Mutex<L2NodeMutex> {
	public:
		L2NodeMutex() { }
		inline void init_impl(PAMI::Memory::MemoryManager *mm) {
			__init(mm, PAMI::Atomic::BGQ::L2A_NODE_SCOPE);
		}
	protected:
	}; // class L2NodeMutex

}; // BGQ namespace
}; // Mutex namespace
}; // PAMI namespace

#endif // __components_atomic_bgq_L2Mutex_h__
