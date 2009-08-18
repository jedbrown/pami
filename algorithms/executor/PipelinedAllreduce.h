/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/executor/PipelinedAllreduce.h
 * \brief ???
 */

#ifndef __pipelined_allreduce_executor_h__
#define __pipelined_allreduce_executor_h__

#include "algorithms/schedule/Schedule.h"
#include "algorithms/executor/Executor.h"
#include "interface/MultiSend.h"
#include "algorithms/connmgr/ConnectionManager.h"
#include "util/logging/LogMgr.h"
#include "AllreduceState.h"
#include "AllreduceBase.h"

#define XMI_MAX_ACTIVE_SENDS  8

namespace CCMI
{
  namespace Executor
  {

    class PipelinedAllreduce : public AllreduceBase
    {

    private:
      /// Static function to be passed into the done of multisend send  
      static void pipeAllreduceNotifySend (void *cd, XMI_Error_t *err)
      {
        SendCallbackData * cdata = ( SendCallbackData *)cd;
        CMQuad *info = (CMQuad *)cd;

        ((PipelinedAllreduce *)(cdata->me))->PipelinedAllreduce::notifySendDone( *info );
      }

      /// Static function to be passed into the done of multisend postRecv
      static void pipeAllreduceNotifyReceive (void *cd, XMI_Error_t *err)
      {
        RecvCallbackData * cdata = (RecvCallbackData *)cd;
        CMQuad *info = (CMQuad *)cd;

        ((PipelinedAllreduce *)cdata->allreduce)->PipelinedAllreduce::notifyRecv 
        ((unsigned)-1, *info, NULL, (unsigned)-1);
      }

    protected:  

      void processDone ()
      {
        _donecount ++;

        TRACE_MSG ((stderr, "<%#.8X:%#.1X>Executor::PipelinedAllreduce Calling Done, count = %d \n", (int)this,ThreadID(), _donecount));

        if(_donecount == 2 && _cb_done)
        {
          _cb_done (_clientdata, NULL); 
          //_curSendPhase = -1;
          //_curRecvPhase = -1;
        }
      }

      void advanceSend  (); // __attribute__((noinline));
      void advanceRecv  (); // __attribute__((noinline));
      void advanceBcast ();
      void sendMessage  (const char             * buf, 
                         unsigned                 offset,
                         unsigned                 bytes,
                         unsigned               * dstpes,
                         unsigned                 ndst,
                         CCMI_Subtask                * dsthints,
                         unsigned                 sphase);

      //Squeeze local state in one cache line
      unsigned         _curRecvPhase;
      unsigned         _curRecvChunk;
      unsigned         _curSendPhase; 
      unsigned         _curSendChunk;

      unsigned        _numActiveSends;
      unsigned        _donecount;
      unsigned        _lastReducePhase;
      unsigned        _numBcastChunksSent;

      unsigned        _log_advancesend;
      unsigned        _log_advancerecv;
      unsigned        _log_notifyrecv;  

      bool            _inadvancesend;
    public: 

      /// Default Constructor
      PipelinedAllreduce () : AllreduceBase(),
      _curRecvPhase ((unsigned) -1), _curRecvChunk((unsigned) -1), 
      _curSendPhase ((unsigned) -1), _curSendChunk((unsigned) -1),
      _lastReducePhase((unsigned) -1)
      {
        _donecount = 0;
        _initialized = false;
        _postReceives = false;
        _nAsyncRcvd = 0;
        _inadvancesend = false;
      }

      ///  Main constructor to initialize the executor
      PipelinedAllreduce 
      (CollectiveMapping *map,
       ConnectionManager::ConnectionManager  * connmgr,
       CCMI_Consistency                        consistency,
       const unsigned                          commID,
       unsigned                                iteration):
      AllreduceBase(map,connmgr, consistency, commID, iteration, true),
      _curRecvPhase ((unsigned) -1), _curRecvChunk((unsigned) -1), 
      _curSendPhase ((unsigned) -1), _curSendChunk((unsigned) -1),
      _donecount (0), _lastReducePhase((unsigned) -1)
      {

        _sendCallbackHandler = pipeAllreduceNotifySend;
        _recvCallbackHandler = pipeAllreduceNotifyReceive;

        _inadvancesend = false;

        Logging::LogMgr *lmgr = Logging::LogMgr::getLogMgr();
        _log_advancesend = lmgr->registerEvent ("Allreduce advance send");
        _log_advancerecv = lmgr->registerEvent ("Allreduce advance recv");
        _log_notifyrecv  = lmgr->registerEvent ("Allreduce notify recv");
      }

