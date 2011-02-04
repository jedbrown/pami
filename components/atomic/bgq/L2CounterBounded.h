/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/atomic/bgq/L2CounterBounded.h
 * \brief BG/Q L2 Atomics implementation of node- and process-scoped \b bounded atomic counters
 */
#ifndef __components_atomic_bgq_L2CounterBounded_h__
#define __components_atomic_bgq_L2CounterBounded_h__

#include "Global.h"
#include "components/atomic/CounterInterface.h"
#include "components/atomic/IndirectInterface.h"
#include "components/atomic/bgq/L2Counter.h"
#include "util/common.h"

#include "common/bgq/L2AtomicFactory.h"
#include <spi/include/l2/atomic.h>

namespace PAMI
{
  namespace Counter
  {
    namespace BGQ
    {

      class IndirectL2Bounded : public PAMI::Counter::Interface<IndirectL2Bounded>,
          public PAMI::Counter::Interface<IndirectL2Bounded>::Bounded<IndirectL2>,
          public PAMI::Atomic::Indirect<IndirectL2Bounded>
      {
        public:

          friend class PAMI::Counter::Interface<IndirectL2Bounded>;
          friend class PAMI::Counter::Interface<IndirectL2Bounded>::Bounded<IndirectL2>;
          friend class PAMI::Atomic::Indirect<IndirectL2Bounded>;

          static const bool indirect = true;

          inline IndirectL2Bounded () :
              PAMI::Counter::Interface<IndirectL2Bounded> (),
              PAMI::Counter::Interface<IndirectL2Bounded>::Bounded<IndirectL2> (),
              PAMI::Atomic::Indirect<IndirectL2Bounded> (),
              _counter(NULL)
          {};

          inline ~IndirectL2Bounded() {};

          static inline bool checkCtorMm(PAMI::Memory::MemoryManager *mm)
          {
            // must not be shared memory.
            return ((mm->attrs() & PAMI::Memory::PAMI_MM_NODESCOPE) == 0);
          }

          static inline bool checkDataMm(PAMI::Memory::MemoryManager *mm)
          {
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
          inline void init_impl (T_MemoryManager * mm, const char * key)
          {
            pami_result_t rc;

            if ((mm->attrs() & PAMI::Memory::PAMI_MM_L2ATOMIC) != 0)
              {
                rc = mm->memalign((void **) & _counter, 32, 3 * sizeof(*_counter), key,
                                  IndirectL2Bounded::counter_initialize, NULL);
              }
            else
              {
                rc = __global.l2atomicFactory.__nodescoped_mm.memalign((void **) & _counter,
                                                                       32, 3 * sizeof(*_counter), key,
                                                                       IndirectL2Bounded::counter_initialize, NULL);
              }

            PAMI_assertf(rc == PAMI_SUCCESS, "Failed to allocate memory from mm %p with key \"%s\"", mm, key);

            _lower.set (_counter);
            _upper.set (_counter + 2);

          };


          inline void clone_impl (IndirectL2Bounded & atomic)
          {
            _counter = atomic._counter;
            _lower.clone(atomic._lower);
            _upper.clone(atomic._upper);
          };

          // -------------------------------------------------------------------
          // PAMI::Counter::Interface<T> implementation
          // -------------------------------------------------------------------

          inline size_t fetch_impl()
          {
            return L2_AtomicLoad(_counter + 1);
          }

          inline size_t fetch_and_inc_impl()
          {
            return L2_AtomicLoadIncrement(_counter + 1);
          }

          inline size_t fetch_and_dec_impl()
          {
            return L2_AtomicLoadDecrement(_counter + 1);
          }

          inline size_t fetch_and_clear_impl()
          {
            return L2_AtomicLoadClear(_counter + 1);
          }

          inline void clear_impl()
          {
            L2_AtomicLoadClear(_counter + 1);
          }

          // -------------------------------------------------------------------
          // PAMI::Counter::Interface<T>::Bounded<T_Counter> implementation
          // -------------------------------------------------------------------

          inline bool fetch_and_inc_bounded_impl (size_t & value)
          {
            value = L2_AtomicLoadIncrementBounded(_counter + 1);
            return (bool) (value != 0x8000000000000000);
          };

          inline bool fetch_and_dec_bounded_impl (size_t & value)
          {
            value = L2_AtomicLoadDecrementBounded(_counter + 1);
            return (bool) (value != 0x8000000000000000);
          };

          inline IndirectL2 & upper_impl ()
          {
            return _upper;
          };

          inline IndirectL2 & lower_impl ()
          {
            return _lower;
          };

          // -------------------------------------------------------------------
          // Memory manager counter initialization function
          // -------------------------------------------------------------------

          ///
          /// \brief Initialize the counter resources
          ///
          /// \see PAMI::Memory::MM_INIT_FN
          ///
          /* Assuming that memory returned has three consecutive uint64_t locations */
          static void counter_initialize (void       * memory,
                                          size_t       bytes,
                                          const char * key,
                                          unsigned     attributes,
                                          void       * cookie)
          {
            volatile uint64_t * counter = (volatile uint64_t *) memory;
            L2_AtomicLoadClear(counter);
            L2_AtomicLoadClear(counter + 1);
            L2_AtomicLoadClear(counter + 2);
          };

          volatile uint64_t * _counter;
          IndirectL2 _lower;
          IndirectL2 _upper;

      }; // class     PAMI::Counter::BGQ::IndirectL2Bounded
    };   // namespace PAMI::Counter::BGQ
  };     // namespace PAMI::Counter
};       // namespace PAMI

#endif // __components_atomic_bgq_L2CounterBounded_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
