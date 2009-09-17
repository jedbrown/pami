/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/time/bgp/BgpTime.h
/// \brief ???
///
#ifndef __components_time_mpi_mpitime_h__
#define __components_time_mpi_mpitime_h__

#define XMI_TIME_CLASS XMI::Time::MPITime

#include "sys/xmi.h"
#include <sys/time.h>
#include "components/time/BaseTime.h"
#include <mpi.h>


namespace XMI
{
  namespace Time
  {
#if defined(__i386) || defined(__amd64__)
    static inline uint64_t tb() 
    {
      uint32_t lo, hi;
      __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
      return (uint64_t)hi << 32 | lo;
    }
#elif defined(__powerpc) || defined(__powerpc__) || defined(__POWERPC__)
#define SPRN_TBRU 0x10D
#define SPRN_TBRL 0x10C
    static inline uint64_t tb()
    {
      unsigned temp;
      union
      {
        struct { unsigned hi, lo; } w;
        uint64_t d;
      } result;

      do {
        asm volatile ("mfspr %0,%1" : "=r" (temp)        : "i" (SPRN_TBRU));
        asm volatile ("mfspr %0,%1" : "=r" (result.w.lo) : "i" (SPRN_TBRL));
        asm volatile ("mfspr %0,%1" : "=r" (result.w.hi) : "i" (SPRN_TBRU));
      } while (temp != result.w.hi);
      return result.d;
    }
#else
    static inline uint64_t tb() { assert(0); return 0;}
#endif

    static unsigned long timeGetTime( void ) 
    {
      struct timeval tv;
      gettimeofday( &tv, 0 );
      return tv.tv_sec * 1000 + tv.tv_usec / 1000;
    }

    class MPITime : public Interface::BaseTime<MPITime>
    {
    public:

      inline MPITime () :
        Interface::BaseTime<MPITime>(),
        _clockMHz(0)
        {
          init_impl();
          _clockMHz = clockMHz_impl();
          //fprintf(stderr, "clockmhz=%lld\n", _clockMHz);
        };

      ///
      /// \brief Initialize the time object.
      ///
      inline xmi_result_t init_impl ()
        {
          _clockMHz      = clockMHz()/1e6;
          _sec_per_cycle = 1.0 / ((double)_clockMHz * 1000000.0);
          if(_clockMHz == -1ULL)
            return XMI_ERROR;
          else
            return XMI_SUCCESS;
        };

      ///
      /// \brief The processor clock in MHz.
      ///
      /// \warning This returns \b mega hertz. Do not be confused.
      ///
      size_t clockMHz_impl ()
        {
          if(_clockMHz == 0.0)
              {
                uint64_t sampleTime = 100; //sample time in usec
                uint64_t timeStart=0, timeStop=0;
                uint64_t startBase=0, endBase=0;
                uint64_t overhead=0, tbf=0, tbi=0;
                uint64_t ticks=0;
                int      iter=0;
                do
                    {
                      tbi = tb();
                      tbf = tb();
                      tbi = tb();
                      tbf = tb();

                      overhead = tbf - tbi;
                      timeStart = timeGetTime();
                      while(timeGetTime() == timeStart)
                        timeStart = timeGetTime();

                      while(1)
                          {
                            timeStop = timeGetTime();
                            if((timeStop - timeStart) > 1)
                                {
                                  startBase = tb();
                                  break;
                                }
                          }
                      timeStart = timeStop;

                      while(1)
                          {
                            timeStop = timeGetTime();
                            if((timeStop - timeStart) > sampleTime)
                                {
                                  endBase = tb();
                                  break;
                                }
                          }
                      ticks = ((endBase - startBase) + (overhead));
                      iter++;
                      if(iter==10)
                          {
                            fprintf(stderr,"Warning: unable to initialize high resolution timer.\n");
                            return -1;
                          }
                    }
                while(endBase < startBase);
                return ticks/(sampleTime*1e-6);
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
    };
  };
};
#endif // __components_time_time_h__

