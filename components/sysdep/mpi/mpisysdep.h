/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
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

#include "components/sysdep/SysDep.h"
#include "components/mapping/mpi/mpimapping.h"
#include "components/memory/heap/HeapMemoryManager.h"
#include "components/time/mpi/mpitime.h"

namespace XMI
{
  namespace SysDep
  {
    typedef SysDep<XMI::Memory::HeapMemoryManager, XMI::Mapping::MPIMapping, XMI::Time::MPITime> MPISysDep;
  };
};
#endif // __components_sysdep_mpi_mpisysdep_h__
