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

#include "Arch.h"

#if defined(__64BIT__) && !defined(_LAPI_LINUX)
extern "C" {
#include "zcmem.h"
}
#endif


// Collective shared memory settings
#define CACHEBLOCKSZ 128
#if defined(__64BIT__) && !defined(_LAPI_LINUX)
// #define XMEM_THRESH  32768
#define XMEM_THRESH  (32768*4096)
#else
#define XMEM_THRESH ULONG_MAX
#endif

#define COLLSHM_DEVICE_NUMSYNCS 2
#define COLLSHM_DEVICE_SYNCCOUNT 16
#define COLLSHM_SEGSZ 0x10000000
#define COLLSHM_PAGESZ 4096
#define COLLSHM_WINGROUPSZ ((COLLSHM_DEVICE_NUMSYNCS * COLLSHM_DEVICE_SYNCCOUNT + 1) * CACHEBLOCKSZ)
#define COLLSHM_BUFSZ  32768
#define COLLSHM_KEY 0x900dc0df
#define COLLSHM_INIT_BUFCNT 128
#define COLLSHM_INIT_CTLCNT 128

// P2P Shared memory settings
// This is a bit of magic to construct the shared memory window for p2p
// We should have some interface to query the amount of shared memory required
#define MAX_CONTEXTS        (64)  
#define P2PSHM_HDRSIZE      (64)
#define P2PSHM_PKTSIZE      (1024)
#define P2PSHM_TOTALPKTSIZE (P2PSHM_PKTSIZE + P2PSHM_HDRSIZE)
#define P2PSHM_FIFOSIZE     (128)
#define P2PSHM_ALLOCATION   ((((P2PSHM_TOTALPKTSIZE + sizeof(size_t))*P2PSHM_FIFOSIZE) + (2*sizeof(size_t)))*MAX_CONTEXTS)
#define P2PSHM_METADATA     (sizeof(size_t)*MAX_CONTEXTS)
#define P2PSHM_MEMSIZE      (P2PSHM_ALLOCATION + P2PSHM_METADATA)
#endif // __common_lapiunix_platform_h__
