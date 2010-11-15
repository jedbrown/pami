/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
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

#include "arch/include/bpcore/ppc450_core.h"

/// \brief Constant used to specify alignment in declarations of lwarx/stwcx atomics.
#define PAMI_BGP_NATIVE_ATOMIC_ALIGN    L1D_CACHE_LINE_SIZE

#endif // __common_bgp_platform_h__
