/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file arch/x86_64/Arch.h
 * \brief ???
 */

#ifndef __arch_x86_64_Arch_h__
#define __arch_x86_64_Arch_h__

#include "arch/ArchInterface.h"

// Here go things specific to this processor architecture

inline void mem_sync()
{
  // todo:  implement
}

#define mem_isync()   mem_sync()



#endif // __pami_arch_i386_h__
