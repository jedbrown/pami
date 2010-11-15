/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/atomic/noop/Noop.h
 * \brief ???
 */
#ifndef __components_atomic_noop_Noop_h__
#define __components_atomic_noop_Noop_h__

#include "components/atomic/CounterInterface.h"
#include "components/atomic/MutexInterface.h"

namespace PAMI
{
  namespace Atomic
  {
    ///
    /// \brief A "noop" atomic counter and mutex
    ///
    class Noop : public PAMI::Counter::Interface <Noop>,
                 public PAMI::Mutex::Interface <Noop>
    {
      public:

        friend class PAMI::Counter::Interface <Noop>;
        friend class PAMI::Mutex::Interface <Noop>;

        Noop () :
            PAMI::Counter::Interface <Noop> (),
            PAMI::Mutex::Interface <Noop> ()
        {};

        ~Noop () {};

      protected:

        // -------------------------------------------------------------------
        // PAMI::Counter::Interface<T> implementation
        // -------------------------------------------------------------------

        inline size_t fetch_impl () { return 0; };

        inline size_t fetch_and_inc_impl () { return 0; };

        inline size_t fetch_and_dec_impl () { return 0; };

        inline size_t fetch_and_clear_impl () { return 0; };

        inline void clear_impl () {};

        inline bool compare_and_swap_impl (size_t compare, size_t swap) { return true; }

        // -------------------------------------------------------------------
        // PAMI::Mutex::Interface<T> implementation
        // -------------------------------------------------------------------

        inline void acquire_impl () {};

        inline void release_impl () {};

        inline bool tryAcquire_impl () { return true; };

        inline bool isLocked_impl () { return false; };
    };
  };
};

#endif // __components_atomic_noop_Noop_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
