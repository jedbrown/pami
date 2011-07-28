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

// Enable multi-context CCMI/Optimized collective registration.  Default is only PGAS on multi-contexts.
// #define PAMI_ENABLE_COLLECTIVE_MULTICONTEXT

// New BGQ optimized shmem stack:
#define PAMI_ENABLE_NEW_SHMEM

// Enable BGQ shmem stack on sub-node geometries
//#define PAMI_ENABLE_SHMEM_SUBNODE

// Enable experimental (X0) collective protocols
//#define PAMI_ENABLE_X0_PROTOCOLS

#define PAMI_ALGOLISTS_MAX_NUM 17 // Maximum number of algorithms per collective per geometry

#endif // __common_bgq_platform_h__
