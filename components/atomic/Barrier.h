/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef __components_atomic_Barrier_h__
#define __components_atomic_Barrier_h__

#include <pami.h>

#include "components/memory/MemoryManager.h"

////////////////////////////////////////////////////////////////////////
///  \file components/atomic/Barrier.h
///  \brief Barrier Objects for Hardware and Software Barrieres
///
///  This object is a portability layer that abstracts local barriers
///  - Access to the hardware barriers
///  - Access to the software barriers
///  - Enter methods provided
///  - Allocation/Deallocation handled by constructor/destructor
///
///  Definitions:
///  - Hardware Barrier:  a barrier that is assisted by hardware
///  - Software Barrier, ie an atomic memory barrier
///
///  Namespace:  DCMF, the messaging namespace
///  Notes:  This is currently indended for use only by the lock manager
///
////////////////////////////////////////////////////////////////////////

/// \page pami_atomic_examples
///
/// How to use a Barrier atomic object:
///
/// // following changes with desired type of barrier...
/// #include "collectives/components/atomic/bgp/LockBoxBarrier.h"
/// typedef PAMI::Barrier::BGP::LockBoxNodeCoreBarrier MY_BARRIER;
///
/// MY_BARRIER _barrier;
/// pami_result_t status;
/// _barrier.init(sysdep, status); // get 'sysdep' wherever you can...
/// PAMI_assert(status == PAMI_SUCCESS);
///
/// _barrier.enter(); // perform actual barrier
///
///
namespace PAMI
{
namespace Atomic
{
namespace Interface
{
  enum barrierPollStatus
            {
              Uninitialized=0,
              Initialized,
              Entered,
              Done
            };
  typedef void  (* pollFcn)(void *);

  ///
  /// \brief Barrier object interface
  ///
  /// \param T  Barrier object derived class
  ///
  template <class T>
  class Barrier
    {
    public:
      ///
      /// \brief  Construct a barrier
      ///
      Barrier() { }

      ///
      /// \brief  destruct a barrier
      ///
      ~Barrier() { /* need to call de_init... */}

      ///
      /// \brief Initialize the local barrier objecti
      ///
      /// \param[in] mm     Memory Manager
      /// \param[in] participants Number of participants for the barrier
      /// \param[in] participants Is caller the master of group
      ///
      /// \todo Need to find a way to initialize object by only one entity
      ///
      inline void init(PAMI::Memory::MemoryManager *mm, size_t participants, bool master);


      ///
      /// \brief Enter a local blocking barrier operation
      ///
      /// Does not return until all participants have entered the barrier.
      ///
      inline pami_result_t enter();

      ///
      /// \brief  Enter a barrier and Poll
      ///
      /// This actually performs the barrier, does not return until
      /// all participants have entered. But 'fcn' is called, with
      /// 'arg', while polling completion of barrier.
      ///
      inline void enterPoll(pollFcn fcn, void *arg);

      ///
      /// \brief  Enter/Init a barrier and return, nonblocking
      ///
      inline void pollInit();

      ///
      /// \brief   Poll an initialized barrier, nonblocking
      /// \returns a polling status, "Done" when barrier is completed.
      ///
      inline barrierPollStatus poll();

      ///
      /// \brief  Provide access to the raw barrier var/data
      /// This is not a usefull method, as the thing returned
      /// does not represent any externally-visible entity.
      ///
      inline void * returnBarrier();

      ///
      /// \brief  Debug routine to dump state of a barrier
      ///
      inline void dump(const char *string = NULL);

    private:
    }; // class Barrier

template <class T>
inline void Barrier<T>::init(PAMI::Memory::MemoryManager *mm, size_t participants, bool master)
{
        static_cast<T*>(this)->init_impl(mm, participants, master);
}

template <class T>
inline pami_result_t Barrier<T>::enter()
{
        return static_cast<T*>(this)->enter_impl();
}

template <class T>
inline void Barrier<T>::dump(const char *string)
{
        static_cast<T*>(this)->dump_impl(string);
}

template <class T>
inline void Barrier<T>::enterPoll(pollFcn fcn, void *arg)
{
        static_cast<T*>(this)->enterPoll_impl(fcn, arg);
}

template <class T>
inline void Barrier<T>::pollInit()
{
        static_cast<T*>(this)->pollInit_impl();
}

template <class T>
inline barrierPollStatus Barrier<T>::poll()
{
        return static_cast<T*>(this)->poll_impl();
}

template <class T>
inline void *Barrier<T>::returnBarrier()
{
        return static_cast<T*>(this)->returnBarrier_impl();
}

}; // namespace Interface
}; // namespace Atomic
}; // namespace PAMI

#endif // __components_atomic_barrier_h__
