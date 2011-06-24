/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file arch/i386/Memory.h
 * \brief Defines i386 memory specializations
 */

#ifndef __arch_i386_Memory_h__
#define __arch_i386_Memory_h__

#define mem_barrier() __sync_synchronize()

#include "arch/MemoryInterface.h"

#undef  mem_barrier

#endif // __arch_i386_Memory_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
