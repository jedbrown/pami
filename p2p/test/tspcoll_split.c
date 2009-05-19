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

#include "pgasrt.h"
#include "testing_framework.c"

int main(int argc, char ** argv)
{
  /* ------------------------- */
  /* ------------------------- */

  __pgasrt_tsp_setup           (1, &argc, &argv);
  __pgasrt_tspcoll_comm_init   ();
  __pgasrt_tsp_barrier         ();
  int rank = __pgasrt_tspcoll_comm_rank(0);
  // int size = __pgasrt_tspcoll_comm_size(0);

  /* ------------------------- */
  /*      do the split         */
  /* ------------------------- */


  int commsize = argc>1 ? atoi(argv[1]) : 3;
  __pgasrt_tspcoll_comm_split (0, 11, rank/commsize, rank%commsize);
  int rank2 = __pgasrt_tspcoll_comm_rank (11);
  int size2 = __pgasrt_tspcoll_comm_size (11);

  /* ------------------------- */
  /* print out my communicator */
  /* ------------------------- */

  if (rank2==0)
    {
      int i;
      printf ("%d: new comm = { ", rank);
      for (i=0; i<size2; i++) 
	printf ("%d ", __pgasrt_tspcoll_comm_rankof (11,i));
      printf ("}\n");
      fflush(stdout);
    }
  __pgasrt_tspcoll_barrier(0);


  /* allgather */

  test_allgather (11, rank2, size2, 1024*1024, 10,
		   __pgasrt_tspcoll_allgather, 
		   __pgasrt_tspcoll_barrier);

  /* allgatherv */

  test_allgatherv (11, rank2, size2, 1024*1024, 10,
		   __pgasrt_tspcoll_allgatherv, 
		   __pgasrt_tspcoll_barrier);


  /* broadcast */
  test_bcast (11, rank2, size2, 1024*1024, 10,
	      __pgasrt_tspcoll_bcast,
	      __pgasrt_tspcoll_barrier);

#if 0
  /* scatter */
  test_scatter (11, rank2, size2, 1024*1024, 10,
		__pgasrt_tspcoll_scatter,
		__pgasrt_tspcoll_barrier);

  /* scatterv */
  test_scatterv (11, rank2, size2, 1024*1024, 10,
		__pgasrt_tspcoll_scatterv,
		__pgasrt_tspcoll_barrier);

#endif  

  /* allreduce */
  test_allreduce (11, rank2, size2, 1024*1024, 10,
		  __pgasrt_tspcoll_allreduce,
		  __pgasrt_tspcoll_barrier);

  __pgasrt_tsp_barrier();
  __pgasrt_tsp_finish();
  return 0;
}
