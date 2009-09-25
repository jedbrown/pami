/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef __components_atomic_gcccounter_h__
#define __components_atomic_gcccounter_h__

/**
 * \file components/atomic/gcc/GccCounter.h
 * \brief gcc builtin atomics implementation of Node- or Process-scoped Counter
 *
 */
#include "components/atomic/Counter.h"
#include "components/atomic/gcc/GccBuiltin.h"

namespace XMI {
namespace Counter {

	template <class T_Sysdep>
	class GccNodeCounter : public XMI::Atomic::Interface::Counter<T_Sysdep, GccNodeCounter<T_Sysdep> > {
	public:
		GccNodeCounter() {}
		~GccNodeCounter() {}
		inline void init_impl(T_Sysdep *sd) {
			sd->mm.memalign(&_addr, sizeof(*_addr), sizeof(*_addr));
			_addr->init(sd);
		}
		inline size_t fetch_impl() {
			return _addr->fetch();
		}
		inline size_t fetch_and_inc_impl() {
			return _addr->fetch_and_inc();
		}
		inline size_t fetch_and_dec_impl() {
			return _addr->fetch_and_dec();
		}
		inline size_t fetch_and_clear_impl() {
			return _addr->fetch_and_clear();
		}
		void *returnLock_impl() { return _addr->returnLock(); }
	protected:
		XMI::Atomic::GccBuiltin *_addr;
	}; // class GccNodeCounter

	template <class T_Sysdep>
	class GccProcCounter : public XMI::Atomic::Interface::Counter<T_Sysdep, GccProcCounter<T_Sysdep> > {
	public:
		GccProcCounter() {}
		~GccProcCounter() {}
		inline void init_impl(T_Sysdep *sd) {
			_addr.init(sd);
		}
		inline size_t fetch_impl() {
			return _add.fetch();
		}
		inline size_t fetch_and_inc_impl() {
			return _add.fetch_and_inc();
		}
		inline size_t fetch_and_dec_impl() {
			return _add.fetch_and_dec();
		}
		inline size_t fetch_and_clear_impl() {
			return _add.fetch_and_clear();
		}
		void *returnLock_impl() { return _addr.returnLock(); }
	protected:
		XMI::Atomic::GccBuiltin _addr;
	}; // class GccProcCounter

}; // Counter namespace
}; // XMI namespace

#endif // __components_atomic_gcccounter_h__
