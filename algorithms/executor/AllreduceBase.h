/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/executor/AllreduceBase.h
 * \brief ???
 */

#ifndef __allreduce_base_executor_h__
#define __allreduce_base_executor_h__

#include "algorithms/schedule/Schedule.h"
#include "algorithms/executor/Executor.h"
#include "algorithms/connmgr/ConnectionManager.h"
#include "util/logging/LogMgr.h"
#include "util/ccmi_debug.h"
#include "math/math_coremath.h"
#include "./AllreduceState.h"

namespace CCMI
{
  namespace Executor
  {
    template<class T_Mcastinterface, class T_Mcast, class T_Mcastrecv, class T_Sysdep>
    class AllreduceBase : public Executor
    {
    public:
      struct SendState
      {
        CollHeaderData    sndInfo __attribute__((__aligned__(16)));
        SendCallbackData  sndClientData;
        XMI_Request_t    sndReq __attribute__((__aligned__(16)));
      } __attribute__((__aligned__(16)));
    private:
      /// \brief Static function to be passed into the done of multisend send
      static void staticNotifySendDone (void *cd, xmi_result_t *err)
      {
        SendCallbackData * cdata = ( SendCallbackData *)cd;
        xmi_quad_t *info = (xmi_quad_t *)cd;
        TRACE_FLOW((stderr,"<%#.8X>Executor::AllreduceBase::staticNotifySendDone() enter\n",(int)cdata->me));
        ((AllreduceBase *)(cdata->me))->AllreduceBase::notifySendDone(*info);
        TRACE_FLOW((stderr,"<%#.8X>Executor::AllreduceBase::staticNotifySendDone() exit\n",(int)cdata->me));
      }

      /// \brief Static function to be passed into the done of multisend postRecv
      static void staticNotifyReceiveDone (void *cd, xmi_result_t *err)
      {
        RecvCallbackData * cdata = (RecvCallbackData *)cd;
        TRACE_FLOW((stderr,"<%#.8X>Executor::AllreduceBase::staticNotifyReceiveDone() enter\n",(int)cdata->allreduce));
        xmi_quad_t *info = (xmi_quad_t *)cd;

        ((AllreduceBase *)cdata->allreduce)->AllreduceBase::notifyRecv((unsigned)-1, *info, NULL, (unsigned)-1);
        TRACE_FLOW((stderr,"<%#.8X>Executor::AllreduceBase::staticNotifyReceiveDone() exit\n",(int)cdata->allreduce));
      }

      static void short_recv_done (void *me, xmi_result_t *)
      {
        AllreduceBase *allreduce = (AllreduceBase *)me;
        TRACE_FLOW((stderr,"<%#.8X>Executor::AllreduceBase::short_recv_done enter\n", (int)allreduce));
        if(allreduce->_sState && allreduce->_sState->sndClientData.isDone)  //send has finished
          allreduce->advance();
        TRACE_FLOW((stderr,"<%#.8X>Executor::AllreduceBase::short_recv_done exit\n", (int)allreduce));
      }

    protected:
      typedef void (*Callback_t) (void *, xmi_result_t *);

      void inline_math_isum (void *dst, void *src1, void *src2, xmi_op op, xmi_dt dt, unsigned count)
      {
        int *idst  = (int *) dst;
        int *isrc1 = (int *) src1;
        int *isrc2 = (int *) src2;

        for(unsigned c = 0; c < count; c++)
          idst[c] = isrc1[c] + isrc2[c];
      }

      void inline_math_dsum (void *dst, void *src1, void *src2, xmi_op op, xmi_dt dt, unsigned count)
      {
        double *idst  = (double *) dst;
        double *isrc1 = (double *) src1;
        double *isrc2 = (double *) src2;

        for(unsigned c = 0; c < count; c++)
          idst[c] = isrc1[c] + isrc2[c];
      }

        void inline_math_dmin (void *dst, void *src1, void *src2, xmi_op op, xmi_dt dt, unsigned count)
        {
          double *idst  = (double *) dst;
          double *isrc1 = (double *) src1;
          double *isrc2 = (double *) src2;

          for(unsigned c = 0; c < count; c++)
            idst[c] = (isrc1[c] < isrc2[c]) ? isrc1[c] : isrc2[c];
        }

        void inline_math_dmax (void *dst, void *src1, void *src2, xmi_op op, xmi_dt dt, unsigned count)
        {
          double *idst  = (double *) dst;
          double *isrc1 = (double *) src1;
          double *isrc2 = (double *) src2;

          for(unsigned c = 0; c < count; c++)
            idst[c] = (isrc1[c] > isrc2[c]) ? isrc1[c] : isrc2[c];
        }

