/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file arch/a2qpx/Memory.h
 * \brief Defines a2qpx memory specializations
 */

#ifndef __arch_a2qpx_Memory_h__
#define __arch_a2qpx_Memory_h__

#include <hwi/include/bqc/A2_inlines.h>

#define mem_barrier() { do { asm volatile ("mbar"  : : : "memory"); } while(0); }
//{ do { asm volatile ("msync" : : : "memory"); } while(0); }

#include "arch/MemoryInterface.h"

#undef  mem_barrier

#include <hwi/include/bqc/l2_central_inlines.h>
#include <hwi/include/bqc/MU_Macros.h>

namespace PAMI
{
  namespace Memory
  {
    template <> const bool supports <instruction>  () { return true; };
    template <> const bool supports <remote_msync> () { return true; };
    template <> const bool supports <l1p_flush>    () { return true; };

    template <> void sync <instruction>  ()
    {
      do { asm volatile ("isync" : : : "memory"); } while(0);
    };
    
    template <> void sync <remote_msync> ()
    {
      do { asm volatile ("mbar"  : : : "memory"); } while(0);
    };

    template <> void sync <l1p_flush> ()
    {
      volatile uint64_t *dummy_mu_reg =
        (uint64_t *) (BGQ_MU_STATUS_CONTROL_REGS_START_OFFSET(0, 0) +
                      0x030 - PHYMAP_PRIVILEGEDOFFSET);
      *dummy_mu_reg = 0;
    };
  };
};

#endif // __arch_a2qpx_Memory_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
