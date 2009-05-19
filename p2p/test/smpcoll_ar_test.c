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

  /* allreduce */
  test_allreduce (0, rank, size, 1024*1024, 1000,
		  __pgasrt_smpcoll_allreduce,
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

