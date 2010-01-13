/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef __components_atomic_bgp_LwarxStwcxMutex_h__
#define __components_atomic_bgp_LwarxStwcxMutex_h__

/**
 * \file components/atomic/bgp/LwarxStwcxMutex.h
 * \brief BGP implementation of Node- or Process-scoped Mutexes
 *
 * LockBox is the only implementation, currently. Additional implementations
 * would be new classes, all inheriting from "Mutex".
 *
 *
 */
#include "SysDep.h"
#include "components/atomic/Mutex.h"
#include <spi/bgp_SPI.h>
#include <bpcore/bgp_atomic_ops.h>

namespace XMI {
namespace Atomic {
namespace BGP {
	class LwarxStwcxProcMutex {
		// in order to avoid excessive size due to ALIGN_L1D_CACHE alignment of
		// _BGP_Atomic, we use the recommended 8B alignment here.
		typedef struct {
			volatile uint32_t atom;
		} __attribute__ ((aligned(8))) _BGPLM_Atomic;
	public:
		LwarxStwcxProcMutex() { _addr.atom = 0; }
		inline void init_impl() {
			_addr.atom = 0;
		}
		inline void acquire_impl() {
			while (!_bgp_test_and_set((_BGP_Atomic *)&_addr, 1));
		}
		inline void release_impl() {
			_addr.atom = 0;
		}
		inline bool tryAcquire_impl() {
			return (_bgp_test_and_set((_BGP_Atomic *)&_addr, 1) != 0);
		}
		inline bool isLocked_impl() {
			return (_addr.atom != 0);
		}
		inline void *returnLock_impl() { return (void *)&_addr.atom; }
	private:
		_BGPLM_Atomic _addr;
	}; // class LwarxStwcxProcMutex

	class LwarxStwcxNodeMutex {
	public:
		LwarxStwcxNodeMutex() { _addr = NULL; }
		inline void init_impl() {
			_addr = (_BGP_Atomic *)sd->memoryManager().scratchpad_dynamic_area_memalign(sizeof(long), sizeof(*_addr));
		}
		inline void acquire_impl() {
			while (!_bgp_test_and_set(_addr, 1));
		}
		inline void release_impl() {
			_addr->atom = 0;
		}
		inline bool tryAcquire_impl() {
			return (_bgp_test_and_set(_addr, 1) != 0);
		}
		inline bool isLocked_impl() {
			return (_addr->atom != 0);
		}
		inline void *returnLock_impl() { return (void *)&_addr->atom; }
	private:
		_BGP_Atomic *_addr;
	}; // class LwarxStwcxNodeMutex

}; // BGP namespace
}; // Atomic namespace
}; // XMI namespace

#endif // __components_atomic_bgp_lwarxstwcxmutex_h__
