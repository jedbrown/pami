/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file arch/x86_64/Arch.h
 * \brief ???
 */

#ifndef __arch_x86_64_Arch_h__
#define __arch_x86_64_Arch_h__

#include "arch/ArchInterface.h"

// Here go things specific to this processor architecture

#define mem_sync()    __sync_synchronize()
#define mem_isync()   mem_sync()
#define mem_barrier() __sync_synchronize()

#endif // __arch_x86_64_Arch_h__
