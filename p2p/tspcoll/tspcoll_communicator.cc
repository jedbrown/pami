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

#include "./Communicator.h"
#include "./Barrier.h"
#include "./Allgather.h"
#include "./Allgatherv.h"
#include "./Scatter.h"
//#include "./Gather.h"
#include "./BinomBcast.h"
#include "./ScBcast.h"
#include "./Allreduce.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#undef TRACE
//#define TRACE(x) fprintf x
#define TRACE(x)

/* ************************************************************************ */
/*                  communicator constructor                                */
/* ************************************************************************ */

TSPColl::Communicator::Communicator (int r, int s) : _rank (r), _size(s)
{
}

/* ************************************************************************ */
/*                create all collectives in a communicator                  */
/* ************************************************************************ */

void TSPColl::Communicator::setup()
{
  _barrier    = NBCollManager::instance()->allocate (this, BarrierTag);
  _allgather  = NBCollManager::instance()->allocate (this, AllgatherTag);
  _allgatherv = NBCollManager::instance()->allocate (this, AllgathervTag);
  _bcast      = NBCollManager::instance()->allocate (this, BcastTag);
  _bcast2     = NBCollManager::instance()->allocate (this, BcastTag2);
  _sar        = NBCollManager::instance()->allocate (this, ShortAllreduceTag);
  _lar        = NBCollManager::instance()->allocate (this, LongAllreduceTag);
  _sct        = NBCollManager::instance()->allocate (this, ScatterTag);
  _sctv       = NBCollManager::instance()->allocate (this, ScattervTag);
}

/* ************************************************************************ */
/*              wait for completion of non-blocking operation               */
/* ************************************************************************ */

void TSPColl::Communicator::nbwait (NBColl * c)
{
  if (!c) return;
  while (!c->isdone()) __pgasrt_tsp_wait (NULL);
  // __pgasrt_tsp_fence(0);
}

/* ************************************************************************ */
/*                 bruck exchange barrier implementation                    */
/* ************************************************************************ */

TSPColl::NBColl * TSPColl::Communicator::ibarrier (void (*cb_complete)(void *),
						   void *arg)
{
  if (!_barrier->isdone()) nbwait (_barrier);
  ((Barrier *)_barrier)->reset();
  _barrier->setComplete(cb_complete, arg);
  _barrier->kick();
  return _barrier;
}

void TSPColl::Communicator::barrier(void (*cb_complete)(void *),
				    void *arg)
{
    nbwait (ibarrier(cb_complete,arg));
}

/* ************************************************************************ */
/*                bruck algorithm allgather implementation                  */
/* ************************************************************************ */

TSPColl::NBColl * TSPColl::Communicator::
iallgather (const void * sbuf, void * rbuf, size_t nbytes,
	    void (*cb_complete)(void *), void *arg)
{
  if (!_allgather->isdone()) nbwait (_allgather);
  ((Allgather *)_allgather)->reset (sbuf, rbuf, nbytes);
  _allgather->setComplete(cb_complete, arg);
  _allgather->kick();
  return _allgather;
}

void TSPColl::Communicator::
allgather (const void * sbuf, void * rbuf, size_t nbytes,
	   void (*cb_complete)(void *), void *arg)
{
  nbwait (iallgather (sbuf, rbuf, nbytes,cb_complete,arg));
}

/* ************************************************************************ */
/*                bruck algorithm allgather implementation                  */
/* ************************************************************************ */

TSPColl::NBColl * TSPColl::Communicator::
iallgatherv (const void * sbuf, void * rbuf, size_t * lengths,
	     void (*cb_complete)(void *), void *arg)
{
  if (!_allgatherv->isdone()) nbwait (_allgatherv);
  ((Allgatherv *)_allgatherv)->reset (sbuf, rbuf, lengths);
  _allgatherv->setComplete(cb_complete, arg);
  _allgatherv->kick();
  return _allgatherv;
}

void TSPColl::Communicator::
allgatherv (const void * sbuf, void * rbuf, size_t * lengths,
	    void (*cb_complete)(void *), void *arg)
{
    nbwait (iallgatherv (sbuf, rbuf, lengths,cb_complete,arg));
}

/* ************************************************************************ */
/*                binomial broadcast implementation                         */
/* ************************************************************************ */

