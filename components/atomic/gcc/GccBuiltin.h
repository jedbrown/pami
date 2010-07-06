/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/atomic/gcc/GccBuiltin.h
 * \brief ???
 */

#ifndef __components_atomic_gcc_GccBuiltin_h__
#define __components_atomic_gcc_GccBuiltin_h__

#include "components/atomic/Counter.h"
#include "util/common.h"


namespace PAMI
{
  namespace Atomic
  {
    ///
    /// \brief CRTP interface implementation for gcc builtins atomic objects.
    ///
    class GccBuiltin : public Interface::Counter <GccBuiltin>
    {
      public:
        GccBuiltin () :
            Interface::Counter <GccBuiltin> ()
        {};

        ~GccBuiltin () {};

        /// \see PAMI::Atomic::Interface::Counter::init
        void init_impl (PAMI::Memory::MemoryManager *mm)
        {
          // MUST NOT DO THIS! other procs might be already using it.
          /// \todo #warning: find a way to ensure memory is zeroed once and only once.
          //fetch_and_clear_impl ();
        };

        /// \see PAMI::Atomic::Interface::Counter::fetch
        inline size_t fetch_impl ()
        {
          // return __sync_fetch_and_or (&_atom, 0);
          // can't use __sync_fetch_and_or... it's broken?
          // instead, ensure "_atom" is volatile
          return _atom;
        };

        /// \see PAMI::Atomic::Interface::Counter::fetch_and_inc
        inline size_t fetch_and_inc_impl ()
        {
          return __sync_fetch_and_add (&_atom, 1);
          // return _atom++;
        };

        /// \see PAMI::Atomic::Interface::Counter::fetch_and_dec
        inline size_t fetch_and_dec_impl ()
        {
          return __sync_fetch_and_sub (&_atom, 1);
        };

        /// \see PAMI::Atomic::Interface::Counter::fetch_and_clear
        inline size_t fetch_and_clear_impl ()
        {
          return __sync_fetch_and_and (&_atom, 0);
          //_atom = 0;
        };

        /// \see PAMI::Atomic::Interface::Counter::clear
        inline void clear_impl ()
        {
          _atom = 0;
        };

        /// \see PAMI::Atomic::Interface::Counter::compare_and_swap
        inline bool compare_and_swap_impl (size_t compare, size_t swap)
        {
          return __sync_bool_compare_and_swap (&_atom, compare, swap);
        };

        inline void *returnLock_impl() { return (void *)&_atom; }

      protected:

        volatile uintptr_t _atom;
    };
  };
};

#endif // __components_atomic_gcc_gccbuiltin_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
