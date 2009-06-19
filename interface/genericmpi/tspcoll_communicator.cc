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

#include "collectives/interface/Communicator.h"
#include "collectives/interface/genericmpi/GenericComm.h"
#include "collectives/interface/genericmpi/Adaptor.h"
#include "collectives/algorithms/protocols/tspcoll/Barrier.h"
#include "collectives/algorithms/protocols/tspcoll/Allgather.h"
#include "collectives/algorithms/protocols/tspcoll/Allgatherv.h"
#include "collectives/algorithms/protocols/tspcoll/Scatter.h"
#include "collectives/algorithms/protocols/tspcoll/BinomBcast.h"
#include "collectives/algorithms/protocols/tspcoll/ScBcast.h"
#include "collectives/algorithms/protocols/tspcoll/Allreduce.h"

//#include "collectives/algorithms/protocols/tspcoll/Gather.h"


#include <assert.h>
#include <stdlib.h>
#include <string.h>

#undef TRACE
//#define TRACE(x) fprintf x
#define TRACE(x)

extern CCMI::Adaptor::Adaptor  * _g_generic_adaptor;


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
  while (!c->isdone()) _g_generic_adaptor->advance();
}

/* ************************************************************************ */
/*                 bruck exchange barrier implementation                    */
/* ************************************************************************ */

TSPColl::NBColl * TSPColl::Communicator::ibarrier (CCMI::MultiSend::MulticastInterface *mcast_iface,
						   void (*cb_complete)(void *),
						   void *arg)
{
  if (!_barrier->isdone()) nbwait (_barrier);
  ((Barrier *)_barrier)->reset();
  _barrier->setComplete(cb_complete, arg);
  _barrier->kick(mcast_iface);
  return _barrier;
}

void TSPColl::Communicator::barrier(CCMI::MultiSend::MulticastInterface *mcast_iface,
				    void (*cb_complete)(void *),
				    void *arg)
{
  nbwait (ibarrier(mcast_iface,cb_complete,arg));
}

/* ************************************************************************ */
/*                bruck algorithm allgather implementation                  */
/* ************************************************************************ */

TSPColl::NBColl * TSPColl::Communicator::
iallgather (CCMI::MultiSend::MulticastInterface *mcast_iface,
	    const void * sbuf, void * rbuf, size_t nbytes,
	    void (*cb_complete)(void *), void *arg)
{
  if (!_allgather->isdone()) nbwait (_allgather);
  ((Allgather *)_allgather)->reset (sbuf, rbuf, nbytes);
  _allgather->setComplete(cb_complete, arg);
  _allgather->kick(mcast_iface);
  return _allgather;
}

void TSPColl::Communicator::
allgather (CCMI::MultiSend::MulticastInterface *mcast_iface,
	   const void * sbuf, void * rbuf, size_t nbytes,
	   void (*cb_complete)(void *), void *arg)
{
  nbwait (iallgather (mcast_iface,sbuf, rbuf, nbytes,cb_complete,arg));
}

/* ************************************************************************ */
/*                bruck algorithm allgather implementation                  */
/* ************************************************************************ */

TSPColl::NBColl * TSPColl::Communicator::
iallgatherv (CCMI::MultiSend::MulticastInterface *mcast_iface,
	     const void * sbuf, void * rbuf, size_t * lengths,
	     void (*cb_complete)(void *), void *arg)
{
  if (!_allgatherv->isdone()) nbwait (_allgatherv);
  ((Allgatherv *)_allgatherv)->reset (sbuf, rbuf, lengths);
  _allgatherv->setComplete(cb_complete, arg);
  _allgatherv->kick(mcast_iface);
  return _allgatherv;
}

void TSPColl::Communicator::
allgatherv (CCMI::MultiSend::MulticastInterface *mcast_iface,
	    const void * sbuf, void * rbuf, size_t * lengths,
	    void (*cb_complete)(void *), void *arg)
{
  nbwait (iallgatherv (mcast_iface,sbuf, rbuf, lengths,cb_complete,arg));
}

/* ************************************************************************ */
/*                binomial broadcast implementation                         */
/* ************************************************************************ */

TSPColl::NBColl * TSPColl::Communicator::
ibcast (CCMI::MultiSend::MulticastInterface *mcast_iface,
	int root, const void * sbuf, void * rbuf, size_t length,
	void (*cb_complete)(void *), void *arg)
{
  if (getenv("NBCAST"))
    {
      if (!_bcast2->isdone()) nbwait (_bcast2);
      ((ScBcast *)_bcast2)->reset (root, sbuf, rbuf, length);
      _bcast2->setComplete(cb_complete, arg);
      _bcast2->kick(mcast_iface);
      return _bcast2;
    }
  else
    {
      if (!_bcast->isdone()) nbwait (_bcast);
      ((BinomBcast *)_bcast)->reset (root, sbuf, rbuf, length);
      _bcast->setComplete(cb_complete, arg);
      _bcast->kick(mcast_iface);
      return _bcast;
    }
}

void TSPColl::Communicator::
bcast (CCMI::MultiSend::MulticastInterface *mcast_iface,
       int root, const void * sbuf, void * rbuf, size_t length,
       void (*cb_complete)(void *), void *arg)
{
  nbwait (ibcast (mcast_iface,root, sbuf, rbuf, length,cb_complete,arg));
}


/* ************************************************************************ */
/*                  butterfly broadcast                                     */
/* ************************************************************************ */
namespace CCMI { namespace Adaptor { namespace Allreduce {
      extern void getReduceFunction(CCMI_Dt, CCMI_Op, unsigned, 
				    unsigned&, CCMI_ReduceFunc&);
    }}};