TSPColl::NBColl * TSPColl::Communicator::
ibcast (int root, const void * sbuf, void * rbuf, size_t length,
	void (*cb_complete)(void *), void *arg)
{
  if (getenv("NBCAST"))
    {
      if (!_bcast2->isdone()) nbwait (_bcast2);
      ((ScBcast *)_bcast2)->reset (root, sbuf, rbuf, length);
      _bcast2->setComplete(cb_complete, arg);
      _bcast2->kick();
      return _bcast2;
    }
  else
    {
      if (!_bcast->isdone()) nbwait (_bcast);
      ((BinomBcast *)_bcast)->reset (root, sbuf, rbuf, length);
      _bcast->setComplete(cb_complete, arg);
      _bcast->kick();
      return _bcast;
    }
}

void TSPColl::Communicator::
bcast (int root, const void * sbuf, void * rbuf, size_t length,
       void (*cb_complete)(void *), void *arg)
{
  nbwait (ibcast (root, sbuf, rbuf, length,cb_complete,arg));
}


/* ************************************************************************ */
/*                  butterfly broadcast                                     */
/* ************************************************************************ */

TSPColl::NBColl * TSPColl::Communicator::
iallreduce  (const void          * s,
	     void                * d,
	     __pgasrt_ops_t        op,
	     __pgasrt_dtypes_t     dtype,
	     unsigned              nelems,
	     void (*cb_complete)(void *),
	     void *arg)
{
  if (Allreduce::datawidthof(dtype) * nelems < Allreduce::Short::MAXBUF)
    {
      if (!_sar->isdone()) nbwait (_sar);
      ((Allreduce::Short *)_sar)->reset (s, d, op, dtype, nelems);
      _sar->setComplete(cb_complete, arg);
      _sar->kick();
      return _sar;
    }
    else
    {
      if (!_lar->isdone()) nbwait (_lar);
      ((Allreduce::Long *)_lar)->reset (s, d, op, dtype, nelems);
      _lar->setComplete(cb_complete, arg);
      _lar->kick();
      return _lar;
    }
}

void TSPColl::Communicator::
allreduce  (const void *s,
	    void * d,
	    __pgasrt_ops_t op,
	    __pgasrt_dtypes_t dtype,
	    unsigned nelems,
	    void (*cb_complete)(void *),
	    void *arg)
{
  nbwait (iallreduce (s, d, op, dtype, nelems,cb_complete,arg));
}

/* ************************************************************************ */
/*                simplistic scatter implementation                         */
/* ************************************************************************ */

TSPColl::NBColl * TSPColl::Communicator::
iscatter (int root, const void * sbuf, void * rbuf, size_t length,
	  void (*cb_complete)(void *), void *arg)
{
  if (!_sct->isdone()) nbwait (_sct);
  ((Scatter *)_sct)->reset (root, sbuf, rbuf, length);
  _sct->setComplete(cb_complete, arg);
  barrier();
  _sct->kick();
  return _sct;
}

void TSPColl::Communicator::
scatter (int r, const void * s, void * d, size_t l,
	 void (*cb_complete)(void *), void *arg)
{
  nbwait (iscatter (r, s, d, l,cb_complete,arg));
}

/* ************************************************************************ */
/*                simplistic scatter implementation                         */
/* ************************************************************************ */

TSPColl::NBColl * TSPColl::Communicator::
iscatterv (int root, const void * sbuf, void * rbuf, size_t * lengths,
	   void (*cb_complete)(void *), void *arg)
{
  if (!_sctv->isdone()) nbwait (_sctv);
  ((Scatterv *)_sctv)->reset (root, sbuf, rbuf, lengths);
  _sctv->setComplete(cb_complete, arg);
  barrier();
  _sctv->kick();
  return _sctv;
}

void TSPColl::Communicator::
scatterv (int root, const void * sbuf, void * rbuf, size_t * lengths,
	  void (*cb_complete)(void *), void *arg)
{
  nbwait (iscatterv (root, sbuf, rbuf, lengths, cb_complete,arg));
}

/* ************************************************************************ */
/*                simplistic scatter implementation                         */
/* ************************************************************************ */

void TSPColl::Communicator::
gather (int root, const void * sbuf, void * rbuf, size_t length)
{
#if 0
  Gather * g = (Gather *) TagList::find (this->id(), GatherTag);
  g->reset (root, sbuf, rbuf, length);
  barrier();
  g->kick(); while (!g->isdone()) __pgasrt_tsp_wait (NULL);
  __pgasrt_tsp_fence(0);
#endif
}

