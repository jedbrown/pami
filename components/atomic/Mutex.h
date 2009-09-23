/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef __xmi_mutex_object_h__
#define __xmi_mutex_object_h__

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
///  Definitions:
///  - Node Mutex:        A mutex where the scope is across all cores on the node.
///  - Process Mutex:     A mutex where the scope is only across the cores/threads that participate in a SMP process.
///
///  Namespace:  DCMF, the messaging namespace
///  Notes:  This is currently indended for use only by the lock manager
///
///
namespace XMI
{
namespace Mutex
{
  ///
  ///  \brief Base Class for Mutex
  ///
  template <class T_Object>
  class MutexObject
    {
    public:
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

      ///
      /// \brief  Alloc and Init
      ///
      inline void init();

      ///
      /// \brief  Provide access to the raw lock var/data
      ///
      inline void * returnLock();
    protected:
      ///
      /// \brief  Construct a lock
      ///
      MutexObject() {};
      ~MutexObject() {};

    private:
    }; // class MutexObject

template <class T_Object>
inline void MutexObject<T_Object>::acquire()
{
	static_cast<T_Object*>(this)->acquire_impl();
}

template <class T_Object>
inline void MutexObject<T_Object>::release()
{
	static_cast<T_Object*>(this)->release_impl();
}

template <class T_Object>
inline bool MutexObject<T_Object>::tryAcquire()
{
	return static_cast<T_Object*>(this)->tryAcquire_impl();
}

template <class T_Object>
inline bool MutexObject<T_Object>::isLocked()
{
	return static_cast<T_Object*>(this)->isLocked_impl();
}

template <class T_Object>
inline void MutexObject<T_Object>::init()
{
	static_cast<T_Object*>(this)->init_impl();
}

template <class T_Object>
inline void *MutexObject<T_Object>::returnLock()
{
	return static_cast<T_Object*>(this)->returnLock_impl();
}

}; // namespace Mutex
}; // namespace XMI

#endif // __xmi_mutex_object_h__
