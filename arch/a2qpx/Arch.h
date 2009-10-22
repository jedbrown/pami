/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file arch/a2qpx/Arch.h
 * \brief ???
 */

#ifndef __arch_a2qpx_Arch_h__
#define __arch_a2qpx_Arch_h__

#include "arch/ArchInterface.h"

#include <hwi/include/bqc/A2_inlines.h>

// Here go things specific to this processor architecture

#undef  mem_sync
#define mem_sync()    ppc_msync()

#undef  mem_barrier
#define mem_barrier() __sync_synchronize()

#endif // __xmi_arch_a2qpx_h__
