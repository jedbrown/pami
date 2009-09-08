#ifndef __components_sysdep_mpi_mpisysdep_h__
#define __components_sysdep_mpi_mpisysdep_h__

#include "components/sysdep/SysDep.h"
#include "components/mapping/mpi/MPIMapping.h"
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
