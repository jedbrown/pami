#ifndef __smpcoll_allreduce_h__
#define __smpcoll_allreduce_h__

#include "../smpcoll/Communicator.h"
#include "../tspcoll/Allreduce.h"

/* ************************************************************************* */
/*                     power-of-2 allreduce                                  */
/* ************************************************************************* */

namespace SMPColl
{
  class Allreduce
  {
    static const int BUSYWAITCYCLES = 1000;
    static const int YIELDCYCLES=10;

  public:
    Allreduce (int thrds);

    void execute (int rank, 
		  void * buf, 
		  __pgasrt_ops_t op, 
		  __pgasrt_dtypes_t dt,
		  int nelems);

  private:
    struct CacheLine_t
    {
      union {
	char        * buf;
	unsigned long long unused;
      } v;
      int           count;      /* current barrier count */
      int           peer;       /* peer thread in current phase */
      volatile int  p;          /* exchange mailbox */
      int           dummy[27];
    };

    void exchange (int rank, int peer, int phase, int value)
    {
      __pgasrt_smp_fence();
      _cache[peer + phase*_thrds].p = value;
      volatile int * p1 = &_cache[rank + phase*_thrds].p;
#pragma nounroll
      for (int z = 0; z<BUSYWAITCYCLES && *p1 < value ; z++) ;
#pragma nounroll
      for (int z = 0; *p1 < value ; z++ ) sched_yield();
      __pgasrt_smp_fence();
    }

    char * buf (int rank) const { return (char *) _cache[rank].v.buf; }

  private:
    CacheLine_t * _cache;
    int _thrds;
    int _logBF;
    int _nonBF;
    int _maxBF;
  };
}

/* ************************************************************************* */
/*                     constructor                                           */
/* ************************************************************************* */

#define CACHE(rank,phase) _cache[(rank)+(phase)*_thrds]


inline SMPColl::Allreduce::Allreduce (int thrds)
{
  assert (sizeof(CacheLine_t)==128);
  _thrds  = thrds;
  for (_logBF=0; (1<<(_logBF+1))<=thrds; _logBF++) ;
  _maxBF = (1<<_logBF);
  _nonBF = _thrds - _maxBF;
  _cache  = (CacheLine_t *)malloc(1+2* sizeof(CacheLine_t) * _thrds *(_logBF+2));
  assert (_cache != NULL);

  /* ------------------------- */
  /* reset counts & mailboxes  */
  /* ------------------------- */

  for (int rank=0; rank<_thrds; rank++)
    {
      _cache[rank].count = 0;
      for (int phase=0; phase<_logBF+2; phase++)
	_cache[rank+_thrds*phase].p = 0;
    }
}

/* ************************************************************************* */
/*                        execute                                            */
/* ************************************************************************* */

inline void SMPColl::Allreduce::execute (int                 rank,
					 void              * mybuf,
					 __pgasrt_ops_t      op,
					 __pgasrt_dtypes_t   dt,
					 int                 nelems)
{
  /* ------------------------------- */
  /*     get allreduce data          */
  /* ------------------------------- */

  TSPColl::Allreduce::cb_Allreduce_t cb;
  cb        = TSPColl::Allreduce::getcallback (op, dt);
  size_t dw = TSPColl::Allreduce::datawidthof (dt);

  /* ------------------------------- */
  /*     start operation             */
  /* ------------------------------- */

  _cache[rank].v.buf = (char *)mybuf;
  int opcount = ++_cache[rank].count;
  int n2 = nelems / 2;
  
  /* ------------------------------- */
  /* ------------------------------- */

  if (_nonBF > 0 && (rank >= _maxBF || rank < _nonBF))
    {
      int peer = (rank>=_maxBF) ? rank-_maxBF : rank+_maxBF;
      exchange (rank, peer, 0, opcount);
      if (rank < peer) cb (mybuf, buf(peer), nelems);
      exchange (rank, peer, 1, opcount);
    }
  
  /* ------------------------------- */
  /* ------------------------------- */

  if (rank < _maxBF)
    for (int l=0;l<_logBF;l++)
      {
	int peer = rank ^ (1<<l);
	exchange (rank, peer, l+l+2, opcount);
	if (rank < peer)
	  {
	    cb (mybuf, buf(peer), n2);
	    memcpy (buf(peer), mybuf, n2*dw);
	  }
	else
	  {
	    cb ((char *)mybuf + n2*dw, buf(peer) + n2*dw, (nelems-n2));
	    memcpy (buf(peer) + n2*dw, (char *)mybuf + n2*dw, (nelems-n2)*dw);
	  }
	exchange (rank, peer, l+l+3, opcount);
      }
  
  /* ------------------------------- */
  /* ------------------------------- */

  if (_nonBF > 0 && (rank >= _maxBF || rank < _nonBF))
    {
      int peer = (rank>=_maxBF) ? rank-_maxBF : rank+_maxBF;
      exchange (rank, peer, 2*_logBF+2, opcount);
      if (rank > peer) memcpy (mybuf, buf(peer), nelems * dw);
      exchange (rank, peer, 2*_logBF+3, opcount);
    }

  __pgasrt_smp_fence();
}

#endif
