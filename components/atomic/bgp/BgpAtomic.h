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
#undef TRACE_ERR
#define TRACE_ERR(x) fprintf x

namespace PAMI
{
  namespace Atomic
  {
  namespace BGP
  {
    ///
    /// \brief CRTP interface for bgp atomic objects.
    ///
    class BgpAtomic : public PAMI::Atomic::Interface::Counter <BgpAtomic>
    {
      public:
        BgpAtomic () :
            PAMI::Atomic::Interface::Counter <BgpAtomic> ()
        {};

        ~BgpAtomic () {};

        /// \see PAMI::Atomic::AtomicObject::init
        void init_impl (PAMI::Memory::MemoryManager *mm)
        {
          //_atom = _BGP_ATOMIC_INIT(0);
          fetch_and_clear_impl ();
        };

        /// \see PAMI::Atomic::AtomicObject::fetch
        inline size_t fetch_impl ()
        {
          return _bgp_fetch_and_add (&_atom, 0);
        };

        /// \see PAMI::Atomic::AtomicObject::fetch_and_inc
        inline size_t fetch_and_inc_impl ()
        {
          return _bgp_fetch_and_add (&_atom, 1);
        };

        /// \see PAMI::Atomic::AtomicObject::fetch_and_dec
        inline size_t fetch_and_dec_impl ()
        {
          return _bgp_fetch_and_add (&_atom, (uint32_t)-1);
        };

        /// \see PAMI::Atomic::AtomicObject::fetch_and_clear
        inline size_t fetch_and_clear_impl ()
        {
          return _bgp_fetch_and_and (&_atom, 0);
        };

        /// \see PAMI::Atomic::AtomicObject::clear
        inline void clear_impl ()
        {
          _atom.atom = 0;
        };

        /// \see PAMI::Atomic::AtomicObject::compare_and_swap
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
}; // namespace BGP
}; // namespace Atomic
namespace Counter {
namespace BGP {

    class BgpProcCounter : public PAMI::Atomic::Interface::Counter <BgpProcCounter>
    {
      public:

        inline BgpProcCounter () :
          PAMI::Atomic::Interface::Counter <BgpProcCounter> (),
          _atomic (0)
        {};

        ~BgpProcCounter () {};

        inline size_t fetch_impl() { return _bgp_fetch_and_add((_BGP_Atomic *)&_atomic, 0); };

        inline size_t fetch_and_inc_impl() { return _bgp_fetch_and_add((_BGP_Atomic *)&_atomic, 1); }

        inline size_t fetch_and_dec_impl() { return _bgp_fetch_and_add((_BGP_Atomic *)&_atomic, -1); }

        inline size_t fetch_and_clear_impl() { return _bgp_fetch_and_and((_BGP_Atomic *)&_atomic, 0); }
        inline void clear_impl() { _atomic = 0; }

        inline void init_impl (PAMI::Memory::MemoryManager *mm)
        {
          // Noop
        };

        /// \see PAMI::Atomic::Interface::Mutex::returnLock
        inline void * returnLock_impl ()
        {
          return (void *) & _atomic;
        };

      private:

        volatile uint32_t _atomic __attribute__ ((aligned(8)));
    };

    class BgpNodeCounter : public PAMI::Atomic::Interface::Counter <BgpNodeCounter>
    {
      public:

        inline BgpNodeCounter () :
          PAMI::Atomic::Interface::Counter <BgpNodeCounter> (),
          _atomic (NULL)
        {};

        ~BgpNodeCounter () {};

        inline size_t fetch_impl() { return _bgp_fetch_and_add((_BGP_Atomic *)_atomic, 0); };

        inline size_t fetch_and_inc_impl() { return _bgp_fetch_and_add((_BGP_Atomic *)_atomic, 1); }

        inline size_t fetch_and_dec_impl() { return _bgp_fetch_and_add((_BGP_Atomic *)_atomic, -1); }

        inline size_t fetch_and_clear_impl() { return _bgp_fetch_and_and((_BGP_Atomic *)_atomic, 0); }
        inline void clear_impl() { *_atomic = 0; }

