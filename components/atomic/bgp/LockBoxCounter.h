/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
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
#include "sys/pami.h"
#include "components/atomic/Counter.h"
#include "Global.h"

#include <spi/bgp_SPI.h>
#include <bpcore/bgp_atomic_ops.h>

namespace PAMI {
namespace Counter {
namespace BGP {
	//
	// This class is used internally ONLY. See following classes for users
	//
	class _LockBoxCounter {
	public:
		_LockBoxCounter() { _addr = NULL; }
		inline void init_impl(PAMI::Memory::MemoryManager *mm) {
			PAMI_abortf("_LockBoxCounter must be a subclass");
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

	class LockBoxNodeCounter : public _LockBoxCounter,
				 public PAMI::Atomic::Interface::Counter<LockBoxNodeCounter> {
	public:
		LockBoxNodeCounter() {}
		~LockBoxNodeCounter() {}
		inline void init_impl(PAMI::Memory::MemoryManager *mm) {
			__global.lockboxFactory.lbx_alloc(&this->_addr, 1, PAMI::Atomic::BGP::LBX_NODE_SCOPE);
		}
	}; // class LockBoxNodeCounter

	class LockBoxProcCounter : public _LockBoxCounter,
				 public PAMI::Atomic::Interface::Counter<LockBoxProcCounter> {
	public:
		LockBoxProcCounter() {}
		~LockBoxProcCounter() {}
		inline void init_impl(PAMI::Memory::MemoryManager *mm) {
			__global.lockboxFactory.lbx_alloc(&this->_addr, 1, PAMI::Atomic::BGP::LBX_PROC_SCOPE);
		}
	}; // class LockBoxProcCounter

}; // BGP namespace
}; // Counter namespace
}; // PAMI namespace

#endif // __components_atomic_bgp_lockboxcounter_h__
