/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file arch/i386/Time.h
/// \brief ???
///
#ifndef __arch_i386_Time_h__
#define __arch_i386_Time_h__

#include <pami.h>
#include <sys/time.h>
#include "arch/TimeInterface.h"
#include <stdio.h>

namespace PAMI
{
  static inline uint64_t tb()
  {
    uint32_t lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return (uint64_t)hi << 32 | lo;
  }

  static unsigned long timeGetTime( void )
  {
    struct timeval tv;
    gettimeofday( &tv, 0 );
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
  }

  class Time : public Interface::Time<Time>
  {
    public:

      inline Time () :
          Interface::Time<Time>(),
          _clockMHz(0)
      {
      };

      ///
      /// \brief Initialize the time object.
      ///
      inline pami_result_t init_impl (size_t dummy)
      {
        _clockMHz      = clockMHz() / 1e9;
        _sec_per_cycle = 1.0 / ((double)_clockMHz * 1000000.0);

        if (_clockMHz == -1ULL)
          return PAMI_ERROR;
        else
          return PAMI_SUCCESS;
      };

      ///
      /// \brief The processor clock in MHz.
      ///
      /// \warning This returns \b mega hertz. Do not be confused.
      ///
      size_t clockMHz_impl ()
      {
        if (_clockMHz == 0.0)
          {
            uint64_t sampleTime = 100; //sample time in usec
            uint64_t timeStart = 0, timeStop = 0;
            uint64_t startBase = 0, endBase = 0;
            uint64_t overhead = 0, tbf = 0, tbi = 0;
            uint64_t ticks = 0;
            int      iter = 0;

            do
              {
                tbi = tb();
                tbf = tb();
                tbi = tb();
                tbf = tb();

                overhead = tbf - tbi;
                timeStart = timeGetTime();

                while (timeGetTime() == timeStart)
                  timeStart = timeGetTime();

                while (1)
                  {
                    timeStop = timeGetTime();

                    if ((timeStop - timeStart) > 1)
                      {
                        startBase = tb();
                        break;
                      }
                  }

                timeStart = timeStop;

                while (1)
                  {
                    timeStop = timeGetTime();

                    if ((timeStop - timeStart) > sampleTime)
                      {
                        endBase = tb();
                        break;
                      }
                  }

                ticks = ((endBase - startBase) + (overhead));
                iter++;

                if (iter == 10)
                  {
                    fprintf(stderr, "Warning: unable to initialize high resolution timer.\n");
                    return -1;
                  }
              }
            while (endBase < startBase);

            return ticks / (sampleTime*1e-6);
          }
        else
          return _clockMHz;
      }
      ///
      /// \brief Returns the number of "cycles" elapsed on the calling processor.
      ///
      unsigned long long timebase_impl ()
      {
        return tb();
      };

      ///
      /// \brief Computes the smallest clock resolution theoretically possible
      ///
      double tick_impl ()
      {
        return _sec_per_cycle;
      };

      ///
      /// \brief Returns an elapsed time on the calling processor.
      ///
      double time_impl ()
      {
        return ((double)tb() * _sec_per_cycle);
      };

    protected:
      uint64_t _clockMHz;
      double _sec_per_cycle;
  };	// class Time
};	// namespace PAMI
#endif // __arch_i386_Time_h__

