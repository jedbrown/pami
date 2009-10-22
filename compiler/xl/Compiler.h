/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file compiler/xl/Compiler.h
 * \brief ???
 */

#ifndef __compiler_xl_Compiler_h__
#define __compiler_xl_Compiler_h__

// Here go things specific to this compiler family

#include "builtins.h"
#define __sync_fetch_and_or(x,y)	__fetch_and_or((volatile unsigned int *)x,y)
#define __sync_fetch_and_and(x,y)	__fetch_and_and((volatile unsigned int *)x,y)
#define __sync_fetch_and_add(x,y)	__fetch_and_add((volatile int *)x,y)
#define __sync_fetch_and_sub(x,y)	__fetch_and_add((volatile int *)x,-(y))
#define __sync_fetch_and_swap(x,y)	__fetch_and_swap((volatile int *)x,y)

#endif // __xmi_compiler_xl_h__
