/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file common/BaseTimeInterface.h
/// \brief ???
///
#ifndef __common_BaseTimeInterface_h__
#define __common_BaseTimeInterface_h__

#ifndef XMI_TIME_CLASS
#error XMI_TIME_CLASS must be defined
#endif

#include "sys/xmi.h"

namespace XMI
{
    namespace Interface
    {
      ///
      /// \param T_Mapping Base time template class
      ///
      template <class T>
      class BaseTime
      {
        public:

          ///
          /// \brief Initialize the time object.
          ///
          inline xmi_result_t init (size_t clockMHz);


          ///
          /// \brief The processor clock in MHz.
          ///
          /// \warning This returns \b mega hertz. Do not be confused.
          ///
          size_t clockMHz ();

          ///
          /// \brief Returns the number of "cycles" elapsed on the calling processor.
          ///
          unsigned long long timebase ();

          ///
          /// \brief Computes the smallest clock resolution theoretically possible
          ///
          double tick ();


          ///
          /// \brief Returns an elapsed time on the calling processor.
          ///
          double time ();
      }; // class BaseTime

      template <class T>
      inline xmi_result_t BaseTime<T>::init (size_t clockMHz)
      {
        return static_cast<T*>(this)->init_impl (clockMHz);
      }

      template <class T>
      size_t BaseTime<T>::clockMHz ()
      {
        return static_cast<T*>(this)->clockMHz_impl ();
      }

      template <class T>
      unsigned long long BaseTime<T>::timebase ()
      {
        return static_cast<T*>(this)->timebase_impl ();
      }

      template <class T>
      double BaseTime<T>::tick ()
      {
        return static_cast<T*>(this)->tick_impl ();
      }

      template <class T>
      double BaseTime<T>::time ()
      {
        return static_cast<T*>(this)->time_impl ();
      }
    };	// namespace Interface
};	// namespace XMI
#endif // __components_time_time_h__