      void advance ();

      void sendMessage (const char         * buffer,
                        unsigned             size,
                        unsigned           * dstPes,
                        unsigned             ndstPes,
                        xmi_subtask_t      * hints,
                        unsigned             phase,
                        SendState          * state);

      /// State variables
      unsigned            _commID;
      unsigned            _curPhase;
      unsigned            _curIdx;
      unsigned            _startPhase;
      unsigned            _endPhase;
      unsigned            _firstCombinePhase;
      unsigned            _nAsyncRcvd;  /// number of messages received in
                                        /// the async callback
      bool                _initialized;
      bool                _postReceives;
      bool                _enablePipelining;

      SendState         * _sState;
      const char        * _srcbuf;
      char              * _dstbuf;

      coremath     _reduceFunc;

      //void (*_sendCallbackHandler) (void*, xmi_result_t *);
      //void (*_recvCallbackHandler) (void*, xmi_result_t *);

      Callback_t         _sendCallbackHandler;
      Callback_t         _recvCallbackHandler;

      T_Mcastinterface  * _msendInterface;
      ConnectionManager::ConnectionManager<T_Sysdep> * _rconnmgr;  ///Reduce connection manager
      ConnectionManager::ConnectionManager<T_Sysdep> * _bconnmgr;  ///Broadcast connction manager

      T_Mcast             _msend_data;
      AllreduceState<T_Mcastrecv, T_Sysdep>                         _astate;

      ///
      /// \brief Ids to the LogMgr table
      ///
      unsigned        _log_postrecv;
      unsigned        _log_advance;
      unsigned        _log_sendmessage;
    public:

      /// Default Destructor
      virtual ~AllreduceBase ()
      {
        TRACE_ALERT((stderr,"<%#.8X>Executor::AllreduceBase::dtor() ALERT:\n",(int)this));
#ifdef XMI_DEBUG
        _curPhase=(unsigned) -1;
        _curIdx=(unsigned) -1;
        _startPhase=(unsigned) -1;
        _endPhase=(unsigned) -1;
        _firstCombinePhase = (unsigned)-1;
        _nAsyncRcvd=0;
        _initialized=false;
        _postReceives=false;
        _enablePipelining=false;
        _sState=NULL;
        _srcbuf=NULL;
        _dstbuf=NULL;
        _reduceFunc=NULL;
        _msendInterface=NULL;
        _rconnmgr=NULL;
        _bconnmgr=NULL;
#endif
      }

      /// Default Constructor
      AllreduceBase () :
      Executor (),
      _curPhase ((unsigned) -1),
      _curIdx ((unsigned) -1),
      _startPhase ((unsigned) -1), _endPhase ((unsigned) -1),
      _firstCombinePhase((unsigned) -1),
      _nAsyncRcvd (0),
      _initialized (false), _postReceives (false),
      _enablePipelining (false),
      _sState (NULL),
      _srcbuf (NULL), _dstbuf (NULL),
      _reduceFunc (NULL),
      _msendInterface (NULL), _rconnmgr (NULL), _bconnmgr(NULL), _msend_data(),
      _astate(-1,XMI_UNDEFINED_RANK)
      {
        TRACE_ALERT((stderr,"<%#.8X>Executor::AllreduceBase::ctor() ALERT:\n",(int)this));
      }

      ///  Main constructor to initialize the executor
      ///  By default it only needs one connection manager
      AllreduceBase(T_Sysdep *map,
                    ConnectionManager::ConnectionManager<T_Sysdep>  * connmgr,
                    xmi_consistency_t                       consistency,
                    const unsigned                          commID,
                    unsigned                                iteration,
                    bool                                    enable_pipe=false):
      Executor(),
      _commID (commID), _curPhase ((unsigned) -1), _curIdx ((unsigned) -1),
      _startPhase ((unsigned) -1),
      _endPhase ((unsigned) -1),
      _nAsyncRcvd (0),
      _initialized (false), _postReceives (false),
      _enablePipelining (enable_pipe),
      _sState (NULL),
      _srcbuf (NULL), _dstbuf (NULL),
      _reduceFunc (NULL),
      _msendInterface (NULL), _rconnmgr (connmgr), _bconnmgr(connmgr), _msend_data(),
      _astate(iteration, map->rank())
      {
        TRACE_ALERT((stderr,"<%#.8X>Executor::AllreduceBase::ctor() ALERT:\n",(int)this));

        // Base executor has the consistency defined
        setConsistency(consistency);

        _msend_data.setConsistency (consistency);
        _msend_data.setFlags (CCMI_FLAGS_UNSET);
        _msend_data.setInfo (NULL, 0);
        _astate.setReduceConnectionManager (connmgr);
        _astate.setBroadcastConnectionManager (connmgr);

        Logging::LogMgr *lmgr = Logging::LogMgr::getLogMgr();
        _log_postrecv    = lmgr->registerEvent ("AllreduceBase post recv");
        _log_advance    = lmgr->registerEvent ("AllreduceBase advance");
        _log_sendmessage = lmgr->registerEvent ("Allreduce send message");

        _sendCallbackHandler = staticNotifySendDone;
        _recvCallbackHandler = staticNotifyReceiveDone;

        _astate.setExecutor (this);
        _astate.setCommID (commID);
      }