/* ************************************************************************ */
/*                simplistic scatter implementation                         */
/* ************************************************************************ */

void TSPColl::Communicator::
gatherv (int root, const void * sbuf, void * rbuf, size_t * lengths)
{

}

/* ************************************************************************ */
/*                   enumerated communicator constructor                    */
/* ************************************************************************ */

TSPColl::EnumComm::EnumComm (int rank, int size, int proclist[]) :
  Communicator (rank, size)
{
  assert (size > 0);
  _proclist = (int *) malloc(sizeof(int)*size);
  if (!_proclist) __pgasrt_fatalerror(-1, "Allocation error");
  memcpy (_proclist, proclist, size * sizeof(int));
}

/* ************************************************************************ */
/*                   ranged communicator constructor                        */
/* ************************************************************************ */

TSPColl::RangedComm::RangedComm (int rank, int numranges, Range rangelist[]):
    Communicator (rank, -1)
{
    assert(numranges > 0);
    _rangelist = (Range*)malloc(sizeof(Range)*numranges);
    if(!_rangelist)__pgasrt_fatalerror(-1, "Allocation error");
    memcpy (_rangelist, rangelist, numranges*sizeof(Range));
    _numranges = numranges;

    /*  Calculate the size */
    _size = 0;
    for(int i=0; i<numranges; i++)
	{
	    assert(_rangelist[i]._hi-rangelist[i]._lo > 0);
	    _size+=(_rangelist[i]._hi-rangelist[i]._lo+1);
	}
    assert (_size > 0);
}

int TSPColl::RangedComm::absrankof (int rank) const
{
    int rankLeft=rank;
    for(int i=0; i<_numranges; i++)
	{
	    int rangeSz  = _rangelist[i]._hi-_rangelist[i]._lo+1;
	    rankLeft    -= rangeSz;
	    if(rankLeft <= 0)
		{
		    int offset = rangeSz+rankLeft;
		    return _rangelist[i]._lo+offset;
		}
	}
    return -1;
}

int TSPColl::RangedComm::virtrankof (int rank) const
{
    int sz=0;
    for(int i=0; i<_numranges; i++)
        {
	    if(rank>=_rangelist[i]._lo && rank<=_rangelist[i]._hi)
		return sz + (rank - _rangelist[i]._lo);
	    sz+=(_rangelist[i]._hi-_rangelist[i]._lo+1);
        }
   assert(0);
}



/* ************************************************************************ */
/* ************************************************************************ */

#define COURSEOF(node) ((node)/(_BF*_ncomms))
#define COMMOF(node) (((node)/_BF)%_ncomms)
#define VIRTOF(node) (((node)%_BF)+(_BF*COURSEOF(node)))


TSPColl::BC_Comm::BC_Comm (int BF, int ncomms):
  Communicator(), _BF(BF), _ncomms(ncomms)
{
  _rank         = VIRTOF(PGASRT_MYNODE);
  _mycomm       = COMMOF(PGASRT_MYNODE);
  int c = COMMOF(PGASRT_NODES);
  if (c < _mycomm)
    {
      _size = _BF * (COURSEOF(PGASRT_NODES));
    }
  else if (c == _mycomm)
    {
      _size = _BF * COURSEOF(PGASRT_NODES) + (PGASRT_NODES%_BF);
    }
  else
    {
      _size = _BF * (COURSEOF(PGASRT_NODES)+1);
    }
}

/* ************************************************************************ */
/*    absolute rank corresponding to virtual rank in *my* communicator      */
/* ************************************************************************ */
/*  rank/BF == block corresponding to rank                                  */
/*  (rank/BF) * BF * ncomms = course                                        */
/* ************************************************************************ */

int TSPColl::BC_Comm::absrankof (int rank) const
{

  return
    (rank/_BF)*_BF*_ncomms +    /* current course of rank               */
    _mycomm * _BF +             /* block in course of *my* communicator */
    (rank%_BF);                 /* rank's phase in block                */
}

/* ************************************************************************ */
/*    virtual rank of a particular absolute rank                            */
/* ************************************************************************ */

int TSPColl::BC_Comm::virtrankof (int rank) const
{
  if (COMMOF(rank)==_mycomm) return VIRTOF(rank);
  return -1;
}




