/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/atomic/counter/CounterBarrier.h
/// \brief ???
///
#ifndef __components_atomic_counter_CounterBarrier_h__
#define __components_atomic_counter_CounterBarrier_h__

#include "components/atomic/Barrier.h"
#include "util/common.h"

namespace PAMI
{
  namespace Barrier
  {
    ///
    /// \brief Generic atomic counter based barrier implementation
    ///
    /// \param T_Counter  Atomic counter object derived class
    ///
    template <class T_Counter>
    class CounterBarrier : public PAMI::Atomic::Interface::Barrier<CounterBarrier<T_Counter> >
    {
      public:
        CounterBarrier () :
          PAMI::Atomic::Interface::Barrier<CounterBarrier<T_Counter> > (),
          _control (_counter[0]),
          _lock (&_counter[1]),
          _stat (&_counter[3]),
          _participants (0),
          _data(0),
          _master(false)
        {};

        ~CounterBarrier () {};

        /// \see PAMI::Atomic::Interface::Barrier::init()
        void init_impl (PAMI::Memory::MemoryManager *mm, size_t participants, bool master)
        {
          unsigned i;
          for (i=0; i<5; i++) _counter[i].init(mm);

          _participants = participants;
          _master = master;
#if 0
fprintf(stderr, "local_barriered_ctrzero<>(%p {%p,%p,%p,%p,%p}, 5, %zd, %d)\n", _counter,
_counter[0].returnLock(),
_counter[1].returnLock(),
_counter[2].returnLock(),
_counter[3].returnLock(),
_counter[4].returnLock(),
participants, master);
#endif
          local_barriered_ctrzero<T_Counter>(_counter, 5, participants, master);
        };

        /// \see PAMI::Atomic::Interface::Barrier::enter()
        inline pami_result_t enter_impl ()
        {
                pollInit_impl();
                while (poll_impl() != PAMI::Atomic::Interface::Done);
                return PAMI_SUCCESS;
        }

        inline void enterPoll_impl(PAMI::Atomic::Interface::pollFcn fcn, void *arg) {
                pollInit_impl();
                while (poll_impl() != PAMI::Atomic::Interface::Done) {
                        fcn(arg);
                }
        }

        inline void pollInit_impl() {
                size_t phase;
                // msync...
                phase = _control.fetch();
                _lock[phase].fetch_and_inc();
                _data = phase;
                _status = PAMI::Atomic::Interface::Entered;
        }

        inline PAMI::Atomic::Interface::barrierPollStatus poll_impl() {
                PAMI_assert(_status == PAMI::Atomic::Interface::Entered);
                size_t value;
                size_t phase = _data;
                if (_lock[phase].fetch() < _participants) return PAMI::Atomic::Interface::Entered;
                _lock[phase].fetch_and_inc();
                do {
                        value = _lock[phase].fetch();
                } while (value > 0 && value < (2 * _participants));
                if (_master) {
                        if (phase) {
                                _control.fetch_and_dec();
                        } else {
                                _control.fetch_and_inc();
                        }
                        _stat[phase].fetch_and_clear();
                        _lock[phase].fetch_and_clear();
                } else {
                        // wait until master releases the barrier by clearing the lock
                        while (_lock[phase].fetch() > 0);
                }
                _status = PAMI::Atomic::Interface::Initialized;
                return PAMI::Atomic::Interface::Done;
        }
        inline void * returnBarrier_impl() { PAMI_abortf("%s<%d>\n",__FILE__,__LINE__); }
        inline void dump_impl(char *string) { PAMI_abortf("%s<%d>\n",__FILE__,__LINE__); }

      protected:

      private:

        T_Counter   _counter[5];

        T_Counter & _control;
        T_Counter * _lock;
        T_Counter * _stat;

        size_t      _participants;
        size_t      _data;
        bool        _master;
        PAMI::Atomic::Interface::barrierPollStatus _status;

    };  // PAMI::Barrier::CounterBarrier class
  };   // PAMI::Barrier namespace
};     // PAMI namespace

#endif // __components_atomic_counter_counterbarier_h__
