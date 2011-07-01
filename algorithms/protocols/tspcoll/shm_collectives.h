/**
 * \file algorithms/protocols/tspcoll/shm_collectives.h
 * \brief ???
 */
#ifndef __algorithms_protocols_tspcoll_shm_collectives_h__
#define __algorithms_protocols_tspcoll_shm_collectives_h__
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sched.h>


#include "algorithms/protocols/tspcoll/local/SingleAllreduce.h"
#include "algorithms/protocols/tspcoll/local/FixedLeader.h"

extern "C" void upc_poll();

namespace xlpgas{

  struct Wait
  {
    static void wait2() { upc_poll (); }
    static void wait1() { sched_yield(); }
  };

  struct IntPlus
  {
    int operator()(const int a, const int b) const { return a+b; }
  };


  void shm_init(int);

}//end namespace xlpgas
#endif
