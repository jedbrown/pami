/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/sysdep/mpi/mpisysdep.h
 * \brief ???
 */

#ifndef __components_sysdep_mpi_mpisysdep_h__
#define __components_sysdep_mpi_mpisysdep_h__

#define XMI_SYSDEP_CLASS XMI::SysDep::MPISysDep

#include "components/sysdep/SysDep.h"
#include "components/mapping/mpi/mpimapping.h"
#include "components/memory/heap/HeapMemoryManager.h"
#include "components/time/mpi/mpitime.h"
#include "components/topology/mpi/mpitopology.h"

namespace XMI
{
  namespace SysDep
  {
    typedef SysDep<XMI::Memory::HeapMemoryManager, XMI::Mapping::MPIMapping, XMI::Time::MPITime, XMI::Topology::MPI> MPISysDep;
  };
};
#endif // __components_sysdep_mpi_mpisysdep_h__
