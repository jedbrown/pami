/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file arch/x86_64/Memory.h
 * \brief Defines x86_64 memory specializations
 */

#ifndef __arch_x86_64_Memory_h__
#define __arch_x86_64_Memory_h__

#define mem_barrier() __sync_synchronize()

#include "arch/MemoryInterface.h"

#undef  mem_barrier

#endif // __arch_x86_64_Memory_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
