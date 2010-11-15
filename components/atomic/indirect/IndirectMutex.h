/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/atomic/indirect/IndirectMutex.h
 * \brief ???
 */
#ifndef __components_atomic_indirect_IndirectMutex_h__
#define __components_atomic_indirect_IndirectMutex_h__

#include "components/atomic/MutexInterface.h"
#include "components/atomic/IndirectInterface.h"

namespace PAMI
{
  namespace Mutex
  {
    ///
    /// \brief Convert an "in place" mutex class into an "indirect" mutex.
    ///
    /// \tparam T Mutex implmentation class of PAMI::Mutex::Interface.
    ///
    template <class T>
    class Indirect : public PAMI::Mutex::Interface< PAMI::Mutex::Indirect<T> >,
                     public PAMI::Atomic::Indirect< PAMI::Mutex::Indirect<T> >
    {
      public:

        friend class PAMI::Mutex::Interface< PAMI::Mutex::Indirect<T> >;
        friend class PAMI::Atomic::Indirect< PAMI::Mutex::Indirect<T> >;

        static const bool indirect = true;

        inline Indirect () :
          PAMI::Mutex::Interface< PAMI::Mutex::Indirect<T> > (),
          PAMI::Atomic::Indirect< PAMI::Mutex::Indirect<T> > (),
          _mutex (NULL)
        {};

        inline ~Indirect () {};

      protected:

        // -------------------------------------------------------------------
        // PAMI::Atomic::Indirect<T> implementation
        // -------------------------------------------------------------------

        template <class T_MemoryManager>
        inline void init_impl (T_MemoryManager * mm, const char * key)
        {
          pami_result_t rc = mm->memalign ((void **) & _mutex,
                                           sizeof(*_mutex),
                                           sizeof(*_mutex),
                                           key);

          PAMI_assertf (rc == PAMI_SUCCESS, "Failed to allocate memory from memory manager (%p) with key (\"%s\")", mm, key);

          new (_mutex) T ();
        };

        inline void clone_impl (Indirect & atomic)
        {
          _mutex = atomic._mutex;
        };

        // -------------------------------------------------------------------
        // PAMI::Mutex::Interface<T> implementation
        // -------------------------------------------------------------------

        inline void acquire_impl ()
        {
          _mutex->acquire ();
        };

        inline void release_impl ()
        {
          _mutex->release ();
        };

        inline bool tryAcquire_impl ()
        {
          return _mutex->tryAcquire ();
        };

        inline bool isLocked_impl ()
        {
          return _mutex->isLocked ();
        };

        T * _mutex;

    }; // PAMI::Mutex::Indirect<T> class
  };   // PAMI::Mutex namespace
};     // PAMI namespace

#endif // __components_atomic_indirect_IndirectMutex_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
