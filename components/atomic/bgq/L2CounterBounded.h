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
 * \brief BG/Q L2 Atomics implementation of node- and process-scoped atomic counters
 */
#ifndef __components_atomic_bgq_L2CounterBounded_h__
#define __components_atomic_bgq_L2CounterBounded_h__

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

      class IndirectL2Bounded : public PAMI::Counter::Interface<IndirectL2Bounded>,
      public PAMI::Atomic::Indirect<IndirectL2Bounded>
      {
        public:

          friend class PAMI::Counter::Interface<IndirectL2Bounded>;
          friend class PAMI::Atomic::Indirect<IndirectL2Bounded>;

          static const bool indirect = true;

          inline IndirectL2Bounded () :
            PAMI::Counter::Interface<IndirectL2Bounded> (),
            _counter(NULL)
        {};

          inline ~IndirectL2Bounded() {};

          inline void set_fifo_bounds(uint64_t lower_bound, uint64_t  upper_bound)
          {
            *_counter = lower_bound;
            *(_counter+2) = upper_bound;

          };	

          inline size_t fetch_and_inc_upper_bound()
          {

            uint64_t val =  L2_AtomicLoadIncrement(_counter+2);
            return (size_t) val;
          }

          inline size_t fetch_and_inc_bounded()
          {

            uint64_t val =  L2_AtomicLoadIncrementBounded(_counter+1);
            return (size_t) val;
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
              rc = __global.l2atomicFactory.__nodescoped_mm.memalign ((void **) & _counter,
                  32,                                                                      
                  3*sizeof(*_counter),
                  key,
                  IndirectL2Bounded::counter_initialize,
                  NULL);

              PAMI_assertf (rc == PAMI_SUCCESS, "Failed to allocate memory from memory manager (%p) with key (\"%s\")", mm, key);
            };


          inline void clone_impl (IndirectL2Bounded & atomic)
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
          /* Assuming that memory returned has three consecutive uint64_t locations */	
          static void counter_initialize (void       * memory,
              size_t       bytes,
              const char * key,
              unsigned     attributes,
              void       * cookie)
          {
            volatile uint64_t * counter = (volatile uint64_t *) memory;
            L2_AtomicLoadClear(counter);
            L2_AtomicLoadClear(counter+1);
            L2_AtomicLoadClear(counter+2);
          };

          volatile uint64_t * _counter;

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