      virtual void notifySendDone( const CMQuad &info );

      virtual void notifyRecv(unsigned src, const CMQuad &info, 
                              char * buf, unsigned bytes);

      /// start allreduce
      virtual void start();

      virtual void reset ()
      {
        AllreduceBase::reset ();

        //The previous allreduce could have enabled this
        _msend_data.setFlags (MultiSend::CCMI_FLAGS_UNSET);
        _initialized = false; 
        _inadvancesend = false;
        _curRecvPhase=_curSendPhase = _astate.getStartPhase();
        _curRecvChunk = _curSendChunk = 0;    
        _lastReducePhase = _astate.getLastReducePhase();
        _donecount = 0;

        if(_astate.getBcastNumDstPes() > 0)
          _numBcastChunksSent = 0;
        else
          _numBcastChunksSent = _astate.getLastChunk() + 1;

        TRACE_INIT ((stderr, "<%#.8X:%#.1X>Executor::PipelinedAllreduce::reset "
                     "_numBcastChunksSent:%d, numDstPes:%d\n", 
                     (int)this,ThreadID(), _numBcastChunksSent,_astate.getBcastNumDstPes()));
      }

      ///Utility functions
      unsigned findPrevRecvPhase (unsigned phase)
      {
        while(phase >= _startPhase)
        {
          unsigned nsrc = _astate.getPhaseNumSrcPes(phase);
          if(nsrc != 0)
            break;
          else
            phase --;
        }
        TRACE_INIT ((stderr, "<%#.8X:%#.1X>Executor::PipelinedAllreduce::findPrevRecvPhase "
                     "phase  %d, _startPhase %d\n", 
                     (int)this,ThreadID(), phase, _startPhase));

        return phase;
      };

    };  //-- PipelinedAllreduce
  };  //-- Executor
};  //-- CCMI


inline void CCMI::Executor::PipelinedAllreduce::start()
{
  _initialized = true; 
  _numActiveSends   = XMI_MAX_ACTIVE_SENDS;

  unsigned count;
  for(count = 0; count < _numActiveSends; count++)
  {
    // reset send info	  
    _sState[count].sndClientData.me        = this;
    _sState[count].sndClientData.isDone    = true;
  }

  CCMI_assert (_startPhase != (unsigned) -1);
  CCMI_assert (_curSendPhase != (unsigned) -1);

  unsigned ndstpes = _astate.getPhaseNumDstPes (_startPhase);  

  // kick off - send local contribution
  if(ndstpes > 0)
  {
    unsigned *dstpes   = _astate.getPhaseDstPes (_startPhase);
    CCMI_Subtask *dsthints = (CCMI_Subtask *)_astate.getPhaseDstHints (_startPhase);    
    sendMessage (_srcbuf, 0, _astate.getBytes(), dstpes, ndstpes, dsthints, 
                 _startPhase);     

    /// There is only one phase, use a shortcut
    if(_startPhase == _endPhase)
      _curSendChunk = _astate.getLastChunk() +  1;
  }

  TRACE_INIT((stderr,"<%#.8X:%#.1X>Executor::PipelinedAllreduce::start() "
              " _startPhase %d,_curSendPhase %d "
              " _curSendchunk %d, _curRecvchunk %d\n", (int)this,ThreadID(), 
              _startPhase, _curSendPhase,_curSendChunk, _curRecvChunk));

  //Process the notify recvs we got before start
  if(_curRecvChunk <= _astate.getLastChunk())
    advanceRecv ();

  TRACE_INIT((stderr,"<%#.8X:%#.1X>Executor::PipelinedAllreduce::start() "
              "_curSendchunk %d, _curRecvchunk %d\n", (int)this,ThreadID(), 
              _curSendChunk, _curRecvChunk));
}


