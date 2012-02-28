/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file arch/ppc64/Memory.h
 * \brief Defines powerpc memory specializations
 */

#ifndef __arch_ppc64_Memory_h__
#define __arch_ppc64_Memory_h__

#define mem_barrier() do { asm volatile ("lwsync"  : : : "memory"); } while(0)

#include "arch/MemoryInterface.h"

#undef  mem_barrier

namespace PAMI
{
  namespace Memory
  {
    template <> bool supports <instruction> () { return true; };

    template <> void sync <instruction> ()
    {
      do { asm volatile ("isync" : : : "memory"); } while(0);
    };
  };
};

#endif // __arch_ppc64_Memory_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
