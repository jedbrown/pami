/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* This is an automatically generated copyright prolog.             */
/* After initializing,  DO NOT MODIFY OR MOVE                       */
/*  --------------------------------------------------------------- */
/* Licensed Materials - Property of IBM                             */
/* Blue Gene/Q 5765-PER 5765-PRP                                    */
/*                                                                  */
/* (C) Copyright IBM Corp. 2011, 2012 All Rights Reserved           */
/* US Government Users Restricted Rights -                          */
/* Use, duplication, or disclosure restricted                       */
/* by GSA ADP Schedule Contract with IBM Corp.                      */
/*                                                                  */
/*  --------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef __pipelined_allreduce_executor_h__
#define __pipelined_allreduce_executor_h__

#include <pami.h>
#include "algorithms/interfaces/Schedule.h"
#include "algorithms/interfaces/Executor.h"
#include "algorithms/connmgr/ConnectionManager.h"
#include "algorithms/interfaces/NativeInterface.h"
#include "algorithms/executor/ScheduleCache.h"
#include "algorithms/executor/AllreduceCache.h"
#include "math_optimath.h"
#include "math/math_coremath.h"

#include "util/ccmi_debug.h"
#include "util/ccmi_util.h"

#include "util/trace.h"

#ifdef CCMI_TRACE_ALL
#define DO_TRACE_ENTEREXIT 1
#define DO_TRACE_DEBUG     1
#else
#define DO_TRACE_ENTEREXIT 0
#define DO_TRACE_DEBUG     0
#endif

//////////////////////////////////////////////////////////////////////
/// Pipelined Allreduce Template                                   ///
/// To keep a simple design we only support reduce+bcast flavor of ///
/// allreduce. Currently we also only support one source per phase ///
//////////////////////////////////////////////////////////////////////

namespace CCMI
{
  namespace Executor
  {
    template <class T_Conn>
      class PipelinedAllreduce : public AllreduceBaseExec<T_Conn, true>    
    {
    private:

      /// \brief Static function to be passed into the done of multisend send
      static void staticPipeNotifySendDone (pami_context_t context, void *cd, pami_result_t err)
      {
	TRACE_FN_ENTER();
	//fprintf(stderr, "Send callback calling process done\n");
	((PipelinedAllreduce<T_Conn> *)(cd))->processDone();
	TRACE_FN_EXIT();
      }

      /// \brief Static function to be passed into the done of multisend send
      static void staticPipeNotifyRecvDone (pami_context_t context, void *cd, pami_result_t err)
      {
	TRACE_FN_ENTER();
	//fprintf(stderr, "Recv callback calling process done\n");
	((PipelinedAllreduce<T_Conn> *)(cd))->processDone();
	TRACE_FN_EXIT();
      }

      ///\brief Static function to be passed into the pwq user info
      static void staticPipeNotifyRecv (pami_context_t context, void *cd, pami_result_t err)
      {
	TRACE_FN_ENTER();
	//fprintf(stderr, "staticPipeNotifyRecv\n");
	((PipelinedAllreduce<T_Conn> *)cd)->advance();
	TRACE_FN_EXIT();
      }

    protected:  

      void processDone ()
      {
        _donecount --;	
	//fprintf(stderr, "In process done %d\n", _donecount);		
        if(_donecount <= 0 && this->_cb_done)
        {
	  //fprintf(stderr, "All Done for color %d\n", this->_acache.getColor());
	  this->_isSendDone = false; //Process an early arrival packet
	  this->_initialized = false; //Call application done callback
          this->_cb_done (this->_context, this->_clientdata, PAMI_SUCCESS); 
        }
      }

      //static const unsigned _pipelineWidth = 1024;

      //Squeeze local state in one cache line
      unsigned         _curRecvIdx;
      unsigned         _curRecvChunk;
      unsigned         _donecount;
      unsigned         _firstRecvPhase;   
      unsigned         _nSums;
      PAMI::PipeWorkQueue *_dstpwq;
      coremath             _reduceFunc4;

    public: 

      /// Default Constructor
      PipelinedAllreduce<T_Conn>() : AllreduceBaseExec<T_Conn, true>(),
	_curRecvIdx ((unsigned) -1), _curRecvChunk((unsigned) -1),
	_firstRecvPhase((unsigned) -1)
      {
        _donecount = 0;
        this->_initialized = false;
        this->_postReceives = false;
      }

      PipelinedAllreduce<T_Conn>(Interfaces::NativeInterface    * native,
				 T_Conn                         * connmgr,
				 const unsigned                   commID):
	AllreduceBaseExec<T_Conn, true>(native, connmgr, commID, true),
      _curRecvIdx ((unsigned) -1), _curRecvChunk((unsigned) -1), 
      _donecount (0),_firstRecvPhase((unsigned) -1)
      {
	//fprintf(stderr, "PipelinedAllreduce Constructor\n");
	this->_postReceives = true;
	this->_enablePipelining = true;
      }

      virtual ~PipelinedAllreduce<T_Conn> () 
      {
	TRACE_FN_ENTER();
	//fprintf(stderr, "Pipelined Allreduce destructor\n");
	TRACE_FN_EXIT();
      }

      pami_result_t advance (); // __attribute__((noinline));

      /// start allreduce
      virtual void start();
	
      virtual void reset ()
      {
        AllreduceBaseExec<T_Conn, true>::reset ();

        this->_initialized = false; 
        _curRecvIdx   = 0;
        _curRecvChunk = 0;    
	
	unsigned start_phase = _firstRecvPhase = this->_scache.getStartPhase();

	while (_firstRecvPhase <= this->_endPhase &&
               !this->_scache.getNumSrcRanks(_firstRecvPhase))
	  _firstRecvPhase++;
	
	//Reduce might not have a combine phase (on non-roots)
	if ((this->_scache.getRoot() != -1) && 
	    (_firstRecvPhase > this->_endPhase))
	  _firstRecvPhase = this->_scache.getStartPhase();
	//printf("first %u and end %u\n", _firstRecvPhase,this->_endPhase);
	CCMI_assert (_firstRecvPhase <= this->_endPhase);
	
	_donecount = 0;
	if (this->_scache.getNumTotalSrcRanks() == 0 
	    || _firstRecvPhase > this->_lastReducePhase)
	  _curRecvChunk = this->_acache.getLastChunk() + 1;
	
	unsigned p = start_phase;
	_nSums = 1; //local sum
	while (p <= this->_lastReducePhase) {
	  if (this->_scache.getNumSrcRanks(p) > 0)
	    _nSums ++;
	  p ++;
	}
	_dstpwq = NULL;

	coremath  reduce_func = MATH_OP_FUNCS(this->_acache.getDt(), this->_acache.getOp(), 2);
	//Update with optimized math function if its available
	if (reduce_func != NULL)
	  this->_reduceFunc = reduce_func; 
	
	reduce_func = MATH_OP_FUNCS(this->_acache.getDt(), this->_acache.getOp(), 4);
	//Update with optimized math function if its available
	if (reduce_func != NULL)
	  _reduceFunc4 = reduce_func; 
      }

      ///
      /// \brief compute an n-way sum
      /// \param [inout] summed buffer
      /// \param [in]    App src buf
      /// \param [in]    vector of network buffers
      /// \param [in]    Offset from start
      /// \param [in]    sum count
      /// \param [in]    network sums
      ///
      void nway_sum           (char          * dstbuf, 
			       const char    * srcbuf, 
			       char         ** buflist,
			       size_t          bufoffset,  
			       size_t          count, 
			       unsigned        nsum) 
      {
	CCMI_assert (this->_reduceFunc != NULL);
	char *buf[4]; //we use max 4-way sums
	char *dst = dstbuf + bufoffset;	
	unsigned start = 0;

	buf[0] = (char *)srcbuf + bufoffset;
	if (nsum >= 4 && _reduceFunc4 != NULL) {
	  buf[1] = buflist[0] + bufoffset;
	  buf[2] = buflist[1] + bufoffset;
	  buf[3] = buflist[2] + bufoffset;
	  _reduceFunc4(dst, (void**)buf, 4, count);
	  start = 3;
	}
	else {
	  buf[1] = buflist[0] + bufoffset;
	  this->_reduceFunc(dst, (void**)buf, 2, count);
	  start = 1;
	}

	buf[0] = dst;
	for (unsigned i = start; i < nsum-1; ++i) {
	  buf[1] = buflist[i] + bufoffset;
	  this->_reduceFunc(dst, (void**)buf, 2, count);
	}
      }

      void postReceives ();
      
      void sendMessage (const char             * buf,
			unsigned                 bytes,
			PAMI::Topology          * dst_topology,
			unsigned                 sphase);
      
    };  //-- PipelinedAllreduce
  };  //-- Executor
};  //-- CCMI

template <class T_Conn>
inline void CCMI::Executor::PipelinedAllreduce<T_Conn>::start()
{
  TRACE_FN_ENTER();

  this->_initialized = true; 
  char * reducebuf = (char*)this->_srcbuf;
  //Do we have a sum operation
  if (_firstRecvPhase <= this->_lastReducePhase) {
    reducebuf = this->_reducebuf;
  }
  
  //O(Phase) loop may lead to startup overheads !!
  for (unsigned p = this->_scache.getStartPhase(); p <= this->_scache.getEndPhase(); ++p) {
    unsigned ndstranks = this->_scache.getNumDstRanks (p);

    if (ndstranks > 0)
    {
      if (_dstpwq == NULL) { //Find the first dest pwq
	if (p > this->_lastReducePhase)
	  _dstpwq = this->_acache.getBcastPipeWorkQueue();
	else
	  _dstpwq = this->_acache.getDestPipeWorkQueue();
      }

      if (p > this->_lastReducePhase)
	reducebuf = this->_reducebuf;
      
      _donecount ++;
      PAMI::Topology *dst_topology   = this->_scache.getDstTopology(p);

      //      printf("phase %d, ndstranks %d dstrank[0] %d\n", p, ndstranks,
      //     dst_topology->index2Endpoint(0));    
      sendMessage (reducebuf, this->_acache.getBytes(), dst_topology, p);
    }
  }
  
  //Process the notify recvs we got before start
  if(_curRecvChunk <= this->_acache.getLastChunk())
    advance();  
  //if not root send my src buf
  else if (this->_native->endpoint() != (size_t)this->_scache.getRoot())
  {
    CCMI_assert (_dstpwq != NULL);
    _dstpwq->produceBytes (this->_acache.getBytes());
  }

  //fprintf(stderr, "After PipelinedAllreduce::start()\n");
  TRACE_FN_EXIT();
}


///
///  \brief Send the next message by calling the msend interface
///
template <class T_Conn>
inline void CCMI::Executor::PipelinedAllreduce<T_Conn>::sendMessage
(const char                               * buf,
 unsigned                                   bytes,
 PAMI::Topology                           * dst_topology,
 unsigned                                   sphase)
{
  TRACE_FN_ENTER();
  //Request buffer and callback set in setSendState !!
  CCMI_assert (dst_topology->size() > 0);

#if 0
  T_Conn *connmgr = this->_rconnmgr;
  if (sphase > this->_lastReducePhase) {
    connmgr = this->_bconnmgr;          
    this->_msend.connection_id = connmgr->getConnectionId
      ( this->_acache.getCommID(), 
	this->_scache.getRoot(), 
	this->_acache.getColor(),
	sphase,
	(unsigned) -1 //Notify bcast
	);
  }
  else {
    //Axial topology skip self rank
    pami_task_t dst_id = dst_topology->index2Endpoint(0);
    if (dst_id == this->_native->endpoint() && 
	dst_topology->type() == PAMI_AXIAL_TOPOLOGY)
      dst_id = 	dst_topology->index2Endpoint(1);        
    CCMI_assert(dst_id != this->_native->endpoint());

    this->_msend.connection_id = connmgr->getConnectionId
      ( this->_acache.getCommID(), 
	this->_scache.getRoot(), 
	this->_acache.getColor(),
	sphase,
	dst_id);    
  }
#else  
  this->_msend.connection_id = this->_acache.getPhaseSendConnectionId(sphase);
#endif

  this->_msend.bytes         = bytes;

  PAMI::PipeWorkQueue *pwq = NULL;  
  if (sphase > this->_lastReducePhase)
    pwq = this->_acache.getBcastPipeWorkQueue();
  else
    pwq = this->_acache.getDestPipeWorkQueue();
  
  pwq->configure((char *)buf, bytes, 0);

  //if (this->_native->endpoint() == 0)
  //printf ("send pwq %p buf %p phase %d connid %d color %d\n", pwq, buf, sphase, this->_msend.connection_id, this->_acache.getColor());
  
  this->_msend.src           = (pami_pipeworkqueue_t *) pwq;
  this->_msend.dst           = NULL;
  this->_msend.src_participants = (pami_topology_t *) & this->_selftopology;
  this->_msend.dst_participants = (pami_topology_t *) this->_scache.getDstTopology(sphase);

  this->_msend.cb_done.function =  staticPipeNotifySendDone;
  this->_msend.cb_done.clientdata = this;  

#if 0
  PAMI::Topology *topo = this->_scache.getDstTopology(sphase);
  if (topo->type() != PAMI_AXIAL_TOPOLOGY)
    printf ("Phase %d Sending to %d, lrp %d conn %d\n", sphase, 
	    (int)topo->index2Endpoint(0), this->_lastReducePhase,
	    this->_msend.connection_id);
  else
    printf ("Axial Topo Phase %d Sending to %d ranks lrp %d conn %d\n", 
	    sphase, (int)topo->size(), this->_lastReducePhase,
	    this->_msend.connection_id);    
#endif

  this->_native->multicast (&this->_msend);
  TRACE_FN_EXIT();
}

template <class T_Conn>
inline pami_result_t CCMI::Executor::PipelinedAllreduce<T_Conn>::advance()
{
  TRACE_FN_ENTER();

  if (unlikely(!this->_initialized)) {    
    //fprintf(stderr, "Not initialized\n");
    TRACE_FN_EXIT();
    return PAMI_EAGAIN;
  }
  
  if (unlikely(_curRecvChunk > this->_acache.getLastChunk()))
  {
    //fprintf(stderr, "Collective finished exiting\n");
    TRACE_FN_EXIT();
    return PAMI_SUCCESS;
  }

  pami_result_t rc = PAMI_SUCCESS;
  unsigned count = this->_acache.getFullChunkCount(); 
  unsigned last_chunk = this->_acache.getLastChunk();
  unsigned pipe_width = this->_acache.getPipelineWidth(); 
  unsigned min_bytes = (_curRecvChunk) * pipe_width;
  unsigned cur_bytes = pipe_width;
  
  if (_curRecvChunk >= last_chunk) {
    cur_bytes = this->_acache.getBytes() - min_bytes;
    count = this->_acache.getLastChunkCount();
  }
  min_bytes += cur_bytes;

  while (_curRecvChunk <= last_chunk) {
    while (_curRecvIdx < _nSums - 1) {
      if (this->_acache.getPipeWorkQueueByIdx(_curRecvIdx)->bytesAvailableToConsume() < min_bytes)
      {
	TRACE_FN_EXIT();
	return PAMI_EAGAIN;
      }
      _curRecvIdx ++;
    }
    
    nway_sum (this->_reducebuf, this->_srcbuf, this->_acache.getAllrecvBufs(), _curRecvChunk * pipe_width, count, _nSums);
    
    _curRecvChunk ++;      

    //If we are here we did some math, update pwqs
    PAMI::Memory::sync(); //ppc_msync();
    _dstpwq->produceBytes(cur_bytes);

    cur_bytes = pipe_width; 
    if (_curRecvChunk >= last_chunk) {
      count = this->_acache.getLastChunkCount();    
      cur_bytes = this->_acache.getBytes() - min_bytes;
    }
    min_bytes += cur_bytes; 
    _curRecvIdx = 0; 
  }

  rc = PAMI_SUCCESS;  

  TRACE_FN_EXIT();
  return rc;
}

template <class T_Conn>
inline void CCMI::Executor::PipelinedAllreduce<T_Conn>::postReceives()
{
  TRACE_FN_ENTER();
  // post receives for each expected incoming message
  this->_postReceives = true;
  
  pami_multicast_t mrecv;
  mrecv.msginfo     = NULL;
  mrecv.msgcount    = 0;
  mrecv.roles       = -1U;
  mrecv.cb_done.function = staticPipeNotifyRecvDone;
  mrecv.cb_done.clientdata = this;
  mrecv.bytes         = this->_acache.getBytes();
  mrecv.src           = NULL;
  
  for (unsigned p = this->_scache.getStartPhase(); p <= this->_scache.getEndPhase(); p++)
  {
    if (this->_scache.getNumSrcRanks(p) > 0)
    {
      _donecount ++;

      PAMI::PipeWorkQueue *pwq = this->_acache.getPhasePipeWorkQueues(p, 0);
      if (p > this->_lastReducePhase)
	pwq->configure(this->_dstbuf, this->_acache.getBytes(), 0); 
      pwq->reset_nosync();

      assert (pwq->bufferToProduce() != NULL);

      mrecv.dst = (pami_pipeworkqueue_t *) pwq;

      //fprintf (stderr, "Post receive pwq %p, phase %d\n", pwq, 
      
      T_Conn *connmgr = this->_bconnmgr;
      int src_id = -1;
      
      if (p <= this->_lastReducePhase) {
	src_id = this->_scache.getSrcTopology(p)->index2Endpoint(0);
	connmgr = this->_rconnmgr;      
      }      

      mrecv.connection_id = connmgr->getRecvConnectionId
	( this->_acache.getCommID(), 
	  this->_scache.getRoot(), 
	  src_id,
	  p,
	  this->_acache.getColor() );
      
      mrecv.src_participants = NULL; 
      mrecv.dst_participants = (pami_topology_t *) & this->_selftopology;      
      
      //printf ("posting recv check conn %d\n", 
      //      mrecv.connection_id);
      
      //printf ("posting recv on conn %d phase %d color %d src %d\n", 
      //      mrecv.connection_id,
      //      p,
      //      this->_acache.getColor(), src_id);
      
      this->_native->multicast(&mrecv);
    }
  }

  TRACE_FORMAT("<%p>", this);
  TRACE_FN_EXIT();
}

//#undef DO_TRACE_ENTEREXIT 
//#undef DO_TRACE_DEBUG     

#endif /* __simple_allreduce_executor_h__ */
