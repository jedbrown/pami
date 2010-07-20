/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file arch/power7/Arch.h
 * \brief ???
 */

#ifndef __arch_power7_Arch_h__
#define __arch_power7_Arch_h__

#include "arch/ArchInterface.h"

// Here go things specific to this processor architecture

//#define mem_sync() __sync()
//#define mem_isync() __isync()
//#define mem_barrier() __lwsync()

#define mem_sync() { asm volatile ("sync"); }
#define mem_isync() { asm volatile ("isync"); }
#define mem_barrier() { asm volatile ("lwsync"); }

#endif // __pami_arch_power7_h__
