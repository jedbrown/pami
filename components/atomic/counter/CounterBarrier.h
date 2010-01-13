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

namespace XMI
{
  namespace Barrier
  {
    ///
    /// \brief Generic atomic counter based barrier implementation
    ///
    /// \param T_Counter  Atomic counter object derived class
    ///
    template <class T_Counter>
    class CounterBarrier : public XMI::Atomic::Interface::Barrier<CounterBarrier<T_Counter> >
    {
      public:
        CounterBarrier () :
          XMI::Atomic::Interface::Barrier<CounterBarrier<T_Counter> > (),
          _control (_counter[0]),
          _lock (&_counter[1]),
          _stat (&_counter[3]),
          _participants (0),
	  _data(0),
	  _master(false)
        {};

        ~CounterBarrier () {};

        /// \see XMI::Atomic::Interface::Barrier::init()
        void init_impl (XMI::SysDep *sd, size_t participants, bool master)
        {
          unsigned i;
          for (i=0; i<5; i++) _counter[i].init(sd);

          _participants = participants;
          _master = master;
	  local_barriered_ctrzero<T_Counter>(_counter, 5, participants, master);
        };

        /// \see XMI::Atomic::Interface::Barrier::enter()
        inline xmi_result_t enter_impl ()
        {
		pollInit_impl();
		while (poll_impl() != XMI::Atomic::Interface::Done);
		return XMI_SUCCESS;
	}

        inline void enterPoll_impl(XMI::Atomic::Interface::pollFcn fcn, void *arg) {
		pollInit_impl();
		while (poll_impl() != XMI::Atomic::Interface::Done) {
			fcn(arg);
		}
	}

        inline void pollInit_impl() {
		size_t phase;
		// msync...
		phase = _control.fetch();
		_lock[phase].fetch_and_inc();
		_data = phase;
		_status = XMI::Atomic::Interface::Entered;
	}

        inline XMI::Atomic::Interface::barrierPollStatus poll_impl() {
		XMI_assert(_status == XMI::Atomic::Interface::Entered);
		size_t value;
		size_t phase = _data;
		if (_lock[phase].fetch() < _participants) return XMI::Atomic::Interface::Entered;
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
		_status = XMI::Atomic::Interface::Initialized;
		return XMI::Atomic::Interface::Done;
	}
        inline void * returnBarrier_impl() { XMI_abort(); }
        inline void dump_impl(char *string) { XMI_abort(); }

      protected:

      private:

        T_Counter   _counter[5];

        T_Counter & _control;
        T_Counter * _lock;
        T_Counter * _stat;

        size_t      _participants;
        size_t      _data;
	bool        _master;
	XMI::Atomic::Interface::barrierPollStatus _status;

    };  // XMI::Barrier::CounterBarrier class
  };   // XMI::Barrier namespace
};     // XMI namespace

#endif // __components_atomic_counter_counterbarier_h__