inline void CCMI::Executor::PipelinedAllreduce::notifyRecv 
(unsigned                     src, 
 const CMQuad             & info, 
 char                       * buf, 
 unsigned                     bytes)
{
  RecvCallbackData * cdata = (RecvCallbackData *)(&info);

  TRACE_MSG ((stderr, "<%#.8X:%#.1X>Executor::PipelinedAllreduce::notifyRecv "
              "_initialized %d, bytes %d, srcPeIndex %d, cdata->phase %d,_lastReducePhase %d\n", 
              (int)this,ThreadID(), _initialized, bytes, cdata->srcPeIndex,cdata->phase,_lastReducePhase));

  // update state  (we dont support multiple sources per phase yet)
  _astate.incrementPhaseChunksRcvd(cdata->phase, 0 /*cdata->srcPeIndex*/);
  if(_initialized)
  {
    if(cdata->phase <= _lastReducePhase)
      advanceRecv();
    else
    {
      advanceBcast ();

      unsigned last_chunk = _astate.getLastChunk();
      if(_astate.getPhaseChunksRcvd(cdata->phase, 0) > last_chunk)
        processDone(); //finished receiving the broadcast

    }
  }

}


inline void CCMI::Executor::PipelinedAllreduce::notifySendDone 
( const CMQuad & info)
{
  // update state
  TRACE_MSG ((stderr, "<%#.8X:%#.1X>Executor::PipelinedAllreduce::notifySendDone "
              "_numActiveSends %d, _inadvancesend %d, "
              "_astate.getLastChunk() %d,_curSendChunk %d,_numBcastChunksSent %d\n", (int)this,ThreadID(),
              _numActiveSends,_inadvancesend,_astate.getLastChunk(),_curSendChunk,_numBcastChunksSent));

  SendCallbackData * cdata = (SendCallbackData *)(&info);
  cdata->isDone = true;  
  _numActiveSends ++;

  //short message fast path, may call notifysendone in the send call
  //itself
  if(_inadvancesend)
      return;

  unsigned last_chunk = _astate.getLastChunk();

  if((_curSendChunk > last_chunk)       &&
     (_numBcastChunksSent > last_chunk) && 
     (_numActiveSends == XMI_MAX_ACTIVE_SENDS))
    processDone();
  else
  {
    if(_curSendChunk <= last_chunk)
      advanceSend ();

    if(_numBcastChunksSent <= last_chunk)
      advanceBcast ();
  }
}


