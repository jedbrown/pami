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

#ifdef __cplusplus
extern "C" 
#endif
void __pgasrt_tspcoll_comm_block (int, int, int);

int main(int argc, char ** argv)
{
  /* ------------------------- */
  /* ------------------------- */

  __pgasrt_tsp_setup           (1, &argc, &argv);
  __pgasrt_tspcoll_comm_init   ();
  __pgasrt_tsp_barrier         ();

  /* ------------------------- */
  /*      do the split         */
  /* ------------------------- */


  int bf = argc>1 ? atoi(argv[1]) : 1;
  int ncomms = argc>2 ? atoi(argv[2]): PGASRT_NODES;
  __pgasrt_tspcoll_comm_block (11, bf, ncomms);

  int rank2 = __pgasrt_tspcoll_comm_rank (11);
  int size2 = __pgasrt_tspcoll_comm_size (11);

  /* ------------------------- */
  /* print out my communicator */
  /* ------------------------- */

  int i;
  for (i=0; i<PGASRT_NODES; i++)
    {
      if (i==PGASRT_MYNODE)
	{
	  int i;
	  printf ("%d: new comm = { ", PGASRT_MYNODE);
	  for (i=0; i<size2; i++) 
	    printf ("%d ", __pgasrt_tspcoll_comm_rankof (11,i));
	  printf ("}\n");
	  fflush(stdout);
	}
      __pgasrt_tspcoll_barrier(0);
    }
  
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

  /* scatter */
  test_scatter (11, rank2, size2, 1024*1024, 10,
		__pgasrt_tspcoll_scatter,
		__pgasrt_tspcoll_barrier);

  /* scatterv */
  test_scatterv (11, rank2, size2, 1024*1024, 10,
		__pgasrt_tspcoll_scatterv,
		__pgasrt_tspcoll_barrier);
  
  /* allreduce */
  test_allreduce (11, rank2, size2, 1024*1024, 10,
		  __pgasrt_tspcoll_allreduce,
		  __pgasrt_tspcoll_barrier);

  __pgasrt_tsp_barrier();
  __pgasrt_tsp_finish();
  return 0;
}
