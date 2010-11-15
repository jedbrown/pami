/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef __components_atomic_MutexInterface_h__
#define __components_atomic_MutexInterface_h__

///
///  \file components/atomic/Mutex.h
///  \brief Mutex Objects for Hardware and Software Mutexes
///
///  This object is a portability layer that abstracts atomic locking
///  - Access to the hardware locks
///  - Access to the software locks
///  - Lock/Unlock methods are provided
///  - Allocation/Deallocation handled by constructor/destructor
///
namespace PAMI
{
  namespace Mutex
  {
    ///
    /// \brief Atomic mutex interface class
    ///
    /// This is an interface class and may not be directly instantiated.
    ///
    /// \tparam T Atomic mutex implementation class
    ///
    template <class T>
    class Interface
    {
      protected:

        inline Interface () {};
        inline ~Interface () {};

      public:

        static const bool indirect = false;

        ///
        /// \brief  Acquire a lock atomically
        ///
        inline void acquire();

        ///
        /// \brief  Release a lock atomically
        ///
        inline void release();

        ///
        /// \brief  Try to acquire a lock atomically
        ///
        inline bool tryAcquire();

        ///
        /// \brief  Test if mutex is locked
        ///
        inline bool isLocked();

    }; // class PAMI::Mutex::Interface<T>

    template <class T>
    inline void Interface<T>::acquire()
    {
      static_cast<T*>(this)->acquire_impl();
    }

    template <class T>
    inline void Interface<T>::release()
    {
      static_cast<T*>(this)->release_impl();
    }

    template <class T>
    inline bool Interface<T>::tryAcquire()
    {
      return static_cast<T*>(this)->tryAcquire_impl();
    }

    template <class T>
    inline bool Interface<T>::isLocked()
    {
      return static_cast<T*>(this)->isLocked_impl();
    }
  }; // namespace Mutex
}; // namespace PAMI

#endif // __components_atomic_MutexInterface_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//


