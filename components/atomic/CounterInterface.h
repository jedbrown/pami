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
      protected:

        inline Interface() {}
        inline ~Interface() {}

      public:

        template <class T_Counter>
        class Bounded
        {
          public:

            ///
            /// \brief Fetch, then increment the atomic counter object value
            ///
            /// \attention All bounded atomic counter object derived classes
            ///            \b must implement the fetch_and_inc_bounded_impl()
            ///            method.
            ///
            /// \param [out] value Previous counter value
            ///
            /// \return true  Increment operation was not bounded; value is valid
            /// \return false Increment operation was bounded; value is invalid
            ///
            inline bool fetch_and_inc_bounded (size_t & value)
            {
              return static_cast<T*>(this)->fetch_and_inc_bounded_impl(value);
            };

            ///
            /// \brief Fetch, then decrement the atomic counter object value
            ///
            /// \attention All bounded atomic counter object derived classes
            ///            \b must implement the fetch_and_dec_bounded_impl()
            ///            method.
            ///
            /// \param [out] value Previous counter value
            ///
            /// \return true  Operation was not bounded; value is valid
            /// \return false Operation was bounded; value is invalid
            ///
            inline bool fetch_and_dec_bounded (size_t & value)
            {
              return static_cast<T*>(this)->fetch_and_dec_bounded_impl(value);
            };

            ///
            /// \brief Reference to the upper bound atomic
            ///
            /// \see fetch_and_inc_bounded()
            ///
            inline T_Counter & upper()
            {
              return static_cast<T*>(this)->upper_impl();
            };

            ///
            /// \brief Reference to the lower bound atomic
            ///
            /// \see fetch_and_dec_bounded()
            ///
            inline T_Counter & lower()
            {
              return static_cast<T*>(this)->lower_impl();
            };
        };  // PAMI::Counter::Interface::Bounded class

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
