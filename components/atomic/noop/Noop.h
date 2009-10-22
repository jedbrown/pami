/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
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

#include "components/atomic/Counter.h"
#include "SysDep.h"

namespace XMI
{
  namespace Atomic
  {
    ///
    /// \brief CRTP interface for "noop" atomic objects.
    ///
    class Noop : public Interface::Counter <XMI::Interface::NullSysDep, Noop>
    {
      public:
        Noop () :
            Interface::Counter <XMI::Interface::NullSysDep, Noop> ()
        {};

        ~Noop () {};

        /// \see XMI::Atomic::AtomicObject::init
        void init_impl (XMI::Interface::NullSysDep *sd)
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
  };
};


#endif // __xmi_atomic_noop_noop_h__
