/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

///
/// \file common/bgp/Platform.h
/// \brief ???
///

#ifndef __common_bgp_Platform_h__
#define __common_bgp_Platform_h__

#include "Arch.h"

#define NUM_CORES 4
#define NUM_SMT 1

// these are based on what the CNK allows
#define XMI_MAX_PROC_PER_NODE	(NUM_CORES * NUM_SMT)
#define XMI_MAX_THREAD_PER_PROC	(NUM_CORES * NUM_SMT)
#define XMI_DEF_SH_WORKSIZE   8192 // Default shared work queue worksize
#define XMI_DEF_SH_WORKUNITS  32   // Default shared work queue workunits

#endif // __common_bgp_platform_h__
