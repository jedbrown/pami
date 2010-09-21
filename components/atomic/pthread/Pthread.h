/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/atomic/pthread/Pthread.h
 * \brief ???
 */

#ifndef __components_atomic_pthread_Pthread_h__
#define __components_atomic_pthread_Pthread_h__

#include "components/atomic/Counter.h"

#include <pthread.h>

namespace PAMI
{
  namespace Counter
  {
    ///
    /// \brief CRTP interface for pthread atomic objects.
    ///
      class Pthread : public PAMI::Atomic::Interface::InPlaceCounter <Pthread>
    {
      public:
        Pthread (){}

        ~Pthread () {}

        /// \see PAMI::Atomic::AtomicObject::init
        void init_impl ()
        {
          pthread_mutex_init (&_mutex, NULL);
          fetch_and_clear_impl ();
        }

        /// \see PAMI::Atomic::AtomicObject::fetch
        inline size_t fetch_impl ()
        {
          pthread_mutex_lock (&_mutex);
          size_t value = _atom;
          pthread_mutex_unlock (&_mutex);

          return value;
        };

        /// \see PAMI::Atomic::AtomicObject::fetch_and_inc
        inline size_t fetch_and_inc_impl ()
        {
          pthread_mutex_lock (&_mutex);
          size_t value = _atom;
          _atom++;
          pthread_mutex_unlock (&_mutex);

          return value;
        };

        /// \see PAMI::Atomic::AtomicObject::fetch_and_dec
        inline size_t fetch_and_dec_impl ()
        {
          pthread_mutex_lock (&_mutex);
          size_t value = _atom;
          _atom--;
          pthread_mutex_unlock (&_mutex);

          return value;
        };

        /// \see PAMI::Atomic::AtomicObject::fetch_and_clear
        inline size_t fetch_and_clear_impl ()
        {
          pthread_mutex_lock (&_mutex);
          size_t value = _atom;
          _atom = 0;
          pthread_mutex_unlock (&_mutex);

          return value;
        };

        /// \see PAMI::Atomic::AtomicObject::clear
        inline void clear_impl ()
        {
          pthread_mutex_lock (&_mutex);
          _atom = 0;
          pthread_mutex_unlock (&_mutex);
        };

        /// \see PAMI::Atomic::AtomicObject::compare_and_swap
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


#endif // __pami_atomic_pthread_pthread_h__
