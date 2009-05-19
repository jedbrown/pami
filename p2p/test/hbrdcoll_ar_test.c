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


  /* allreduce */
  test_allreduce (0, rank, size, 1024*1024, 10,
		  __pgasrt_hbrdcoll_allreduce,
		  __pgasrt_hbrdcoll_barrier);

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
