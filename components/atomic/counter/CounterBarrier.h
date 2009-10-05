/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/atomic/counter/CounterBarrier.h
/// \brief ???
///
#ifndef __components_atomic_counter_counterbarrier_h__
#define __components_atomic_counter_counterbarrier_h__

#include "components/atomic/Barrier.h"

namespace XMI
{
  namespace Atomic
  {
    ///
    /// \brief Generic atomic counter based barrier implementation
    ///
    /// \param T_Counter  Atomic counter object derived class
    ///
    template <class T_Sysdep, class T_Counter>
    class CounterBarrier : public Interface::Barrier<T_Sysdep, CounterBarrier<T_Sysdep, T_Counter> >
    {
      public:
        CounterBarrier () :
          Interface::Barrier<T_Sysdep, CounterBarrier<T_Sysdep, T_Counter> > (),
          _control (_counter[0]),
          _lock (&_counter[1]),
          _status (&_counter[3]),
          _participants (0)
        {};

        ~CounterBarrier () {};

        /// \see XMI::Atomic::Interface::Barrier::init()
        void init_impl (T_Sysdep *sd, size_t participants)
        {
          unsigned i;
          for (i=0; i<5; i++) _counter[i].init(sd);

          _participants = participants;
        };

        /// \see XMI::Atomic::Interface::Barrier::enter()
        inline xmi_result_t enter_impl ()
        {
          // Determine the lock phase, either 0 or 1. This safely prevents
          // subsequent barriers from corrupting active counters.

          size_t phase = _control.fetch ();

          // Enter the barrier by incrementing the lock counter. This
          // participant is the master if the value returned is zero.

          size_t participant = _lock[phase].fetch_and_inc ();

          // Busy wait until all participants have entered the barrier.

          size_t parties = _participants;
          while (poll(phase, parties, participant) == XMI_EAGAIN);

          return XMI_SUCCESS;
        };

        inline void enterPoll_impl(pollFcn fcn, void *arg) { XMI_abort(); }
        inline void pollInit_impl() { XMI_abort(); }
        inline barrierPollStatus poll_impl() { XMI_abort(); }
        inline void * returnBarrier_impl() { XMI_abort(); }
        inline void dump_impl(char *string) { XMI_abort(); }

      protected:

        inline xmi_result_t poll (size_t phase, size_t parties, size_t participant)
        {
          // Return immediately if not all participants have checked in by
          // incrementing the lock.

          if (_lock[phase].fetch() < parties) return XMI_EAGAIN;

          size_t value = 0;

          // All participants have entered the barrier and must block until the
          // master participant atomically clears the lock and exits.

          _lock[phase].fetch_and_inc ();
          do
          {
            value = _lock[phase].fetch ();
          } while (value > 0 && value < (2 * parties));

          if (participant == 0)
          {
            // Flip the barrier phase.
            if (phase == 1)
              _control.fetch_and_dec ();
            else
              _control.fetch_and_inc ();

            // Clear the status counters. .... why?
            _status[phase].fetch_and_clear ();

            // Clear the lock. This allows the other participants to exit.
            _lock[phase].fetch_and_clear ();
          }
          else
          {
            // Wait until master releases the barrier by clearing the lock
            while (_lock[phase].fetch() > 0);
          }

          return XMI_SUCCESS;
        };

      private:

        T_Counter   _counter[5];

        T_Counter & _control;
        T_Counter * _lock;
        T_Counter * _status;

        size_t      _participants;

    };  // XMI::Atomic::CounterBarrier class
  };   // XMI::Atomic namespace
};     // XMI namespace

#endif // __components_atomic_counter_counterbarier_h__
