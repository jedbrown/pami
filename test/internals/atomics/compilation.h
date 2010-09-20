/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/// \file test/internals/atomics/compilation.h
/// \brief Basic comilation helper macros
///

#ifndef __test_internals_atomics_compilation_h__
#define __test_internals_atomics_compilation_h__

#include <stdio.h>
#include "util/common.h"
#include "components/memory/MemoryManager.h"

#define COUNTER_HELPER(class, var, key)	\
        class var;			\
        var.init(key);			\
        var.fetch_and_inc();		\
        var.fetch_and_dec();		\
        var.fetch_and_clear();

#define MUTEX_HELPER(class, var, key)	\
        class var;			\
        var.init(key);			\
        var.acquire();			\
        var.tryAcquire();		\
        var.release();			\
        var.isLocked();

#define BARRIER_HELPER(class, var, key, partic, master)	\
        class var;					\
        var.init(key, partic, master);			\
        var.enter();

#endif // __test_atomics_compilation_h__