TSPColl::NBColl * TSPColl::Communicator::
iallreduce  (CCMI::MultiSend::MulticastInterface *mcast_iface,
	     const void          * s,
	     void                * d,
	     CCMI_Op               op,
	     CCMI_Dt               dtype,
	     unsigned              nelems,
	     void (*cb_complete)(void *),
	     void *arg)
{
  unsigned        datawidth;
  CCMI_ReduceFunc cb_allreduce;
  CCMI::Adaptor::Allreduce::getReduceFunction(dtype, op, nelems, datawidth, cb_allreduce);
  //  if (Allreduce::datawidthof(dtype) * nelems < Allreduce::Short::MAXBUF)
  if (datawidth * nelems < Allreduce::Short::MAXBUF)
    {
      if (!_sar->isdone()) nbwait (_sar);
      ((Allreduce::Short *)_sar)->reset (s, d, op, dtype, nelems);
      _sar->setComplete(cb_complete, arg);
      _sar->kick(mcast_iface);
      return _sar;
    }
    else
    {
      if (!_lar->isdone()) nbwait (_lar);
      ((Allreduce::Long *)_lar)->reset (s, d, op, dtype, nelems);
      _lar->setComplete(cb_complete, arg);
      _lar->kick(mcast_iface);
      return _lar;
    }
}

void TSPColl::Communicator::
allreduce  (CCMI::MultiSend::MulticastInterface *mcast_iface,
	    const void *s,
	    void * d,
	    CCMI_Op op,
	    CCMI_Dt dtype,
	    unsigned nelems,
	    void (*cb_complete)(void *),
	    void *arg)
{
  nbwait (iallreduce (mcast_iface,s, d, op, dtype, nelems,cb_complete,arg));
}

/* ************************************************************************ */
/*                simplistic scatter implementation                         */
/* ************************************************************************ */

TSPColl::NBColl * TSPColl::Communicator::
iscatter (CCMI::MultiSend::MulticastInterface *barrier_iface,
	  CCMI::MultiSend::MulticastInterface *scatter_iface,
	  int root, const void * sbuf, void * rbuf, size_t length,
	  void (*cb_complete)(void *), void *arg)
{
  if (!_sct->isdone()) nbwait (_sct);
  ((Scatter *)_sct)->reset (root, sbuf, rbuf, length);
  _sct->setComplete(cb_complete, arg);
  barrier(barrier_iface);
  _sct->kick(scatter_iface);
  return _sct;
}

void TSPColl::Communicator::
scatter (CCMI::MultiSend::MulticastInterface *barrier_iface,
	 CCMI::MultiSend::MulticastInterface *scatter_iface,

	 int r, const void * s, void * d, size_t l,
	 void (*cb_complete)(void *), void *arg)
{
  nbwait (iscatter (barrier_iface, scatter_iface,r, s, d, l,cb_complete,arg));
}

/* ************************************************************************ */
/*                simplistic scatter implementation                         */
/* ************************************************************************ */

TSPColl::NBColl * TSPColl::Communicator::
iscatterv (CCMI::MultiSend::MulticastInterface *barrier_iface,
	   CCMI::MultiSend::MulticastInterface *scatterv_iface,
	   int root, const void * sbuf, void * rbuf, size_t * lengths,
	   void (*cb_complete)(void *), void *arg)
{
  if (!_sctv->isdone()) nbwait (_sctv);
  ((Scatterv *)_sctv)->reset (root, sbuf, rbuf, lengths);
  _sctv->setComplete(cb_complete, arg);
  barrier(barrier_iface);
  _sctv->kick(scatterv_iface);
  return _sctv;
}

void TSPColl::Communicator::
scatterv (CCMI::MultiSend::MulticastInterface *barrier_iface,
	  CCMI::MultiSend::MulticastInterface *scatterv_iface,
	  int root, const void * sbuf, void * rbuf, size_t * lengths,
	  void (*cb_complete)(void *), void *arg)
{
  nbwait (iscatterv (barrier_iface,scatterv_iface,root, sbuf, rbuf, lengths, cb_complete,arg));
}

/* ************************************************************************ */
/*                simplistic scatter implementation                         */
/* ************************************************************************ */

void TSPColl::Communicator::
gather (CCMI::MultiSend::MulticastInterface *mcast_iface,
	int root, const void * sbuf, void * rbuf, size_t length)
{
#if 0
  Gather * g = (Gather *) TagList::find (this->id(), GatherTag);
  g->reset (root, sbuf, rbuf, length);
  barrier(mcast_iface);
  g->kick(mcast_iface); while (!g->isdone()) __pgasrt_tsp_wait (NULL);
  __pgasrt_tsp_fence(0);
#endif
}

/* ************************************************************************ */
/*                simplistic scatter implementation                         */
/* ************************************************************************ */

void TSPColl::Communicator::
gatherv (CCMI::MultiSend::MulticastInterface *mcast_iface,
	 int root, const void * sbuf, void * rbuf, size_t * lengths)
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
  if (!_proclist) CCMI_FATALERROR(-1, "Allocation error");
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
    if(!_rangelist)CCMI_FATALERROR(-1, "Allocation error");
    memcpy (_rangelist, rangelist, numranges*sizeof(Range));
    _numranges = numranges;

    /*  Calculate the size */
    this->_size = 0;
    for(int i=0; i<numranges; i++)
	{
	  //	    assert(_rangelist[i]._hi-rangelist[i]._lo > 0);
	    this->_size+=(_rangelist[i]._hi-rangelist[i]._lo+1);
	}
    assert (this->_size > 0);
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



