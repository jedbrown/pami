/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef __common_socklinux_Platform_h__
#define __common_socklinux_Platform_h__
///
/// \file common/socklinux/Platform.h
/// \brief ???
///

#define NUM_CORES 1
#define NUM_SMT 1

// somewhat arbitrary...
#define PAMI_MAX_PROC_PER_NODE	32
#define PAMI_MAX_THREAD_PER_PROC	32
#define PAMI_DEF_SH_WORKSIZE   4096 // Default shared work queue worksize
#define PAMI_DEF_SH_WORKUNITS  32   // Default shared work queue workunits


#endif // __common_socklinux_Platform_h__
