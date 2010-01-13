/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef __components_atomic_gcc_GccCounter_h__
#define __components_atomic_gcc_GccCounter_h__

/**
 * \file components/atomic/gcc/GccCounter.h
 * \brief gcc builtin atomics implementation of Node- or Process-scoped Counter
 *
 */
#include "components/atomic/Counter.h"
#include "components/atomic/gcc/GccBuiltin.h"

namespace XMI {
namespace Counter {

	class GccNodeCounter : public XMI::Atomic::Interface::Counter<GccNodeCounter> {
	public:
		GccNodeCounter() {}
		~GccNodeCounter() {}
		inline void init_impl(XMI::SysDep *sd) {
			sd->mm.memalign((void **)&_addr, sizeof(*_addr), sizeof(*_addr));
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

	class GccProcCounter : public XMI::Atomic::Interface::Counter<GccProcCounter> {
	public:
		GccProcCounter() {}
		~GccProcCounter() {}
		inline void init_impl(XMI::SysDep *sd) {
			_addr.init(sd);
		}
		inline size_t fetch_impl() {
			return _addr.fetch();
		}
		inline size_t fetch_and_inc_impl() {
			return _addr.fetch_and_inc();
		}
		inline size_t fetch_and_dec_impl() {
			return _addr.fetch_and_dec();
		}
		inline size_t fetch_and_clear_impl() {
			return _addr.fetch_and_clear();
		}
		void *returnLock_impl() { return _addr.returnLock(); }
	protected:
		XMI::Atomic::GccBuiltin _addr;
	}; // class GccProcCounter

}; // Counter namespace
}; // XMI namespace

#endif // __components_atomic_gcccounter_h__
