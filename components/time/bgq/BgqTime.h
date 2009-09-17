/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/time/bgq/BgqTime.h
/// \brief ???
///
#ifndef __components_time_bgq_bgqtime_h__
#define __components_time_bgq_bgqtime_h__

#define XMI_TIME_CLASS XMI::Time::BgqTime

#include "sys/xmi.h"

#include "components/time/BaseTime.h"
#include "components/sysdep/bgq/BgqGlobal.h"

namespace XMI
{
  namespace Time
  {
    class BgqTime : public Interface::BaseTime<BgqTime>
    {
      public:

        inline BgqTime () :
            Interface::BaseTime<BgqTime> ()
        {};

        ///
        /// \brief Initialize the time object.
        ///
        inline xmi_result_t init_impl ()
        {
          return XMI_UNIMPL;
        };

        ///
        /// \brief The processor clock in MHz.
        ///
        /// \warning This returns \b mega hertz. Do not be confused.
        ///
        inline size_t clockMHz ()
        {
          return __global.personality.clockMHz ();
        };

        ///
        /// \brief Returns the number of "cycles" elapsed on the calling processor.
        ///
        inline unsigned long long timebase ()
        {
#if 0
          unsigned temp;
          union
          {
            struct { unsigned hi, lo; } w;
            unsigned long long d;
          } result;

          do
            {
asm volatile ("mfspr %0,%1" : "=r" (temp)        : "i" (SPRN_TBRU));
asm volatile ("mfspr %0,%1" : "=r" (result.w.lo) : "i" (SPRN_TBRL));
asm volatile ("mfspr %0,%1" : "=r" (result.w.hi) : "i" (SPRN_TBRU));
            }
          while (temp != result.w.hi);

          return result.d;
#else
          return 0;
#endif
        };

        ///
        /// \brief Computes the smallest clock resolution theoretically possible
        ///
        inline double tick ()
        {
          return XMI::Time::BgqTime::seconds_per_cycle;
        };

        ///
        /// \brief Returns an elapsed time on the calling processor.
        ///
        inline double time ()
        {
          return ((double)timebase() * XMI::Time::BgqTime::seconds_per_cycle);
        };

      protected:

        /// \brief BG/P compute node processors run at 850 MHz
        static const double seconds_per_cycle = 1.176470588235294033e-09;
    };
  };
};
#endif // __components_time_bgq_bgqtime_h__
