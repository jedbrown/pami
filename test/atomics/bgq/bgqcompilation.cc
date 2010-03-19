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

#include "../compilation.h"

#include "components/atomic/bgq/L2Counter.h"
#include "components/atomic/bgq/L2Mutex.h"
#include "components/atomic/bgq/L2Barrier.h"

int main(int argc, char **argv) {
	XMI::Memory::MemoryManager mm;
	mm.init(argv, argc); // anything

	COUNTER_HELPER(XMI::Counter::BGQ::L2ProcCounter, counter1, mm);
	COUNTER_HELPER(XMI::Counter::BGQ::L2NodeCounter, counter2, mm);

	MUTEX_HELPER(XMI::Mutex::BGQ::L2ProcMutex, mutex1, mm);
	MUTEX_HELPER(XMI::Mutex::BGQ::L2NodeMutex, mutex2, mm);

	BARRIER_HELPER(XMI::Barrier::BGQ::L2NodeCoreBarrier, barrier1, mm, argc, (argc == 0));
	BARRIER_HELPER(XMI::Barrier::BGQ::L2NodeProcBarrier, barrier2, mm, argc, (argc == 0));

	return 1;
}
