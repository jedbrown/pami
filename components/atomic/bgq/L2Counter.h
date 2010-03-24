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
class _L2Counter : public PAMI::Atomic::Interface::Counter<T_Counter> {
public:
        _L2Counter() :
                PAMI::Atomic::Interface::Counter<T_Counter>()
        {}

        ~_L2Counter() {}

        void __init(PAMI::Memory::MemoryManager *mm,
                                PAMI::Atomic::BGQ::l2x_scope_t scope) {
                pami_result_t rc = __global.l2atomicFactory.l2x_alloc((void **)&_counter,
                                                                        1, scope);
                PAMI_assertf(rc == PAMI_SUCCESS, "Failed to allocate L2 Atomic Counter");
                // MUST NOT DO THIS! other procs might be already using it.
                // TODO: find a way to ensure memory is zeroed once and only once.
                //fetch_and_clear_impl();
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

        /// \see PAMI::Atomic::Interface::Counter::compare_and_swap
        /// Since BG/Q L2 Atomics don't implement compare-and-swap, we use
        /// the GCC builtin and hope for the best.
        inline bool compare_and_swap_impl(size_t compare, size_t swap) {
                return __sync_bool_compare_and_swap(&_counter, compare, swap);
        }

        inline void *returnLock_impl() { return (void *)_counter; }

protected:

        volatile uint64_t *_counter;

}; // class _L2Counter

///
/// \brief BG/Q L2 Atomics based Proc-scoped Atomic Counter
///
/// Currently, for 4GB RAM nodes, L2 Atomics may be performed anywhere in-place.
/// Later, we may need to go to a scheme that maps a block of memory for use
/// in L2 Atomics, and allocates counters out of it a la BG/P Lockboxes.
///
class L2ProcCounter : public _L2Counter<L2ProcCounter> {
public:
        L2ProcCounter() :
        _L2Counter<L2ProcCounter>()
        {}

        ~L2ProcCounter() {}

        /// \see PAMI::Atomic::Interface::Counter::init
        void init_impl(PAMI::Memory::MemoryManager *mm) {
                __init(mm, PAMI::Atomic::BGQ::L2A_PROC_SCOPE);
                // MUST NOT DO THIS! other procs might be already using it.
                // TODO: find a way to ensure memory is zeroed once and only once.
                //fetch_and_clear_impl ();
        }

protected:
}; // class L2ProcCounter

///
/// \brief BG/Q L2 Atomics based Node-scoped Atomic Counter
///
/// Currently, for 4GB RAM nodes, L2 Atomics may be performed anywhere in-place.
/// Later, we may need to go to a scheme that maps a block of memory for use
/// in L2 Atomics, and allocates counters out of it a la BG/P Lockboxes.
///
class L2NodeCounter : public _L2Counter<L2NodeCounter> {
public:
        L2NodeCounter() :
        _L2Counter<L2NodeCounter>()
        {}

        ~L2NodeCounter() {}

        /// \see PAMI::Atomic::Interface::Counter::init
        void init_impl(PAMI::Memory::MemoryManager *mm) {
                __init(mm, PAMI::Atomic::BGQ::L2A_NODE_SCOPE);
                // MUST NOT DO THIS! other procs might be already using it.
                // TODO: find a way to ensure memory is zeroed once and only once.
                //fetch_and_clear_impl ();
        }

protected:
}; // class L2NodeCounter

}; // namespace BGQ
}; // namespace Atomic
};   // namespace PAMI

#endif // __components_atomic_bgq_L2Counter_h__
