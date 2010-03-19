/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef __components_atomic_counter_CounterMutex_h__
#define __components_atomic_counter_CounterMutex_h__

/**
 * \file components/atomic/counter/CounterMutex.h
 * \brief generic Atomic Counter implementation of Node- or Process-scoped Mutexes
 *
 * This may not be quite right yet - might not work for things like LockBoxCounter.
 */
#include "components/atomic/Mutex.h"

namespace XMI {
namespace Mutex {

	template <class T_Counter>
	class CounterMutex : public XMI::Atomic::Interface::Mutex<CounterMutex<T_Counter> > {
	public:
		CounterMutex() {}
		~CounterMutex() {}
		inline void init_impl(XMI::Memory::MemoryManager *mm) {
			_counter.init(mm);
		}
		void acquire_impl() {
			while (_counter.fetch_and_inc() != 0);
		}
		void release_impl() {
			_counter.fetch_and_clear();
		}
		bool tryAcquire_impl() {
			return (_counter.fetch_and_inc() == 0) ? true : false;
		}
		bool isLocked_impl() {
			return (_counter.fetch() > 0) ? true : false;
		}
		void *returnLock_impl() { return _counter.returnLock(); }
	protected:
		T_Counter _counter;
	}; // class CounterMutex

}; // Mutex namespace
}; // XMI namespace

#endif // __components_atomic_countermutex_h__
