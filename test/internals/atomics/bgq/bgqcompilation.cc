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
#include "components/atomic/counter/CounterBarrier.h"

int main(int argc, char **argv) {
	if (argc >= 0) {
		fprintf(stderr, "This program is not intended to be run!\n");
		exit(1);
	}
        PAMI::Memory::GenMemoryManager mm;
        mm.init(&mm, argc); // anything

        COUNTER_HELPER(PAMI::Counter::BGQ::IndirectL2, counter1, &mm, argv[1]);
        COUNTER_HELPER2(PAMI::Counter::BGQ::L2, counter2);

        MUTEX_HELPER(PAMI::Mutex::BGQ::IndirectL2, mutex1, &mm, argv[1]);
        MUTEX_HELPER2(PAMI::Mutex::BGQ::L2, mutex2);

        BARRIER_HELPER(PAMI::Barrier::BGQ::L2NodeCoreBarrier, barrier1, &mm, argv[1], argc, (argc == 0));
        BARRIER_HELPER(PAMI::Barrier::BGQ::L2NodeProcBarrier, barrier2, &mm, argv[1], argc, (argc == 0));
        BARRIER_HELPER2(PAMI::Barrier::Counter<PAMI::Counter::BGQ::L2>, barrier3, &mm, argv[1], argc, (argc == 0));
        BARRIER_HELPER(PAMI::Barrier::IndirectCounter<PAMI::Counter::BGQ::IndirectL2>, barrier4, &mm, argv[1], argc, (argc == 0));

        return 1;
}