inline void CCMI::Executor::PipelinedAllreduce::advanceRecv()
{

  Logging::LogMgr::getLogMgr()->startCounter (_log_advancerecv);

  CCMI_assert_debug (_initialized);

  const char *mysrcbuf;
  char *mydstbuf;
  unsigned count;
  unsigned bufOffset;

  unsigned last_chunk = _astate.getLastChunk();

  CCMI_assert_debug (_curRecvChunk <= last_chunk);

  unsigned cur_nsrc = _astate.getPhaseNumSrcPes(_curRecvPhase);

TRACE_MSG((stderr, "<%#.8X:%#.1X>Executor::PipelinedAllreduce::advanceRecv "
           "cur_nsrc %d, _astate.getPhaseChunksRcvd(_curRecvPhase, 0) %d,"
           "_curRecvPhase%d, _curRecvChunk %d, lastChunk %d\n", (int)this,ThreadID(),
           cur_nsrc,cur_nsrc?_astate.getPhaseChunksRcvd(_curRecvPhase, 0):-2,_curRecvPhase, _curRecvChunk, last_chunk));
  while(((cur_nsrc) &&
         (_astate.getPhaseChunksRcvd(_curRecvPhase, 0) > _curRecvChunk))
        ||  (!cur_nsrc))
  {

    if(( cur_nsrc ) && 
       (_astate.getPhaseSrcHints(_curRecvPhase, 0) == CCMI_COMBINE_SUBTASK))
    {
      if(_curRecvPhase == _startPhase)
        mysrcbuf = _srcbuf;
      else
      {
        unsigned phase = findPrevRecvPhase (_curRecvPhase - 1);
        mysrcbuf = _astate.getPhaseRecvBufs (phase, 0);         
      } 
      CCMI_assert (mysrcbuf != NULL); 

      //mydstbuf  = (_curRecvPhase != _endPhase) ? 
      //  _astate.getPhaseRecvBufs (_curRecvPhase, 0) : _dstbuf;

      mydstbuf  = (_curRecvPhase != (unsigned)_astate.getLastCombinePhase()) ? 
                  _astate.getPhaseRecvBufs (_curRecvPhase, 0) : _dstbuf;

      count     = (_curRecvChunk < last_chunk) ? 
                  _astate.getFullChunkCount() : _astate.getLastChunkCount();
      bufOffset = _curRecvChunk * _astate.getPipelineWidth();

      Logging::LogMgr::getLogMgr()->stopCounter (_log_advancerecv, 0);
      void *bufs[2] = {
        (void *)(mysrcbuf + bufOffset),
        _astate.getPhaseRecvBufs (_curRecvPhase, 0) + bufOffset,
      };
      TRACE_REDUCEOP((stderr,"<%#.8X:%#.1X>Executor::PipelinedAllreduce::advance() OP curphase:%#X curChunk:%#X "
                      "bufs[0] %#X, bufs[1] %#X, func(%#X), count = %#X\n",(int)this,ThreadID(),
                      _curRecvPhase, _curRecvChunk, (int) bufs[0], (int) bufs[1],
                      (int)_reduceFunc,count));
      TRACE_DATA(("localbuf",(const char*)bufs[0], count*_astate.getSizeOfType()));
      TRACE_DATA(("input buf",(const char*)bufs[1], count*_astate.getSizeOfType()));
      TRACE_DATA(("reduceBuf", mydstbuf + bufOffset, 1));  // Just to trace the input pointer
      _reduceFunc( mydstbuf  + bufOffset,
                   bufs, 2,
                   count); 
      TRACE_DATA(("reduceBuf", mydstbuf + bufOffset, count*_astate.getSizeOfType()));
      TRACE_REDUCEOP((stderr,"<%#.8X:%#.1X>Executor::PipelinedAllreduce::advance() OP reducebuf %#X, "
                      "reduceBuf[0]:%#X\n",(int)this,ThreadID(),((int)mydstbuf+bufOffset),
                      *(unsigned *)((int)mydstbuf+bufOffset)));
      Logging::LogMgr::getLogMgr()->startCounter (_log_advancerecv);
    }
    //    _curRecvPhase++;
   _curRecvPhase = _astate.getNextActivePhase (_curRecvPhase);
    if(_curRecvPhase > _lastReducePhase)
    {
      _curRecvChunk ++;      
      _curRecvPhase = _startPhase;
    }
//  TRACE_MSG((stderr, "<%#.8X:%#.1X>Executor::PipelinedAllreduce::advanceRecv loop 2"
//             "_curRecvPhase %d, _lastReducePhase %d"
//             "_curSendChunk %d, _curRecvChunk %d, last_chunk %d\n", (int)this,ThreadID(),
//             _curRecvPhase, _lastReducePhase,_curSendChunk, _curRecvChunk, last_chunk));

    Logging::LogMgr::getLogMgr()->stopCounter (_log_advancerecv, 0);
    if(_curSendChunk <= last_chunk)
      advanceSend ();
    Logging::LogMgr::getLogMgr()->startCounter (_log_advancerecv);

    if(_curRecvChunk > last_chunk)
    {
      ///We have no broadcast messages to receive    
      if(_lastReducePhase == _endPhase)
        processDone ();
      break;
    }

    cur_nsrc = _astate.getPhaseNumSrcPes(_curRecvPhase);
    TRACE_MSG((stderr, "<%#.8X:%#.1X>Executor::PipelinedAllreduce::advanceRecv endloop "
               "cur_nsrc %d, _astate.getPhaseChunksRcvd(_curRecvPhase, 0) %d,"
               "_curRecvPhase%d, _curRecvChunk %d\n", (int)this,ThreadID(), 
               cur_nsrc,cur_nsrc?_astate.getPhaseChunksRcvd(_curRecvPhase, 0):-2,_curRecvPhase, _curRecvChunk));
  }

  Logging::LogMgr::getLogMgr()->stopCounter (_log_advancerecv, 1);
}


