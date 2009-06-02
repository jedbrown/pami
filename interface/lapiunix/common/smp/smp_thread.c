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

#ifndef __PGASRT_DISTRIBUTED

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <math.h>

//#define DEBUG_THREADS_START
#ifdef DEBUG_THREADS_START
#define TRACE(x) fprintf x
#else
#define TRACE(x)
#endif


/* ********************************************************************* */
/*           data structure to describe running threads                  */
/* ********************************************************************* */

typedef struct ThreadInfo
{
  __pgasrt_thread_t     smp_ID;
  __pgasrt_thread_t     hbrd_ID;
  int                   argc;
  char               ** argv;
  int                (* main) (int, char **);
  pthread_t             pthread;
  pthread_attr_t        pthreadAttr;
  void                * stack_ptr;
  size_t                stacksize;
  int                   rc;
} 
ThreadInfo;

static ThreadInfo    * threadInfo;          /* individual thread IDs */

/* ********************************************************************* */
/*             retrieve local thread ID                                  */
/* ********************************************************************* */

pthread_key_t        __pgasrt_smp_IDKey;    /* key holds local thread ID */

#pragma weak __pgasrt_smp_myID=___pgasrt_smp_myID
__pgasrt_thread_t ___pgasrt_smp_myID (void) 
{
  return (__pgasrt_thread_t) pthread_getspecific (__pgasrt_smp_IDKey);
}

/* ********************************************************************* */
/*                deal with # of threads in each SMP                     */
/* ********************************************************************* */

__pgasrt_thread_t    __pgasrt_smp_nthreads           = -1; 
int                  __pgasrt_smp_log_nthreads       = -1;
int                  __pgasrt_smp_log_tmask          =  0;

#pragma weak __pgasrt_smp_threads=___pgasrt_smp_threads
__pgasrt_thread_t    ___pgasrt_smp_threads   (void)
{
  return __pgasrt_smp_nthreads;
}

#pragma weak __pgasrt_smp_set_threads=___pgasrt_smp_set_threads
void ___pgasrt_smp_set_threads (__pgasrt_thread_t tpn)
{
  __pgasrt_smp_nthreads = tpn;
  __pgasrt_smp_log_nthreads = (int) (0.5+log((double)tpn)/log(2.0));
  __pgasrt_smp_log_tmask    = (1<<__pgasrt_smp_log_nthreads)-1;
  
  if (((__pgasrt_thread_t)(1<<__pgasrt_smp_log_nthreads)) != tpn)
    {
      __pgasrt_smp_log_nthreads = -1;
      __pgasrt_smp_log_tmask    =  0;
    }
}

/* ********************************************************************* */
/*            retrieve global (hybrid) thread ID                         */
/* ********************************************************************* */

pthread_key_t        __pgasrt_hbrd_IDKey;  /* key holds upc thread ID */

#pragma weak __pgasrt_hbrd_myID=___pgasrt_hbrd_myID
__pgasrt_thread_t ___pgasrt_hbrd_myID (void) 
{
  return (__pgasrt_thread_t) pthread_getspecific (__pgasrt_hbrd_IDKey);
}

/* ********************************************************************* */
/*       return the total number of threads running this program         */
/* ********************************************************************* */

__pgasrt_thread_t    __pgasrt_hbrd_nthreads = 0;
int                  __pgasrt_hbrd_log_nthreads = -1;
int                  __pgasrt_hbrd_log_tmask = 0;

#pragma weak __pgasrt_hbrd_threads=___pgasrt_hbrd_threads
__pgasrt_thread_t ___pgasrt_hbrd_threads        (void)
{
  return __pgasrt_hbrd_nthreads;
}

/* ********************************************************************* */
/*             set   # threads                                           */
/* ********************************************************************* */

