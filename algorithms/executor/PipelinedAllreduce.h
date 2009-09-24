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
#include "algorithms/connmgr/ConnectionManager.h"
#include "AllreduceState.h"
#include "AllreduceBase.h"
//#include "util/logging/LogMgr.h"


#define XMI_MAX_ACTIVE_SENDS  8

namespace CCMI
{
  namespace Executor
  {
    template<class T_Mcast, class T_Sysdep, class T_ConnectionManager>
    class PipelinedAllreduce : public AllreduceBase<T_Mcast, T_Sysdep>
    {

    private:
      /// Static function to be passed into the done of multisend send
      static void pipeAllreduceNotifySend (void *cd, xmi_result_t *err)
      {
        SendCallbackData * cdata = ( SendCallbackData *)cd;
        xmi_quad_t *info = (xmi_quad_t *)cd;

        ((PipelinedAllreduce *)(cdata->me))->PipelinedAllreduce::notifySendDone( *info );
      }

      /// Static function to be passed into the done of multisend postRecv
      static void pipeAllreduceNotifyReceive (void *cd, xmi_result_t *err)
      {
        RecvCallbackData * cdata = (RecvCallbackData *)cd;
        xmi_quad_t *info = (xmi_quad_t *)cd;

        ((PipelinedAllreduce *)cdata->allreduce)->PipelinedAllreduce::notifyRecv
        ((unsigned)-1, *info, NULL, (unsigned)-1);
      }

    protected:

