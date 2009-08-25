/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file atomic/AtomicObject.h
 * \brief ???
 */

#ifndef __xmi_atomic_atomicobject_h__
#define __xmi_atomic_atomicobject_h__

namespace XMI
{
  namespace Atomic
  {
    ///
    /// \brief CRTP for all atomic objects.
    ///
    /// \param T_Object  Atomic object template class
    ///
    template <class T_Object>
    class AtomicObject
    {
      public:
        AtomicObject  () {}
        ~AtomicObject () {}

        void init ();

        ///
        /// \brief Fetch the atomic object value
        ///
        /// \attention All atomic object derived classes \b must
        ///            implement the fetch_impl() method.
        ///
        /// \return Atomic object value
        ///
        inline size_t fetch () const;

        ///
        /// \brief Fetch, then increment the atomic object value
        ///
        /// \attention All atomic object derived classes \b must
        ///            implement the fetch_and_inc_impl() method.
        ///
        /// \return Atomic object value
        ///
        inline size_t fetch_and_inc ();

        ///
        /// \brief Fetch, then decrement the atomic object value
        ///
        /// \attention All atomic object derived classes \b must
        ///            implement the fetch_and_dec_impl() method.
        ///
        /// \return Atomic object value
        ///
        inline size_t fetch_and_dec ();

        ///
        /// \brief Fetch, then clear the atomic object value
        ///
        /// \attention All atomic object derived classes \b must
        ///            implement the fetch_and_clear_impl() method.
        ///
        /// \return Atomic object value
        ///
        inline size_t fetch_and_clear ();

        ///
        /// \brief Atomic compare and swap operation
        ///
        /// If the current value of the atomic object is the \c compare value,
        /// then write the \c swap value into the atomic object and return the
        /// original value.
        ///
        /// \attention All atomic object derived classes \b must
        ///            implement the compare_and_swap_impl() method.
        ///
        /// \retval true Comparison was successful and swap value was written.
        ///
        inline bool compare_and_swap (size_t compare, size_t swap);
    };

    template <class T_Object>
    inline void AtomicObject<T_Object>::init ()
    {
      static_cast<T_Object*>(this)->init_impl();
    }

    template <class T_Object>
    inline size_t AtomicObject<T_Object>::fetch () const
    {
      return static_cast<T_Object*>(this)->fetch_impl();
    }

    template <class T_Object>
    inline size_t AtomicObject<T_Object>::fetch_and_inc ()
    {
      return static_cast<T_Object*>(this)->fetch_and_inc_impl();
    }

    template <class T_Object>
    inline size_t AtomicObject<T_Object>::fetch_and_dec ()
    {
      return static_cast<T_Object*>(this)->fetch_and_dec_impl();
    }

    template <class T_Object>
    inline size_t AtomicObject<T_Object>::fetch_and_clear ()
    {
      return static_cast<T_Object*>(this)->fetch_and_clear_impl();
    }

    template <class T_Object>
    inline bool AtomicObject<T_Object>::compare_and_swap (size_t compare, size_t swap)
    {
      return static_cast<T_Object*>(this)->compare_and_swap_impl(compare, swap);
    }
  };
};

#endif // __xmi_atomic_atomicobject_h__