inline void CCMI::Executor::PipelinedAllreduce::advanceSend()
{

  Logging::LogMgr::getLogMgr()->startCounter (_log_advancesend);

  _inadvancesend = true;

  TRACE_MSG((stderr, "<%#.8X:%#.1X>Executor::PipelinedAllreduce::advanceSend "
             "_numActiveSends %d\n", (int)this,ThreadID(), 
             _numActiveSends));

  CCMI_assert_debug (_initialized == true);  
  unsigned last_chunk  = _astate.getLastChunk();  
  CCMI_assert (_curSendChunk <= last_chunk);

  while(_numActiveSends > 0)
  {
    TRACE_MSG ((stderr, "<%#.8X:%#.1X>Executor::PipelinedAllreduce::advanceSend "
                "_curRecvPhase, %d, _curRecvChunk %d, "
                "_curSendPhase %d, _curSendChunk %d, _astate.getPhaseChunksSent (_curSendPhase) %d\n", 
                (int)this,ThreadID(), 
                _curRecvPhase, _curRecvChunk, 
                _curSendPhase, _curSendChunk,
                _astate.getPhaseChunksSent (_curSendPhase)));

    /// The send advance can only be one chunk ahead of the receive adavnce
    if((_curSendChunk > _curRecvChunk) ||
       (_curSendChunk == _curRecvChunk && _curSendPhase > _curRecvPhase))
        break;

    //Dont send messages in the first phase
    if((_curSendPhase != _startPhase) && 
       (_astate.getPhaseNumDstPes(_curSendPhase) > 0) &&
       (_curSendChunk == _astate.getPhaseChunksSent(_curSendPhase)))
    {

      /// If this phase has lots of data to send, we should try to
      /// send all those bytes
      unsigned npw = _curRecvChunk - _curSendChunk;

      /// send and recv are in the same phase and hence there is one
      /// chunk to send
      if(npw == 0) npw = 1;

      unsigned phase = findPrevRecvPhase (_curSendPhase - 1);
      char *rcv_buf = (phase != (unsigned)_astate.getLastCombinePhase())? 
                      _astate.getPhaseRecvBufs (phase, 0) :  _dstbuf;      
      CCMI_assert (rcv_buf != NULL);

      unsigned bufOffset = _curSendChunk * _astate.getPipelineWidth();

      unsigned count     = 0;       
      if((_curSendChunk + npw) <= last_chunk)
        count = _astate.getFullChunkCount()*npw;
      else
        count =_astate.getFullChunkCount()*(npw-1)+_astate.getLastChunkCount();

      Logging::LogMgr::getLogMgr()->stopCounter (_log_advancesend, 0);      

      sendMessage (rcv_buf, bufOffset, count * _astate.getSizeOfType(),
                   _astate.getPhaseDstPes(_curSendPhase), 
                   _astate.getPhaseNumDstPes(_curSendPhase),
                   (CCMI_Subtask *)_astate.getPhaseDstHints (_curSendPhase),
                   _curSendPhase);

      Logging::LogMgr::getLogMgr()->startCounter (_log_advancesend);

      _astate.incrementPhaseChunksSent(_curSendPhase, npw);
    }
    //_curSendPhase ++;
    _curSendPhase = _astate.getNextActivePhase (_curSendPhase);

    if(_curSendPhase > _lastReducePhase)
    {
      _curSendPhase = _startPhase;
      _curSendChunk ++;

      if(_curSendChunk > last_chunk)
      {
        ///All sends done and all broadcasts done and no messages in
        ///flight
        if(_numActiveSends == XMI_MAX_ACTIVE_SENDS 
           && _numBcastChunksSent > last_chunk)
          processDone();
        break;
      }
    }
  }

  _inadvancesend = false;

  Logging::LogMgr::getLogMgr()->stopCounter (_log_advancesend, 1);
}


