/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/atomic/gcc/GccCounter.h
 * \brief gcc builtin atomics implementation of the atomic counter interface
 */
#ifndef __components_atomic_gcc_GccCounter_h__
#define __components_atomic_gcc_GccCounter_h__

#include <stdint.h>

#include "components/atomic/CounterInterface.h"

namespace PAMI
{
  namespace Counter
  {
    ///
    /// \brief PAMI::Counter::Interface implementation using gcc builtin atomics
    ///
    /// The PAMI::Counter::Gcc class is considered an "in place" implementation
    /// because the storage for the actual atomic resource is embedded within
    /// the class instance.
    ///
    /// Any "in place" counter implementation may be converted to an "indirect"
    /// counter implementation, where the atomic resource is located outside
    /// of the class instance, by using the PAMI::Counter::Indirect<T> class
    /// instead of the native "in place" implementation.
    ///
    class Gcc : public PAMI::Counter::Interface<Gcc>
    {
      public:

        friend class PAMI::Counter::Interface<Gcc>;

        inline Gcc() {};

        inline ~Gcc() {};
      
        // Todo:  move this to an "Extended interface"
        // This member needs to be public because
        // it is not part of the parent interface
        inline size_t fetch_and_add (size_t inc)
        {
          return __sync_fetch_and_add(&_atom, inc);
        };
      
      protected:

        // -------------------------------------------------------------------
        // PAMI::Counter::Interface<T> implementation
        // -------------------------------------------------------------------

        inline size_t fetch_impl ()
        {
          // return __sync_fetch_and_or (&_atom, 0);
          // can't use __sync_fetch_and_or... it's broken?
          // instead, ensure "_atom" is volatile
          return _atom;
        };

        inline size_t fetch_and_inc_impl ()
        {
          return __sync_fetch_and_add (&_atom, 1);
        };

        inline size_t fetch_and_dec_impl ()
        {
          return __sync_fetch_and_sub (&_atom, 1);
        };

        inline size_t fetch_and_clear_impl ()
        {
          return __sync_fetch_and_and (&_atom, 0);
        };

        inline void clear_impl ()
        {
          _atom = 0;
        };

        inline bool compare_and_swap_impl (size_t compare, size_t swap)
        {
          return __sync_bool_compare_and_swap (&_atom, compare, swap);
        };

        volatile uintptr_t _atom;

    }; // PAMI::Counter::Gcc class
  }; //   PAMI::Counter namespace
}; //     PAMI namespace

#endif // __components_atomic_gcc_GccCounter_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