      void setSendState (XMI_CollectiveRequest_t* storage)
      {
        // See _compile_time_assert_() for storage assertions
        _sState = (SendState*)storage;

        TRACE_INIT((stderr, "<%#.8X>Executor::AllreduceBase::setSendState(%#.8X) _sndReq %#.8X, _sndClientData %#.8X, _sndInfo %#.8X\n",(int)this,
                    (int)storage,(int)&_sState->sndReq,(int)&_sState->sndClientData,(int)&_sState->sndInfo));

        // Must be 16 byte aligned
        XMI_assert ((((unsigned long)&_sState->sndReq) & 0x0f)== 0);
        XMI_assert ((((unsigned long)&_sState->sndInfo) & 0x0f)== 0);

        _msend_data.setRequestBuffer (&(_sState->sndReq));

        _msend_data.setCallback (_sendCallbackHandler,
                                 &_sState->sndClientData);
      }

      void setSchedule (Schedule::Schedule *schedule, unsigned color=0)
      {
        _astate.setSchedule(schedule);
        _astate.setColor(color);
      }

      void setReduceConnectionManager (ConnectionManager::ConnectionManager<T_Sysdep> *cmgr)
      {
        _rconnmgr = cmgr;
        _astate.setReduceConnectionManager (cmgr);
      }

      void setBroadcastConnectionManager (ConnectionManager::ConnectionManager<T_Sysdep> *cmgr)
      {
        _bconnmgr = cmgr;
        _astate.setBroadcastConnectionManager (cmgr);
      }

      ///entry method
      virtual void notifySendDone( const xmi_quad_t &info );

      ///entry method
      virtual void notifyRecv(unsigned src, const xmi_quad_t &info,
                              char * buf, unsigned bytes);

      /// entry method : start allreduce
      virtual void start();

      /// \brief Handle async receive headers
      /// \brief  Set the actual parameters for this [all]reduce operation
      ///         and calculate some member data based on them.
      ///
      /// \param[in]   info       Our header (includes fields for:
      ///                           root of the collective
      ///                           communicator
      ///                           length of the message
      ///                           phase of the collective
      /// \param[in]   count      Count of quads in info
      /// \param[in]   peer       Sender's rank
      /// \param[in]   sndlen     Message length
      /// \param[in]   conn_id    Connection id
      /// \param[in]   arg        Not used
      /// \param[out]  rcvlen     size of rcvbuf
      /// \param[out]  rcvbuf     buffer to receive datatype
      /// \param[out]  pipeWidth  pipeline width
      /// \param[out]  cb_done    receive callback function
      ///
      virtual XMI_Request_t *   notifyRecvHead(const xmi_quad_t  * info,
                                                unsigned          count,
                                                unsigned          peer,
                                                unsigned          sndlen,
                                                unsigned          conn_id,
                                                void            * arg,
                                                unsigned        * rcvlen,
                                                char           ** rcvbuf,
                                                unsigned        * pipewidth,
                                                XMI_Callback_t * cb_done);

      ///
      ///  \fast callback for short allreduce operations
      ///  This callback does not return a request
      ///
      inline void notifyRecvShort           (unsigned          phase,
                                             unsigned          sndlen,
                                             unsigned          srcindex,
                                             unsigned        * rcvlen,
                                             char           ** rcvbuf,
                                             XMI_Callback_t * cb_done);

      ///
      /// \brief Set the buffer info for the allreduce collective
      ///        These members are always set.
      /// \param[in]  srcbuf
      /// \param[in]  dstbuf
      /// Note setDataInfo should be called before setReduceInfo!!
      ///

      inline void setDataInfo ( const void     * srcbuf,
                                void           * dstbuf )
      {
        _srcbuf        = (char *) srcbuf;
        _dstbuf        = (char *) dstbuf;
        //          _astate.setDstBuf (&_dstbuf);
      }

