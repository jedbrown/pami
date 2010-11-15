/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/atomic/counter/CounterMutex.h
 * \brief generic Atomic Counter implementation of Node- or Process-scoped Mutexes
 *
 * This may not be quite right yet - might not work for things like LockBoxCounter.
 */

#ifndef __components_atomic_counter_CounterMutex_h__
#define __components_atomic_counter_CounterMutex_h__

#include "components/atomic/IndirectInterface.h"
#include "components/atomic/MutexInterface.h"

namespace PAMI
{
  namespace Mutex
  {
    ///
    /// \brief Template class for a counter mutex - a.k.a. an "in place" counter mutex
    ///
    /// PAMI::Mutex::Counter<PAMI::Counter::Gcc> mutex; // "in place counter mutex"
    /// mutex.acquire();
    /// foo++;
    /// mutex.release();
    ///
    /// \tparam T PAMI::Counter::Interface implementation class
    ///
    template <class T>
    class Counter : public PAMI::Mutex::Interface<PAMI::Mutex::Counter<T> >
    {
      public:

        friend class PAMI::Mutex::Interface<PAMI::Mutex::Counter<T> >;

        inline Counter() :
            PAMI::Mutex::Interface<PAMI::Mutex::Counter<T> > (),
            _counter()
        {};

        inline ~Counter() {};

      protected:

        // -------------------------------------------------------------------
        // PAMI::Mutex::Interface<T> implementation
        // -------------------------------------------------------------------

        inline void init_impl()
        {
          _counter.init();
        };

        void acquire_impl()
        {
          while (_counter.fetch_and_inc() != 0);
        };

        void release_impl()
        {
          _counter.clear();
        };

        bool tryAcquire_impl()
        {
          return (_counter.fetch_and_inc() == 0) ? true : false;
        };

        bool isLocked_impl()
        {
          return (_counter.fetch() > 0) ? true : false;
        };

        T _counter;

    }; // PAMI::Mutex::Counter<T> class

    ///
    /// \brief Mutex implementation using an "indirect" counter class
    ///
    /// This class implements an indirect mutex via an indirect counter. The
    /// indirect interfaces are implemented merely as a pass-through to the
    /// counter indirect interface implementation.
    ///
    /// \note There is a subtle difference between an "indirect mutex counter"
    ///       and an "indirect counter mutex" (this class).
    ///
    template <class T>
    class IndirectCounter : public PAMI::Mutex::Interface< PAMI::Mutex::IndirectCounter<T> >,
                            public PAMI::Atomic::Indirect< PAMI::Mutex::IndirectCounter<T> >
    {
      public:

        friend class PAMI::Mutex::Interface< PAMI::Mutex::IndirectCounter<T> >;
        friend class PAMI::Atomic::Indirect< PAMI::Mutex::IndirectCounter<T> >;

        inline IndirectCounter() :
            PAMI::Mutex::Interface< PAMI::Mutex::IndirectCounter<T> > (),
            PAMI::Atomic::Indirect< PAMI::Mutex::IndirectCounter<T> > (),
            _counter()
        {};

        inline ~IndirectCounter() {};

      protected:

        // -------------------------------------------------------------------
        // PAMI::Atomic::Indirect<T> implementation
        // -------------------------------------------------------------------

        template <class T_MemoryManager>
        inline void init_impl (T_MemoryManager * mm, const char * key)
        {
          _counter.init (mm, key);
        };

        inline void clone_impl (IndirectCounter & atomic)
        {
          _counter.clone (atomic);
        };

        // -------------------------------------------------------------------
        // PAMI::Mutex::Interface<T> implementation
        // -------------------------------------------------------------------

        inline void init_impl()
        {
          _counter.init();
        };

        void acquire_impl()
        {
          while (_counter.fetch_and_inc() != 0);
        };

        void release_impl()
        {
          _counter.clear();
        };

        bool tryAcquire_impl()
        {
          return (_counter.fetch_and_inc() == 0) ? true : false;
        };

        bool isLocked_impl()
        {
          return (_counter.fetch() > 0) ? true : false;
        };

        T _counter;

    }; // PAMI::Mutex::IndirectCounter<T> class
  };   // PAMI::Mutex namespace
};     // PAMI namespace

#endif // __components_atomic_counter_CounterMutex_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
