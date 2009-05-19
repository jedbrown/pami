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

#include "pgasrt.h"
#include "testing_framework.c"

double timer();

/* ********************************************************************** */
/* ********************************************************************** */

int thread_main (int argc, char ** argv)
{
  __pgasrt_hbrd_barrier ();
  __pgasrt_hbrdcoll_comm_init ();
  __pgasrt_hbrd_barrier ();


  int rank = PGASRT_MYTHREAD;
  int size = PGASRT_THREADS;


  /* allgather */

  test_allgather (0, rank, size, 1024*1024, 10,
		  __pgasrt_hbrdcoll_allgather,
		  __pgasrt_hbrdcoll_barrier);

  /* allgatherv */

  test_allgatherv (0, rank, size, 1024*1024, 10,
		   __pgasrt_hbrdcoll_allgatherv, 
		   __pgasrt_hbrdcoll_barrier);

  /* broadcast */
  test_bcast (0, rank, size, 1024*1024, 10,
	      __pgasrt_hbrdcoll_bcast,
	      __pgasrt_hbrdcoll_barrier);

#if 0
  /* scatter */
  test_scatter (0, rank, size, 1024*1024, 10,
		__pgasrt_hbrdcoll_scatter,
		__pgasrt_hbrdcoll_barrier);

  /* scatterv */
  test_scatterv (0, rank, size, 1024*1024, 10,
		__pgasrt_hbrdcoll_scatterv,
		__pgasrt_hbrdcoll_barrier);
  
#endif

  /* allreduce */
  test_allreduce (0, rank, size, 1024*1024, 10,
		  __pgasrt_hbrdcoll_allreduce,
		  __pgasrt_hbrdcoll_barrier);

#if 0

  /* gather */
  test_gather (0, rank, size, 1024*1024, 10,
               __pgasrt_hbrdcoll_gather,
               __pgasrt_hbrdcoll_barrier);

#endif

  return 0;
}

/* ********************************************************************** */
/* ********************************************************************** */

int main(int argc, char ** argv)
{
  char * upc_nthreads = getenv ("UPC_NTHREADS");
  int thrds = upc_nthreads ? atoi(upc_nthreads) : 0;
  return __pgasrt_hbrd_start (argc, argv, thread_main, thrds, 0);
}
