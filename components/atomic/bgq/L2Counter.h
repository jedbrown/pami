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

namespace XMI
{
  namespace Atomic
  {
  namespace BGQ
  {
    ///
    /// \brief CRTP interface for BG/Q L2 Atomics builtins atomic objects.
    ///
    /// Currently, for 4GB RAM nodes, L2 Atomics may be performed anywhere in-place.
    /// Later, we may need to go to a scheme that maps a block of memory for use
    /// in L2 Atomics, and allocates counters out of it a la BG/P Lockboxes.
    ///
    class L2ProcCounter : public Interface::Counter <L2ProcCounter>
    {
      public:
        L2ProcCounter () :
            Interface::Counter <L2ProcCounter> ()
        {};

        ~L2ProcCounter () {};

        /// \see XMI::Atomic::Interface::Counter::init
        void init_impl (XMI::SysDep *sd)
        {
	  // MUST NOT DO THIS! other procs might be already using it.
	  // TODO: find a way to ensure memory is zeroed once and only once.
          //fetch_and_clear_impl ();
        };

        /// \see XMI::Atomic::Interface::Counter::fetch
        inline size_t fetch_impl ()
        {
          return L2_AtomicLoad(&_counter);
        };

        /// \see XMI::Atomic::Interface::Counter::fetch_and_inc
        inline size_t fetch_and_inc_impl ()
        {
	 return L2_AtomicLoadIncrement(&_counter);
        };

        /// \see XMI::Atomic::Interface::Counter::fetch_and_dec
        inline size_t fetch_and_dec_impl ()
        {
          return L2_AtomicLoadDecrement(&_counter);;
        };

        /// \see XMI::Atomic::Interface::Counter::fetch_and_clear
        inline size_t fetch_and_clear_impl ()
        {
	  return L2_AtomicLoadClear(&_counter);
        };

        /// \see XMI::Atomic::Interface::Counter::compare_and_swap
	/// Since BG/Q L2 Atomics don't implement compare-and-swap, we use
	/// the GCC builtin and hope for the best.
        inline bool compare_and_swap_impl (size_t compare, size_t swap)
        {
          return __sync_bool_compare_and_swap (&_counter, compare, swap);
        };

	inline void *returnLock_impl() { return (void *)&_counter; }

      protected:

	volatile __attribute__((aligned(L1D_CACHE_LINE_SIZE))) uint64_t _counter;

    }; // class L2ProcCounter

    ///
    /// \brief BG/Q L2 Atomics based Node-scoped Atomci Counter
    ///
    /// Currently, for 4GB RAM nodes, L2 Atomics may be performed anywhere in-place.
    /// Later, we may need to go to a scheme that maps a block of memory for use
    /// in L2 Atomics, and allocates counters out of it a la BG/P Lockboxes.
    ///
    class L2NodeCounter : public Interface::Counter <L2NodeCounter>
    {
      public:
        L2NodeCounter () :
            Interface::Counter <L2NodeCounter> ()
        {};

        ~L2NodeCounter () {};

        /// \see XMI::Atomic::Interface::Counter::init
        void init_impl (XMI::SysDep *sd)
        {
	  xmi_result_t rc = sd->mm.memalign((void **)&_counter,
					L1D_CACHE_LINE_SIZE, sizeof(*_counter));
	  XMI_assertf(rc == XMI_SUCCESS, "Failed to allocate shared memory for Node Counter");
	  // MUST NOT DO THIS! other procs might be already using it.
	  // TODO: find a way to ensure memory is zeroed once and only once.
          //fetch_and_clear_impl ();
        };

        /// \see XMI::Atomic::Interface::Counter::fetch
        inline size_t fetch_impl ()
        {
          return L2_AtomicLoad(_counter);
        };

        /// \see XMI::Atomic::Interface::Counter::fetch_and_inc
        inline size_t fetch_and_inc_impl ()
        {
	 return L2_AtomicLoadIncrement(_counter);
        };

        /// \see XMI::Atomic::Interface::Counter::fetch_and_dec
        inline size_t fetch_and_dec_impl ()
        {
          return L2_AtomicLoadDecrement(_counter);;
        };

        /// \see XMI::Atomic::Interface::Counter::fetch_and_clear
        inline size_t fetch_and_clear_impl ()
        {
	  return L2_AtomicLoadClear(_counter);
        };

        /// \see XMI::Atomic::Interface::Counter::compare_and_swap
	/// Since BG/Q L2 Atomics don't implement compare-and-swap, we use
	/// the GCC builtin and hope for the best.
        inline bool compare_and_swap_impl (size_t compare, size_t swap)
        {
          return __sync_bool_compare_and_swap (_counter, compare, swap);
        };

	inline void *returnLock_impl() { return (void *)_counter; }

      protected:

	volatile __attribute__((aligned(L1D_CACHE_LINE_SIZE))) uint64_t *_counter;

    }; // class L2NodeCounter

  }; // namespace BGQ
  }; // namespace Atomic
};   // namespace XMI

#endif // __components_atomic_bgq_L2Counter_h__
