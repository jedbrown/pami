/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file common/bgq/Wtime.h
/// \brief ???
///
#ifndef __common_bgq_Wtime_h__
#define __common_bgq_Wtime_h__

#include "sys/xmi.h"

#include "common/BaseTimeInterface.h"

namespace XMI
{
    class Time : public Interface::BaseTime<Time>
    {
      public:

        inline Time () :
            Interface::BaseTime<Time> (),
	    _mhz(0)
        {};

        ///
        /// \brief Initialize the time object.
        ///
        inline xmi_result_t init_impl (size_t clockMHz)
        {
	  _mhz = clockMHz;
          return XMI_SUCCESS;
        };

        ///
        /// \brief The processor clock in MHz.
        ///
        /// \warning This returns \b mega hertz. Do not be confused.
        ///
        inline size_t clockMHz ()
        {
          return _mhz;
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
          return XMI::Time::seconds_per_cycle;
        };

        ///
        /// \brief Returns an elapsed time on the calling processor.
        ///
        inline double time ()
        {
          return ((double)timebase() * XMI::Time::seconds_per_cycle);
        };

      protected:

        /// \brief BG/P compute node processors run at 850 MHz
        static const double seconds_per_cycle = 1.176470588235294033e-09;
	size_t _mhz;
    };	// class Time
};	// namespace XMI
#endif // __components_time_bgq_bgqtime_h__
