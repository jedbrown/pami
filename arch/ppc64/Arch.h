/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file arch/ppc64/Arch.h
 * \brief ???
 */

#ifndef __arch_ppc64_Arch_h__
#define __arch_ppc64_Arch_h__

#include "arch/ArchInterface.h"

// Here go things specific to this processor architecture

//#define mem_sync()    do { asm volatile ("sync" : : : "memory"); } while(0)
//#define mem_isync()   mem_sync()
//#define mem_barrier() do { asm volatile ("eieio"  : : : "memory"); } while(0)

#define mem_sync()    __sync()
#define mem_isync()   __isync()
#define mem_barrier() __lwsync()
#endif // __pami_arch_ppc64_h__
