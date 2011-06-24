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

#define NUM_CORES 1
#define NUM_SMT   1

// somewhat arbitrary...
#define PAMI_MAX_PROC_PER_NODE   128
#define PAMI_MAX_THREAD_PER_PROC 32
#define MATH_MAX_NSRC 2

#define mem_sync()    do { asm volatile ("sync" : : : "memory"); } while(0)
#define mem_isync()   do { asm volatile ("isync" : : : "memory"); } while(0)

#endif // __pami_arch_ppc64_h__
