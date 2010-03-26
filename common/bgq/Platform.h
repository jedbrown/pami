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

#define NUM_CORES	16
#define NUM_SMT		4

// these are based on what the CNK allows
#define PAMI_MAX_PROC_PER_NODE	(NUM_CORES * NUM_SMT)
#define PAMI_MAX_THREAD_PER_PROC	(NUM_CORES * NUM_SMT)
#define PAMI_DEF_SH_WORKSIZE   4096 //8192 // Default shared work queue worksize
#define PAMI_DEF_SH_WORKUNITS  16 //32   // Default shared work queue workunits

#endif // __common_bgq_platform_h__
