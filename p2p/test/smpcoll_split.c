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


/* ************************************************************************* */
/*                      all tests                                            */
/* ************************************************************************* */

int thread_main (int argc, char ** argv)
{
  __pgasrt_hbrd_barrier ();
  __pgasrt_smpcoll_comm_init ();
  __pgasrt_hbrd_barrier ();
  int rank = __pgasrt_smpcoll_comm_rank(0);

  /* ------------------------- */
  /*      do the split         */
  /* ------------------------- */

  int commsize = argc>2 ? atoi(argv[2]) : 3;
  __pgasrt_smpcoll_comm_split (0, 11, rank/commsize, rank%commsize);
  int rank2 = __pgasrt_smpcoll_comm_rank (11);
  int size2 = __pgasrt_smpcoll_comm_size (11);

  /* ------------------------- */
  /* print out my communicator */
  /* ------------------------- */

  if (rank2==0)
    {
      int i;
      printf ("%d: new comm = { ", rank);
      for (i=0; i<size2; i++)
        printf ("%d ", __pgasrt_smpcoll_comm_rankof (11,i));
      printf ("}\n");
      fflush(stdout);
    }
  __pgasrt_smpcoll_barrier(0);

  /* allgather */

  test_allgather (11, rank2, size2, 1024*1024, 10,
		   __pgasrt_smpcoll_allgather, 
		   __pgasrt_smpcoll_barrier);

  /* allgatherv */

  test_allgatherv (11, rank2, size2, 1024*1024, 10,
		   __pgasrt_smpcoll_allgatherv, 
		   __pgasrt_smpcoll_barrier);


  /* broadcast */
  test_bcast (11, rank2, size2, 1024*1024, 10,
	      __pgasrt_smpcoll_bcast,
	      __pgasrt_smpcoll_barrier);

  /* scatter */
  test_scatter (11, rank2, size2, 1024*1024, 10,
		__pgasrt_smpcoll_scatter,
		__pgasrt_smpcoll_barrier);

  /* scatterv */
  test_scatterv (11, rank2, size2, 1024*1024, 10,
		__pgasrt_smpcoll_scatterv,
		__pgasrt_smpcoll_barrier);
  

  /* allreduce */
  test_allreduce (11, rank2, size2, 1024*1024, 10,
		  __pgasrt_smpcoll_allreduce,
		  __pgasrt_smpcoll_barrier);


  return 0;
}

/* ************************************************************************* */
/* ************************************************************************* */

int main(int argc, char ** argv)
{
  // int i; for (i=0; i<128; i++) _g_counter[i] = 0;
  __pgasrt_tsp_setup (1, &argc, &argv); /* necessary */
  int nthrds = argc > 1 ? atoi(argv[1]) : 10;
  __pgasrt_smp_start (argc, argv, thread_main,
                      nthrds * PGASRT_NODES,
                      nthrds,
                      0);
  return 0;
}