      /// \ brief Reset the final receive buffer to be the dstbuf.  This is
      /// an abbreviated reset() and reset() should have already been called.
      ///
      inline void resetDstBuf()
      {
        _astate.setDstBuf (&_dstbuf);
      }

      /// \brief Set the parameters related to the reduce.  When
      ///  parameters change the allreduce state changes.
      ///
      /// \param[in]  count
      /// \param[in]  pipelineWidth
      /// \param[in]  sizeOfType
      /// \param[in]  func            reduce function

      inline void setReduceInfo( unsigned         count,
                                 unsigned         pipelineWidth,
                                 unsigned         sizeOfType,
                                 coremath  func,
                                 xmi_op          op = XMI_UNDEFINED_OP,
                                 xmi_dt          dt = XMI_UNDEFINED_DT)
      {
        TRACE_INIT((stderr,"<%#.8X>Executor::AllreduceBase::setReduceInfo() "
                    "count %#X, pipelineWidth %#X, sizeOfType %#X, func %#X, op %#X, dt %#X\n",
                    (int)this,count,pipelineWidth,sizeOfType,(int)func,op,dt));
        XMI_assert (pipelineWidth % sizeOfType == 0);

        _reduceFunc    = func;

        _msend_data.setReduceInfo (op, dt);

        //override the pipeline width as we do not support
        //pipelining in this simple executor.
        if(!_enablePipelining)
          pipelineWidth = count * sizeOfType;

        _astate.setDataFunc(pipelineWidth,count,sizeOfType,op,dt);

        TRACE_INIT((stderr,"<%#.8X>Executor::AllreduceBase::setReduceInfo() exit\n",
                    (int)this));
      }

      Callback_t  getRecvCallbackHandler ()
      {
        return _recvCallbackHandler;
      }

      ///
      ///  \brief Set the mode of collective to be synchronous using post receives
      ///
      void setPostReceives ()
      {
        _postReceives = true;
      }

      /// \brief Set the root for reduction
      ///
      /// \param[in]  root  default/-1 indicates allreduce, any other
      ///                   root indicates reduce
      inline void setRoot(int root=-1)
      {
        _astate.setRoot(root);
      }

      /// \brief Get the root for reduction
      ///
      /// \return  root -1 indicates allreduce, any other
      ///                   root indicates reduce root
      inline int getRoot()
      {
        return _astate.getRoot();
      }

      /// \brief Get the async iteration id
      ///
      /// \return  iteration id
      inline unsigned getIteration()
      {
        return _astate.getIteration();
      }

      void postReceives ();
      //void postRecv (unsigned p, unsigned next);

      /// \brief Register the multicast interface
      ///
      /// \param[in]  mf
      void setMulticastInterface(T_Mcastinterface * mf)
      {
        _msendInterface = mf;
      }

      T_Mcastinterface *getMulticastInterface()
      {
        return _msendInterface;
      }

      virtual void reset ()
      {
        _astate.resetPhaseData ();
        _astate.setDstBuf (&_dstbuf);

        _startPhase = _curPhase = _astate.getStartPhase();
        _curIdx = 0;
        _endPhase = _astate.getEndPhase();
        _initialized = false;
        _nAsyncRcvd = 0;

        _astate.resetReceives(false);

        _firstCombinePhase = _startPhase;
        while(!_astate.getPhaseNumSrcPes(_firstCombinePhase) && _firstCombinePhase <= _endPhase)
          _firstCombinePhase++;

        //Reduce might not have a combine phase (on non-roots)
        if((_astate.getRoot()!=-1) && (_firstCombinePhase > _endPhase))
          _firstCombinePhase = _startPhase;

        XMI_assert (_firstCombinePhase <= _endPhase);
      }

      void operator delete(void * p)
      {
        XMI_abort();
      }

      ///
      /// \bf Query functions
      ///
      inline AllreduceState<T_Mcastrecv, T_Sysdep> * getAllreduceState ()
      {
        return &_astate;
      }

      inline xmi_op    getOp ()
      {
        return  _astate.getOp();
      }
      inline xmi_dt    getDt ()
      {
        return  _astate.getDt();
      }
      inline unsigned   getCount ()
      {
        return  _astate.getCount();
      }
      static inline void _compile_time_assert_ ()
      {
#if 0
        // Compile time assert
        // SendState storage must must fit in a request
        COMPILE_TIME_ASSERT(sizeof(CCMI::Executor::AllreduceBase::SendState) <= sizeof(XMI_CollectiveRequest_t));
#endif
      }
    }; // AllreduceBase
  } // Executor
} // CCMI


