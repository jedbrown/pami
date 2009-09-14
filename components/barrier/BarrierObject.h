/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef __xmi_barrier_object_h__
#define __xmi_barrier_object_h__

#include "AtomicFactory.h"

////////////////////////////////////////////////////////////////////////
///  \file sysdep/Barrier.h
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
namespace XMI
{
namespace Barrier
{
  typedef enum lockPollStatus
            {
              Uninitialized=0,
              Initialized,
              Entered,
              Done
            };

  //////////////////////////////////////////////////////////////////////
  ///  \brief Base Class for Barrier
  //////////////////////////////////////////////////////////////////////
  template <T_Object>
  class BarrierObject
    {
    public:
      typedef void  (* pollFcn)(void *);
      //////////////////////////////////////////////////////////////////
      /// \brief  Alloc and Init
      //////////////////////////////////////////////////////////////////
      inline void init();

      inline void dump(const char *string = NULL);

      //////////////////////////////////////////////////////////////////
      /// \brief  Enter a barrier
      /// This actually performs the barrier, does not return until
      /// all participants have entered.
      //////////////////////////////////////////////////////////////////
      inline void enter();

      //////////////////////////////////////////////////////////////////
      /// \brief  Enter a barrier and Poll
      /// This actually performs the barrier, does not return until
      /// all participants have entered. But 'fcn' is called, with
      /// 'arg', while polling completion of barrier.
      //////////////////////////////////////////////////////////////////
      inline void enterPoll(pollFcn fcn, void *arg);

      //////////////////////////////////////////////////////////////////
      /// \brief  Enter/Init a barrier and return, nonblocking
      //////////////////////////////////////////////////////////////////
      inline void pollInit();

      //////////////////////////////////////////////////////////////////
      /// \brief   Poll an initialized barrier, nonblocking
      /// \returns a polling status, "Done" when barrier is completed.
      //////////////////////////////////////////////////////////////////
      inline lockPollStatus poll();

      //////////////////////////////////////////////////////////////////
      /// \brief  Provide access to the raw barrier var/data
      /// This is not a usefull method, as the thing returned
      /// does not represent any externally-visible entity.
      //////////////////////////////////////////////////////////////////
      inline void * returnBarrier();

    protected:
      //////////////////////////////////////////////////////////////////
      /// \brief  Construct a lock
      //////////////////////////////////////////////////////////////////
      Barrier() { }

      //////////////////////////////////////////////////////////////////
      /// \brief  destruct a lock
      //////////////////////////////////////////////////////////////////
      ~Barrier() { /* need to call de_init... */}

    private:
    }; // class BarrierObject

template <class T_Object>
inline void BarrierObject<T_Object>::init()
{
	static_cast<T_Object*>(this)->init_impl();
}

template <class T_Object>
inline void BarrierObject<T_Object>::dump(const char *string)
{
	static_cast<T_Object*>(this)->dump_impl(string);
}

template <class T_Object>
inline void BarrierObject<T_Object>::enter()
{
	static_cast<T_Object*>(this)->enter_impl();
}

template <class T_Object>
inline void BarrierObject<T_Object>::enterPoll(pollFcn fcn, void *arg)
{
	static_cast<T_Object*>(this)->enterPoll_impl(fcn, arg);
}

template <class T_Object>
inline void BarrierObject<T_Object>::pollInit()
{
	static_cast<T_Object*>(this)->pollInit_impl();
}

template <class T_Object>
inline lockPollStatus BarrierObject<T_Object>::poll()
{
	return static_cast<T_Object*>(this)->poll_impl();
}

template <class T_Object>
inline void *BarrierObject<T_Object>::returnBarrier()
{
	return static_cast<T_Object*>(this)->returnBarrier_impl();
}

}; // namespace Barrier
}; // namespace XMI

#endif // __xmi_barrier_object_h__