inline void CCMI::Executor::PipelinedAllreduce::sendMessage 
(const char             * buf, 
 unsigned                 offset,
 unsigned                 bytes,
 unsigned               * dstpes,
 unsigned                 ndst,
 CCMI_Subtask                * dsthints,
 unsigned                 sphase)
{

  CCMI_assert (_curSendChunk   >= 0);
  CCMI_assert (_numActiveSends > 0);

  _numActiveSends --;

  TRACE_MSG ((stderr, "<%#.8X:%#.1X>Executor::PipelinedAllreduce::sendMessage _curSendChunk %d,_numActiveSends %d, offset %d,bytes %d\n", 
              (int)this,ThreadID(), 
              _curSendChunk,_numActiveSends, offset, bytes));

  int index = XMI_MAX_ACTIVE_SENDS - 1;
  if(!_sState[index].sndClientData.isDone)
  {
    while((index --) && (!_sState[index].sndClientData.isDone));
  }
  //we must find an available request 
  CCMI_assert (index >= 0);

  if(offset == 0)
    _msend_data.setFlags (MultiSend::CCMI_FLAGS_UNSET);
  else
    _msend_data.setFlags (MultiSend::CCMI_PERSISTENT_MESSAGE);

  _msend_data.setRequestBuffer (&(_sState[index].sndReq));
  _msend_data.setCallback (_sendCallbackHandler, 
                           &_sState[index].sndClientData);  

  CCMI_assert (offset + bytes <= _astate.getBytes());

  AllreduceBase::sendMessage (buf + offset, bytes, dstpes, ndst, dsthints, 
                              sphase, &_sState[index]);
}

inline void CCMI::Executor::PipelinedAllreduce::advanceBcast ()
{
  unsigned bcastRecvPhase = _astate.getBcastRecvPhase();
  TRACE_MSG ((stderr, "<%#.8X:%#.1X>Executor::PipelinedAllreduce::advanceBcast "
              "bcastRecvPhase%d, _numActiveSends %d,"
              "_astate.getPhaseChunksRcvd(bcastRecvPhase, 0) %d,_numBcastChunksSent %d\n", 
              (int)this,ThreadID(), 
              bcastRecvPhase, _numActiveSends,
              _astate.getPhaseChunksRcvd(bcastRecvPhase, 0),_numBcastChunksSent));

  if(bcastRecvPhase < 0)
      return;

  if(_numActiveSends <= XMI_MAX_ACTIVE_SENDS/2)
      return;

  if(_astate.getPhaseChunksRcvd(bcastRecvPhase, 0) > _numBcastChunksSent)
  {
    unsigned * dstpes     = _astate.getBcastDstPes();
    CCMI_Subtask  * dsthints   = (CCMI_Subtask *) _astate.getBcastDstHints();
    unsigned   ndst       = _astate.getBcastNumDstPes();    
    unsigned   last_chunk = _astate.getLastChunk();

    int npw = _astate.getPhaseChunksRcvd(bcastRecvPhase,0)-_numBcastChunksSent;
    int count = 0;
    int sizeOfType = _astate.getSizeOfType();

    if(_astate.getPhaseChunksRcvd (bcastRecvPhase, 0) <= last_chunk)
      count = npw * _astate.getFullChunkCount();
    else
      count = (npw - 1) * _astate.getFullChunkCount() 
              + _astate.getLastChunkCount();

    unsigned bcast_offset = _numBcastChunksSent *  
                            _astate.getPipelineWidth();

    _numBcastChunksSent += npw;

    sendMessage (_dstbuf, bcast_offset, count*sizeOfType, 
                 dstpes, ndst, dsthints, _astate.getBcastSendPhase());
  }
}


#endif /* __simple_allreduce_executor_h__ */
