/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file arch/ppc450d/Memory.h
 * \brief Defines ppc450d memory specializations
 */

#ifndef __arch_ppc450d_Memory_h__
#define __arch_ppc450d_Memory_h__

#include <bpcore/ppc450_inlines.h>
#include <bpcore/bgp_atomic_ops.h>
#include <spi/kernel_interface.h>

#define mem_barrier() _bgp_mbar()

#include "arch/MemoryInterface.h"

#undef  mem_barrier

namespace PAMI
{
  namespace Memory
  {
    template <> const bool supports <instruction> () { return true; };

    template <> void sync <instruction> () { _bgp_msync(); };
  };
};

#endif // __arch_ppc450d_Memory_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