        inline void init_impl (PAMI::Memory::MemoryManager *mm)
        {
          _atomic = NULL;
            TRACE_ERR((stderr,  "%s enter\n", __PRETTY_FUNCTION__));
          mm->memalign((void **)&_atomic, 8, sizeof(*_atomic));
          PAMI_assertf(_atomic, "Failed to get shmem for BgpNodeCounter");
        };

        /// \see PAMI::Atomic::Interface::Mutex::returnLock
        inline void * returnLock_impl ()
        {
          return (void *) _atomic;
        };

      private:

        volatile uint32_t *_atomic;
    };

}; // namespace BGP
}; // namespace Counter
namespace Mutex {
namespace BGP {

    class BgpProcMutex : public PAMI::Atomic::Interface::Mutex <BgpProcMutex>
    {
      public:

        inline BgpProcMutex () :
          PAMI::Atomic::Interface::Mutex <BgpProcMutex> (),
          _atomic (0)
        {};

        ~BgpProcMutex () {};

        /// \see PAMI::Atomic::Interface::Mutex::acquire
        inline void acquire_impl ()
        {
          while (!_bgp_test_and_set((_BGP_Atomic *)&_atomic, 1));
        };

        /// \see PAMI::Atomic::Interface::Mutex::release
        inline void release_impl ()
        {
          _atomic = 0;
        };

        /// \see PAMI::Atomic::Interface::Mutex::tryAcquire
        inline bool tryAcquire_impl ()
        {
          return (_bgp_test_and_set((_BGP_Atomic *)&_atomic, 1) != 0);
        };

        /// \see PAMI::Atomic::Interface::Mutex::isLocked
        inline bool isLocked_impl ()
        {
          return (_atomic != 0);
        };

        /// \see PAMI::Atomic::Interface::Mutex::init
        inline void init_impl (PAMI::Memory::MemoryManager *mm)
        {
        };

        /// \see PAMI::Atomic::Interface::Mutex::returnLock
        inline void * returnLock_impl ()
        {
          return (void *) & _atomic;
        };

      private:

        volatile uint32_t _atomic __attribute__ ((aligned(8)));
    };

    class BgpNodeMutex : public PAMI::Atomic::Interface::Mutex <BgpNodeMutex>
    {
      public:

        inline BgpNodeMutex () :
          PAMI::Atomic::Interface::Mutex <BgpNodeMutex> (),
          _atomic (NULL)
        {};

        ~BgpNodeMutex () {};

        /// \see PAMI::Atomic::Interface::Mutex::acquire
        inline void acquire_impl ()
        {
          while (!_bgp_test_and_set((_BGP_Atomic *)_atomic, 1));
        };

        /// \see PAMI::Atomic::Interface::Mutex::release
        inline void release_impl ()
        {
          *_atomic = 0;
        };

        /// \see PAMI::Atomic::Interface::Mutex::tryAcquire
        inline bool tryAcquire_impl ()
        {
          return (_bgp_test_and_set((_BGP_Atomic *)_atomic, 1) != 0);
        };

        /// \see PAMI::Atomic::Interface::Mutex::isLocked
        inline bool isLocked_impl ()
        {
          return (*_atomic != 0);
        };

        /// \see PAMI::Atomic::Interface::Mutex::init
        inline void init_impl (PAMI::Memory::MemoryManager *mm)
        {
          _atomic = NULL;
            TRACE_ERR((stderr,  "%s enter\n", __PRETTY_FUNCTION__));
          mm->memalign((void **)&_atomic, 8, sizeof(*_atomic));
          PAMI_assertf(_atomic, "Failed to get shmem for BgpNodeMutex");
        };

        /// \see PAMI::Atomic::Interface::Mutex::returnLock
        inline void * returnLock_impl ()
        {
          return (void *) _atomic;
        };

      private:

        volatile uint32_t *_atomic;
    };
}; // namespace BGP
}; // namespace Mutex
}; // namespace PAMI


#endif // __pami_atomic_bgp_bgpatomic_h__
