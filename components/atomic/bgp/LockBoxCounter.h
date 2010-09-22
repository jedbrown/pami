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
#include <pami.h>
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
        class LockBoxCounter : public PAMI::Atomic::Interface::IndirCounter<LockBoxCounter> {
        public:
                LockBoxCounter() { _addr = NULL; }
                ~LockBoxCounter() {}
		static bool checkCtorMm(PAMI::Memory::MemoryManager *mm) {
			return true;	// all participants get the same lockbox id,
					// so memory type doesn't matter.
		}
		static bool checkDataMm(PAMI::Memory::MemoryManager *mm) {
			return true; // for now, until a lockbox mm is done
			// return ((mm->attrs() & PAMI::Memory::PAMI_MM_LOCKBOX) != 0);
		}
                inline void init_impl(PAMI::Memory::MemoryManager *mm, const char *key) {
                        __global.lockboxFactory.lbx_alloc(&_addr, 1,
					key ? PAMI::Atomic::BGP::LBX_NODE_SCOPE : PAMI::Atomic::BGP::LBX_PROC_SCOPE);
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
                inline void clear_impl() {
                        LockBox_FetchAndClear((LockBox_Counter_t)_addr);
                }
/* no such thing exists for BG/P lockboxes.
                inline bool compare_and_swap_impl(size_t compare, size_t swap) {
                }
*/
                void *returnLock_impl() { return _addr; }
        protected:
                void *_addr;
        }; // class LockBoxCounter

}; // BGP namespace
}; // Counter namespace
}; // PAMI namespace

#endif // __components_atomic_bgp_lockboxcounter_h__
