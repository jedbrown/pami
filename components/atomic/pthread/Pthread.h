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

#include "components/atomic/CounterInterface.h"

#include <pthread.h>

namespace PAMI
{
  namespace Counter
  {
    ///
    /// \brief PAMI::Counter::Interface implementation using pthread atomics
    ///
    class Pthread : public PAMI::Counter::Interface <Pthread>
    {
      public:

        friend class PAMI::Counter::Interface <Pthread>;

        inline Pthread ()
        {
          pthread_mutex_init (&_mutex, NULL);
        };

        inline ~Pthread () {};

      protected:

        // -------------------------------------------------------------------
        // PAMI::Counter::Interface<T> implementation
        // -------------------------------------------------------------------

        inline size_t fetch_impl ()
        {
          pthread_mutex_lock (&_mutex);
          size_t value = _atom;
          pthread_mutex_unlock (&_mutex);

          return value;
        };

        inline size_t fetch_and_inc_impl ()
        {
          pthread_mutex_lock (&_mutex);
          size_t value = _atom;
          _atom++;
          pthread_mutex_unlock (&_mutex);

          return value;
        };

        inline size_t fetch_and_dec_impl ()
        {
          pthread_mutex_lock (&_mutex);
          size_t value = _atom;
          _atom--;
          pthread_mutex_unlock (&_mutex);

          return value;
        };

        inline size_t fetch_and_clear_impl ()
        {
          pthread_mutex_lock (&_mutex);
          size_t value = _atom;
          _atom = 0;
          pthread_mutex_unlock (&_mutex);

          return value;
        };

        inline void clear_impl ()
        {
          pthread_mutex_lock (&_mutex);
          _atom = 0;
          pthread_mutex_unlock (&_mutex);
        };

      protected:

        size_t          _atom;
        pthread_mutex_t _mutex;
    };
  };
};


#endif // __components_atomic_pthread_Pthread_h__
