/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

///
/// \file common/bgq/Platform.h
/// \brief ???
///

#ifndef __common_bgq_Platform_h__
#define __common_bgq_Platform_h__

#include "Arch.h"
#include "hwi/include/bqc/A2_core.h"

/// \brief Constant used to specify alignment in declarations of lwarx/stwcx atomics.
#define PAMI_NATIVE_ATOMIC_ALIGN	L1D_CACHE_LINE_SIZE

// New converged shmem stack:
//#define _COLLSHM

// New BGQ optimized shmem stack:
#define ENABLE_NEW_SHMEM

// Enable New BGQ optimized shmem stack on sub-node geometries
//#define ENABLE_NEW_SHMEM_SUBNODE

#ifdef _COLLSHM   // New Collective Shmem Protocol defines

#include <sched.h>
#include <limits.h>
inline void yield()
{
    sched_yield();
}

#define CACHEBLOCKSZ 128
#define XMEM_THRESH ULONG_MAX
//#define XMEM_THRESH  (32768*4096)

#define COLLSHM_DEVICE_NUMSYNCS 2
#define COLLSHM_DEVICE_SYNCCOUNT 16

#define COLLSHM_SEGSZ 32 * 1024 * 1024
#define COLLSHM_PAGESZ 4096
#define COLLSHM_WINGROUPSZ ((COLLSHM_DEVICE_NUMSYNCS * COLLSHM_DEVICE_SYNCCOUNT + 1) * CACHEBLOCKSZ)
#define COLLSHM_BUFSZ  8192

#define COLLSHM_INIT_BUFCNT 64
#define COLLSHM_INIT_CTLCNT 64
#endif // _COLLSHM

#endif // __common_bgq_platform_h__