/////////////////////////////////////////////
///   Protected Methods
/////////////////////////////////////////////
template<class T_Mcastinterface, class T_Mcast, class T_Mcastrecv, class T_Sysdep>
inline void CCMI::Executor::AllreduceBase<T_Mcastinterface, T_Mcast, T_Mcastrecv, T_Sysdep>::advance ()
{

  Logging::LogMgr::getLogMgr()->startCounter (_log_advance);

  TRACE_MSG ((stderr, "<%#.8X>Executor::AllreduceBase::advance _curPhase %d,_endPhase %d,_curIdx %d\n", (int) this,
              _curPhase,_endPhase,_curIdx));

  XMI_assert_debug (_initialized);
  XMI_assert_debug (_sState->sndClientData.isDone == true);

  char * reducebuf = NULL;

  //Allreduce or I am the root
  if((_astate.getRoot() == -1) ||
     (_astate.getRoot() == (int)_astate.getMyRank()))
    reducebuf = _dstbuf;
  else
    //Reduce operation and I am not the root
    reducebuf = _astate.getTempBuf();

  while(_curPhase <= _endPhase)
  {
    unsigned nsrcpes = _astate.getPhaseNumSrcPes(_curPhase);
    void *src1 = (void *) (((_curIdx==0)&&(_curPhase==_firstCombinePhase)) ? _srcbuf : reducebuf);
    void *src2 = NULL;

    xmi_op op = _astate.getOp();
    xmi_dt dt = _astate.getDt();
    unsigned count = _astate.getCount();

    if(op == XMI_SUM && dt == XMI_SIGNED_INT)
    {
      for(; (_curIdx < nsrcpes) && (_astate.getPhaseChunksRcvd(_curPhase, _curIdx) > 0); _curIdx ++)
      {
        if(_astate.getPhaseSrcHints(_curPhase, _curIdx) == XMI_COMBINE_SUBTASK)
        {
          src2 = _astate.getPhaseRecvBufs (_curPhase, _curIdx);
          inline_math_isum (reducebuf, src1, src2, op, dt, count);
          src1 = reducebuf;
        }
      }
    }
    else if(op == XMI_SUM && dt == XMI_DOUBLE)
    {
      for(; (_curIdx < nsrcpes) && (_astate.getPhaseChunksRcvd(_curPhase, _curIdx) > 0); _curIdx ++)
      {
        if(_astate.getPhaseSrcHints(_curPhase, _curIdx) == XMI_COMBINE_SUBTASK)
        {
          src2 = _astate.getPhaseRecvBufs (_curPhase, _curIdx);
          inline_math_dsum (reducebuf, src1, src2, op, dt, count);
          src1 = reducebuf;
        }
      }
    }
    else if(op == XMI_MIN && dt == XMI_DOUBLE)
    {
      for(; (_curIdx < nsrcpes)
            && (_astate.getPhaseChunksRcvd(_curPhase, _curIdx) > 0);
          _curIdx ++)
      {
        if(_astate.getPhaseSrcHints(_curPhase,_curIdx) == XMI_COMBINE_SUBTASK)
        {
          src2 = _astate.getPhaseRecvBufs (_curPhase, _curIdx);
          inline_math_dmin (reducebuf, src1, src2, op, dt, count);
          src1 = reducebuf;
        }
      }
    }
    else if(op == XMI_MAX && dt == XMI_DOUBLE)
    {
      for(; (_curIdx < nsrcpes)
            && (_astate.getPhaseChunksRcvd(_curPhase, _curIdx) > 0);
          _curIdx ++)
      {
        if(_astate.getPhaseSrcHints(_curPhase,_curIdx) == XMI_COMBINE_SUBTASK)
        {
          src2 = _astate.getPhaseRecvBufs (_curPhase, _curIdx);
          inline_math_dmax (reducebuf, src1, src2, op, dt, count);
          src1 = reducebuf;
        }
      }
    }
    else
    {
      for(; (_curIdx < nsrcpes) && (_astate.getPhaseChunksRcvd(_curPhase, _curIdx) > 0); _curIdx ++)
      {
        if(_astate.getPhaseSrcHints(_curPhase, _curIdx) == XMI_COMBINE_SUBTASK)
        {
          src2 =  _astate.getPhaseRecvBufs (_curPhase, _curIdx);
          void * bufs[2];
          bufs[0] = src1;
          bufs[1] = src2;
          TRACE_DATA(("localbuf",(char*)bufs[0], count*_astate.getSizeOfType()));
          TRACE_DATA(("input buf",(char*)bufs[1], count*_astate.getSizeOfType()));
          TRACE_DATA(("reduceBuf",(char*)reducebuf, 1));  // Just to trace the input pointer
          _reduceFunc( reducebuf, (void **)bufs, 2, count);

          TRACE_DATA(("reduceBuf",(char*)reducebuf, count*_astate.getSizeOfType()));
          src1 = reducebuf;
        }
      }
    }

    if(_curIdx != nsrcpes) //we are waiting for more data
      break;

    if(_curPhase == _endPhase)
    {
      // Call application done callback
      if(_cb_done)
        _cb_done (_clientdata, NULL);

      break;
    }

    _curIdx = 0; //Start the next phase
    _curPhase ++;

    unsigned ndstpes = _astate.getPhaseNumDstPes (_curPhase);
    if(ndstpes > 0)
    {
      unsigned *dstpes   = _astate.getPhaseDstPes (_curPhase);
      xmi_subtask_t *dsthints  = (xmi_subtask_t *)_astate.getPhaseDstHints (_curPhase);
      sendMessage (reducebuf, _astate.getBytes(), dstpes, ndstpes, dsthints,
                   _curPhase, _sState);
      //wait for send to finish
      break;
    }
  }

  Logging::LogMgr::getLogMgr()->stopCounter (_log_advance, 1);
}


