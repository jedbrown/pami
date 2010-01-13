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

// somewhat arbitrary...
#define XMI_MAX_PROC_PER_NODE	32
#define XMI_MAX_THREAD_PER_PROC	32
#define XMI_DEF_SH_WORKSIZE   8192 // Default shared work queue worksize
#define XMI_DEF_SH_WORKUNITS  32   // Default shared work queue workunits

#endif // __common_lapiunix_platform_h__
