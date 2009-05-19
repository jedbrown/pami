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

#ifndef __tspcoll_barrier_h__
#define __tspcoll_barrier_h__

#include <assert.h>
#include <stdio.h>
#include <sched.h>
#include <stdlib.h>
//#define DEBUG_SMPBARRIER 1
#undef TRACE
#ifdef DEBUG_SMPBARRIER
#define TRACE(x)  fprintf x;
#else
#define TRACE(x)
#endif

namespace SMPColl
{
  class Barrier
  {
    static const int BUSYWAITCYCLES=1000;
    static const int YIELDCYCLES=10;

  public:
    Barrier (int thrds);
    // ~Barrier ();
    void       execute (int rank);
    void       setaddr (int rank, void * addr) { _cache[rank].v.addr = addr; }
    void *     getaddr (int rank) const { return _cache[rank].v.addr; }
    
  private:
    struct CacheLine_t 
    { 
      volatile int  p;          /* phase id */
      int           count;      /* current barrier count */
      union {
	long long   ull;
	void      * addr;
      }             v;
      int           dummy[28]; 
    };

  private:
    CacheLine_t * _cache;
    int           _thrds;
    int           _phases;
  };
};

/* ************************************************************************* */
/*                  barrier constructor                                      */
/* ************************************************************************* */

inline SMPColl::Barrier::Barrier (int thrds)
{
  assert (sizeof(CacheLine_t)==128);
  _thrds  = thrds;
  _phases = (unsigned)-1; for (int n=2*_thrds-1; n>0; n>>=1) _phases++;
  TRACE((stderr, "%d: Barrier constructor(threads=%d phases=%d)\n",
	 PGASRT_MYSMPTHREAD, thrds, _phases));
  _cache  = (CacheLine_t *)malloc(sizeof(CacheLine_t) * _thrds * (1+_phases));
  assert (_cache != NULL);
  for (int thrd=0; thrd<_thrds; thrd++)
    {
      for (int l=0; l<_phases; l++) _cache[thrd+l*_thrds].p = 0;
      _cache[thrd].count = 0;
    }
}

/* ************************************************************************* */
/*                  deallocate storage taken by barrier                      */
/* ************************************************************************* */

#if 0
inline SMPColl::Barrier::~Barrier ()
{
  free (_cache);
}
#endif

/* ************************************************************************* */
/*         execute barrier. All ranks (0.._thrds-1) execute together         */
/* ************************************************************************* */

inline void SMPColl::Barrier::execute (int rank)
{
  int barrno = ++_cache[rank].count;
  TRACE((stderr, "%d: barrier %d size=%d phases=%d\n", rank, barrno, _thrds, _phases));
  for (int l=0; l<_phases; l++)
    {
      unsigned neighbor           = (rank+(1<<(l)))%_thrds;
      _cache[l*_thrds+neighbor].p = barrno;                    /* SEND */
      volatile int * p1 = &_cache [l*_thrds+rank].p; /* RECV */
#pragma nounroll
      for (int z = 0; z<BUSYWAITCYCLES && *p1<barrno ; z++) ;
#pragma nounroll
      for (int z = 0; z<YIELDCYCLES && *p1<barrno ; z++ ) sched_yield();
#pragma nounroll
      for (; *p1 < barrno; ) 
	{
	  // if (rank==0) __pgasrt_tsp_wait (NULL);
	  // else sched_yield();
	  sched_yield();
	}
    }
}

#endif
