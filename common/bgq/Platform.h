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

//#define _COLLSHM

#ifdef _COLLSHM   // New Collective Shmem Protocol defines

#define _POSIX_SHM_OPEN

#include <sched.h>
inline void yield()
{
    sched_yield();
}

#define CACHEBLOCKSZ 128
#define XMEM_THRESH  (32768*4096)

#define COLLSHM_DEVICE_NUMSYNCS 2
#define COLLSHM_DEVICE_SYNCCOUNT 16

#define COLLSHM_SEGSZ 64 * 1024 * 1024
#define COLLSHM_PAGESZ 4096
#define COLLSHM_WINGROUPSZ ((COLLSHM_DEVICE_NUMSYNCS * COLLSHM_DEVICE_SYNCCOUNT + 1) * CACHEBLOCKSZ)
#define COLLSHM_BUFSZ  1024

#define COLLSHM_INIT_BUFCNT 64
#define COLLSHM_INIT_CTLCNT 64
#endif

#endif // __common_bgq_platform_h__
