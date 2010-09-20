/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/atomic/bgq/L2Counter.h
 * \brief BG/Q L2 Atomics implementation of node- and process-scoped atomic counters
 */

#ifndef __components_atomic_bgq_L2Counter_h__
#define __components_atomic_bgq_L2Counter_h__

#include "components/atomic/Counter.h"
#include "util/common.h"
#include <spi/include/l2/atomic.h>
#include "Global.h"

namespace PAMI {
namespace Counter {
namespace BGQ {

///
/// \brief CRTP interface for BG/Q L2 Atomics builtins atomic objects.
///
template <class T_Counter>
class L2Counter : public PAMI::Atomic::Interface::IndirCounter<T_Counter> {
public:
        L2Counter() :
	PAMI::Atomic::Interface::IndirCounter<T_Counter>(),
	_counter(NULL)
        {}

        ~L2Counter() {}

        /// \see PAMI::Atomic::Interface::Counter::init
        void init_impl(PAMI::Memory::MemoryManager *mm, const char *key) {
		PAMI_assert_debugf((mm->attrs() & PAMI::Memory::PAMI_MM_L2ATOMIC) != 0,
			"mm is not L2-Atomic capable");
		PAMI_assert_debugf(!_counter, "Re-init or object is in shmem");
                pami_result_t rc = mm->memalign((void **)&_counter, sizeof(*_counter),
							sizeof(*_counter), key);
                PAMI_assertf(rc == PAMI_SUCCESS, "Failed to get L2 Atomic Counter");
        }

        /// \see PAMI::Atomic::Interface::Counter::fetch
        inline size_t fetch_impl() {
                return L2_AtomicLoad(_counter);
        }

        /// \see PAMI::Atomic::Interface::Counter::fetch_and_inc
        inline size_t fetch_and_inc_impl() {
                return L2_AtomicLoadIncrement(_counter);
        }

        /// \see PAMI::Atomic::Interface::Counter::fetch_and_dec
        inline size_t fetch_and_dec_impl() {
                return L2_AtomicLoadDecrement(_counter);
        }

        /// \see PAMI::Atomic::Interface::Counter::fetch_and_clear
        inline size_t fetch_and_clear_impl() {
                return L2_AtomicLoadClear(_counter);
        }

        /// \see PAMI::Atomic::Interface::Counter::fetch_and_clear
        inline void clear_impl() {
                L2_AtomicLoadClear(_counter);
        }

        /// \see PAMI::Atomic::Interface::Counter::compare_and_swap
        /// Since BG/Q L2 Atomics don't implement compare-and-swap, we use
        /// the GCC builtin and hope for the best.
        inline bool compare_and_swap_impl(size_t compare, size_t swap) {
                return __sync_bool_compare_and_swap(&_counter, compare, swap);
        }

        inline void *returnLock_impl() { return (void *)_counter; }

protected:

        volatile uint64_t *_counter;

}; // class L2Counter

}; // namespace BGQ
}; // namespace Atomic
};   // namespace PAMI

#endif // __components_atomic_bgq_L2Counter_h__
