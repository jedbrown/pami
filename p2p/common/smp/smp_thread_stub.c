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
#include "../include/pgasrt.h"

#if defined(__cplusplus)
extern "C" {
#endif


#ifdef __PGASRT_DISTRIBUTED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//#define DEBUG_THREADS_START
#ifdef DEBUG_THREADS_START
#define TRACE(x) fprintf x
#else
#define TRACE(x)
#endif

/* ********************************************************************* */
/*             retrieve local thread ID                                  */
/* ********************************************************************* */

#pragma weak __pgasrt_smp_myID=___pgasrt_smp_myID
__pgasrt_thread_t ___pgasrt_smp_myID (void) 
{
  return 0;
}

/* ********************************************************************* */
/*       return the local thread ID by dereferencing a pthread key       */
/* ********************************************************************* */

__pgasrt_thread_t    __pgasrt_smp_nthreads;  /* #threads in each SMP space */

#pragma weak __pgasrt_smp_threads=___pgasrt_smp_threads
__pgasrt_thread_t    ___pgasrt_smp_threads   (void)
{
  return __pgasrt_smp_nthreads;
}

/* ********************************************************************* */
/*            retrieve global (hybrid) thread ID                         */
/* ********************************************************************* */

#pragma weak __pgasrt_hbrd_myID=___pgasrt_hbrd_myID
__pgasrt_thread_t ___pgasrt_hbrd_myID (void) 
{
  return (__pgasrt_thread_t) __pgasrt_tsp_myID ();
}

/* ********************************************************************* */
/*       return the total number of threads running this program         */
/* ********************************************************************* */

__pgasrt_thread_t    __pgasrt_hbrd_nthreads;

#pragma weak __pgasrt_hbrd_threads=___pgasrt_hbrd_threads
__pgasrt_thread_t ___pgasrt_hbrd_threads        (void)
{
  return __pgasrt_hbrd_nthreads;
}

/* ********************************************************************* */
/*                    force # of threads                                 */
/* ********************************************************************* */

#pragma weak __pgasrt_set_static_threads=___pgasrt_set_static_threads
void ___pgasrt_set_static_threads(int threads) 
{
  if (__pgasrt_hbrd_nthreads==0) 
    __pgasrt_hbrd_nthreads = threads;
  else
    __pgasrt_fatalerror (-1, "%s: threads already set before", __FUNCTION__);
}

/* *********************************************************************** */
/* standard exit (preferrable to calling exit() because no messing with    */
/* exit handlers).                                                         */
/* *********************************************************************** */

#pragma weak __pgasrt_smp_exit=___pgasrt_smp_exit
void ___pgasrt_smp_exit (int exitcode)
{
  TRACE((stderr,"%s: thread %d called exit!\n",__FUNCTION__,PGASRT_MYTHREAD));
  __pgasrt_call_fini_modules();
}

/* *********************************************************************** */
/*   Immediate exit. What this does to LAPI etc. is not really defined     */
/* *********************************************************************** */

#pragma weak __pgasrt_smp_exitNow=___pgasrt_smp_exitNow
void ___pgasrt_smp_exitNow (int status) 
{
  TRACE((stderr, "%d: Immediate exit, status %d\n", PGADSRT_MYTHREAD, status));
  fflush(NULL);
  _exit(status);
}

/* ********************************************************************* */
/*       run pgasrt_main on multiple threads in this node.                */
/* ********************************************************************* */

#pragma weak __pgasrt_smp_start=___pgasrt_smp_start
int  ___pgasrt_smp_start (unsigned     argc,
                          char      ** argv,
                          int       (* pmain) (int , char **),
                          __pgasrt_thread_t totalthreads,
                          __pgasrt_thread_t threadsPerNode,
                          unsigned     lstack)
{
  if (threadsPerNode != 1)
    __pgasrt_fatalerror (-1, "Max 1 thread/node supported on this system");

  __pgasrt_smp_nthreads      = 1;
  __pgasrt_hbrd_nthreads     = __pgasrt_tsp_numnodes ();
 
  /* --------------------------------------------------- */
  /*   initialize SMP collectives                        */
  /* --------------------------------------------------- */

  __pgasrt_smp_barrier_init (1);

  /* --------------------------------------------------- */
  /* initialize modules. This is a collective operation. */
  /* --------------------------------------------------- */

  __pgasrt_call_init_modules();

  /* --------------------------------------------------- */
  /* ready to run: call user code                        */
  /* --------------------------------------------------- */
  
  int rc = pmain (argc, argv);

  /* --------------------------------------------------- */
  /* cleanup comes with a barrier.                       */
  /* --------------------------------------------------- */

  __pgasrt_call_fini_modules();

 return rc;
}

int __pgasrt_stackcheck (void)
{
  return 0; /* there is no spoo^H^H^H^H stack */
}


#endif

#if defined(__cplusplus)
}
#endif

