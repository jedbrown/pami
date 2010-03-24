/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/// \file test/atomics/compilation.h
/// \brief Basic comilation helper macros
///

#ifndef __test_atomics_compilation_h__
#define __test_atomics_compilation_h__

#include <stdio.h>
#include "util/common.h"
#include "components/memory/MemoryManager.h"

#define COUNTER_HELPER(class, var, mm)	\
        class var;			\
        var.init(&mm);			\
        var.fetch_and_inc();		\
        var.fetch_and_dec();		\
        var.fetch_and_clear();

#define MUTEX_HELPER(class, var, mm)	\
        class var;			\
        var.init(&mm);			\
        var.acquire();			\
        var.tryAcquire();		\
        var.release();			\
        var.isLocked();

#define BARRIER_HELPER(class, var, mm, partic, master)	\
        class var;					\
        var.init(&mm, partic, master);			\
        var.enter();

#endif // __test_atomics_compilation_h__
