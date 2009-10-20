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

#ifndef __xmi_compiler_xl_h__
#define __xmi_compiler_xl_h__

// Here go things specific to this compiler family

#include "builtins.h"
#define __sync_fetch_and_or(x,y)	__fetch_and_or(x,y)
#define __sync_fetch_and_and(x,y)	__fetch_and_and(x,y)
#define __sync_fetch_and_add(x,y)	__fetch_and_add(x,y)
#define __sync_fetch_and_sub(x,y)	__fetch_and_add(x,-(y))
#define __sync_fetch_and_swap(x,y)	__fetch_and_swap(x,y)

#endif // __xmi_compiler_xl_h__
