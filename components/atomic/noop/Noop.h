/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/atomic/noop/Noop.h
 * \brief ???
 */

#ifndef __components_atomic_noop_Noop_h__
#define __components_atomic_noop_Noop_h__

#include "SysDep.h"
#include "components/atomic/Counter.h"
#include "components/atomic/Mutex.h"

namespace XMI
{
  namespace Atomic
  {
    ///
    /// \brief CRTP interface for a "noop" atomic counter.
    ///
    class Noop : public Interface::Counter <Noop>
    {
      public:
        Noop () :
            Interface::Counter <Noop> ()
        {};

        ~Noop () {};

        /// \see XMI::Atomic::AtomicObject::init
        void init_impl (XMI::SysDep *sd)
        {
          return;
        };

        /// \see XMI::Atomic::AtomicObject::fetch
        inline size_t fetch_impl ()
        {
          return 0;
        };

        /// \see XMI::Atomic::AtomicObject::fetch_and_inc
        inline size_t fetch_and_inc_impl ()
        {
          return 0;
        };

        /// \see XMI::Atomic::AtomicObject::fetch_and_dec
        inline size_t fetch_and_dec_impl ()
        {
          return 0;
        };

        /// \see XMI::Atomic::AtomicObject::fetch_and_clear
        inline size_t fetch_and_clear_impl ()
        {
          return 0;
        };

        /// \see XMI::Atomic::AtomicObject::compare_and_swap
        inline bool compare_and_swap_impl (size_t compare, size_t swap)
        {
          return true;
        }
    };


    ///
    /// \brief CRTP interface for a "noop" atomic mutex.
    ///
    class NoopMutex : public Interface::Mutex <NoopMutex>
    {
      public:
        NoopMutex () :
          Interface::Mutex <NoopMutex> ()
        {};

        ~NoopMutex () {};

        /// \see XMI::Atomic::Interface::Mutex::acquire
        inline void acquire_impl () {};

        /// \see XMI::Atomic::Interface::Mutex::release
        inline void release_impl () {};

        /// \see XMI::Atomic::Interface::Mutex::tryAcquire
        inline bool tryAcquire_impl () { return true; };

        /// \see XMI::Atomic::Interface::Mutex::isLocked
        inline bool isLocked_impl () { return false; };

        /// \see XMI::Atomic::Interface::Mutex::init
        inline void init_impl (XMI::SysDep *sd) {};

        /// \see XMI::Atomic::Interface::Mutex::returnLock
        inline void * returnLock_impl () { return NULL; };
    };
  };
};


#endif // __xmi_atomic_noop_noop_h__
