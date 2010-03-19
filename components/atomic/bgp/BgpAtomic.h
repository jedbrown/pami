/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/atomic/bgp/BgpAtomic.h
 * \brief ???
 */

#ifndef __components_atomic_bgp_BgpAtomic_h__
#define __components_atomic_bgp_BgpAtomic_h__

#include "components/atomic/Counter.h"
#include "components/atomic/Mutex.h"

#include <spi/bgp_SPI.h>
#include <bpcore/bgp_atomic_ops.h>

namespace XMI
{
  namespace Atomic
  {
    ///
    /// \brief CRTP interface for bgp atomic objects.
    ///
    class BgpAtomic : public Interface::Counter <BgpAtomic>
    {
      public:
        BgpAtomic () :
            Interface::Counter <BgpAtomic> ()
        {};

        ~BgpAtomic () {};

        /// \see XMI::Atomic::AtomicObject::init
        void init_impl (XMI::Memory::MemoryManager *mm)
        {
          //_atom = _BGP_ATOMIC_INIT(0);
          fetch_and_clear_impl ();
        };

        /// \see XMI::Atomic::AtomicObject::fetch
        inline size_t fetch_impl ()
        {
          return _bgp_fetch_and_add (&_atom, 0);
        };

        /// \see XMI::Atomic::AtomicObject::fetch_and_inc
        inline size_t fetch_and_inc_impl ()
        {
          return _bgp_fetch_and_add (&_atom, 1);
        };

        /// \see XMI::Atomic::AtomicObject::fetch_and_dec
        inline size_t fetch_and_dec_impl ()
        {
          return _bgp_fetch_and_add (&_atom, (uint32_t)-1);
        };

        /// \see XMI::Atomic::AtomicObject::fetch_and_clear
        inline size_t fetch_and_clear_impl ()
        {
          return _bgp_fetch_and_and (&_atom, 0);
        };

        /// \see XMI::Atomic::AtomicObject::compare_and_swap
        inline bool compare_and_swap_impl (size_t compare, size_t swap)
        {
          size_t tmp = compare;
          return (bool) _bgp_compare_and_swap (&_atom, &tmp, swap);
        };

        inline void * returnLock()
        {
          return &_atom;
        };

      protected:

        _BGP_Atomic _atom;
    };


    class BgpMutex : public Interface::Mutex <BgpMutex>
    {
      public:

        inline BgpMutex () :
          Interface::Mutex <BgpMutex> (),
          _atomic (0)
        {};

        ~BgpMutex () {};

        /// \see XMI::Atomic::Interface::Mutex::acquire
        inline void acquire_impl ()
        {
          while (!_bgp_test_and_set((_BGP_Atomic *)&_atomic, 1));
        };

        /// \see XMI::Atomic::Interface::Mutex::release
        inline void release_impl ()
        {
          _atomic = 0;
        };

        /// \see XMI::Atomic::Interface::Mutex::tryAcquire
        inline bool tryAcquire_impl ()
        {
          return (_bgp_test_and_set((_BGP_Atomic *)&_atomic, 1) != 0);
        };

        /// \see XMI::Atomic::Interface::Mutex::isLocked
        inline bool isLocked_impl ()
        {
          return (_atomic != 0);
        };

        /// \see XMI::Atomic::Interface::Mutex::init
        inline void init_impl (XMI::Memory::MemoryManager *mm)
        {
          // Noop
        };

        /// \see XMI::Atomic::Interface::Mutex::returnLock
        inline void * returnLock_impl ()
        {
          return (void *) & _atomic;
        };

      private:

        volatile uint32_t _atomic __attribute__ ((aligned(8)));
    };
  };
};


#endif // __xmi_atomic_bgp_bgpatomic_h__
