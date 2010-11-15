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

#include "components/atomic/CounterInterface.h"
#include "util/common.h"

namespace PAMI
{
  namespace Counter
  {
    ///
    /// \brief PAMI::Counter::Interface implementation using xlc builtin atomics
    ///
    /// The PAMI::Counter::Xlc class is considered an "in place" implementation
    /// because the storage for the actual atomic resource is embedded within
    /// the class instance.
    ///
    /// Any "in place" counter implementation may be converted to an "indirect"
    /// counter implementation, where the atomic resource is located outside
    /// of the class instance, by using the PAMI::Counter::Indirect<T> class
    /// instead of the native "in place" implementation.
    ///
    template <class T>
    class XlcBuiltinT : public PAMI::Counter::Interface <XlcBuiltinT < T > >
    {
      public:

        friend class PAMI::Counter::Interface <XlcBuiltinT < T > >;

        XlcBuiltinT () :
            PAMI::Counter::Interface <XlcBuiltinT < T > > ()
        {
          COMPILE_TIME_ASSERT(sizeof(XlcBuiltinT) == sizeof(T));
        };

        ~XlcBuiltinT () {};

      protected:

        // -------------------------------------------------------------------
        // PAMI::Counter::Interface<T> implementation
        // -------------------------------------------------------------------

        inline T fetch_impl ()
        {
          return _atom;
        };

        inline T fetch_and_inc_impl ()
        {
          return __fetch_and_add((volatile int *)&_atom, 1);
        };

        inline T fetch_and_dec_impl ()
        {
          return __fetch_and_add ((volatile int *)&_atom, -1);
        };

        inline T fetch_and_clear_impl ()
        {
          return __fetch_and_and((unsigned int *)(&_atom), 0);
        };

        inline void clear_impl ()
        {
          _atom = 0;
        };

        // using  Interface::Counter<XlcBuiltinT < T > >::fetch_and_add;
        inline T fetch_and_add(T val)
        {
          return __fetch_and_add((volatile int *)&_atom, val);
        }

        inline bool compare_and_swap_impl (T compare, T swap);

        // using Interface::Counter<XlcBuiltinT < T > >::compare_and_swap;

        // inline int compare_and_swap (T *compare, T swap);

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
#endif // __components_atomic_xlc_XlcBuiltinT_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
