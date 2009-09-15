/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/atomic/Barrier.h
/// \brief ???
///
#ifndef __components_atomic_barier_h__
#define __components_atomic_barier_h__

#include "sys/xmi.h"

namespace XMI
{
  namespace Atomic
  {
    namespace Interface
    {
      ///
      /// \brief Barrier object interface
      ///
      /// \param T  Barrier object derived class
      ///
      template <class T>
      class Barrier
      {
        public:
          Barrier  () {};
          ~Barrier () {};

        ///
        /// \brief Initialize the local barrier objecti
        ///
        /// \param[in] participants Number of participants for the barrier
        ///
        void init (size_t participants);

        ///
        /// \brief Enter a local blocking barrier operation
        ///
        /// Does not return until all participants have entered the barrier.
        ///
        inline xmi_result_t enter ();

      };  // XMI::Atomic::Interface::Barrier class

      template <class T>
      inline void Barrier<T>::init (size_t participants)
      {
        static_cast<T*>(this)->init_impl(participants);
      };

      template <class T>
      inline xmi_result_t Barrier<T>::enter ()
      {
        return static_cast<T*>(this)->enter_impl();
      };
    }; // XMI::Atomic::Interface namespace
  };   // XMI::Atomic namespace
};     // XMI namespace

#endif // __components_atomic_barier_h__


