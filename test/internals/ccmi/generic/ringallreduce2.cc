/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/tests/generic/ringallreduce2.cc
 * \brief allreduce for different counts
 */

//#define TRACE_TEST_VERBOSE(x) fprintf x
//#define TRACE_TEST(x) fprintf x
#define STRICT_CHECKING 1

#include "allreduce.h"

int main(int argc, char **argv)
{

  int ret = 0;

  setup(argc, argv);

  // optionally override defaults/args here or test them (short counts, etc)

  initialize(CCMI_BINOMIAL_BARRIER_PROTOCOL,
             CCMI_BINOMIAL_BARRIER_PROTOCOL,
             CCMI_RING_ALLREDUCE_PROTOCOL); // protocol to test

  allocate_buffers();

  // Simple functional run:
  //
  // Try different counts: 1, 2, 4, 8 ... count
  for(unsigned i = 1; i <= count; i*=2)
  {
    setBuffers(srcbuf, dstbuf, i);
    if(rank == 0) TRACE_TEST((stderr, "allreduce size %d\n", i));
    allreduce_advance(srcbuf,dstbuf,i);
    ret += checkBuffers(srcbuf, dstbuf, i);
  }

  cleanup(ret);

  return 0;
}
