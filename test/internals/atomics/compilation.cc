/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/// \file test/internals/atomics/compilation.cc
/// \brief Basic comilation test to ensure objects do not get broken accidentally
///
/// This program should never be run!

#include "compilation.h"

#include "components/atomic/gcc/GccCounter.h"
#include "components/atomic/counter/CounterMutex.h"
#include "components/atomic/counter/CounterBarrier.h"
#include "components/atomic/pthread/Pthread.h"

int main(int argc, char **argv) {
	if (argc >= 0) {
		fprintf(stderr, "This program is not intended to be run!\n");
		exit(1);
	}
	PAMI::Memory::GenMemoryManager mm;
	mm.init(&mm, argc); // anything

        COUNTER_HELPER(PAMI::Counter::GccIndirCounter, counter1, &mm, argv[1]);
        COUNTER_HELPER2(PAMI::Counter::GccInPlaceCounter, counter2);
        COUNTER_HELPER2(PAMI::Counter::Pthread, counter3);

        MUTEX_HELPER(PAMI::Mutex::CounterMutex<PAMI::Counter::GccIndirCounter>, mutex1, &mm, argv[1]);

        BARRIER_HELPER(PAMI::Barrier::CounterBarrier<PAMI::Counter::GccInPlaceCounter>, barrier1, &mm, argv[1], argc, (argc == 0));
        BARRIER_HELPER(PAMI::Barrier::CounterBarrier<PAMI::Counter::GccIndirCounter>, barrier2, &mm, argv[1], argc, (argc == 0));

        return 1;
}