      void processDone ()
      {
        _donecount ++;

        TRACE_MSG ((stderr, "<%#.8X:%#.1X>Executor::PipelinedAllreduce Calling Done, count = %d \n", (int)this,ThreadID(), _donecount));

        if(_donecount == 2 && this->_cb_done)
        {
          this->_cb_done (this->_clientdata, NULL);
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
                         xmi_subtask_t          * dsthints,
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
      PipelinedAllreduce () : AllreduceBase<T_Mcast, T_Sysdep>(),
      _curRecvPhase ((unsigned) -1), _curRecvChunk((unsigned) -1),
      _curSendPhase ((unsigned) -1), _curSendChunk((unsigned) -1),
      _lastReducePhase((unsigned) -1)
      {
        this->_donecount = 0;
        this->_initialized = false;
        this->_postReceives = false;
        this->_nAsyncRcvd = 0;
        this->_inadvancesend = false;
      }

      ///  Main constructor to initialize the executor
      PipelinedAllreduce
      (T_Sysdep             * map,
       T_ConnectionManager  * connmgr,
       xmi_consistency_t      consistency,
       const unsigned         commID,
       unsigned               iteration):
        AllreduceBase<T_Mcast, T_Sysdep>(map,connmgr, consistency, commID, iteration, true),
      _curRecvPhase ((unsigned) -1), _curRecvChunk((unsigned) -1),
      _curSendPhase ((unsigned) -1), _curSendChunk((unsigned) -1),
      _donecount (0), _lastReducePhase((unsigned) -1)
      {

        this->_sendCallbackHandler = pipeAllreduceNotifySend;
        this->_recvCallbackHandler = pipeAllreduceNotifyReceive;

        _inadvancesend = false;

//        Logging::LogMgr *lmgr = Logging::LogMgr::getLogMgr();
//        _log_advancesend = lmgr->registerEvent ("Allreduce advance send");
//        _log_advancerecv = lmgr->registerEvent ("Allreduce advance recv");
//        _log_notifyrecv  = lmgr->registerEvent ("Allreduce notify recv");
      }

      virtual void notifySendDone( const xmi_quad_t &info );

      virtual void notifyRecv(unsigned src, const xmi_quad_t &info,
                              char * buf, unsigned bytes);

      /// start allreduce
      virtual void start();

      virtual void reset ()
      {
        AllreduceBase<T_Mcast, T_Sysdep>::reset ();

        //The previous allreduce could have enabled this
//        _msend_data.setFlags (MultiSend::CCMI_FLAGS_UNSET);
        this->_msend_data.flags = 0;  // FLAGS_UNSET
        this->_initialized = false;
        this->_inadvancesend = false;
        _curRecvPhase=_curSendPhase = this->_astate.getStartPhase();
        _curRecvChunk = _curSendChunk = 0;
        _lastReducePhase = this->_astate.getLastReducePhase();
        _donecount = 0;

        if(this->_astate.getBcastNumDstPes() > 0)
          _numBcastChunksSent = 0;
        else
          _numBcastChunksSent = this->_astate.getLastChunk() + 1;

        TRACE_INIT ((stderr, "<%#.8X:%#.1X>Executor::PipelinedAllreduce::reset "
                     "_numBcastChunksSent:%d, numDstPes:%d\n",
                     (int)this,ThreadID(), _numBcastChunksSent,_astate.getBcastNumDstPes()));
      }

      ///Utility functions
      unsigned findPrevRecvPhase (unsigned phase)
      {
        while(phase >= this->_startPhase)
        {
          unsigned nsrc = this->_astate.getPhaseNumSrcPes(phase);
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

template<class T_Mcast, class T_Sysdep, class T_ConnectionManager>
inline void CCMI::Executor::PipelinedAllreduce<T_Mcast, T_Sysdep, T_ConnectionManager>::start()
{
  this->_initialized = true;
  _numActiveSends   = XMI_MAX_ACTIVE_SENDS;

  unsigned count;
  for(count = 0; count < _numActiveSends; count++)
  {
    // reset send info
    this->_sState[count].sndClientData.me        = this;
    this->_sState[count].sndClientData.isDone    = true;
  }

  CCMI_assert (this->_startPhase != (unsigned) -1);
  CCMI_assert (_curSendPhase != (unsigned) -1);

  unsigned ndstpes = this->_astate.getPhaseNumDstPes (this->_startPhase);

  // kick off - send local contribution
  if(ndstpes > 0)
  {
    unsigned *dstpes   = this->_astate.getPhaseDstPes (this->_startPhase);
    xmi_subtask_t *dsthints = (xmi_subtask_t *)this->_astate.getPhaseDstHints (this->_startPhase);
    sendMessage (this->_srcbuf, 0, this->_astate.getBytes(), dstpes, ndstpes, dsthints,
                 this->_startPhase);

    /// There is only one phase, use a shortcut
    if(this->_startPhase == this->_endPhase)
      _curSendChunk = this->_astate.getLastChunk() +  1;
  }

  TRACE_INIT((stderr,"<%#.8X:%#.1X>Executor::PipelinedAllreduce::start() "
              " this->_startPhase %d,_curSendPhase %d "
              " _curSendchunk %d, _curRecvchunk %d\n", (int)this,ThreadID(),
              this->_startPhase, _curSendPhase,_curSendChunk, _curRecvChunk));

  //Process the notify recvs we got before start
  if(_curRecvChunk <= this->_astate.getLastChunk())
    advanceRecv ();

  TRACE_INIT((stderr,"<%#.8X:%#.1X>Executor::PipelinedAllreduce::start() "
              "_curSendchunk %d, _curRecvchunk %d\n", (int)this,ThreadID(),
              _curSendChunk, _curRecvChunk));
}

template<class T_Mcast, class T_Sysdep, class T_ConnectionManager>
inline void CCMI::Executor::PipelinedAllreduce<T_Mcast, T_Sysdep, T_ConnectionManager>::notifyRecv
(unsigned                     src,
 const xmi_quad_t             & info,
 char                       * buf,
 unsigned                     bytes)
{
  CCMI::Executor::RecvCallbackData * cdata = (CCMI::Executor::RecvCallbackData *)(&info);

  TRACE_MSG ((stderr, "<%#.8X:%#.1X>Executor::PipelinedAllreduce::notifyRecv "
              "_initialized %d, bytes %d, srcPeIndex %d, cdata->phase %d,_lastReducePhase %d\n",
              (int)this,ThreadID(), _initialized, bytes, cdata->srcPeIndex,cdata->phase,_lastReducePhase));

  // update state  (we dont support multiple sources per phase yet)
  this->_astate.incrementPhaseChunksRcvd(cdata->phase, 0 /*cdata->srcPeIndex*/);
  if(this->_initialized)
  {
    if(cdata->phase <= _lastReducePhase)
      advanceRecv();
    else
    {
      advanceBcast ();

      unsigned last_chunk = this->_astate.getLastChunk();
      if(this->_astate.getPhaseChunksRcvd(cdata->phase, 0) > last_chunk)
        processDone(); //finished receiving the broadcast

    }
  }

}

template<class T_Mcast, class T_Sysdep, class T_ConnectionManager>
inline void CCMI::Executor::PipelinedAllreduce<T_Mcast, T_Sysdep, T_ConnectionManager>::notifySendDone
( const xmi_quad_t & info)
{
  // update state
  TRACE_MSG ((stderr, "<%#.8X:%#.1X>Executor::PipelinedAllreduce::notifySendDone "
              "_numActiveSends %d, _inadvancesend %d, "
              "_astate.getLastChunk() %d,_curSendChunk %d,_numBcastChunksSent %d\n", (int)this,ThreadID(),
              _numActiveSends,_inadvancesend,this->_astate.getLastChunk(),_curSendChunk,_numBcastChunksSent));

  SendCallbackData * cdata = (SendCallbackData *)(&info);
  cdata->isDone = true;
  _numActiveSends ++;

  //short message fast path, may call notifysendone in the send call
  //itself
  if(_inadvancesend)
      return;

  unsigned last_chunk = this->_astate.getLastChunk();

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

template<class T_Mcast, class T_Sysdep, class T_ConnectionManager>
inline void CCMI::Executor::PipelinedAllreduce<T_Mcast, T_Sysdep, T_ConnectionManager>::advanceRecv()
{

//  Logging::LogMgr::getLogMgr()->startCounter (_log_advancerecv);

  CCMI_assert_debug (_initialized);

  const char *mysrcbuf;
  char *mydstbuf;
  unsigned count;
  unsigned bufOffset;

  unsigned last_chunk = this->_astate.getLastChunk();

  CCMI_assert_debug (_curRecvChunk <= last_chunk);

  unsigned cur_nsrc = this->_astate.getPhaseNumSrcPes(_curRecvPhase);

TRACE_MSG((stderr, "<%#.8X:%#.1X>Executor::PipelinedAllreduce::advanceRecv "
           "cur_nsrc %d, this->_astate.getPhaseChunksRcvd(_curRecvPhase, 0) %d,"
           "_curRecvPhase%d, _curRecvChunk %d, lastChunk %d\n", (int)this,ThreadID(),
           cur_nsrc,cur_nsrc?this->_astate.getPhaseChunksRcvd(_curRecvPhase, 0):-2,_curRecvPhase, _curRecvChunk, last_chunk));
  while(((cur_nsrc) &&
         (this->_astate.getPhaseChunksRcvd(_curRecvPhase, 0) > _curRecvChunk))
        ||  (!cur_nsrc))
  {

    if(( cur_nsrc ) &&
       (this->_astate.getPhaseSrcHints(_curRecvPhase, 0) == XMI_COMBINE_SUBTASK))
    {
      if(_curRecvPhase == this->_startPhase)
        mysrcbuf = this->_srcbuf;
      else
      {
        unsigned phase = findPrevRecvPhase (_curRecvPhase - 1);
        mysrcbuf = this->_astate.getPhaseRecvBufs (phase, 0);
      }
      CCMI_assert (mysrcbuf != NULL);

      //mydstbuf  = (_curRecvPhase != _endPhase) ?
      //  this->_astate.getPhaseRecvBufs (_curRecvPhase, 0) : _dstbuf;

      mydstbuf  = (_curRecvPhase != (unsigned)this->_astate.getLastCombinePhase()) ?
                  this->_astate.getPhaseRecvBufs (_curRecvPhase, 0) : this->_dstbuf;

      count     = (_curRecvChunk < last_chunk) ?
                  this->_astate.getFullChunkCount() : this->_astate.getLastChunkCount();
      bufOffset = _curRecvChunk * this->_astate.getPipelineWidth();

//      Logging::LogMgr::getLogMgr()->stopCounter (_log_advancerecv, 0);
      void *bufs[2] = {
        (void *)(mysrcbuf + bufOffset),
        this->_astate.getPhaseRecvBufs (_curRecvPhase, 0) + bufOffset,
      };
      TRACE_REDUCEOP((stderr,"<%#.8X:%#.1X>Executor::PipelinedAllreduce::advance() OP curphase:%#X curChunk:%#X "
                      "bufs[0] %#X, bufs[1] %#X, func(%#X), count = %#X\n",(int)this,ThreadID(),
                      _curRecvPhase, _curRecvChunk, (int) bufs[0], (int) bufs[1],
                      (int)_reduceFunc,count));
      TRACE_DATA(("localbuf",(const char*)bufs[0], count*this->_astate.getSizeOfType()));
      TRACE_DATA(("input buf",(const char*)bufs[1], count*this->_astate.getSizeOfType()));
      TRACE_DATA(("reduceBuf", mydstbuf + bufOffset, 1));  // Just to trace the input pointer
      this->_reduceFunc( mydstbuf  + bufOffset,
                   bufs, 2,
                   count);
      TRACE_DATA(("reduceBuf", mydstbuf + bufOffset, count*this->_astate.getSizeOfType()));
      TRACE_REDUCEOP((stderr,"<%#.8X:%#.1X>Executor::PipelinedAllreduce::advance() OP reducebuf %#X, "
                      "reduceBuf[0]:%#X\n",(int)this,ThreadID(),((int)mydstbuf+bufOffset),
                      *(unsigned *)((int)mydstbuf+bufOffset)));
//      Logging::LogMgr::getLogMgr()->startCounter (_log_advancerecv);
    }
    //    _curRecvPhase++;
   _curRecvPhase = this->_astate.getNextActivePhase (_curRecvPhase);
    if(_curRecvPhase > _lastReducePhase)
    {
      _curRecvChunk ++;
      _curRecvPhase = this->_startPhase;
    }
//  TRACE_MSG((stderr, "<%#.8X:%#.1X>Executor::PipelinedAllreduce::advanceRecv loop 2"
//             "_curRecvPhase %d, _lastReducePhase %d"
//             "_curSendChunk %d, _curRecvChunk %d, last_chunk %d\n", (int)this,ThreadID(),
//             _curRecvPhase, _lastReducePhase,_curSendChunk, _curRecvChunk, last_chunk));

//    Logging::LogMgr::getLogMgr()->stopCounter (_log_advancerecv, 0);
    if(_curSendChunk <= last_chunk)
      advanceSend ();
//    Logging::LogMgr::getLogMgr()->startCounter (_log_advancerecv);

    if(_curRecvChunk > last_chunk)
    {
      ///We have no broadcast messages to receive
      if(_lastReducePhase == this->_endPhase)
        processDone ();
      break;
    }

    cur_nsrc = this->_astate.getPhaseNumSrcPes(_curRecvPhase);
    TRACE_MSG((stderr, "<%#.8X:%#.1X>Executor::PipelinedAllreduce::advanceRecv endloop "
               "cur_nsrc %d, this->_astate.getPhaseChunksRcvd(_curRecvPhase, 0) %d,"
               "_curRecvPhase%d, _curRecvChunk %d\n", (int)this,ThreadID(),
               cur_nsrc,cur_nsrc?this->_astate.getPhaseChunksRcvd(_curRecvPhase, 0):-2,_curRecvPhase, _curRecvChunk));
  }

//  Logging::LogMgr::getLogMgr()->stopCounter (_log_advancerecv, 1);
}

template<class T_Mcast, class T_Sysdep, class T_ConnectionManager>
inline void CCMI::Executor::PipelinedAllreduce<T_Mcast, T_Sysdep, T_ConnectionManager>::advanceSend()
{

//  Logging::LogMgr::getLogMgr()->startCounter (_log_advancesend);

  _inadvancesend = true;

  TRACE_MSG((stderr, "<%#.8X:%#.1X>Executor::PipelinedAllreduce::advanceSend "
             "_numActiveSends %d\n", (int)this,ThreadID(),
             _numActiveSends));

  CCMI_assert_debug (_initialized == true);
  unsigned last_chunk  = this->_astate.getLastChunk();
  CCMI_assert (_curSendChunk <= last_chunk);

  while(_numActiveSends > 0)
  {
    TRACE_MSG ((stderr, "<%#.8X:%#.1X>Executor::PipelinedAllreduce::advanceSend "
                "_curRecvPhase, %d, _curRecvChunk %d, "
                "_curSendPhase %d, _curSendChunk %d, this->_astate.getPhaseChunksSent (_curSendPhase) %d\n",
                (int)this,ThreadID(),
                _curRecvPhase, _curRecvChunk,
                _curSendPhase, _curSendChunk,
                this->_astate.getPhaseChunksSent (_curSendPhase)));

    /// The send advance can only be one chunk ahead of the receive adavnce
    if((_curSendChunk > _curRecvChunk) ||
       (_curSendChunk == _curRecvChunk && _curSendPhase > _curRecvPhase))
        break;

    //Dont send messages in the first phase
    if((_curSendPhase != this->_startPhase) &&
       (this->_astate.getPhaseNumDstPes(_curSendPhase) > 0) &&
       (_curSendChunk == this->_astate.getPhaseChunksSent(_curSendPhase)))
    {

      /// If this phase has lots of data to send, we should try to
      /// send all those bytes
      unsigned npw = _curRecvChunk - _curSendChunk;

      /// send and recv are in the same phase and hence there is one
      /// chunk to send
      if(npw == 0) npw = 1;

      unsigned phase = findPrevRecvPhase (_curSendPhase - 1);
      char *rcv_buf = (phase != (unsigned)this->_astate.getLastCombinePhase())?
                      this->_astate.getPhaseRecvBufs (phase, 0) :  this->_dstbuf;
      CCMI_assert (rcv_buf != NULL);

      unsigned bufOffset = _curSendChunk * this->_astate.getPipelineWidth();

      unsigned count     = 0;
      if((_curSendChunk + npw) <= last_chunk)
        count = this->_astate.getFullChunkCount()*npw;
      else
        count =this->_astate.getFullChunkCount()*(npw-1)+this->_astate.getLastChunkCount();

//      Logging::LogMgr::getLogMgr()->stopCounter (_log_advancesend, 0);

      sendMessage (rcv_buf, bufOffset, count * this->_astate.getSizeOfType(),
                   this->_astate.getPhaseDstPes(_curSendPhase),
                   this->_astate.getPhaseNumDstPes(_curSendPhase),
                   (xmi_subtask_t *)this->_astate.getPhaseDstHints (_curSendPhase),
                   _curSendPhase);

//      Logging::LogMgr::getLogMgr()->startCounter (_log_advancesend);

      this->_astate.incrementPhaseChunksSent(_curSendPhase, npw);
    }
    //_curSendPhase ++;
    _curSendPhase = this->_astate.getNextActivePhase (_curSendPhase);

    if(_curSendPhase > _lastReducePhase)
    {
      _curSendPhase = this->_startPhase;
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

//  Logging::LogMgr::getLogMgr()->stopCounter (_log_advancesend, 1);
}

template<class T_Mcast, class T_Sysdep, class T_ConnectionManager>
inline void CCMI::Executor::PipelinedAllreduce<T_Mcast, T_Sysdep, T_ConnectionManager>::sendMessage
(const char             * buf,
 unsigned                 offset,
 unsigned                 bytes,
 unsigned               * dstpes,
 unsigned                 ndst,
 xmi_subtask_t                * dsthints,
 unsigned                 sphase)
{

  CCMI_assert (_curSendChunk   >= 0);
  CCMI_assert (_numActiveSends > 0);

  _numActiveSends --;

  TRACE_MSG ((stderr, "<%#.8X:%#.1X>Executor::PipelinedAllreduce::sendMessage _curSendChunk %d,_numActiveSends %d, offset %d,bytes %d\n",
              (int)this,ThreadID(),
              _curSendChunk,_numActiveSends, offset, bytes));

  int index = XMI_MAX_ACTIVE_SENDS - 1;
  if(!this->_sState[index].sndClientData.isDone)
  {
    while((index --) && (!this->_sState[index].sndClientData.isDone));
  }
  //we must find an available request
  CCMI_assert (index >= 0);

  if(offset == 0)
//    this->_msend_data.setFlags (MultiSend::CCMI_FLAGS_UNSET);
    this->_msend_data.flags=0; // (MultiSend::CCMI_FLAGS_UNSET);
  else
//    this->_msend_data.setFlags (MultiSend::CCMI_PERSISTENT_MESSAGE);
    this->_msend_data.flags=1; // (MultiSend::CCMI_PERSISTENT_MESSAGE);

//  _msend_data.setRequestBuffer (&(_sState[index].sndReq));
//  _msend_data.setCallback (_sendCallbackHandler,
//                           &_sState[index].sndClientData);
  this->_msend_data.request            = (&(this->_sState[index].sndReq));
  this->_msend_data.cb_done.function   = this->_sendCallbackHandler;
  this->_msend_data.cb_done.clientdata = &this->_sState[index].sndClientData;

  CCMI_assert (offset + bytes <= this->_astate.getBytes());

  AllreduceBase<T_Mcast, T_Sysdep>::sendMessage (buf + offset, bytes, dstpes, ndst, dsthints,
                                                 sphase, &this->_sState[index]);
}

template<class T_Mcast, class T_Sysdep, class T_ConnectionManager>
inline void CCMI::Executor::PipelinedAllreduce<T_Mcast, T_Sysdep, T_ConnectionManager>::advanceBcast ()
{
  unsigned bcastRecvPhase = this->_astate.getBcastRecvPhase();
  TRACE_MSG ((stderr, "<%#.8X:%#.1X>Executor::PipelinedAllreduce::advanceBcast "
              "bcastRecvPhase%d, _numActiveSends %d,"
              "this->_astate.getPhaseChunksRcvd(bcastRecvPhase, 0) %d,_numBcastChunksSent %d\n",
              (int)this,ThreadID(),
              bcastRecvPhase, _numActiveSends,
              this->_astate.getPhaseChunksRcvd(bcastRecvPhase, 0),_numBcastChunksSent));

  if(bcastRecvPhase < 0)
      return;

  if(_numActiveSends <= XMI_MAX_ACTIVE_SENDS/2)
      return;

  if(this->_astate.getPhaseChunksRcvd(bcastRecvPhase, 0) > _numBcastChunksSent)
  {
    unsigned * dstpes     = this->_astate.getBcastDstPes();
    xmi_subtask_t  * dsthints   = (xmi_subtask_t *) this->_astate.getBcastDstHints();
    unsigned   ndst       = this->_astate.getBcastNumDstPes();
    unsigned   last_chunk = this->_astate.getLastChunk();

    int npw = this->_astate.getPhaseChunksRcvd(bcastRecvPhase,0)-_numBcastChunksSent;
    int count = 0;
    int sizeOfType = this->_astate.getSizeOfType();

    if(this->_astate.getPhaseChunksRcvd (bcastRecvPhase, 0) <= last_chunk)
      count = npw * this->_astate.getFullChunkCount();
    else
      count = (npw - 1) * this->_astate.getFullChunkCount()
              + this->_astate.getLastChunkCount();

    unsigned bcast_offset = _numBcastChunksSent *
                            this->_astate.getPipelineWidth();

    _numBcastChunksSent += npw;

    sendMessage (this->_dstbuf, bcast_offset, count*sizeOfType,
                 dstpes, ndst, dsthints, this->_astate.getBcastSendPhase());
  }
}


#endif /* __simple_allreduce_executor_h__ */
