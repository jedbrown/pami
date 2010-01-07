/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file arch/i386/Arch.h
 * \brief ???
 */

#ifndef __arch_i386_Arch_h__
#define __arch_i386_Arch_h__

#include "arch/ArchInterface.h"

// Here go things specific to this processor architecture

#define mem_sync()    __sync_synchronize()
#define mem_isync()   mem_sync()
#define mem_barrier() __sync_synchronize()

#endif // __xmi_arch_i386_h__
