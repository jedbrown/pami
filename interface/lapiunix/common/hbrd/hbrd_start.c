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
#include <string.h>

#include "../include/pgasrt.h"

#if defined(__cplusplus)
extern "C" {
#endif


//#define DEBUG_START 1
#ifdef DEBUG_START
#define TRACE(x)  fprintf x;
#else
#define TRACE(x)
#endif

extern void __upcrt_initialize (void);
extern void __upcrt_finish (void);

#pragma weak __pgasrt_hbrd_start=___pgasrt_hbrd_start
int ___pgasrt_hbrd_start (int                       argc, 
			  char                   ** argv, 
			  int                    (* pmain) (int, char **), 
			  __pgasrt_thread_t         totalthreads, 
			  unsigned                  lstack)
{
  /* -------------------------------------------------- */
  /*   setup the transport: PGASRT_NODES is now set.    */
  /* -------------------------------------------------- */

  __pgasrt_tsp_setup (totalthreads, &argc, &argv);
  __pgasrt_tsp_barrier();

  /* -------------------------------------------------------- */
  /*   if the compiler restricts the number of nodes we can   */
  /*   run on, make sure we are running on that restriction.  */
  /* -------------------------------------------------------- */

  if( __pgasrt_tsp_expected_nodes != 0 && __pgasrt_tsp_expected_nodes != PGASRT_NODES )
      __pgasrt_fatalerror(-1, "Running on %d nodes but compiler asks for %d",
              PGASRT_NODES, __pgasrt_tsp_expected_nodes);

  /* -------------------------------------------------------- */
  /*   check that requested total # of threads is feasible    */
  /* -------------------------------------------------------- */

  if (totalthreads == 0) totalthreads = PGASRT_NODES;
  __pgasrt_thread_t tpn = totalthreads / PGASRT_NODES; 
  if (totalthreads != PGASRT_NODES * tpn)
    __pgasrt_fatalerror(-1, "User requested %d threads: not a multiple "
			"of # tasks running this job (%d)\n",
			totalthreads, PGASRT_NODES);

  if (tpn > PGASRT_SMPTHREADS_MAX)
    __pgasrt_fatalerror(-1, "Too many (%d) SMP threads requested", tpn);

  __pgasrt_hbrd_set_threads (totalthreads);
  __pgasrt_smp_set_threads (tpn);

  /* ----------------------------------------------------------- */
  /*         Initialize various parts of the runtime             */
  /* ----------------------------------------------------------- */

#if 0
  __pgasrt_svd_init (totalthreads+1);
  __upcrt_initialize ();
#endif
  // cafrt_initonce ();

  /* ---------------------------------------------------------- */
  /*     spawn threads and run main program chosen by user      */
  /* ---------------------------------------------------------- */

  __pgasrt_tsp_barrier ();
  int rc = __pgasrt_smp_start (argc, argv, pmain, totalthreads, tpn, lstack);
  __pgasrt_tsp_barrier();

  /* ---------------------------------------------------------- */
  /*       invoke deallocators in reverse order                 */
  /* ---------------------------------------------------------- */

  // __cafrt_finish();
#if 0
  __upcrt_finish();
  __pgasrt_svd_free();
#endif
  __pgasrt_tsp_finish();
  return rc;
}

#if defined(__cplusplus)
}
#endif
