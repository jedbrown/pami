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

#ifndef __common_bgp_platform_h__
#define __common_bgp_platform_h__

#define NUM_CORES 4
#define NUM_SMT 1

// these are based on what the CNK allows
#define XMI_MAX_PROC_PER_NODE	(NUM_CORES * NUM_SMT)
#define XMI_MAX_THREAD_PER_PROC	(NUM_CORES * NUM_SMT)

#endif // __common_bgp_platform_h__
