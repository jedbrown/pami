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
#include "../lapi/include/lapi_base_type.h"
#include "../lapi/include/lapi_env.h"

// Collective shared memory settings
#define CACHEBLOCKSZ 128
#if defined(__64BIT__) && !defined(_LAPI_LINUX)
// #define XMEM_THRESH  32768
#define XMEM_THRESH  (32768*4096)
#else
#define XMEM_THRESH ULONG_MAX
#endif

#define COLLSHM_DEVICE_NUMSYNCS 2
#define COLLSHM_DEVICE_SYNCCOUNT 64
#define COLLSHM_PAGESZ 4096
#define COLLSHM_WINGROUPSZ ((COLLSHM_DEVICE_NUMSYNCS * COLLSHM_DEVICE_SYNCCOUNT + 1) * CACHEBLOCKSZ)

#define COLLSHM_CTLCNT 8192
#define COLLSHM_BUFCNT 4608 
#define COLLSHM_LGBUFCNT 5 

#define COLLSHM_BUFSZ  32768
#define COLLSHM_LGBUFSZ  524288

// P2P Shared memory settings
// This is a bit of magic to construct the shared memory window for p2p
// We should have some interface to query the amount of shared memory required
#define MAX_CONTEXTS        (128)  
#define P2PSHM_HDRSIZE      (64)
#define P2PSHM_PKTSIZE      (1024)
#define P2PSHM_TOTALPKTSIZE (P2PSHM_PKTSIZE + P2PSHM_HDRSIZE)
#define P2PSHM_FIFOSIZE     (128)
#define P2PSHM_ALLOCATION   ((((P2PSHM_TOTALPKTSIZE + sizeof(size_t))*P2PSHM_FIFOSIZE) + (2*sizeof(size_t)))*MAX_CONTEXTS)
#define P2PSHM_METADATA     (sizeof(size_t)*MAX_CONTEXTS)
#define P2PSHM_MEMSIZE      (P2PSHM_ALLOCATION + P2PSHM_METADATA)

#define PAMI_ENDPOINT_INIT(client,task,offset) ((task << _Lapi_env.endpoints_shift) + offset)
#define PAMI_ENDPOINT_INFO(endpoint,task,offset) { task   = endpoint >> _Lapi_env.endpoints_shift;             \
                                                   offset = endpoint - (task << _Lapi_env.endpoints_shift); }  \

#endif // __common_lapiunix_platform_h__
