/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
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

namespace PAMI
{
  namespace Atomic
  {
  namespace Interface
  {
    ///
    /// \brief Interface for atomic counter objects
    ///
    /// This is a base classs and is not used directly.
    ///
    /// \param T_Object  Atomic counter object derived class
    ///
    template <class T_Object> class InPlaceCounter;
    template <class T_Object> class IndirCounter;
    template <class T_Object>
    class Counter
    {
      private:
	friend class InPlaceCounter<T_Object>;
	friend class IndirCounter<T_Object>;
        Counter() {
		// need equiv check for methods...
		// ENFORCE_CLASS_MEMBER(T_Object,init);
	}
        ~Counter() {}
      public:

        ///
        /// \brief Fetch the atomic counter object value
        ///
        /// \attention All atomic counter object derived classes \b must
        ///            implement the fetch_impl() method.
        ///
        /// \return Atomic counter object value
        ///
        inline size_t fetch()

        ///
        /// \brief Fetch, then increment the atomic counter object value
        ///
        /// \attention All atomic counter object derived classes \b must
        ///            implement the fetch_and_inc_impl() method.
        ///
        /// \return Atomic counter object value
        ///
        inline size_t fetch_and_inc()

        ///
        /// \brief Fetch, then decrement the atomic counter object value
        ///
        /// \attention All atomic counter object derived classes \b must
        ///            implement the fetch_and_dec_impl() method.
        ///
        /// \return Atomic counter object value
        ///
        inline size_t fetch_and_dec()

        ///
        /// \brief Fetch, then clear the atomic counter object value
        ///
        /// \attention All atomic counter object derived classes \b must
        ///            implement the fetch_and_clear_impl() method.
        ///
        /// \return Atomic counter object value
        ///
        inline size_t fetch_and_clear()

        ///
        /// \brief Clear the atomic counter object value
        ///
        /// \attention All atomic counter object derived classes \b must
        ///            implement the clear_impl() method.
        ///
        /// This is needed to make Mutexes deterministic on some implementations
        ///
        inline void clear()

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
        inline bool compare_and_swap(size_t compare, size_t swap)
    }; // class Counter

    template <class T_Object>
    inline size_t Counter<T_Object>::fetch()
    {
      return static_cast<T_Object*>(this)->fetch_impl();
    }

    template <class T_Object>
    inline size_t Counter<T_Object>::fetch_and_inc()
    {
      return static_cast<T_Object*>(this)->fetch_and_inc_impl();
    }

    template <class T_Object>
    inline size_t Counter<T_Object>::fetch_and_dec()
    {
      return static_cast<T_Object*>(this)->fetch_and_dec_impl();
    }

    template <class T_Object>
    inline size_t Counter<T_Object>::fetch_and_clear()
    {
      return static_cast<T_Object*>(this)->fetch_and_clear_impl();
    }

    template <class T_Object>
    inline void Counter<T_Object>::clear()
    {
      static_cast<T_Object*>(this)->clear_impl();
    }

    template <class T_Object>
    inline bool Counter<T_Object>::compare_and_swap(size_t compare, size_t swap)
    {
      return static_cast<T_Object*>(this)->compare_and_swap_impl(compare, swap);
    }

    ///
    /// \brief Interface for atomic counter objects that operate in-place.
    ///
    /// \param T_Object  Atomic counter object derived class
    ///
    template <class T_Object>
    class InPlaceCounter : public Counter<T_Object>
    {
      protected:
        InPlaceCounter() {};
        ~InPlaceCounter() {};

      public:
        ///
        /// \brief Initialize counter object
        ///
	/// \todo Need to find a way to initialize object by only one entity
        ///
        void init();

        inline void *returnLock();

    }; // PAMI::Atomic::Interface::InPlaceCounter class

    template <class T_Object>
    inline void InPlaceCounter<T_Object>::init()
    {
      static_cast<T_Object*>(this)->init_impl();
    }

    template <class T_Object>
    inline void * InPlaceCounter<T_Object>::returnLock()
    {
      return static_cast<T_Object*>(this)->returnLock_impl();
    }

    ///
    /// \brief Interface for atomic counter objects using indirect storage.
    ///
    /// \param T_Object  Atomic counter object derived class
    ///
    template <class T_Object>
    class IndirCounter : public Counter<T_Object>
    {
      protected:
        IndirCounter() {};
        ~IndirCounter() {};

      public:
        ///
        /// \brief Initialize counter object
        ///
	/// \todo Need to find a way to initialize object by only one entity
        ///
        void init(PAMI::Memory::MemoryManager *mm, const char *key);

        inline void *returnLock();

    }; // PAMI::Atomic::Interface::IndirCounter class

    template <class T_Object>
    inline void IndirCounter<T_Object>::init(PAMI::Memory::MemoryManager *mm, const char *key)
    {
      static_cast<T_Object*>(this)->init_impl(mm, key);
    }

    template <class T_Object>
    inline void * IndirCounter<T_Object>::returnLock()
    {
      return static_cast<T_Object*>(this)->returnLock_impl();
    }

    }; // PAMI::Atomic::Interface namespace
  };   // PAMI::Atomic namespace
};     // PAMI namespace

#endif // __components_atomic_counter_h__