///
///  \brief Send the next message by calling the msend interface
///
template<class T_Mcastinterface, class T_Mcast, class T_Mcastrecv, class T_Sysdep>
inline void CCMI::Executor::AllreduceBase<T_Mcastinterface, T_Mcast, T_Mcastrecv, T_Sysdep>::sendMessage
(const char             * buf,
 unsigned                 bytes,
 unsigned               * dstpes,
 unsigned                 ndst,
 xmi_subtask_t                * dsthints,
 unsigned                 sphase,
 SendState              * s_state)
{
  //Request buffer and callback set in setSendState !!
  XMI_assert (ndst > 0);
  XMI_assert (dstpes != NULL);
  XMI_assert (dsthints != NULL);

  _msend_data.setConnectionId (_astate.getPhaseSendConnectionId (sphase));
  _msend_data.setSendData (buf, bytes);
  _msend_data.setRanks (dstpes, ndst);
  _msend_data.setOpcodes (dsthints);

  //the message has been sent and send state slot is unavailable
  s_state->sndClientData.isDone = false;

  if(!_postReceives)
  {
    s_state->sndInfo._comm    = _commID;
    s_state->sndInfo._count   = _astate.getCount();
    s_state->sndInfo._phase   = sphase;
    s_state->sndInfo._dt      = _astate.getDt();
    s_state->sndInfo._op      = _astate.getOp();
    s_state->sndInfo._iteration = _astate.getIteration();
    s_state->sndInfo._root    = (unsigned)_astate.getRoot();
    _msend_data.setInfo ((xmi_quad_t *)(void *)&s_state->sndInfo, 1);
  }

  TRACE_MSG ((stderr, "<%#.8X>Executor::AllreduceBase::sendMessage connid %#X curphase:%#X "
              "bytes:%#X destPe:%#X dstHints %#X ndst %#X cData:%#.8X \n",
              (int) this,
              _astate.getPhaseSendConnectionId (sphase),
              sphase, bytes, dstpes[0], dsthints[0], ndst,
              (unsigned) &s_state->sndInfo));

  Logging::LogMgr::getLogMgr()->startCounter (_log_sendmessage);

  TRACE_DATA(("_msend_data",buf, bytes));
  _msendInterface->send (&_msend_data);

  Logging::LogMgr::getLogMgr()->stopCounter (_log_sendmessage, 1);
}

////////////////////////////////////////////////////////
///
///  Public methods that can be called externally
///
////////////////////////////////////////////////////////
template<class T_Mcastinterface, class T_Mcast, class T_Mcastrecv, class T_Sysdep>
inline void CCMI::Executor::AllreduceBase<T_Mcastinterface, T_Mcast, T_Mcastrecv, T_Sysdep>::start()
{
  _initialized = true;
  _sState->sndClientData.me        = this;
  _sState->sndClientData.isDone    = true;

  //XMI_assert_debug (_startPhase != (unsigned) -1);
  //XMI_assert_debug (_curPhase != (unsigned) -1);

  // Skip bogus initial phase(s)
  while(!_astate.getPhaseNumDstPes (_curPhase) && !_astate.getPhaseNumSrcPes (_curPhase))
    ++_curPhase;

  unsigned ndstpes = _astate.getPhaseNumDstPes (_curPhase);
  if(ndstpes)
  {
    unsigned *dstpes   = _astate.getPhaseDstPes (_curPhase);
    xmi_subtask_t *dsthints  = (xmi_subtask_t *)_astate.getPhaseDstHints (_curPhase);
    sendMessage (_srcbuf, _astate.getBytes(), dstpes, ndstpes, dsthints,
                 _curPhase, _sState);
  }
  else
    advance ();

  TRACE_INIT ((stderr,"<%#.8X>Executor::AllreduceBase start()\n",(int)this));
}

