/* ************************************************************************* */
/*                            IBM Confidential                               */
/*                          OCO Source Materials                             */
/*                      IBM XL UPC Alpha Edition, V0.9                       */
/*                                                                           */
/*                      Copyright IBM Corp. 2005, 2007.                      */
/*                                                                           */
/* The source code for this program is not published or otherwise divested   */
/* of its trade secrets, irrespective of what has been deposited with the    */
/* U.S. Copyright Office.                                                    */
/* ************************************************************************* */

#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>

#include "pgasrt.h"

/* ************************************************************************* */
/* ************************************************************************* */

static double timer()
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return 1e6*(double)tv.tv_sec + (double)tv.tv_usec;
}

int main(int argc, char ** argv)
{
  int niters = 10000;
  struct { double d; int i; } sbuf, rbuf;


  /* set up communicator */

  __pgasrt_tsp_setup         (1, &argc, &argv);
  __pgasrt_tspcoll_comm_init ();
  __pgasrt_tsp_barrier       ();
  int rank = __pgasrt_tspcoll_comm_rank(0);
  int size = __pgasrt_tspcoll_comm_size(0);


  sbuf.d = (double) rank;
  sbuf.i = rank;

  double t0 = timer();
  for (int i=0; i<niters; i++)
    {
      __pgasrt_tspcoll_allreduce (0, &sbuf, &rbuf, 
				  PGASRT_OP_MAX,
				  PGASRT_DT_dblint,
				  1);
    }
  double t1 = timer();

  if (rank==0) printf ("Latency=%g microseconds\n", (t1-t0)/niters);
  return 0;
}
