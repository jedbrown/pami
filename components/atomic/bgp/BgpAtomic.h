/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/atomic/bgp/BgpAtomic.h
 * \brief ???
 */

#ifndef __xmi_atomic_bgp_bgpatomic_h__
#define __xmi_atomic_bgp_bgpatomic_h__

#include "components/atomic/AtomicObject.h"

#include <spi/bgp_SPI.h>
#include <bpcore/bgp_atomic_ops.h>

namespace XMI
{
  namespace Atomic
  {
    ///
    /// \brief CRTP interface for bgp atomic objects.
    ///
    class BgpAtomic : public AtomicObject <BgpAtomic>
    {
      public:
        BgpAtomic () :
            AtomicObject <BgpAtomic> ()
        {};

        ~BgpAtomic () {};

        /// \see XMI::Atomic::AtomicObject::init
        void init_impl ()
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

      protected:

        _BGP_Atomic _atom;
    };
  };
};


#endif // __xmi_atomic_bgp_bgpatomic_h__