template<class T_Mcastinterface, class T_Mcast, class T_Mcastrecv, class T_Sysdep>
inline void CCMI::Executor::AllreduceBase<T_Mcastinterface, T_Mcast, T_Mcastrecv, T_Sysdep>::notifyRecv
(unsigned                     src,
 const xmi_quad_t             & info,
 char                       * buf,
 unsigned                     bytes)
{
  RecvCallbackData * cdata = (RecvCallbackData *)(&info);

  TRACE_MSG ((stderr, "<%#.8X>Executor::AllreduceBase::notifyRecv %#X, %#X\n",
              (int) this, cdata->phase, cdata->srcPeIndex));

  // update state  (we dont support multiple sources per phase yet)
  _astate.incrementPhaseChunksRcvd(cdata->phase, 0);

  //The send for the previous phase has finished and we have received
  //data for the current phase
  if(cdata->phase == _curPhase      &&  //got the data for my phase
     _sState && _sState->sndClientData.isDone  &&  //send has finished
     _initialized)
    advance();
}

template<class T_Mcastinterface, class T_Mcast, class T_Mcastrecv, class T_Sysdep>
inline void CCMI::Executor::AllreduceBase<T_Mcastinterface, T_Mcast, T_Mcastrecv, T_Sysdep>::notifySendDone
( const xmi_quad_t & info)
{
  // update state
  TRACE_MSG((stderr, "<%#.8X>Executor::AllreduceBase::notifySendDone, cur phase %#X\n",
             (int)this, _curPhase));

  SendCallbackData * cdata = (SendCallbackData *)(&info);
  cdata->isDone = true;

  //Verify both send and receive have arrived for that phase before
  //calling advance
  advance ();
}

template<class T_Mcastinterface, class T_Mcast, class T_Mcastrecv, class T_Sysdep>
inline void CCMI::Executor::AllreduceBase<T_Mcastinterface, T_Mcast, T_Mcastrecv, T_Sysdep>::postReceives ()
{
  Logging::LogMgr::getLogMgr()->startCounter (_log_postrecv);

  // post receives for each expected incoming message
  _postReceives = true;

  for(unsigned p = _startPhase; p <= _endPhase; p++)
  {
    if(_astate.getPhaseNumSrcPes(p) > 0)
    {
      T_Mcastrecv *recv = _astate.getPhaseMcastRecv (p,0);
      recv->cb_done.function = _recvCallbackHandler;
      _msendInterface->postRecv(recv);
    }
  }

  Logging::LogMgr::getLogMgr()->stopCounter (_log_postrecv);

  TRACE_INIT((stderr,"<%#.8X>Executor::AllreduceBase::postReceives() exit\n",
              (int)this));
}

