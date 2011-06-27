/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
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

#define NUM_CORES 1
#define NUM_SMT   1

// somewhat arbitrary...
#define PAMI_MAX_PROC_PER_NODE   32
#define PAMI_MAX_THREAD_PER_PROC 32

#endif // __pami_arch_i386_h__
