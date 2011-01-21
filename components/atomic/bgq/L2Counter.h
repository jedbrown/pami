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

#include "Global.h"
#include "components/atomic/CounterInterface.h"
#include "components/atomic/IndirectInterface.h"
#include "util/common.h"

#include "common/bgq/L2AtomicFactory.h"
#include <spi/include/l2/atomic.h>

namespace PAMI
{
  namespace Counter
  {
    namespace BGQ
    {
      ///
      /// \brief PAMI::Counter::Interface implementation using bgq L2 atomics
      ///
      /// The PAMI::Counter::BGQ::L2 class is considered an "in place" implementation
      /// because the storage for the actual atomic resource is embedded within
      /// the class instance.
      ///
      /// Any "in place" counter implementation may be converted to an "indirect"
      /// counter implementation, where the atomic resource is located outside
      /// of the class instance, by using the PAMI::Counter::Indirect<T> class
      /// instead of the native "in place" implementation.
      ///
      /// Alternatively, the PAMI::Counter::BGQ::IndirectL2 may be used which
      /// will allocate the L2 resources using the L2 Atomic Factory
      ///
      /// \note This "in place" class is used internally ONLY.
      ///
      class L2 : public PAMI::Counter::Interface<L2>
      {
        public:

          friend class PAMI::Counter::Interface<L2>;

          inline L2 () :
            PAMI::Counter::Interface<L2> (),
            _counter()
          {};

          inline ~L2() {};

	static inline bool checkCtorMm(PAMI::Memory::MemoryManager *mm) {
		return ((mm->attrs() & PAMI::Memory::PAMI_MM_L2ATOMIC) != 0);
	}

	static inline bool checkDataMm(PAMI::Memory::MemoryManager *mm) {
		return true; // no mm used - no init.
	}

        protected:

          // -------------------------------------------------------------------
          // PAMI::Counter::Interface<T> implementation
          // -------------------------------------------------------------------

          inline size_t fetch_impl()
          {
            return L2_AtomicLoad(&_counter);
          }

          inline size_t fetch_and_inc_impl()
          {
            return L2_AtomicLoadIncrement(&_counter);
          }

          inline size_t fetch_and_dec_impl()
          {
            return L2_AtomicLoadDecrement(&_counter);
          }

          inline size_t fetch_and_clear_impl()
          {
            return L2_AtomicLoadClear(&_counter);
          }

          inline void clear_impl()
          {
            L2_AtomicLoadClear(&_counter);
          }

          volatile uint64_t _counter;

      }; // class PAMI::Counter::BGQ::L2

      class IndirectL2 : public PAMI::Counter::Interface<IndirectL2>,
                         public PAMI::Atomic::Indirect<IndirectL2>
      {
        public:

          friend class PAMI::Counter::Interface<IndirectL2>;
          friend class PAMI::Atomic::Indirect<IndirectL2>;

          static const bool indirect = true;

          inline IndirectL2 () :
            PAMI::Counter::Interface<IndirectL2> (),
            _counter(NULL)
          {};

          inline ~IndirectL2() {};

	static inline bool checkCtorMm(PAMI::Memory::MemoryManager *mm) {
		// must not be shared memory.
		return ((mm->attrs() & PAMI::Memory::PAMI_MM_NODESCOPE) == 0);
	}

	static inline bool checkDataMm(PAMI::Memory::MemoryManager *mm) {
		return ((mm->attrs() & PAMI::Memory::PAMI_MM_L2ATOMIC) != 0);
	}

        protected:

          // -------------------------------------------------------------------
          // PAMI::Atomic::Indirect<T> implementation
          // -------------------------------------------------------------------

          ///
          /// \brief Initialize the indirect L2 atomic counter
          ///
          /// Does not use the memory manager from the user, but instead uses
          /// the BGQ-specific L2 Atomic Factory to access the L2 Memory Manager
          /// to allocate the memory.
          ///
          template <class T_MemoryManager>
          inline void init_impl(T_MemoryManager *mm, const char *key)
          {
            pami_result_t rc;
	    if ((mm->attrs() & PAMI::Memory::PAMI_MM_L2ATOMIC) != 0) {
            	rc = mm->memalign((void **)&_counter, sizeof(*_counter),
                                         sizeof(*_counter), key);
	    } else {
		fprintf(stderr, "PAMI::Counter::BGQ::IndirectL2: WARNING: using __global.l2atomicFactory.__nodescoped_mm\n");
            	rc = __global.l2atomicFactory.__nodescoped_mm.memalign((void **)&_counter,
                                                                    sizeof(*_counter),
                                                                    sizeof(*_counter),
                                                                    key);
	    }

            PAMI_assertf(rc == PAMI_SUCCESS, "Failed to allocate memory from mm %p with key \"%s\"", mm, key);
          };

          inline void clone_impl (IndirectL2 & atomic)
          {
            _counter = atomic._counter;
          };

          // -------------------------------------------------------------------
          // PAMI::Counter::Interface<T> implementation
          // -------------------------------------------------------------------

          inline size_t fetch_impl()
          {
            return L2_AtomicLoad(_counter);
          }

          inline size_t fetch_and_inc_impl()
          {
            return L2_AtomicLoadIncrement(_counter);
          }

          inline size_t fetch_and_dec_impl()
          {
            return L2_AtomicLoadDecrement(_counter);
          }

          inline size_t fetch_and_clear_impl()
          {
            return L2_AtomicLoadClear(_counter);
          }

          inline void clear_impl()
          {
            L2_AtomicLoadClear(_counter);
          }

          // -------------------------------------------------------------------
          // Memory manager counter initialization function
          // -------------------------------------------------------------------

          ///
          /// \brief Initialize the counter resources
          ///
          /// \see PAMI::Memory::MM_INIT_FN
          ///
          static void counter_initialize (void       * memory,
                                          size_t       bytes,
                                          const char * key,
                                          unsigned     attributes,
                                          void       * cookie)
          {
            volatile uint64_t * counter = (volatile uint64_t *) memory;
            L2_AtomicLoadClear(counter);
          };

          volatile uint64_t * _counter;

      }; // class     PAMI::Counter::BGQ::IndirectL2
    };   // namespace PAMI::Counter::BGQ
  };     // namespace PAMI::Counter
};       // namespace PAMI

#endif // __components_atomic_bgq_L2Counter_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
