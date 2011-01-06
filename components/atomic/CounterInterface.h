/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/atomic/CounterInterface.h
 * \brief ???
 */

#ifndef __components_atomic_CounterInterface_h__
#define __components_atomic_CounterInterface_h__

namespace PAMI
{
  namespace Counter
  {
    ///
    /// \brief Atomic counter interface class
    ///
    /// This is an interface class and may not be directly instantiated.
    ///
    /// \tparam T  Atomic counter implementation class
    ///
    template <class T>
    class Interface
    {
    public:

        inline Interface() {}
        inline ~Interface() {}

      public:

        static const bool indirect = false;

        ///
        /// \brief Fetch the atomic counter object value
        ///
        /// \attention All atomic counter object derived classes \b must
        ///            implement the fetch_impl() method.
        ///
        /// \return Atomic counter object value
        ///
        inline size_t fetch();

        ///
        /// \brief Fetch, then increment the atomic counter object value
        ///
        /// \attention All atomic counter object derived classes \b must
        ///            implement the fetch_and_inc_impl() method.
        ///
        /// \return Atomic counter object value
        ///
        inline size_t fetch_and_inc();

        ///
        /// \brief Fetch, then decrement the atomic counter object value
        ///
        /// \attention All atomic counter object derived classes \b must
        ///            implement the fetch_and_dec_impl() method.
        ///
        /// \return Atomic counter object value
        ///
        inline size_t fetch_and_dec();

        ///
        /// \brief Fetch, then clear the atomic counter object value
        ///
        /// \attention All atomic counter object derived classes \b must
        ///            implement the fetch_and_clear_impl() method.
        ///
        /// \return Atomic counter object value
        ///
        inline size_t fetch_and_clear();

        ///
        /// \brief Clear the atomic counter object value
        ///
        /// \attention All atomic counter implementation classes \b must
        ///            implement the clear_impl() method.
        ///
        /// This is needed to make Mutexes deterministic on some implementations
        ///
        inline void clear();

        ///
        /// \brief Atomic compare and swap operation
        ///
        /// If the current value of the atomic object is the \c compare value,
        /// then write the \c swap value into the atomic object.
        ///
        /// \attention All atomic counter implementation classes \b must
        ///            implement the compare_and_swap_impl() method.
        ///
        /// \retval true Comparison was successful and swap value was written.
        ///
        inline bool compare_and_swap(size_t compare, size_t swap);

    }; // PAMI::Counter::Interface class

    template <class T>
    inline size_t Interface<T>::fetch()
    {
      return static_cast<T*>(this)->fetch_impl();
    }

    template <class T>
    inline size_t Interface<T>::fetch_and_inc()
    {
      return static_cast<T*>(this)->fetch_and_inc_impl();
    }

    template <class T>
    inline size_t Interface<T>::fetch_and_dec()
    {
      return static_cast<T*>(this)->fetch_and_dec_impl();
    }

    template <class T>
    inline size_t Interface<T>::fetch_and_clear()
    {
      return static_cast<T*>(this)->fetch_and_clear_impl();
    }

    template <class T>
    inline void Interface<T>::clear()
    {
      static_cast<T*>(this)->clear_impl();
    }

    template <class T>
    inline bool Interface<T>::compare_and_swap(size_t compare, size_t swap)
    {
      return static_cast<T*>(this)->compare_and_swap_impl(compare, swap);
    }
  };  // PAMI::Counter namespace
};    // PAMI namespace

#endif // __components_atomic_CounterInterface_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
