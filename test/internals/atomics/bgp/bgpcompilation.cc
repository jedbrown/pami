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
        PAMI::Memory::MemoryManager mm;
        mm.init(argv, argc); // anything

        COUNTER_HELPER(PAMI::Counter::BGP::LockBoxNodeCounter, counter1, mm);

        COUNTER_HELPER(PAMI::Counter::BGP::LockBoxProcCounter, counter2, mm);

        COUNTER_HELPER(PAMI::Counter::BGP::BgpProcCounter, counter3, mm);

        COUNTER_HELPER(PAMI::Counter::BGP::BgpNodeCounter, counter4, mm);




        MUTEX_HELPER(PAMI::Mutex::BGP::BgpProcMutex, mutex1, mm);

        MUTEX_HELPER(PAMI::Mutex::BGP::BgpNodeMutex, mutex2, mm);

        MUTEX_HELPER(PAMI::Mutex::BGP::LockBoxProcMutex, mutex3, mm);

        MUTEX_HELPER(PAMI::Mutex::BGP::LockBoxNodeMutex, mutex4, mm);

        MUTEX_HELPER(PAMI::Mutex::BGP::FairLockBoxProcMutex, mutex5, mm);

        MUTEX_HELPER(PAMI::Mutex::BGP::FairLockBoxNodeMutex, mutex6, mm);

        MUTEX_HELPER(PAMI::Mutex::BGP::LwarxStwcxProcMutex, mutex7, mm);

        MUTEX_HELPER(PAMI::Mutex::BGP::LwarxStwcxNodeMutex, mutex8, mm);




        BARRIER_HELPER(PAMI::Barrier::BGP::LockBoxNodeProcBarrier, barrier1, mm, argc, (argc == 0));

        BARRIER_HELPER(PAMI::Barrier::BGP::LockBoxNodeCoreBarrier, barrier2, mm, argc, (argc == 0));

        BARRIER_HELPER(PAMI::Barrier::BGP::LwarxStwcxNodeProcBarrier, barrier3, mm, argc, (argc == 0));

        BARRIER_HELPER(PAMI::Barrier::BGP::LwarxStwcxNodeCoreBarrier, barrier4, mm, argc, (argc == 0));

        return 1;
}
