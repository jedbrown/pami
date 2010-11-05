/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/// \file test/internals/atomics/bgp/bgpcompilation.cc
/// \brief BG/P comilation test to ensure objects do not get broken accidentally
///
/// This program should never be run!

#include "../compilation.h"

#include "components/atomic/bgp/BgpAtomic.h"
#include "components/atomic/bgp/LockBoxCounter.h"
#include "components/atomic/bgp/LockBoxMutex.h"
#include "components/atomic/bgp/LockBoxBarrier.h"
#include "components/atomic/bgp/LwarxStwcxMutex.h"

int main(int argc, char **argv) {
	if (argc >= 0) {
		fprintf(stderr, "This program is not intended to be run!\n");
		exit(1);
	}
	PAMI::Memory::GenMemoryManager mm;
	mm.init(&mm, argc); // anything

        COUNTER_HELPER(PAMI::Counter::BGP::LockBoxCounter, counter1, &mm, argv[1]);

        COUNTER_HELPER(PAMI::Counter::BGP::BgpProcCounter, counter3, &mm, argv[1]);

        COUNTER_HELPER(PAMI::Counter::BGP::BgpNodeCounter, counter4, &mm, argv[1]);




        MUTEX_HELPER(PAMI::Mutex::BGP::BgpProcMutex, mutex1, &mm, argv[1]);

        MUTEX_HELPER(PAMI::Mutex::BGP::BgpNodeMutex, mutex2, &mm, argv[1]);

        MUTEX_HELPER(PAMI::Mutex::BGP::LockBoxMutex, mutex3, &mm, argv[1]);

        MUTEX_HELPER(PAMI::Mutex::BGP::FairLockBoxMutex, mutex5, &mm, argv[1]);

        MUTEX_HELPER(PAMI::Mutex::BGP::LwarxStwcxInPlaceMutex, mutex7, &mm, argv[1]);

        MUTEX_HELPER(PAMI::Mutex::BGP::LwarxStwcxIndirMutex, mutex8, &mm, argv[1]);




        BARRIER_HELPER(PAMI::Barrier::BGP::LockBoxNodeProcBarrier, barrier1, &mm, argv[1], argc, (argc == 0));

        BARRIER_HELPER(PAMI::Barrier::BGP::LockBoxNodeCoreBarrier, barrier2, &mm, argv[1], argc, (argc == 0));

        return 1;
}
