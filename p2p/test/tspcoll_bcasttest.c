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


#include "testing_framework.c"

int _g_counter; /* make sure it's same address everywhere */

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

  test_bcast (0, rank, size, 1024*1024, 10,
	      __pgasrt_tspcoll_bcast,
	      __pgasrt_tspcoll_barrier);

  __pgasrt_tsp_barrier();
  __pgasrt_tsp_finish();
  return 0;
}



