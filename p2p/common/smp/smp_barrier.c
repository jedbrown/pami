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

#include <assert.h>
#include <stdio.h>
#include "../include/pgasrt.h"

#if defined(__cplusplus)
extern "C" {
#endif


//#define DEBUG_SMP_BARRIER
#ifdef DEBUG_SMP_BARRIER
#define TRACE(x) fprintf x
#else
#define TRACE(x)
#endif

/* *********************************************************************** */
/*                   data structure for SMP barrier                        */
/* *********************************************************************** */

#define MAXT PGASRT_SMPTHREADS_MAX
typedef struct SMPBarrier
{
  struct { volatile unsigned c[32]; } msg  [8][MAXT];
  struct { volatile unsigned c[32]; } ctr [MAXT];
  unsigned                            N;
  unsigned                            logN;
}
SMPBarrier;

static SMPBarrier * _barrier;

/* *********************************************************************** */
/*                       SMP collective initialization                     */
/* *********************************************************************** */
/* Needs called only by one thread.                                        */
/* *********************************************************************** */

void __pgasrt_smp_barrier_init (int N)
{
  SMPBarrier * b = (_barrier= (SMPBarrier *) malloc (sizeof(SMPBarrier)));
  b->N = N;
  if (b->N > MAXT) __pgasrt_fatalerror (-1, "Too many SMP threads");
  b->logN = (unsigned)-1; int n; for (n=2*b->N-1; n>0; n>>=1) b->logN++;
  unsigned myproc; for (myproc=0; myproc<b->N; myproc++)
    {
      unsigned l; for (l=0; l< b->logN+1; l++) b->msg[l][myproc].c[0]  = 0;
      b->ctr[myproc].c[0] = 0;
    }
}

/* *********************************************************************** */
/*  execute a barrier exchange  (recursive doubling algorithm)             */
/* *********************************************************************** */

#define BUSYWAITCYCLES 1000
#pragma weak __pgasrt_smp_barrier=___pgasrt_smp_barrier
void ___pgasrt_smp_barrier (void)
{
  SMPBarrier * b  = (SMPBarrier*) _barrier;   assert (b != NULL);
  int       myID  = PGASRT_MYSMPTHREAD;
  int          N  = b->N;

  /* ------------------------------------------ */
  /*   start barrier execution                  */
  /* ------------------------------------------ */

  unsigned barrno = ++(b->ctr[myID].c[0]);
  unsigned l,z; 
  for (l=0; l< b->logN; l++)
    {
      unsigned neighbor           = (myID + (1<<(l)))%N;
      b->msg[l][neighbor].c[0]    = barrno;
      volatile unsigned * p1      = & b->msg [l][myID].c[0];
      
      /* ------------------- */
      /* part 1: busywaiting */
      /* ------------------- */
      
#pragma nounroll
      for (z = 0; z<BUSYWAITCYCLES && *p1<barrno ; z++) ;
      
      /* ------------------------------- */
      /* part 2: settle in for long wait */
      /* ------------------------------- */
      
#pragma nounroll
      for (; *p1<barrno ; ) 
	{
	  sched_yield();
	  __pgasrt_tsp_wait(NULL);
	}
    }
}

#if defined(__cplusplus)
}
#endif
