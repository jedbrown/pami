/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/// \file test/internals/atomics/bgq/bgqcompilation.cc
/// \brief Basic comilation test to ensure objects do not get broken accidentally
///
/// This program should never be run!

#include "../compilation.h"

#include "components/atomic/bgq/L2Counter.h"
#include "components/atomic/bgq/L2Mutex.h"
#include "components/atomic/bgq/L2Barrier.h"

int main(int argc, char **argv) {
	if (argc >= 0) {
		fprintf(stderr, "This program is not intended to be run!\n");
		exit(1);
	}
        PAMI::Memory::MemoryManager mm;
        mm.init(argv, argc); // anything

        COUNTER_HELPER(PAMI::Counter::BGQ::L2ProcCounter, counter1, mm);
        COUNTER_HELPER(PAMI::Counter::BGQ::L2NodeCounter, counter2, mm);

        MUTEX_HELPER(PAMI::Mutex::BGQ::L2ProcMutex, mutex1, mm);
        MUTEX_HELPER(PAMI::Mutex::BGQ::L2NodeMutex, mutex2, mm);

        BARRIER_HELPER(PAMI::Barrier::BGQ::L2NodeCoreBarrier, barrier1, mm, argc, (argc == 0));
        BARRIER_HELPER(PAMI::Barrier::BGQ::L2NodeProcBarrier, barrier2, mm, argc, (argc == 0));

        return 1;
}
