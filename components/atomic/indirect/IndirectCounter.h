/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/atomic/indirect/IndirectCounter.h
 * \brief ???
 */

#ifndef __components_atomic_indirect_IndirectCounter_h__
#define __components_atomic_indirect_IndirectCounter_h__

#include "components/atomic/CounterInterface.h"
#include "components/atomic/IndirectInterface.h"

namespace PAMI
{
  namespace Counter
  {
    ///
    /// \brief Convert an "in place" counter class into an "indirect" counter.
    ///
    /// \tparam T PAMI::Counter::Interface<T> implementation class.
    ///
    template <class T>
    class Indirect : public PAMI::Counter::Interface< PAMI::Counter::Indirect<T> >,
                     public PAMI::Atomic::Indirect< PAMI::Counter::Indirect<T> >
    {
      public:

        friend class PAMI::Counter::Interface< PAMI::Counter::Indirect<T> >;
        friend class PAMI::Atomic::Indirect< PAMI::Counter::Indirect<T> >;

        static const bool indirect = true;

        inline Indirect () :
          PAMI::Counter::Interface< PAMI::Counter::Indirect<T> > (),
          PAMI::Atomic::Indirect< PAMI::Counter::Indirect<T> > (),
          _counter (NULL)
        {
        };

      protected:

        // -------------------------------------------------------------------
        // PAMI::Atomic::Indirect implementation
        // -------------------------------------------------------------------

        template <class T_MemoryManager>
        inline void init_impl (T_MemoryManager * mm, const char * key)
        {
          pami_result_t rc;
	  rc = mm->memalign ((void **) & _counter,
                                           sizeof(*_counter),
                                           sizeof(*_counter),
                                           key,
                                           Indirect::counter_initialize,
                                           NULL);

          PAMI_assertf (rc == PAMI_SUCCESS, "Failed to allocate memory from memory manager (%p) with key (\"%s\")", mm, key);
        };

        inline void clone_impl (Indirect & atomic)
        {
          _counter = atomic._counter;
        };

        // -------------------------------------------------------------------
        // PAMI::Counter::Interface implementation
        // -------------------------------------------------------------------

        inline size_t fetch_impl ()
        {
          return _counter->fetch ();
        };

        inline size_t fetch_and_inc_impl ()
        {
          return _counter->fetch_and_inc ();
        };

        inline size_t fetch_and_dec_impl ()
        {
          return _counter->fetch_and_dec ();
        };

        inline size_t fetch_and_clear_impl ()
        {
          return _counter->fetch_and_clear ();
        };

        inline void clear_impl ()
        {
          _counter->clear ();
        };

        inline bool compare_and_swap_impl (size_t compare, size_t swap)
        {
          return _counter->compare_and_swap (compare, swap);
        };

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
          T * counter = (T *) memory;
          new (counter) T ();
        };

        T * _counter;

    }; // PAMI::Counter::Indirect<T> class
  };   // PAMI::Counter namespace
};     // PAMI namespace

#endif // __components_atomic_indirect_IndirectCounter_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
