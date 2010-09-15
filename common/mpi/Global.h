/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
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
#include <mpi.h>
#include "components/memory/heap/HeapMemoryManager.h"
#include "components/memory/shmem/SharedMemoryManager.h"

namespace PAMI
{
    static void shutdownfunc()
    {
      MPI_Finalize();
    }

    class Global : public Interface::Global<PAMI::Global>
    {
          // Simple class to control MPI initialization independent of other classes.
          class MPI
                {
                public:
                  inline MPI()
                  {
                        int rc = MPI_Init(0, NULL);
                        if(rc != MPI_SUCCESS)
                        {
                          fprintf(stderr, "Unable to initialize context:  MPI_Init failure\n");
                          PAMI_abort();
                        }
                  }
          };

      public:

        inline Global () :
          Interface::Global<PAMI::Global>(),
          mapping()
        {
          // MPI::Time gets its own clockMHz
          time.init(0);
	  heap_mm = new (_heap_mm) PAMI::Memory::HeapMemoryManager();
	  shared_mm = new (_shared_mm) PAMI::Memory::SharedMemoryManager();

          {
                size_t min, max, num, *ranks;
//              int rc = MPI_Init(0, NULL);
//              if(rc != MPI_SUCCESS)
//                  {
//                    fprintf(stderr, "Unable to initialize context:  MPI_Init failure\n");
//                    PAMI_abort();
//                  }
                atexit(shutdownfunc);

                mapping.init(min, max, num, &ranks);

                PAMI::Topology::static_init(&mapping);
                /** \todo remove these casts when conversion to pami_task_t is complete */
                if (mapping.size() == max - min + 1) {
                        new (&topology_global) PAMI::Topology((pami_task_t)min, (pami_task_t)max);
                } else {
                        PAMI_abortf("failed to build global-world topology %zu:: %zu..%zu", mapping.size(), min, max);
                }
                new (&topology_local) PAMI::Topology((pami_task_t *)ranks, num);
                // could try to optimize list into range, etc...
          }
        };



        inline ~Global () {};

      public:
        MPI        		    mpi; // First data member to initialize MPI first.
        PAMI::Mapping		mapping;

  };   // class Global
};     // namespace PAMI

extern PAMI::Global __global;

#endif // __pami_common_mpi_global_h__
