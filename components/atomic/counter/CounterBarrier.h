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
          _control (NULL),
          _lock (NULL),
          _stat (NULL),
          _participants (0),
          _data(0),
          _master(false)
        {
	}

        ~CounterBarrier () {}

        /// \see PAMI::Atomic::Interface::Barrier::init()
        void init_impl (PAMI::Memory::MemoryManager *mm, const char *key, size_t participants, bool master)
        {
	  PAMI_assert_debugf(!_control, "Re-init or object is in shmem");
          unsigned i;
          _participants = participants;
          _master = master;
	  if (T_Counter::isIndirect) {
		char k[PAMI::Memory::MMKEYSIZE];
		if (key) {
			strncpy(k, key, PAMI::Memory::MMKEYSIZE);
		} else {
			sprintf(k, "/pami-CounterBarrier-%p", this);
		}
	  	unsigned n = strlen(k);
	  	PAMI_assert_debugf(n + 2 < PAMI::Memory::MMKEYSIZE,
			"overflow mm key");
	  	k[n++] = '-';
	  	k[n+1] = '\0';
          	for (i=0; i<5; i++) {
			k[n] = "0123456789"[i];
			new (&_counter[i]) T_Counter();
			_counter[i].init(mm, k);
	  	}
	  	_control = &_counter[0];
	  	_lock = &_counter[1];
	  	_stat = &_counter[3];
	  } else {
		PAMI_assertf(T_Counter::checkCtorMm(mm),
			"counters are incompatible with given mm");
		pami_result_t rc = mm->memalign((void **)&_counters, sizeof(void *),
					5 * sizeof(T_Counter), key);
		PAMI_assertf(rc == PAMI_SUCCESS,
			"Failed to allocate counters for Barrier from provided mm");
          	for (i = 0; i < 5; ++i) {
			new (&_counters[i]) T_Counter();
			_counters[i].init();
	  	}
	  	_control = &_counters[0];
	  	_lock = &_counters[1];
	  	_stat = &_counters[3];
	  }
#if 0
fprintf(stderr, "local_barriered_ctrzero<>(%p {%p,%p,%p,%p,%p}, 5, %zd, %d)\n", _counter,
_counter[0].returnLock(),
_counter[1].returnLock(),
_counter[2].returnLock(),
_counter[3].returnLock(),
_counter[4].returnLock(),
participants, master);
#endif
        }

	static bool checkCtorMm(PAMI::Memory::MemoryManager *mm) {
		// This is an indirect object, cannot instantiate in shared memory.
		return ((mm->attrs() & PAMI::Memory::PAMI_MM_NODESCOPE) == 0);
	}
	static bool checkDataMm(PAMI::Memory::MemoryManager *mm) {
		if (T_Counter::isIndirect) {
			return T_Counter::checkDataMm(mm);
		} else {
			return T_Counter::checkCtorMm(mm) &&
				T_Counter::checkDataMm(mm); // redundant?
		}
	}

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
                phase = _control->fetch();
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
                                _control->fetch_and_dec();
                        } else {
                                _control->fetch_and_inc();
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

        T_Counter   _counter[5]; // can't put things with ctors in union,
        T_Counter   *_counters;  // so these are distinct members for now...

        T_Counter * _control;
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
