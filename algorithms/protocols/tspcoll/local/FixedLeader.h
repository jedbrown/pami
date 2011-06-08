/* *********************************************************************** */
/* *********************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <builtins.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef __xlpgas_local_FixedLeader_h__
#define __xlpgas_local_FixedLeader_h__

#include "../cau_collectives.h" //for ops on one element 

#undef TRACE
//#define TRACE_FL
#ifdef TRACE_FL
#define TRACE(x) fprintf x
#else
#define TRACE(x)
#endif

/* *********************************************************************** */
/*           Fixed-leader broadcast and reduction                          */
/* *********************************************************************** */
/* Wait is a class with a function called wait() that progresses the network */
/* T is a type with a size of <= 8 bytes */
/* Op is a class with an operator() that performs the reduction */

namespace xlpgas
{
  namespace local
  {
    template <class Wait> 
    class FixedLeader
    {
    public:
      FixedLeader (int N, int myindex, int leader, void * shmem
,  int nc=2);
      void bcast (xlpgas_local_addr_t buffer, size_t len);
      void reduce (int64_t* val, int64_t* dest, const cau_reduce_op_t&);
      
    public:
      struct State
      {
	unsigned char buffer [252];        /* 124 bytes */
	int32_t counter;                   /*   4 bytes */
      } 
      __attribute__((__aligned__(16)));
      
    private:
      State    * _state;
      int        _me, _N, _nchildren;
      int        _children[16], _parent;
      static const int BusyWaitCycles=100000;

    private:
      void wait (int dest, int counter) const;
      bool haveParent   () const { return (_parent>=0); }
    }; /* FixedLeader */
  } /* local */
} /* xlpgas */


/* *********************************************************************** */
/*                   allreduce constructor                                 */
/* *********************************************************************** */

template <class Wait> 
inline xlpgas::local::FixedLeader<Wait>::
FixedLeader (int N, int me, int leader, void * shmem, int nchildren): 
_N(N), _me(me), _nchildren(nchildren)
{
  assert (nchildren <= 16);
  int k = (me-leader+N) %N;  // my distance from leader, modulo N

  for (int c=0; c<_nchildren; c++)
    if (_nchildren*k+c+1<N) _children[c] = (_nchildren*k+c+1+leader)%N;
    else                    _children[c] = -1;
    
  /* parent: (k+1)/_nchildren from leader */
  if (k>0) _parent = ((k-1)/_nchildren+leader)%N; else _parent = -1;

  _state = (State *) shmem;

  //memory is set to zero when allocated; reset to zero is a bug
  // when sub teams are created; they will wipe out whatever was here;
  //subteams are guarded by another if and they won't call hybrid collectives
  //memset ((void *)(_state + me), 0, sizeof(State));

#if 0
  fprintf (stderr, "%d: PID=%d k=%d parent=%d\n", me, getpid(), k, _parent);
#endif

}

/* *********************************************************************** */
/*                     reduce (blocking)                                   */
/* *********************************************************************** */
template <class Wait>
void xlpgas::local::FixedLeader<Wait>::reduce (int64_t* val, 
					       int64_t* dest, 
					       const cau_reduce_op_t& op)
{
  /* ------------------------------------------------------------- */
  /* wait for children to have data                                */
  /* perform operations using children's values                    */
  /* ------------------------------------------------------------- */
  for (int c=0; c<_nchildren; c++) 
    {
      wait (_children[c], _state[_me].counter+1);
      if (_children[c]>=0)
	reduce_op(val, (int64_t *) _state[_children[c]].buffer, op);
    }

  /* ------------------------------------------------------------- */
  /* wait for parent to have read my *previous* posted value.      */
  /* and then post my new result                                   */
  /* ------------------------------------------------------------- */
  
  wait (_parent, _state[_me].counter);
  * (volatile int64_t *) _state[_me].buffer = *val;
  
  /* ------------------------------------------------------- */
  /* put out notice that I have data for this iteration      */
  /* ------------------------------------------------------- */

  __lwsync(); /* write barrier */
  volatile int32_t * p = (volatile int32_t *) &_state[_me].counter;
  (*p) = (*p)+1;
  __lwsync(); /* write barrier */

  *dest = *val;
}


/* *********************************************************************** */
/*                     broadcast (blocking)                                */
/* *********************************************************************** */
template <class Wait>
  void xlpgas::local::FixedLeader<Wait>::
  bcast (xlpgas_local_addr_t buf, size_t len)
{
  /* ------------------------------------------------------------------ */
  /* wait until both children say they have data for previous iteration */
  /* ------------------------------------------------------------------ */
  for (int c=0; c<_nchildren; c++) wait (_children[c], _state[_me].counter);

  /* -------------------------------------------------- */
  /* wait for my parent to have data for this iteration */
  /* -------------------------------------------------- */
  wait (_parent, _state[_me].counter+1);

  /* ------------------------------------------------------- */
  /* copy data from my parent (or from input if I am leader) */
  /* ------------------------------------------------------- */
  __lwsync(); /* read barrier */
  memcpy ((void *)(&_state[_me].buffer), 
	  haveParent()?(void *)_state[_parent].buffer:buf, len);
  __lwsync(); /* write barrier */

  /* ------------------------------------------------------- */
  /* put out notice that I have data for this iteration      */
  /* ------------------------------------------------------- */
  volatile int32_t * p = (volatile int32_t *) &_state[_me].counter;
  (*p) = (*p) + 1;
  __lwsync(); /* write barrier */

  /* ------------------------------------------------------- */
  /*   copy data to output buffer                            */
  /* ------------------------------------------------------- */
  if (haveParent()) memcpy (buf, (void *)(&_state[_me].buffer), len);
}

/* *********************************************************************** */
/*              waiting for announcement                                   */
/* *********************************************************************** */

template <class Wait> inline void
  xlpgas::local::FixedLeader<Wait>::
  wait (int dest, int cntr) const
{
  if (dest<0) return;
  volatile int * p1 = & _state[dest].counter;
#pragma nounroll
  for (int z = 0; z<BusyWaitCycles && *p1<cntr ; z++) ;
#pragma nounroll
  for (int z = 0; z<BusyWaitCycles && *p1<cntr ; z++) Wait::wait1();
#pragma nounroll
  for (; *p1<cntr ; ) Wait::wait2();
}


#endif

