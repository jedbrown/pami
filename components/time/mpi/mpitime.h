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
#include "components/time/BaseTime.h"
#include <mpi.h>


namespace XMI
{
  namespace Time
  {
    class MPITime : public Interface::BaseTime<MPITime>
    {
      public:

        inline MPITime () :
            Interface::BaseTime<MPITime> ()
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
        static size_t clockMHz ()
        {

        };

        ///
        /// \brief Returns the number of "cycles" elapsed on the calling processor.
        ///
        static unsigned long long timebase ()
        {
        };

        ///
        /// \brief Computes the smallest clock resolution theoretically possible
        ///
        static double tick ()
        {
        };

        ///
        /// \brief Returns an elapsed time on the calling processor.
        ///
        static double time ()
        {
        };

      protected:
    };
  };
};
#endif // __components_time_time_h__

