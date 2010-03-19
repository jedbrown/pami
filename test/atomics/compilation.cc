/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/// \file test/atomics/compilation.cc
/// \brief Basic comilation test to ensure objects do not get broken accidentally
///
/// This program should never be run!

#include "compilation.h"

#include "components/atomic/gcc/GccCounter.h"
#include "components/atomic/counter/CounterMutex.h"
#include "components/atomic/counter/CounterBarrier.h"
#include "components/atomic/pthread/Pthread.h"

int main(int argc, char **argv) {
	XMI::Memory::MemoryManager mm;
	mm.init(argv, argc); // anything

	COUNTER_HELPER(XMI::Counter::GccNodeCounter, counter1, mm);
	COUNTER_HELPER(XMI::Counter::GccProcCounter, counter2, mm);
	COUNTER_HELPER(XMI::Counter::Pthread, counter3, mm);

	MUTEX_HELPER(XMI::Mutex::CounterMutex<XMI::Counter::GccNodeCounter>, mutex1, mm);

	BARRIER_HELPER(XMI::Barrier::CounterBarrier<XMI::Counter::GccNodeCounter>, barrier1, mm, argc, (argc == 0));

	return 1;
}
