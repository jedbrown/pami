#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "pgasrt.h"
#include "testing_framework.c"


double timer();

#define MAXBUFSIZE 100000

/* ************************************************************************* */
/*                  barrier test helper functions                            */
/* ************************************************************************* */

int _g_counter[128];

static int get (int commID, int rank, void * arg)
{
  int grank = __pgasrt_smpcoll_comm_rankof (commID, rank);
  return _g_counter [grank];
}

static void set (int commID, int rank, void *arg, int val)
{
  int grank = __pgasrt_smpcoll_comm_rankof (commID, rank);
  _g_counter [grank] = val;
}

/* ************************************************************************* */
/*                      all tests                                            */
/* ************************************************************************* */

int thread_main (int argc, char ** argv)
{
  __pgasrt_hbrd_barrier ();
  __pgasrt_smpcoll_comm_init();
  __pgasrt_hbrd_barrier ();


  int rank = PGASRT_MYSMPTHREAD;
  int size = PGASRT_SMPTHREADS;

#if 1
  /* barrier */

  test_barrier   (0, rank, size, 1000,
		  NULL,
		  __pgasrt_smpcoll_barrier,
		  get,
		  set);

  /* allgather */

  test_allgather (0, rank, size, 1024*1024, 10,
		  __pgasrt_smpcoll_allgather,
		  __pgasrt_smpcoll_barrier);

  /* allgatherv */

  test_allgatherv (0, rank, size, 1024*1024, 10,
		   __pgasrt_smpcoll_allgatherv, 
		   __pgasrt_smpcoll_barrier);


  /* broadcast */
  test_bcast (0, rank, size, 1024*1024, 10,
	      __pgasrt_smpcoll_bcast,
	      __pgasrt_smpcoll_barrier);

  /* scatter */
  test_scatter (0, rank, size, 1024*1024, 10,
		__pgasrt_smpcoll_scatter,
		__pgasrt_smpcoll_barrier);

  /* scatterv */
  test_scatterv (0, rank, size, 1024*1024, 10,
		__pgasrt_smpcoll_scatterv,
		__pgasrt_smpcoll_barrier);

#endif  

  /* allreduce */
  test_allreduce (0, rank, size, 1024*1024, 10,
		  __pgasrt_smpcoll_allreduce,
		  __pgasrt_smpcoll_barrier);

  /* gather */
  test_gather (0, rank, size, 1024*1024, 10,
               __pgasrt_smpcoll_gather,
               __pgasrt_smpcoll_barrier);



  return 0;
}

/* ************************************************************************* */
/* ************************************************************************* */

int main(int argc, char ** argv)
{

  int i; for (i=0; i<128; i++) _g_counter[i] = 0;
  __pgasrt_tsp_setup (1, &argc, &argv); /* necessary */
  int nthrds = getenv("UPC_NTHREADS") ? atoi (getenv("UPC_NTHREADS")): 1;
  printf ("%s: Running on %d threads/node\n", __FILE__, nthrds);
  __pgasrt_smp_start (argc, argv, thread_main, 
		      nthrds * PGASRT_NODES,
		      nthrds,
		      0);
  return 0;
}

