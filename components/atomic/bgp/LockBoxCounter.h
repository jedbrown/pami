/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef __components_atomic_bgp_LockBoxCounter_h__
#define __components_atomic_bgp_LockBoxCounter_h__

/**
 * \file components/atomic/bgp/LockBoxCounter.h
 * \brief BGP implementation of Node- or Process-scoped Counter
 *
 * LockBox is the only implementation, currently. Additional implementations
 * would be new classes, all inheriting from "Counter".
 *
 *
 */
#include "sys/xmi.h"
#include "components/atomic/Counter.h"
#include "Global.h"

#include <spi/bgp_SPI.h>
#include <bpcore/bgp_atomic_ops.h>

namespace XMI {
namespace Counter {
	//
	// This class is used internally ONLY. See following classes for users
	//
	template <class T_Sysdep>
	class _LockBoxCounter {
	public:
		_LockBoxCounter() { _addr = NULL; }
		inline void init_impl(T_Sysdep *sd) {
			XMI_abortf("_LockBoxCounter must be a subclass");
		}
		inline size_t fetch_impl() {
			return LockBox_Query((LockBox_Counter_t)_addr);
		}
		inline size_t fetch_and_inc_impl() {
			return LockBox_FetchAndInc((LockBox_Counter_t)_addr);
		}
		inline size_t fetch_and_dec_impl() {
			return LockBox_FetchAndDec((LockBox_Counter_t)_addr);
		}
		inline size_t fetch_and_clear_impl() {
			return LockBox_FetchAndClear((LockBox_Counter_t)_addr);
		}
/* no such thing exists for BG/P lockboxes.
		inline bool compare_and_swap_impl(size_t compare, size_t swap) {
		}
*/
		void *returnLock_impl() { return _addr; }
	protected:
		void *_addr;
	}; // class _LockBoxCounter

	//
	// Here are the actual classes to be used:
	//

	template <class T_Sysdep>
	class LockBoxNodeCounter : public _LockBoxCounter<T_Sysdep>,
				 public XMI::Atomic::Interface::Counter<T_Sysdep, LockBoxNodeCounter<T_Sysdep> > {
	public:
		LockBoxNodeCounter() {}
		~LockBoxNodeCounter() {}
		inline void init_impl(T_Sysdep *sd) {
			__global.lockboxFactory.lbx_alloc(&this->_addr, 1, XMI::Atomic::BGP::LBX_NODE_SCOPE);
		}
	}; // class LockBoxNodeCounter

	template <class T_Sysdep>
	class LockBoxProcCounter : public _LockBoxCounter<T_Sysdep>,
				 public XMI::Atomic::Interface::Counter<T_Sysdep, LockBoxProcCounter<T_Sysdep> > {
	public:
		LockBoxProcCounter() {}
		~LockBoxProcCounter() {}
		inline void init_impl(T_Sysdep *sd) {
			__global.lockboxFactory.lbx_alloc(&this->_addr, 1, XMI::Atomic::BGP::LBX_PROC_SCOPE);
		}
	}; // class LockBoxProcCounter

}; // Counter namespace
}; // XMI namespace

#endif // __components_atomic_bgp_lockboxcounter_h__
