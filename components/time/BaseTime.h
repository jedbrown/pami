/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/time/BaseTime.h
/// \brief ???
///
#ifndef __components_time_basetime_h__
#define __components_time_basetime_h__

#ifndef XMI_TIME_CLASS
#error XMI_TIME_CLASS must be defined
#endif

#include "sys/xmi.h"

namespace XMI
{
  namespace Time
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
          inline xmi_result_t init ();


          ///
          /// \brief The processor clock in MHz.
          ///
          /// \warning This returns \b mega hertz. Do not be confused.
          ///
          static size_t clockMHz ()
          {
            return 0;
          };

          ///
          /// \brief Returns the number of "cycles" elapsed on the calling processor.
          ///
          static unsigned long long timebase ()
          {
            return 0UL;
          };

          ///
          /// \brief Computes the smallest clock resolution theoretically possible
          ///
          static double tick ()
          {
            return 0.0;
          };

          ///
          /// \brief Returns an elapsed time on the calling processor.
          ///
          static double time ()
          {
            return 0.0;
          };
      };

      template <class T>
      inline xmi_result_t BaseTime<T>::init ()
      {
        return static_cast<T*>(this)->init_impl ();
      }
#if 0
      template <class T>
      size_t BaseTime<T>::clockMHz ()
      {
        return T::clockMHz_impl ();
      }

      template <class T>
      unsigned long long BaseTime<T>::timebase ()
      {
        return T::timebase_impl ();
      }

      template <class T>
      double BaseTime<T>::tick ()
      {
        return T::tick_impl ();
      }

      template <class T>
      double BaseTime<T>::timer ()
      {
        return T::timer_impl ();
      }
#endif
    };
  };
};
#endif // __components_time_time_h__