#pragma weak __pgasrt_hbrd_set_threads=___pgasrt_hbrd_set_threads
void ___pgasrt_hbrd_set_threads(__pgasrt_thread_t threads) 
{
  if (__pgasrt_hbrd_nthreads >0 && __pgasrt_hbrd_nthreads!=threads)
    __pgasrt_fatalerror (-1, "%s: attempt to set threads twice", __FUNCTION__);
  
  __pgasrt_hbrd_nthreads     = threads;
  __pgasrt_hbrd_log_nthreads = (int) (0.5+log((double)threads)/log(2.0));
  __pgasrt_hbrd_log_tmask    = (1<<__pgasrt_hbrd_log_nthreads)-1;
  
  if (((__pgasrt_thread_t)(1<<__pgasrt_hbrd_log_nthreads)) != threads)
    {
      __pgasrt_hbrd_log_nthreads =-1;
      __pgasrt_hbrd_log_tmask    = 0;
    }
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
  pthread_exit((void *)exitcode);
}

#ifndef PTHREAD_STACK_MIN
#  define PTHREAD_STACK_MIN 1024
#endif

/* ********************************************************************* */
/*       run pgasrt_main on multiple threads in this node.                */
/* ********************************************************************* */
static void * run_one_thread (void * arg);

#pragma weak __pgasrt_smp_start=___pgasrt_smp_start
int  ___pgasrt_smp_start (unsigned          argc,
			  char           ** argv,
			  int             (* pmain) (int , char **),
			  __pgasrt_thread_t  totalthreads,
			  __pgasrt_thread_t  threadsPerNode,
			  unsigned           lstack)
{
  __pgasrt_thread_t i;

  __pgasrt_hbrd_set_threads (totalthreads);
  __pgasrt_smp_set_threads  (threadsPerNode);

  /* ---------------------------------------- */
  /*      consistency check                   */
  /* ---------------------------------------- */

  assert (PGASRT_THREADS > 0);
  assert (PGASRT_SMPTHREADS > 0);
  assert (PGASRT_THREADS == PGASRT_NODES * PGASRT_SMPTHREADS);
  assert (PGASRT_SMPTHREADS <= PGASRT_SMPTHREADS_MAX);

  /* ---------------------------------------- */
  /*   initialize static data structures      */
  /* ---------------------------------------- */

  threadInfo = (ThreadInfo *) calloc (threadsPerNode, sizeof(ThreadInfo));
  if (!threadInfo) __pgasrt_fatalerror(-1, "No memory for thread spawn");

  pthread_key_create (&__pgasrt_smp_IDKey,  NULL);
  pthread_key_create (&__pgasrt_hbrd_IDKey, NULL);

  /* -------------------------------------- */
  /*    initialize SMP collectives          */
  /* -------------------------------------- */
  
  __pgasrt_smp_barrier_init (threadsPerNode);

  /* -------------------------------------- */
  /*    set thread-local stack size         */
  /* -------------------------------------- */

  if (lstack > 0) lstack += PTHREAD_STACK_MIN;
  else            lstack =  PTHREAD_STACK_MIN + 16*1024*1024;

  /* -------------------------------------- */
  /* initialize threadInfo, start threads   */
  /* -------------------------------------- */

  TRACE((stderr, "%d: about to spawn %d threads\n", 
	 PGASRT_MYNODE, threadsPerNode));

  for (i=0; i< threadsPerNode; i++)
    {
      threadInfo[i].smp_ID    = i;
      threadInfo[i].hbrd_ID   = i + PGASRT_MYNODE * threadsPerNode;
      threadInfo[i].argc      = argc;
      threadInfo[i].argv      = argv;
      threadInfo[i].main      = pmain;
      threadInfo[i].stacksize = lstack - PTHREAD_STACK_MIN;

      pthread_attr_init (&threadInfo[i].pthreadAttr);
      
      int rc = pthread_attr_setstacksize (&threadInfo[i].pthreadAttr, lstack);
      if (rc != 0) __pgasrt_fatalerror (-1, "cannot set thread attributes");
#if TRANSPORT == bgp
      if (i!=0) 
#endif
	{
	  rc = pthread_create (&threadInfo[i].pthread,
			       &threadInfo[i].pthreadAttr,
			       run_one_thread, 
			       &threadInfo[i]);
	  if (rc != 0) __pgasrt_fatalerror (-1, "thread creation failed");
	}
    }

#if TRANSPORT == bgp
  run_one_thread (&threadInfo[0]);
#endif

  /* -------------------------------------- */
  /* wait for all threads to join           */
  /* -------------------------------------- */

  void * status;
#if TRANSPORT == bgp
  for (i=1; i<threadsPerNode; i++) 
#else
  for (i=0; i<threadsPerNode; i++)
#endif
    pthread_join (threadInfo[i].pthread, &status);

  TRACE((stderr, "%d: all threads have joined\n", PGASRT_MYNODE));

  /* -------------------------------------- */
  /* find first non-zero exit code (if any) */
  /* -------------------------------------- */

  int rc = 0;
  for (i=0; i<threadsPerNode; i++)
    if (threadInfo[i].rc != 0)
      { 
	rc = threadInfo[i].rc;
	break;
      }
  
  /* -------------------------------------- */
  /* clean global data structures           */
  /* -------------------------------------- */

  free (threadInfo);
  pthread_key_delete(__pgasrt_smp_IDKey);
  pthread_key_delete(__pgasrt_hbrd_IDKey);
  return rc;
}

