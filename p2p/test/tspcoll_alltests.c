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


#include "./testing_framework.c"

int _g_counter; /* make sure it's same address everywhere */

static int get (int commID, int rank, void * arg)
{
  int result, grank = __pgasrt_tspcoll_comm_rankof (commID, rank);
  if (grank==PGASRT_MYNODE) return _g_counter;
  else
    {
      void * r = __pgasrt_tsp_dget (0, 
				    grank, 
				    (__pgasrt_local_addr_t)&result, 
				    (__pgasrt_addrdesc_t) &_g_counter, 
				    (size_t)0, 
				    sizeof(int),
				    NULL, NULL);
      __pgasrt_tsp_wait (r);
      return result;
    }
}

static void set (int commID, int rank, void *arg, int val)
{
  int grank = __pgasrt_tspcoll_comm_rankof (commID, rank);

  if (grank==PGASRT_MYNODE) _g_counter = val;
  else
    {
      void * r = __pgasrt_tsp_dput (0, 
				    grank, 
				    (__pgasrt_addrdesc_t) &_g_counter, 
				    (size_t)0, 
				    (__pgasrt_local_addr_t) &val, 
				    sizeof(int),
				    NULL, NULL);
      __pgasrt_tsp_wait (r);
    }
}

/* ************************************************************************* */
/* ************************************************************************* */

int main(int argc, char ** argv)
{

  /* set up communicator */

  __pgasrt_tsp_setup         (1, &argc, &argv);
  __pgasrt_tspcoll_comm_init ();
  __pgasrt_tsp_barrier       ();
  int rank = __pgasrt_tspcoll_comm_rank(0);
  int size = __pgasrt_tspcoll_comm_size(0);

#if 1
  /* barrier */
  test_barrier   (0, rank, size, 1000,
                  NULL,
                  __pgasrt_tspcoll_barrier,
                  get,
                  set);

  /* allgather */

  test_allgather (0, rank, size, 1024*1024, 10,
		   __pgasrt_tspcoll_allgather, 
		   __pgasrt_tspcoll_barrier);

  /* allgatherv */

  test_allgatherv (0, rank, size, 1024*1024, 10,
		   __pgasrt_tspcoll_allgatherv, 
		   __pgasrt_tspcoll_barrier);
#endif

  /* broadcast */
  test_bcast (0, rank, size, 1024*1024, 10,
	      __pgasrt_tspcoll_bcast,
	      __pgasrt_tspcoll_barrier);

#if 1
  /* allreduce */
  test_allreduce (0, rank, size, 1024*1024, 10,
		  __pgasrt_tspcoll_allreduce,
		  __pgasrt_tspcoll_barrier);


  /* scatter */
  test_scatter (0, rank, size, 1024*1024, 10,
		__pgasrt_tspcoll_scatter,
		__pgasrt_tspcoll_barrier);

  /* scatterv */
  test_scatterv (0, rank, size, 1024*1024, 10,
		__pgasrt_tspcoll_scatterv,
		__pgasrt_tspcoll_barrier);
#endif
  
#if 0
  /* gather */
  test_gather (0, rank, size, 1024*1024, 10,
	       __pgasrt_tspcoll_gather,
	       __pgasrt_tspcoll_barrier);
#endif

  __pgasrt_tsp_barrier();
  __pgasrt_tsp_finish();
  return 0;
}



