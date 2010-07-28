/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/atomic/xlc/XlcBuiltinT.h
 * \brief ???
 */

#ifndef __components_atomic_xlc_XlcBuiltinT_h__
#define __components_atomic_xlc_XlcBuiltinT_h__

#include "components/atomic/Counter.h"
#include "util/common.h"

namespace PAMI
{
  namespace Atomic
  {
    ///
    /// \brief CRTP interface for xlc builtins atomic objects.
    ///
    template <class T>
    class XlcBuiltinT : public Interface::Counter <XlcBuiltinT < T > >
    {
      public:
        XlcBuiltinT () :
            Interface::Counter <XlcBuiltinT <T> > ()
        {
           COMPILE_TIME_ASSERT(sizeof(XlcBuiltinT) == sizeof(T));
        };

        ~XlcBuiltinT () {};

        /// \see PAMI::Atomic::Interface::Counter::init
        void init_impl (PAMI::Memory::MemoryManager *mm)
        {
          // MUST NOT DO THIS! other procs might be already using it.
          /// \todo #warning: find a way to ensure memory is zeroed once and only once.
          //fetch_and_clear_impl ();
        };

        /// \see PAMI::Atomic::Interface::Counter::fetch
        inline T fetch_impl ()
        {
          return _atom;
        };

        /// \see PAMI::Atomic::Interface::Counter::fetch_and_inc
        inline T fetch_and_inc_impl ()
        {
            return __fetch_and_add((volatile int *)&_atom, 1);
        };

        /// \see PAMI::Atomic::Interface::Counter::fetch_and_dec
        inline T fetch_and_dec_impl ()
        {
            return __fetch_and_add ((volatile int *)&_atom, -1);
        };

        /// \see PAMI::Atomic::Interface::Counter::fetch_and_clear
        inline T fetch_and_clear_impl ()
        {
          return __fetch_and_and((unsigned int *)(&_atom), 0);
        };

        /// \see PAMI::Atomic::Interface::Counter::clear
        inline void clear_impl ()
        {
          _atom = 0;
        };

        inline void *returnLock_impl() { return (void *)&_atom; }

        // using  Interface::Counter<XlcBuiltinT < T > >::fetch_and_add;
        inline T fetch_and_add(T val)
        {
          return __fetch_and_add((volatile int *)&_atom, val);
        }

        inline bool compare_and_swap_impl (T compare, T swap);

        // using Interface::Counter<XlcBuiltinT < T > >::compare_and_swap;

        // inline int compare_and_swap (T *compare, T swap);

      protected:

        volatile T _atom;
    };

    template <class T>
    inline bool XlcBuiltinT<T>::compare_and_swap_impl (T compare, T swap)
    {
      T compare_val = compare;
      return __compare_and_swap ((volatile int *)&_atom, (int *)&compare_val, swap);
    };

/*
    template <class T>
    inline int XlcBuiltinT<T>::compare_and_swap (T *compare, T swap)
    {
    return __compare_and_swap((volatile int *)&_atom, (int *)compare, swap);
    }
*/

#ifdef __64BIT__
    template <>
    inline long XlcBuiltinT<long>::fetch_and_inc_impl ()
    {
      return __fetch_and_addlp(&_atom, 1);
    };

    template <>
    inline long XlcBuiltinT<long>::fetch_and_dec_impl ()
    {
      return __fetch_and_addlp(&_atom, -1);
    };

    template<>
    inline long XlcBuiltinT<long>::fetch_and_clear_impl()
    {
      return __fetch_and_andlp((volatile unsigned long *)&_atom, 0);
    };


    template <>
    inline long XlcBuiltinT<long>::fetch_and_add(long val)
    {
      return __fetch_and_addlp(&_atom, val);
    }

    template<>
    inline bool XlcBuiltinT<long>::compare_and_swap_impl(long compare, long swap)
    {
      long compare_val = compare;

      return __compare_and_swaplp (&_atom, &compare_val, swap);
    }

/*
    template<>
    inline int XlcBuiltinT<long>::compare_and_swap(long *compare, long swap)
    {
    return __compare_and_swaplp(&_atom, compare, swap);
    }
*/
#endif


  };
};


#endif // __components_atomic_xlc_xlcbuiltintt_h__
