/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/tests/generic/onesidedbinomialbcast.cc
 * \brief ???
 */

#include "onesidedbcast.h"

int main(int argc, char **argv)
{
  setup(argc, argv);

  // optionally override defaults/args here or test them (short counts, etc)

  initialize(CCMI_BINOMIAL_BARRIER_PROTOCOL,
             CCMI_BINOMIAL_BARRIER_PROTOCOL,
             CCMI_ASYNCBINOMIAL_BROADCAST_PROTOCOL);  // protocol to test

  allocate_buffers();

  if(rank == 0) TRACE_TEST((stdout,"%s:  OnesidedBcast begin performance runs\n", argv0));

  // Performance runs
  double t, latencyt = 0, bandwidtht = 0;

  nreceived_countdown = warmup_repetitions; // reset for first run below (before the barrier)
  barrier_advance();

  // warmup
  onesidedbcast_advance_repetitions (srcbuf,1,0,warmup_repetitions);
  nreceived_countdown = repetitions; // reset for NEXT run below (before the barrier)
  barrier_advance();

  // Latency run (count = 1);
  t = MPI_Wtime();
  onesidedbcast_advance_repetitions (srcbuf,1,0,repetitions);
  nreceived_countdown = warmup_repetitions; // reset for NEXT run below (before the barrier)
  barrier_advance();
  latencyt = MPI_Wtime() - t;

  // warmup
  onesidedbcast_advance_repetitions (srcbuf,count,0,warmup_repetitions);
  nreceived_countdown = repetitions; // reset for NEXT run below (before the barrier)
  barrier_advance();

  // Bandwidth run (count)
  t = MPI_Wtime();
  onesidedbcast_advance_repetitions (srcbuf,count,0,repetitions);
  barrier_advance();
  bandwidtht = MPI_Wtime() - t;

  print_performance(latencyt, bandwidtht);

  cleanup();

  return 0;
}
