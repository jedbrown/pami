/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/mapping/mpi/mpimapping.h
 * \brief ???
 */

#ifndef __components_mapping_mpi_mpimapping_h__
#define __components_mapping_mpi_mpimapping_h__

#include "sys/xmi.h"
#include "components/mapping/BaseMapping.h"
#include <mpi.h>

#define XMI_MAPPING_CLASS XMI::Mapping::MPIMapping

namespace XMI
{
  namespace Mapping
  {
    class MPIMapping : public Interface::Base<MPIMapping>
    {
      public:
        inline MPIMapping () :
            Interface::Base<MPIMapping>()
        {
          MPI_Comm_rank(MPI_COMM_WORLD, (int*)&_task);
          MPI_Comm_size(MPI_COMM_WORLD, (int*)&_size);
        };
      inline ~MPIMapping () {};
    protected:
      size_t    _task;
      size_t    _size;
    public:
      inline xmi_result_t init_impl()
        {
          return XMI_SUCCESS;
        }
      inline size_t task_impl()
        {
          return _task;
        }
      inline size_t size_impl()
        {
          return _size;
        }
    };
  };
};
#endif // __components_mapping_mpi_mpimapping_h__
