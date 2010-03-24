/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file common/socklinux/SysDep.h
/// \brief ???
///
#ifndef __common_socklinux_SysDep_h__
#define __common_socklinux_SysDep_h__

#define PAMI_SYSDEP_CLASS PAMI::SysDep

#include "common/SysDepInterface.h"

// These must be defined early
#include "Platform.h"

#include "Mapping.h"
#include "Wtime.h"
#include "Topology.h"

namespace PAMI
{
    // TODO -- need to make a different shared memory manager so that multiple
    // contexts can get shared memory .. right now the second context to call
    // mm.init() will fail.
    typedef Interface::SysDep SysDep;
};
#endif // __common_socklinux_SysDep_h__
