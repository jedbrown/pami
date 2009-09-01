/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file atomic/noop/Noop.h
 * \brief ???
 */

#ifndef __xmi_atomic_noop_noop_h__
#define __xmi_atomic_noop_noop_h__

#include "../AtomicObject.h"

namespace XMI
{
  namespace Atomic
  {
    ///
    /// \brief CRTP interface for "noop" atomic objects.
    ///
    class Noop : public AtomicObject <Noop>
    {
      public:
        Noop () :
            AtomicObject <Noop> ()
        {};

        ~Noop () {};

        /// \see XMI::Atomic::AtomicObject::init
        void init_impl ()
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


