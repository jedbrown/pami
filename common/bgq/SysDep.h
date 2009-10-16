/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file common/bgq/SysDep.h
/// \brief ???
///
#ifndef __components_sysdep_bgq_bgqsysdep_h__
#define __components_sysdep_bgq_bgqsysdep_h__

#define XMI_SYSDEP_CLASS XMI::SysDep

#include "common/SysDepInterface.h"

// These must be defined early
#include "Platform.h"

#include "Mapping.h"
#include "components/memory/shmem/SharedMemoryManager.h"
#include "Wtime.h"
#include "Topology.h"

namespace XMI
{
    // TODO -- need to make a different shared memory manager so that multiple
    // contexts can get shared memory .. right now the second context to call
    // mm.init() will fail.
    typedef Interface::SysDep<XMI::Memory::SharedMemoryManager> SysDep;
};
#endif // __components_sysdep_bgq_bgqsysdep_h__
