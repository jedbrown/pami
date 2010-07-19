/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

///
/// \file common/lapiunix/Platform.h
/// \brief ???
///

#ifndef __common_lapiunix_Platform_h__
#define __common_lapiunix_Platform_h__

#define NUM_CORES 1
#define NUM_SMT 1

#ifdef _COLLSHM
extern "C" {
#include "zcmem.h"
}

#define CACHEBLOCKSZ 128
// #define XMEM_THRESH  32768
#define XMEM_THRESH  (32768*4096)

#define COLLSHM_DEVICE_NUMSYNCS 2
#define COLLSHM_DEVICE_SYNCCOUNT 16

#define COLLSHM_SEGSZ 0x10000000
#define COLLSHM_PAGESZ 4096
#define COLLSHM_WINGROUPSZ ((COLLSHM_DEVICE_NUMSYNCS * COLLSHM_DEVICE_SYNCCOUNT + 1) * CACHEBLOCKSZ)
#define COLLSHM_BUFSZ  32768
#define COLLSHM_KEY 0x900dc0df

#define COLLSHM_INIT_BUFCNT 128
#define COLLSHM_INIT_CTLCNT 128
#endif // _COLLSHM

// somewhat arbitrary...
#define PAMI_MAX_PROC_PER_NODE	32
#define PAMI_MAX_THREAD_PER_PROC	32
#define PAMI_DEF_SH_WORKSIZE   8192 // Default shared work queue worksize
#define PAMI_DEF_SH_WORKUNITS  32   // Default shared work queue workunits

#endif // __common_lapiunix_platform_h__
