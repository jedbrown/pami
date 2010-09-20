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

        COUNTER_HELPER(PAMI::Counter::GccNodeCounter, counter1, argv[1]);
        COUNTER_HELPER(PAMI::Counter::GccProcCounter, counter2, argv[1]);
        COUNTER_HELPER(PAMI::Counter::Pthread, counter3, argv[1]);

        MUTEX_HELPER(PAMI::Mutex::CounterMutex<PAMI::Counter::GccNodeCounter>, mutex1, argv[1]);

        BARRIER_HELPER(PAMI::Barrier::CounterBarrier<PAMI::Counter::GccNodeCounter>, barrier1, argv[1], argc, (argc == 0));

        return 1;
}
