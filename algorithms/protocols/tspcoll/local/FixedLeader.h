/* *********************************************************************** */
/* *********************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef __xlpgas_local_FixedLeader_h__
#define __xlpgas_local_FixedLeader_h__

#ifdef XLPGAS_PAMI_CAU

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

//#define SHM_BUF_SIZE 131072
//#define SHM_BUF_SIZE 262144
#define SHM_BUF_SIZE 524288

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
      FixedLeader(int N, int myindex, int leader, int nchildren) : _N(N), _me(myindex), _leader(leader), _nchildren(nchildren) {}
      void bcast (xlpgas_local_addr_t buffer, size_t len);
      void reduce (int64_t* val, int64_t* dest, const cau_reduce_op_t&);
      void reset(int leader);
      int root() const {return _leader;}

    public:
      struct State
      {
	unsigned char buffer [60];        /* 124 bytes */
	int32_t counter;                   /*   4 bytes */
      } 
      __attribute__((__aligned__(16)));
      
    protected:
      State*     _state;
      int        _me, _N, _nchildren;
      int        _children[128], _parent, _leader;
      static const int BusyWaitCycles=100000;

      void wait (int dest, int counter) const;
    public:
      bool haveParent   () const { return (_parent>=0); }
    }; /* FixedLeader */


    /* Fixed leader; one buffer; large messages pipelined*/
    template <class Wait>
    class FixedLeaderLB : public FixedLeader<Wait> {
      typedef FixedLeader<Wait> base;
      char* _large_buffer;
    public:
      FixedLeaderLB (int N, int myindex, int leader, void * shmem,int nc=32);
      void bcast (xlpgas_local_addr_t buffer, size_t len);
    };
    
    /******************************************************/
    /* Fixed leader; two buffers; large messages pipelined*/
    template <class Wait>
    class FixedLeader2LB : public FixedLeader<Wait> {
      typedef FixedLeader<Wait> base;
      char* _large_buffer0;
      char* _large_buffer1;
    public:
      FixedLeader2LB (int N, int myindex, int leader, void * shmem,int nc=32);
      void bcast (xlpgas_local_addr_t buffer, size_t len);
    };
    
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

