/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file arch/a2qpx/Arch.h
 * \brief ???
 */

#ifndef __arch_a2qpx_Arch_h__
#define __arch_a2qpx_Arch_h__

#include "arch/ArchInterface.h"

#include <hwi/include/bqc/A2_inlines.h>
#include <hwi/include/bqc/A2_core.h>

// Here go things specific to this processor architecture

#undef  mem_sync
#define mem_sync()    ppc_msync()

#undef  mem_isync
#define mem_isync()   isync()

#undef  mem_barrier
#define mem_barrier() __sync_synchronize()

///
/// \brief Template specialization for size_t copies.
///
template <>
template <unsigned N>
void Type<size_t>::copy (size_t * dst, size_t * src)
{
  size_t i;
  for (i=0; i<(N/sizeof(size_t)); i++) dst[i] = src[i];

  if (N%(sizeof(size_t)))
  {
    uint8_t * const d = (uint8_t * const) dst[N%(sizeof(size_t))];
    uint8_t * const s = (uint8_t * const) src[N%(sizeof(size_t))];

    for (i=0; i<(N%(sizeof(size_t))); i++) d[i] = s[i];
  }
};

#endif // __pami_arch_a2qpx_h__
