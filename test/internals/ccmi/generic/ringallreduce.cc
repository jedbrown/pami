/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/internals/ccmi/generic/ringallreduce.cc
 * \brief performance test
 */

//#define TRACE_TEST_VERBOSE(x) fprintf x
//#define TRACE_TEST(x) fprintf x

#include "allreduce.h"

int main(int argc, char **argv)
{

  setup(argc, argv);

  // optionally override defaults/args here or test them (short counts, etc)

  initialize(CCMI_BINOMIAL_BARRIER_PROTOCOL,
             CCMI_BINOMIAL_BARRIER_PROTOCOL,
             CCMI_RING_ALLREDUCE_PROTOCOL); // protocol to test

  allocate_buffers();

  if(rank == 0) TRACE_TEST((stdout,"%s:  Allreduce begin performance runs\n", argv0));

  // Performance run latency (count = 1);
  int i;
  double t, latencyt = 0, bandwidtht = 0;

  for(i = 0; i < warmup_repetitions; i++) allreduce_advance (srcbuf,dstbuf,1);
  t = MPI_Wtime();
  for(i = 0; i < repetitions; i++) allreduce_advance (srcbuf,dstbuf,1);
  latencyt = MPI_Wtime() - t;

  // Performance run bandwidth
  for(i = 0; i < warmup_repetitions; i++) allreduce_advance (srcbuf,dstbuf,count);
  t = MPI_Wtime();
  for(i = 0; i < repetitions; i++) allreduce_advance (srcbuf,dstbuf,count);
  bandwidtht = MPI_Wtime() - t;

  print_performance(latencyt, bandwidtht);

  cleanup();

  return 0;
}