template <class Wait> 
void inline xlpgas::local::FixedLeader<Wait>::reset (int leader)
{
  _leader = leader;

  //  printf("MME=%d LEADER=%d\n", _me, _leader); 
 
 int k = (_me - _leader + _N) % _N;  // my distance from leader, modulo N
  for (int c=0; c<_nchildren; c++)
    if (_nchildren*k+c+1<_N) _children[c] = (_nchildren*k+c+1+leader) % _N;
    else                    _children[c] = -1;
    
  /* parent: (k+1)/_nchildren from leader */
  if (k>0) _parent = ((k-1)/_nchildren + _leader) % _N; else _parent = -1;
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


template <class Wait> 
inline xlpgas::local::FixedLeaderLB<Wait>::
 FixedLeaderLB (int N, int me, int leader, void * shmem, int nchildren) : xlpgas::local::FixedLeader<Wait>(N,me,leader,nchildren){
  assert (nchildren <= 128);
  int k = (me-leader+N) % N;  // my distance from leader, modulo N
  for (int c=0; c<_nchildren; c++)
    if (_nchildren*k+c+1<N) _children[c] = (_nchildren*k+c+1+leader)%N;
    else                    _children[c] = -1;
    
  /* parent: (k+1)/_nchildren from leader */
  if (k>0) _parent = ((k-1)/_nchildren+leader)%N; else _parent = -1;

  _state = (base::State *) shmem;
  _large_buffer = ((char*)shmem + N * sizeof(base::State));
}

/* *********************************************************************** */
/*                     large message broadcast (blocking)                  */
/* *********************************************************************** */
template <class Wait>
void xlpgas::local::FixedLeaderLB<Wait>::
  bcast (xlpgas_local_addr_t buf, size_t len)
{
  /* ------------------------------------------------------------------ */
  /* wait until both children say they have data for previous iteration */
  /* ------------------------------------------------------------------ */
  for (int c=0; c<_nchildren; c++) base::wait (_children[c], _state[_me].counter);

  /* -------------------------------------------------- */
  /* wait for my parent to have data for this iteration */
  /* -------------------------------------------------- */
  base::wait (_parent, _state[_me].counter+1);

  /* ------------------------------------------------------- */
  /* copy data from my parent (or from input if I am leader) */
  /* ------------------------------------------------------- */
  __lwsync(); /* read barrier */
  if(! haveParent()) memcpy ((void *)(_large_buffer), (void *)buf, len);
  __lwsync(); /* write barrier */

  /* ------------------------------------------------------- */
  /*   children copy data to output buffer                            */
  /* ------------------------------------------------------- */
  __isync();
  if (haveParent()) memcpy (buf, (void *)(_large_buffer), len);

  /* ------------------------------------------------------- */
  /* put out notice that I have data for this iteration      */
  /* ------------------------------------------------------- */
  volatile int32_t * p = (volatile int32_t *) &_state[_me].counter;
  (*p) = (*p) + 1;
  __lwsync(); /* write barrier */
}

/* *********************************************************************** */
/*                     large message broadcast with two buffers (blocking)                  */
/* *********************************************************************** */
template <class Wait> 
inline xlpgas::local::FixedLeader2LB<Wait>::
 FixedLeader2LB (int N, int me, int leader, void * shmem, int nchildren) : xlpgas::local::FixedLeader<Wait>(N,me,leader,nchildren){
  assert (nchildren <= 128);
  int k = (me-leader+N) % N;  // my distance from leader, modulo N
  for (int c=0; c<_nchildren; c++)
    if (_nchildren*k+c+1<N) _children[c] = (_nchildren*k+c+1+leader)%N;
    else                    _children[c] = -1;
    
  /* parent: (k+1)/_nchildren from leader */
  if (k>0) _parent = ((k-1)/_nchildren+leader)%N; else _parent = -1;

  _state = (base::State *) shmem;
  _large_buffer0 = ((char*)shmem + N * sizeof(base::State));
  _large_buffer1 = ((char*)shmem + N * sizeof(base::State) + SHM_BUF_SIZE);
}

template <class Wait>
void xlpgas::local::FixedLeader2LB<Wait>::
  bcast (xlpgas_local_addr_t buf, size_t len)
{
  /* ------------------------------------------------------------------ */
  /* wait until both children say they have data for previous iteration */
  /* ------------------------------------------------------------------ */
  for (int c=0; c<_nchildren; c++) base::wait (_children[c], _state[_me].counter-1);

  /* -------------------------------------------------- */
  /* wait for my parent to have data for this iteration */
  /* -------------------------------------------------- */
  base::wait (_parent, _state[_me].counter+1);

  /* ------------------------------------------------------- */
  /* copy data from my parent (or from input if I am leader) */
  /* ------------------------------------------------------- */
  __lwsync(); /* read barrier */
  if(! haveParent()) {
    if((_state[_me].counter & 1) == 0 )
      memcpy ((void *)(_large_buffer0), (void *)buf, len);
    else
      memcpy ((void *)(_large_buffer1), (void *)buf, len);
  }
  __lwsync(); /* write barrier */

  /* ------------------------------------------------------- */
  /*   children copy data to output buffer                            */
  /* ------------------------------------------------------- */
  __isync();
  if (haveParent()) {
    if((_state[_me].counter & 1) == 0 )
      memcpy (buf, (void *)(_large_buffer0), len);
    else
      memcpy (buf, (void *)(_large_buffer1), len);
  }

  /* ------------------------------------------------------- */
  /* put out notice that I have data for this iteration      */
  /* ------------------------------------------------------- */
  volatile int32_t * p = (volatile int32_t *) &_state[_me].counter;
  (*p) = (*p) + 1;
  __lwsync(); /* write barrier */

}

#endif // XLPGAS_PAMI_CAU


#endif

