/**
 * \file test/internals/fifo/bgq/fifo_func.cc
 * \brief ???
 */

#include "../test.h"

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include "components/memory/heap/HeapMemoryManager.h"
#include "components/memory/shmem/SharedMemoryManager.h"

#include "Global.h"


#include "components/devices/shmem/ShmemPacket.h"

#include "components/fifo/FifoPacket.h"
#include "components/fifo/wrap/WrapFifo.h"
#include "components/atomic/bgq/L2CounterBounded.h"

typedef PAMI::Fifo::FifoPacket <32, 160> ShmemPacket;
//typedef PAMI::Fifo::WrapFifo<ShmemPacket, PAMI::BoundedCounter::BGQ::IndirectL2Bounded, 128, Wakeup::BGQ> ShmemFifo;
typedef PAMI::Fifo::WrapFifo<ShmemPacket, PAMI::BoundedCounter::BGQ::IndirectL2Bounded, 128> ShmemFifo;


int main(int argc, char ** argv)
{


  size_t task = __global.mapping.task();
  size_t size = __global.mapping.size();
  //fprintf (stdout, "%zu of %zu\n", task, size);


  PAMI::Memory::GenMemoryManager mm;
  char shmemfile[PAMI::Memory::MMKEYSIZE];
  snprintf (shmemfile, sizeof(shmemfile) - 1, "/foo");
  mm.init(__global.shared_mm, 8*1024*1024, 1, 1, 0, shmemfile);


  Test<ShmemFifo> test;
  test.init (&mm, task, size);
  test.functional();

  return 0;
}