template<class T_Mcastinterface, class T_Mcast, class T_Mcastrecv, class T_Sysdep>
inline XMI_Request_t *
CCMI::Executor::AllreduceBase<T_Mcastinterface, T_Mcast, T_Mcastrecv, T_Sysdep>::notifyRecvHead
(const xmi_quad_t  * info,
 unsigned          count,
 unsigned          peer,
 unsigned          sndlen,
 unsigned          conn_id,
 void            * arg,
 unsigned        * rcvlen,
 char           ** rcvbuf,
 unsigned        * pipewidth,
 XMI_Callback_t * cb_done)
{
  XMI_assert_debug(count == 1);
  XMI_assert_debug(info);
  CollHeaderData *cdata = (CollHeaderData*) info;

  TRACE_MSG((stderr,"<%#.8X>Executor::AllreduceBase::notifyRecvHead() count: %#X "
             "connID:%#X phase:%#X root:%#X local root:%#X _state->getBytes():%#X "
             "_state->getPipelineWidth():%#X \n",
             (int)this,
             count,
             conn_id,
             cdata->_phase,
             (int) cdata->_root,
             _astate.getRoot(),
             _astate.getBytes(),
             _astate.getPipelineWidth()));

  XMI_assert_debug(cdata->_comm == _commID);
  XMI_assert_debug(cdata->_root == (unsigned) _astate.getRoot());

  XMI_assert_debug(cdata->_phase >= (unsigned)_astate.getStartPhase());
  //XMI_assert_debug(conn_id == _rconnmgr->getRecvConnectionId(cdata->_comm, (unsigned)-1, peer, cdata->_phase, (unsigned) -1));

  XMI_assert(arg && rcvlen && rcvbuf && pipewidth && cb_done);
  //XMI_assert (cdata->_phase <= _endPhase);

  if(cdata->_phase > _endPhase)
    cdata->_phase = _endPhase;

  if(!_astate.getPhaseNumSrcPes(cdata->_phase))
  {
    // Schedule misbehavior patch:
    // src and dst phases *should* match but some schedules
    // send (dst) in phase n and receive (src) in phase n+1.
    // Allow this, I guess, by looking for the next src phase
    // and assuming that's the one we want.  This only works if
    // there are no src's in the specified phase.  If there are,
    // then we expect a match.
    while((!_astate.getPhaseNumSrcPes(cdata->_phase)) &&
          (cdata->_phase < (unsigned)_endPhase))
    {
      TRACE_MSG((stderr, "<%#.8X>Executor::AllreduceBase::notifyRecvHead "
                 "no src in phase %#X\n",
                 (int)this,cdata->_phase));
      cdata->_phase++;
    }
  }
  XMI_assert(_astate.getPhaseNumSrcPes(cdata->_phase) > 0);

  TRACE_MSG ((stderr, "<%#.8X>Executor::AllreduceBase::notifyRecvHead phase %#X, numsrcpes %#X\n", (int) this,
              cdata->_phase, _astate.getPhaseNumSrcPes(cdata->_phase)));

  // In this executor we only support one receive per phase
  // Assert we found a match in less than numSrcPes
  // Later we need to compute the srcpeindex
  unsigned srcPeIndex = 0;
  XMI_assert(_astate.getPhaseSrcPes(cdata->_phase, srcPeIndex) == peer);

  unsigned index = 0;
  unsigned nchunks = 0;

  //This is the fist message from that source
  if((nchunks = _astate.getPhaseChunksRcvd (cdata->_phase, srcPeIndex)) == 0)
    index = _nAsyncRcvd;
  else
  {
    for(index = 0; index < _nAsyncRcvd; index ++)
      // match phase and src rank
      if(_astate.getRecvClientPhase(index) == cdata->_phase &&
         _astate.getRecvClientSrcPeIndex(index) == srcPeIndex)
        break;

    XMI_assert (index < _nAsyncRcvd);
  }

  RecvCallbackData *rdata = _astate.getRecvClient(index);
  if(index == _nAsyncRcvd)
  {
    rdata->allreduce =  this;
    rdata->phase     =  cdata->_phase;
    rdata->srcPeIndex = srcPeIndex;

    _nAsyncRcvd ++;
  }

  * rcvlen    = sndlen;
  //* rcvlen = _astate.getBytes();
  * rcvbuf    = _astate.getPhaseRecvBufs(cdata->_phase, srcPeIndex) +
                nchunks * _astate.getPipelineWidth();;
  * pipewidth = _astate.getPipelineWidth();

  cb_done->function   = _recvCallbackHandler;
  cb_done->clientdata = rdata;

  return _astate.getRecvReq() + index;
}


///
///  \fast callback for short allreduce operations
///  This callback does not return a request
///
template<class T_Mcastinterface, class T_Mcast, class T_Mcastrecv, class T_Sysdep>
inline void
CCMI::Executor::AllreduceBase<T_Mcastinterface, T_Mcast, T_Mcastrecv, T_Sysdep>::notifyRecvShort
(unsigned          phase,
 unsigned          sndlen,
 unsigned          srcindex,
 unsigned        * rcvlen,
 char           ** rcvbuf,
 XMI_Callback_t * cb_done)
{
  *rcvbuf = _astate.getPhaseRecvBufs (phase, srcindex);
  *rcvlen = sndlen;

  ///This is a short callback and at the end of it the data is
  ///guarangeed to be copied in
  _astate.incrementPhaseChunksRcvd(phase, srcindex);

  unsigned nsrcpes = _astate.getPhaseNumSrcPes(phase);

  TRACE_FLOW ((stderr, "notifyRecvShort phase = %d, srcindex = %d, nsrcpes = %d, totalchunksrcvd %d, recvbuf = %#X, len = %d\n",
               phase, srcindex, nsrcpes, _astate.getPhaseTotalChunksRcvd(_curPhase),
               (int)*rcvbuf,
               *rcvlen));

  cb_done->function = NULL;
  ///Trigger advance on the last packet
  if(_astate.getPhaseTotalChunksRcvd(_curPhase) == nsrcpes)
  {
    cb_done->function   = short_recv_done; //short recv callback
    cb_done->clientdata = this;
  }
}



#endif /* __simple_allreduce_executor_h__ */
