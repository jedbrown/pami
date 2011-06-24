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

#define NUM_CORES 1
#define NUM_SMT   1

// somewhat arbitrary...
#define PAMI_MAX_PROC_PER_NODE   128
#define PAMI_MAX_THREAD_PER_PROC 32

#define mem_sync()    __sync_synchronize()

#endif // __arch_x86_64_Arch_h__