/* *********************************************************************** */
/*      run one thread. initialize modules and dive into user code         */
/* *********************************************************************** */

#if TRANSPORT == bgp
  void __pgasrt_tsp_thread_setup(__pgasrt_thread_t hbrd_ID, 
				 __pgasrt_thread_t smp_ID,
				 __pgasrt_thread_t smp_nthreads);
#endif

void * run_one_thread (void * arg)
{
  ThreadInfo * threadInfo = (ThreadInfo *) arg;

  /* ------------ */
  /* set MYTHREAD */
  /* ------------ */

  pthread_setspecific(__pgasrt_smp_IDKey,   (void *)threadInfo->smp_ID);
  pthread_setspecific(__pgasrt_hbrd_IDKey,  (void *)threadInfo->hbrd_ID);
  TRACE((stderr, "PGAS thread %d is starting \n", threadInfo->hbrd_ID));

  /* set the stack bottom */
  threadInfo->stack_ptr = __builtin_frame_address(0);

  /* --------------------------------------------------- */
  /* Thread specific tsp setup                           */
  /* --------------------------------------------------- */
#if TRANSPORT == bgp
  __pgasrt_tsp_thread_setup(threadInfo->hbrd_ID, 
			    threadInfo->smp_ID,
			    __pgasrt_smp_threads());
#endif

  /* --------------------------------------------------- */
  /* bind the thread to one cpu                          */
  /* --------------------------------------------------- */

//  __pgasrt_cpubind (threadInfo->hbrd_ID);

  /* --------------------------------------------------- */
  /* initialize modules. This is a collective operation. */
  /* --------------------------------------------------- */

  __pgasrt_call_init_modules();

  /* --------------------------------------------------- */
  /* ready to run: call user code                        */
  /* --------------------------------------------------- */

  threadInfo->rc = threadInfo->main (threadInfo->argc, threadInfo->argv);

  /* --------------------------------------------------- */
  /* cleanup                                             */
  /* --------------------------------------------------- */

  TRACE((stderr, "PGAS thread %d is ending normally\n", threadInfo->hbrd_ID));
  __pgasrt_call_fini_modules();
  return (void *)threadInfo->rc;
}


#define ABS(a,b) (a>b) ? (a-b) : (b-a)
int __pgasrt_stackcheck (void)
{
  ThreadInfo * tinfo = &threadInfo[PGASRT_MYSMPTHREAD];
  if (tinfo == NULL) return 0; /* not a PGASRT thread: no check */

  /* -------------------------------------- */
  /*       check stack start, current       */
  /* -------------------------------------- */

  long stack_start   = (long) tinfo->stack_ptr;
  long stack_current = (long) __builtin_frame_address(0);
  long stack_used    = ABS (stack_start, stack_current);

  /* -------------------------------------------------------------- */
  /*       check stack_used value against allocated stack size      */
  /* -------------------------------------------------------------- */

  long stacksize     = (long) tinfo->stacksize; 

  if (stack_used > stacksize)
    __pgasrt_fatalerror (-1, "Stack overflow");


  if ((double)stack_used > (double)stacksize*0.8)
    {
      fprintf (stderr, "%d: Warning: %6.4f%% of allocated thread-local stack"
	       " in use\n", (int)PGASRT_MYTHREAD,
	       (double)stack_used/(double)stacksize*100.0);
      return 1;
    }

  return 0;
}

#if defined(__cplusplus)
}
#endif



#endif /* !__PGASRT_DISTRIBUTED */
