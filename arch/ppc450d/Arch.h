/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file arch/ppc450d/Arch.h
 * \brief ???
 */

#ifndef __arch_ppc450d_Arch_h__
#define __arch_ppc450d_Arch_h__

#include "arch/ArchInterface.h"

// Here go things specific to this processor architecture

#define NUM_CORES 4
#define NUM_SMT   1

// These are based on what the CNK allows
#define PAMI_MAX_PROC_PER_NODE   (NUM_CORES * NUM_SMT)
#define PAMI_MAX_THREAD_PER_PROC (NUM_CORES * NUM_SMT)

#include <bpcore/ppc450_inlines.h>
#include <bpcore/bgp_atomic_ops.h>
#include <spi/kernel_interface.h>

#undef  mem_sync
#define mem_sync()      _bgp_msync()

#undef  mem_isync
#define mem_isync()   mem_sync()

#undef  mem_barrier
#define mem_barrier()   _bgp_mbar()

#define LQU(x, ptr, incr) \
asm volatile ("lfpdux %0,%1,%2" : "=f"(x), "+Ob"(ptr) : "r"(incr) : "memory")
#define SQU(x, ptr, incr) \
asm volatile ("stfpdux %2,%0,%1": "+Ob" (ptr) : "r" (incr), "f" (x) : "memory")


#endif // __pami_arch_ppc450d_h__
