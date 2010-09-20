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
#include "components/atomic/bgp/LwarxStwcxBarrier.h"

int main(int argc, char **argv) {
	if (argc >= 0) {
		fprintf(stderr, "This program is not intended to be run!\n");
		exit(1);
	}

        COUNTER_HELPER(PAMI::Counter::BGP::LockBoxNodeCounter, counter1, argv[1]);

        COUNTER_HELPER(PAMI::Counter::BGP::LockBoxProcCounter, counter2, argv[1]);

        COUNTER_HELPER(PAMI::Counter::BGP::BgpProcCounter, counter3, argv[1]);

        COUNTER_HELPER(PAMI::Counter::BGP::BgpNodeCounter, counter4, argv[1]);




        MUTEX_HELPER(PAMI::Mutex::BGP::BgpProcMutex, mutex1, argv[1]);

        MUTEX_HELPER(PAMI::Mutex::BGP::BgpNodeMutex, mutex2, argv[1]);

        MUTEX_HELPER(PAMI::Mutex::BGP::LockBoxProcMutex, mutex3, argv[1]);

        MUTEX_HELPER(PAMI::Mutex::BGP::LockBoxNodeMutex, mutex4, argv[1]);

        MUTEX_HELPER(PAMI::Mutex::BGP::FairLockBoxProcMutex, mutex5, argv[1]);

        MUTEX_HELPER(PAMI::Mutex::BGP::FairLockBoxNodeMutex, mutex6, argv[1]);

        MUTEX_HELPER(PAMI::Mutex::BGP::LwarxStwcxProcMutex, mutex7, argv[1]);

        MUTEX_HELPER(PAMI::Mutex::BGP::LwarxStwcxNodeMutex, mutex8, argv[1]);




        BARRIER_HELPER(PAMI::Barrier::BGP::LockBoxNodeProcBarrier, barrier1, argv[1], argc, (argc == 0));

        BARRIER_HELPER(PAMI::Barrier::BGP::LockBoxNodeCoreBarrier, barrier2, argv[1], argc, (argc == 0));

        BARRIER_HELPER(PAMI::Barrier::BGP::LwarxStwcxNodeProcBarrier, barrier3, argv[1], argc, (argc == 0));

        BARRIER_HELPER(PAMI::Barrier::BGP::LwarxStwcxNodeCoreBarrier, barrier4, argv[1], argc, (argc == 0));

        return 1;
}
