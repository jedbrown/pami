/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/atomic/pthread/Pthread.h
 * \brief ???
 */

#ifndef __xmi_atomic_pthread_pthread_h__
#define __xmi_atomic_pthread_pthread_h__

#include "components/atomic/Counter.h"

#include <pthread.h>

namespace XMI
{
  namespace Counter
  {
    ///
    /// \brief CRTP interface for pthread atomic objects.
    ///
    template <class T_Sysdep>
      class Pthread : public XMI::Atomic::Interface::Counter <T_Sysdep, Pthread<T_Sysdep> >
    {
      public:
        Pthread (){}

        ~Pthread () {}

        /// \see XMI::Atomic::AtomicObject::init
        void init_impl (T_Sysdep *sd)
        {
          pthread_mutex_init (&_mutex, NULL);
          fetch_and_clear_impl ();
        }

        /// \see XMI::Atomic::AtomicObject::fetch
        inline size_t fetch_impl ()
        {
          pthread_mutex_lock (&_mutex);
          size_t value = _atom;
          pthread_mutex_unlock (&_mutex);

          return value;
        };

        /// \see XMI::Atomic::AtomicObject::fetch_and_inc
        inline size_t fetch_and_inc_impl ()
        {
          pthread_mutex_lock (&_mutex);
          size_t value = _atom;
          _atom++;
          pthread_mutex_unlock (&_mutex);

          return value;
        };

        /// \see XMI::Atomic::AtomicObject::fetch_and_dec
        inline size_t fetch_and_dec_impl ()
        {
          pthread_mutex_lock (&_mutex);
          size_t value = _atom;
          _atom--;
          pthread_mutex_unlock (&_mutex);

          return value;
        };

        /// \see XMI::Atomic::AtomicObject::fetch_and_clear
        inline size_t fetch_and_clear_impl ()
        {
          pthread_mutex_lock (&_mutex);
          size_t value = _atom;
          _atom = 0;
          pthread_mutex_unlock (&_mutex);

          return value;
        };

        /// \see XMI::Atomic::AtomicObject::compare_and_swap
        inline bool compare_and_swap_impl (size_t compare, size_t swap)
        {
          bool did_swap = false;
          pthread_mutex_lock (&_mutex);
          if (_atom == compare)
          {
            _atom = swap;
            did_swap = true;
          }
          pthread_mutex_unlock (&_mutex);

          return did_swap;
        }

      protected:

        size_t          _atom;
        pthread_mutex_t _mutex;
    };
  };
};


#endif // __xmi_atomic_pthread_pthread_h__
