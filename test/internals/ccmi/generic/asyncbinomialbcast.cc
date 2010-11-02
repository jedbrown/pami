/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/internals/ccmi/generic/asyncbinomialbcast.cc
 * \brief ???
 */

#include "asyncbcast.h"

int main(int argc, char **argv)
{

  setup(argc, argv);

  // optionally override defaults/args here or test them (short counts, etc)

  initialize(CCMI_BINOMIAL_BARRIER_PROTOCOL,
             CCMI_BINOMIAL_BARRIER_PROTOCOL,
             CCMI_ASYNCBINOMIAL_BROADCAST_PROTOCOL);  // protocol to test

  allocate_buffers();

  if(rank == 0) TRACE_TEST((stdout,"%s:  AsyncBcast begin performance runs\n", argv0));

  // Performance run latency (count = 1);
  int i;
  double t, latencyt = 0, bandwidtht = 0;

  for(i = 0; i < warmup_repetitions; i++) asyncbcast_advance (srcbuf,1,0);

  barrier_advance();

  t = MPI_Wtime();
  for(i = 0; i < repetitions; i++) asyncbcast_advance (srcbuf,1,0);

  barrier_advance();

  latencyt = MPI_Wtime() - t;

  // Performance run bandwidth
  for(i = 0; i < warmup_repetitions; i++) asyncbcast_advance (srcbuf,count,0);

  barrier_advance();

  t = MPI_Wtime();
  for(i = 0; i < repetitions; i++) asyncbcast_advance (srcbuf,count,0);

  barrier_advance();

  bandwidtht = MPI_Wtime() - t;

  print_performance(latencyt, bandwidtht);

  cleanup();

  return 0;
}
