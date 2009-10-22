/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef __common_mpi_Global_h__
#define __common_mpi_Global_h__
///
/// \file common/mpi/Global.h
/// \brief Global Object
///
/// This global object is constructed before main() and is a container class
/// for all other classes that need to be constructed and initialized before
/// the application starts.
///

#include "Platform.h"
#include "util/common.h"
#include "common/GlobalInterface.h"
#include "Mapping.h"
#include "Topology.h"
#include "Wtime.h"
#include <mpi.h>
namespace XMI
{
    static void shutdownfunc()
    {
      MPI_Finalize();
    }


    class Global : public Interface::Global<XMI::Global>
    {
      public:

        inline Global () :
	  Interface::Global<XMI::Global>(),
	  mapping()
        {
	  // Time gets its own clockMHz
	  Interface::Global<XMI::Global>::time.init(0);
	  {
		size_t min, max, num, *ranks;
                int rc = MPI_Init(0, NULL);
                if(rc != MPI_SUCCESS)
                    {
                      fprintf(stderr, "Unable to initialize context:  MPI_Init failure\n");
                      XMI_abort();
                    }
                atexit(shutdownfunc);

                mapping.init(min, max, num, &ranks);

		XMI::Topology::static_init(&mapping);
		if (mapping.size() == max - min + 1) {
			new (&topology_global) XMI::Topology(min, max);
		} else {
			XMI_abortf("failed to build global-world topology %zd:: %zd..%zd", mapping.size(), min, max);
		}
		new (&topology_local) XMI::Topology(ranks, num);
		// could try to optimize list into range, etc...
	  }
        };



        inline ~Global () {};

      public:

	XMI::Mapping		mapping;

  };   // class Global
};     // namespace XMI

extern XMI::Global __global;

#endif // __xmi_common_mpi_global_h__
