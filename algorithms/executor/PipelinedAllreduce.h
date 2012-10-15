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
	  //fprintf(stderr, "All Done\n");
	  this->_isSendDone = false; //Process an early arrival packet
	  this->_initialized = false; //Call application done callback
          this->_cb_done (this->_context, this->_clientdata, PAMI_SUCCESS); 
        }
      }

      //static const unsigned _pipelineWidth = 1024;

      //Squeeze local state in one cache line
      unsigned         _curRecvPhase;
      unsigned         _curRecvChunk;
      unsigned         _donecount;
      unsigned         _firstRecvPhase;   
      PAMI::PipeWorkQueue *_dstpwq;

    public: 

      /// Default Constructor
      PipelinedAllreduce<T_Conn>() : AllreduceBaseExec<T_Conn, true>(),
	_curRecvPhase ((unsigned) -1), _curRecvChunk((unsigned) -1),
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
      _curRecvPhase ((unsigned) -1), _curRecvChunk((unsigned) -1), 
      _donecount (0),_firstRecvPhase((unsigned) -1)
      {
	this->_postReceives = true;
	this->_enablePipelining = true;
      }

      pami_result_t advance (); // __attribute__((noinline));

      /// start allreduce
      virtual void start();
	
      virtual void reset ()
      {
        AllreduceBaseExec<T_Conn, true>::reset ();

        this->_initialized = false; 
        _curRecvPhase = this->_scache.getStartPhase();
        _curRecvChunk = 0;    
	
	_firstRecvPhase = this->_scache.getStartPhase();
	while (!this->_scache.getNumSrcRanks(_firstRecvPhase) && 
	       _firstRecvPhase <= this->_endPhase)
	  _firstRecvPhase++;
	
	//Reduce might not have a combine phase (on non-roots)
	if ((this->_scache.getRoot() != -1) && 
	    (_firstRecvPhase > this->_endPhase))
	  _firstRecvPhase = this->_scache.getStartPhase();
	
	CCMI_assert (_firstRecvPhase <= this->_endPhase);
	
	_donecount = 0;
	if (this->_scache.getNumTotalSrcRanks() == 0 
	    || _firstRecvPhase > this->_lastReducePhase)
	  _curRecvChunk = this->_acache.getLastChunk() + 1;
	
	_dstpwq = NULL;

	coremath  reduce_func = MATH_OP_FUNCS(this->_acache.getDt(), this->_acache.getOp(), 2);
	//Update with optimized math function if its available
	if (reduce_func != NULL)
	  this->_reduceFunc = reduce_func; 
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
    //printf("phase %d, ndstranks %d\n", p, ndstranks);    

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
      sendMessage (reducebuf, this->_acache.getBytes(), dst_topology, p);
    }
  }
  CCMI_assert (_dstpwq != NULL);
  
  //Process the notify recvs we got before start
  if(_curRecvChunk <= this->_acache.getLastChunk())
    advance();  
  else 
    _dstpwq->produceBytes (this->_acache.getBytes());

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
  
  T_Conn *connmgr = this->_rconnmgr;
  if (sphase > this->_lastReducePhase) 
    connmgr = this->_bconnmgr;        
  
  this->_msend.connection_id = connmgr->getConnectionId
    ( this->_acache.getCommID(), 
      this->_scache.getRoot(), 
      dst_topology->index2Endpoint(0),
      sphase,
      this->_acache.getColor() );
  
  this->_msend.bytes         = bytes;

  PAMI::PipeWorkQueue *pwq = NULL;  
  if (sphase > this->_lastReducePhase)
    pwq = this->_acache.getBcastPipeWorkQueue();
  else
    pwq = this->_acache.getDestPipeWorkQueue();
  
  pwq->configure((char *)buf, bytes, 0);

  //fprintf (stderr, "send pwq %p phase %d connid %d\n", pwq, sphase, this->_msend.connection_id);

  this->_msend.src           = (pami_pipeworkqueue_t *) pwq;
  this->_msend.dst           = NULL;
  this->_msend.src_participants = (pami_topology_t *) & this->_selftopology;
  this->_msend.dst_participants = (pami_topology_t *) this->_scache.getDstTopology(sphase);

  this->_msend.cb_done.function =  staticPipeNotifySendDone;
  this->_msend.cb_done.clientdata = this;  
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
  const char *mysrcbuf = this->_srcbuf;
  char *mydstbuf = this->_reducebuf;
  unsigned bufOffset;

  unsigned count = this->_acache.getFullChunkCount(); 
  unsigned last_chunk = this->_acache.getLastChunk();
  unsigned pipe_width = this->_acache.getPipelineWidth(); 
  unsigned min_bytes = (_curRecvChunk) * pipe_width;
  unsigned bytes_produced = 0;
  unsigned cur_bytes = pipe_width;
  
  if (_curRecvChunk >= last_chunk) {
    cur_bytes = this->_acache.getBytes() - min_bytes;
    count = this->_acache.getLastChunkCount();
  }
  min_bytes += cur_bytes;

  while (_curRecvChunk <= last_chunk) {
    while (_curRecvPhase <= this->_lastReducePhase) {
      //Does this phase receive data
      if (likely(this->_acache.isPhaseRecvActive(_curRecvPhase))) {
	if (likely(this->_acache.getPhasePipeWorkQueues(_curRecvPhase,0)->bytesAvailableToConsume() >= min_bytes))
	  {
	    CCMI_assert (mysrcbuf != NULL); 	    
	    bufOffset = _curRecvChunk * pipe_width;	    
	    void *bufs[2] = {
	      (void *)(mysrcbuf + bufOffset),
	    this->_acache.getPhaseRecvBufs (_curRecvPhase, 0) + bufOffset,
	    };
#if 0	    
	    if (((uint64_t)mysrcbuf & 0x1F) !=0)
	      printf ("Unaligned Source\n");

	    if (((uint64_t)bufs[1] & 0x1F) !=0)
	      printf ("Unaligned phase recv buf phase=%d\n", _curRecvPhase);	    

	    if (((uint64_t)mydstbuf & 0x1F) !=0)
	      printf ("Unaligned mydstbuf\n");	 
#endif

	    this->_reduceFunc( mydstbuf+bufOffset, bufs, 2, count);
	    mysrcbuf = this->_reducebuf; 
	  }
	else {
	  if (bytes_produced > 0) { //If we are here we did some math in this function
	    //fprintf(stderr, "Producing %d bytes into dstpwq\n", bytes_produced);
	    _dstpwq->produceBytes(bytes_produced);
	  }	  
	  TRACE_FN_EXIT();
	  return PAMI_EAGAIN;
	}
      }            
      _curRecvPhase = this->_scache.getNextActivePhase (_curRecvPhase);
    }
    
    _curRecvChunk ++;      
    bytes_produced += cur_bytes;
    cur_bytes = pipe_width; 
    if (_curRecvChunk >= last_chunk) {
      count = this->_acache.getLastChunkCount();    
      cur_bytes = this->_acache.getBytes() - min_bytes;
    }
    min_bytes += cur_bytes; 
    _curRecvPhase = this->_scache.getStartPhase();
    mysrcbuf = this->_srcbuf;
  }

  rc = PAMI_SUCCESS;  
  //If we are here we did some math in this function
  //fprintf(stderr, "Producing %d bytes into dstpwq\n", bytes_produced);
  _dstpwq->produceBytes(bytes_produced);
  
  //fprintf (stderr, "After advance phase %d, chunk %d\n", _curRecvPhase, _curRecvChunk);
  
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

      mrecv.dst = (pami_pipeworkqueue_t *) pwq;
      
      T_Conn *connmgr = this->_rconnmgr;
      if (p > this->_lastReducePhase) 
	connmgr = this->_bconnmgr;      

      //fprintf (stderr, "Post receive pwq %p, phase %d\n", pwq, p);
      
      mrecv.connection_id = connmgr->getRecvConnectionId
	( this->_acache.getCommID(), 
	  this->_scache.getRoot(), 
	  this->_scache.getSrcTopology(p)->index2Endpoint(0),
	  p,
	  this->_acache.getColor() );
      
      pami_topology_t *srct  = (pami_topology_t *) this->_scache.getSrcTopology(p);
      CCMI_assert (srct != NULL);
      mrecv.src_participants = srct;
      mrecv.dst_participants = (pami_topology_t *) & this->_selftopology;      
      
      this->_native->multicast(&mrecv);
    }
  }

  TRACE_FORMAT("<%p>", this);
  TRACE_FN_EXIT();
}

//#undef DO_TRACE_ENTEREXIT 
//#undef DO_TRACE_DEBUG     

#endif /* __simple_allreduce_executor_h__ */
