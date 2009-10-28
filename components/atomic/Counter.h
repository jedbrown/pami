/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/atomic/Counter.h
 * \brief ???
 */

#ifndef __components_atomic_Counter_h__
#define __components_atomic_Counter_h__

namespace XMI
{
  namespace Atomic
  {
  namespace Interface
  {
    ///
    /// \brief Interface for all atomic counter objects.
    ///
    /// \param T_Object  Atomic counter object derived class
    ///
    template <class T_Object>
    class Counter
    {
      public:
        Counter  () {};
        ~Counter () {};

        void init (XMI::SysDep *sd);

        ///
        /// \brief Fetch the atomic counter object value
        ///
        /// \attention All atomic counter object derived classes \b must
        ///            implement the fetch_impl() method.
        ///
        /// \return Atomic counter object value
        ///
        inline size_t fetch ();

        ///
        /// \brief Fetch, then increment the atomic counter object value
        ///
        /// \attention All atomic counter object derived classes \b must
        ///            implement the fetch_and_inc_impl() method.
        ///
        /// \return Atomic counter object value
        ///
        inline size_t fetch_and_inc ();

        ///
        /// \brief Fetch, then decrement the atomic counter object value
        ///
        /// \attention All atomic counter object derived classes \b must
        ///            implement the fetch_and_dec_impl() method.
        ///
        /// \return Atomic counter object value
        ///
        inline size_t fetch_and_dec ();

        ///
        /// \brief Fetch, then clear the atomic counter object value
        ///
        /// \attention All atomic counter object derived classes \b must
        ///            implement the fetch_and_clear_impl() method.
        ///
        /// \return Atomic counter object value
        ///
        inline size_t fetch_and_clear ();

        ///
        /// \brief Atomic compare and swap operation
        ///
        /// If the current value of the atomic object is the \c compare value,
        /// then write the \c swap value into the atomic object.
        ///
        /// \attention All atomic counter object derived classes \b must
        ///            implement the compare_and_swap_impl() method.
        ///
        /// \retval true Comparison was successful and swap value was written.
        ///
        inline bool compare_and_swap (size_t compare, size_t swap);

	inline void *returnLock();

    }; // XMI::Atomic::Interface::Counter class

    template <class T_Object>
    inline void Counter<T_Object>::init (XMI::SysDep *sd)
    {
      static_cast<T_Object*>(this)->init_impl(sd);
    }

    template <class T_Object>
    inline size_t Counter<T_Object>::fetch ()
    {
      return static_cast<T_Object*>(this)->fetch_impl();
    }

    template <class T_Object>
    inline size_t Counter<T_Object>::fetch_and_inc ()
    {
      return static_cast<T_Object*>(this)->fetch_and_inc_impl();
    }

    template <class T_Object>
    inline size_t Counter<T_Object>::fetch_and_dec ()
    {
      return static_cast<T_Object*>(this)->fetch_and_dec_impl();
    }

    template <class T_Object>
    inline size_t Counter<T_Object>::fetch_and_clear ()
    {
      return static_cast<T_Object*>(this)->fetch_and_clear_impl();
    }

    template <class T_Object>
    inline bool Counter<T_Object>::compare_and_swap (size_t compare, size_t swap)
    {
      return static_cast<T_Object*>(this)->compare_and_swap_impl(compare, swap);
    }

    template <class T_Object>
    inline void * Counter<T_Object>::returnLock()
    {
      return static_cast<T_Object*>(this)->returnLock_impl();
    }

    }; // XMI::Atomic::Interface namespace
  };   // XMI::Atomic namespace
};     // XMI namespace

#endif // __components_atomic_counter_h__
