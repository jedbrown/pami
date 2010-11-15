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

#include "components/atomic/BarrierInterface.h"
#include "components/atomic/indirect/IndirectBarrier.h"
#include "util/common.h"

namespace PAMI
{
  namespace Barrier
  {
    ///
    /// \brief Atomic counter barrier implementation
    ///
    /// \tparam T PAMI::Counter::Interface implementation class
    ///
    template <class T>
    class Counter : public PAMI::Barrier::Interface<PAMI::Barrier::Counter<T> >
    {
      public:

        friend class PAMI::Barrier::Interface<PAMI::Barrier::Counter<T> >;

        inline Counter (size_t participants, bool master) :
          PAMI::Barrier::Interface<PAMI::Barrier::Counter<T> > (participants, master),
          _participants (participants),
          _master(master),
          _data(0),
          _active(false)
        {};

        inline ~Counter () {};

      protected:

        // -------------------------------------------------------------------
        // PAMI::Barrier::Interface<T> implementation
        // -------------------------------------------------------------------

        inline void enter_impl ()
        {
          this->begin();
          while (this->poll());
          this->end();
        };

        template <class T_Functor>
        inline void enter_impl (T_Functor & functor)
        {
          this->begin();
          while (this->poll())
            functor();
          this->end();
        };

        inline void begin_impl ()
        {
          size_t phase = _control.fetch();
          _lock[phase].fetch_and_inc(); // enter the barrier.
          _data = phase;
          _active = true;
        };

        inline bool poll_impl ()
        {
          PAMI_assert(_active == true);

          size_t value;
          size_t phase = _data;

          if (_lock[phase].fetch() < _participants)
            return true; // barrier is still active.

          _lock[phase].fetch_and_inc();

          const size_t test = 2 * _participants;
          do
            {
              value = _lock[phase].fetch();
            }
          while (value > 0 && value < test);

          if (_master)
            {
              if (phase)
                {
                  _control.fetch_and_dec();
                }
              else
                {
                  _control.fetch_and_inc();
                }

              _stat[phase].fetch_and_clear();
              _lock[phase].fetch_and_clear();
            }
          else
            {
              // wait until master releases the barrier by clearing the lock
              while (_lock[phase].fetch() > 0);
            }

          _active = false;

          return false; // barrier is now complete
        };

        inline void end_impl ()
        {
          // _active is set at the end of the poll() method
        };

        inline void dump_impl (const char * string = NULL)
        {
          PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
        };

      private:

        T _control;
        T _lock[2];
        T _stat[2];

        size_t _participants;
        bool   _master;

        size_t _data;
        bool   _active;

    }; // PAMI::Barrier::Counter<T> class


    ///
    /// \brief Barrier implementation using an "indirect" counter class
    ///
    /// This class implements an indirect barrier via an indirect counter. The
    /// indirect interfaces are implemented merely as a pass-through to the
    /// counter indirect interface implementation.
    ///
    /// \note There is a subtle difference between an "indirect barrier counter"
    ///       and an "indirect counter barrier" (this class).
    ///
    template <class T>
    class IndirectCounter : public PAMI::Barrier::Interface< PAMI::Barrier::IndirectCounter<T> >,
                            public PAMI::Atomic::Indirect< PAMI::Barrier::IndirectCounter<T> >
    {
      public:

        friend class PAMI::Barrier::Interface< PAMI::Barrier::IndirectCounter<T> >;
        friend class PAMI::Atomic::Indirect< PAMI::Barrier::IndirectCounter<T> >;

        inline IndirectCounter(size_t participants, bool master) :
          PAMI::Barrier::Interface< PAMI::Barrier::IndirectCounter<T> > (participants, master),
          PAMI::Atomic::Indirect< PAMI::Barrier::IndirectCounter<T> > (),
          _participants (participants),
          _master(master),
          _data(0),
          _active(false)
        {};

        inline ~IndirectCounter() {};

      protected:

        // -------------------------------------------------------------------
        // PAMI::Atomic::Indirect<T> implementation
        // -------------------------------------------------------------------

        template <class T_MemoryManager>
        inline void init_impl (T_MemoryManager * mm, const char * key)
        {
          char newkey[PAMI::Memory::MMKEYSIZE];

          snprintf (newkey, PAMI::Memory::MMKEYSIZE - 1, "%s-control", key);
          _control.init (mm, newkey);

          snprintf (newkey, PAMI::Memory::MMKEYSIZE - 1, "%s-lock0", key);
          _lock[0].init (mm, newkey);

          snprintf (newkey, PAMI::Memory::MMKEYSIZE - 1, "%s-lock1", key);
          _lock[1].init (mm, newkey);

          snprintf (newkey, PAMI::Memory::MMKEYSIZE - 1, "%s-stat0", key);
          _stat[0].init (mm, newkey);

          snprintf (newkey, PAMI::Memory::MMKEYSIZE - 1, "%s-stat1", key);
          _stat[1].init (mm, newkey);
        };

        inline void clone_impl (IndirectCounter & atomic)
        {
          _control.clone (atomic._control);
          _lock[0].clone (atomic._lock[0]);
          _lock[1].clone (atomic._lock[1]);
          _stat[0].clone (atomic._stat[0]);
          _stat[1].clone (atomic._stat[1]);

          _participants = atomic._participants;
          _master = atomic._master;

          _data = atomic._data;
          _active = atomic._active;
        };

        // -------------------------------------------------------------------
        // PAMI::Barrier::Interface<T> implementation
        // -------------------------------------------------------------------

        inline void enter_impl ()
        {
          this->begin();
          while (this->poll());
          this->end();
        };

        template <class T_Functor>
        inline void enter_impl (T_Functor & functor)
        {
          this->begin();
          while (this->poll())
            functor();
          this->end();
        };

        inline void begin_impl ()
        {
          size_t phase = _control.fetch();
          _lock[phase].fetch_and_inc(); // enter the barrier.
          _data = phase;
          _active = true;
        };

        inline bool poll_impl ()
        {
          PAMI_assert(_active == true);

          size_t value;
          size_t phase = _data;

          if (_lock[phase].fetch() < _participants)
            return true; // barrier is still active.

          _lock[phase].fetch_and_inc();

          const size_t test = 2 * _participants;
          do
            {
              value = _lock[phase].fetch();
            }
          while (value > 0 && value < test);

          if (_master)
            {
              if (phase)
                {
                  _control.fetch_and_dec();
                }
              else
                {
                  _control.fetch_and_inc();
                }

              _stat[phase].fetch_and_clear();
              _lock[phase].fetch_and_clear();
            }
          else
            {
              // wait until master releases the barrier by clearing the lock
              while (_lock[phase].fetch() > 0);
            }

          _active = false;

          return false; // barrier is now complete
        };

        inline void end_impl ()
        {
          // _active is set at the end of the poll() method
        };

        inline void dump_impl (const char * string = NULL)
        {
          PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
        };

      private:

        T _control;
        T _lock[2];
        T _stat[2];

        size_t _participants;
        bool   _master;

        size_t _data;
        bool   _active;

    }; // PAMI::Barrier::IndirectCounter<T> class
  };   // PAMI::Barrier namespace
};     // PAMI namespace

#endif // __components_atomic_counter_counterbarier_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
