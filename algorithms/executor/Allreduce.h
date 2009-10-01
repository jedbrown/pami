/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/executor/Allreduce.h
 * \brief The basic/general [all]reduce executor
 */
#ifndef __allreduce_executor_h__
#define __allreduce_executor_h__

#include "./AllreduceBase.h"

namespace CCMI
{
  namespace Executor
  {

/// Pre-start initial phase (not the same as _state->getStartPhase()).  Not a valid index
/// into phaseVec and not in the range of _state->getStartPhase() ...  _state->getEndPhase()!
#define CCMI_KERNEL_EXECUTOR_ALLREDUCE_INITIAL_PHASE -1

/// Arbitrary maximum number of simultaneously active sends.  We reserve the last
/// one (CCMI_KERNEL_EXECUTOR_ALLREDUCE_INITIAL_SEND_INDEX) for the
/// initial (non-pipelined) startup send (if there is one defined in the schedule).
/// The others (CCMI_KERNEL_EXECUTOR_ALLREDUCE_MAX_PIPELINED_SENDS) are for
/// pipelined sends.
#define CCMI_KERNEL_EXECUTOR_ALLREDUCE_MAX_ACTIVE_SENDS 5
#define CCMI_KERNEL_EXECUTOR_ALLREDUCE_MAX_PIPELINED_SENDS 4
#define CCMI_KERNEL_EXECUTOR_ALLREDUCE_INITIAL_SEND_INDEX 4


    ///
    /// \brief Allreduce (and Reduce) basic general executor
    ///
    /// This should work for any reasonable schedule and tries not to make any
    /// assumptions about what's expected in the schedule other than :
    ///
    /// - send before receive in a phase.
    ///
    /// It may be used with expected or unexpected receive/multisend processing.
    ///
    /// For expected multisend processing, call postReceives() before calling start() on
    /// any rank.
    ///
    /// For unexpected multisend processing, call setupReceives() and create an
    /// unexpected callback function that calls notifyRecvHead() before calling start()
    /// on any rank.
    ///
    /// This executor stores persistent state/phase data, including buffer allocation,
    /// in an external class - AllreduceState.  The client should manage this class
    /// and free allocations when appropriate (AllreduceState::freeAllocations()).
    template<class T_Mcast, class T_Sysdep, class T_ConnectionManager>
    class Allreduce : public AllreduceBase<T_Mcast, T_Sysdep, T_ConnectionManager>
    {
    protected:

      // my persistent state data
      AllreduceState<T_ConnectionManager>*                        _state;

      unsigned _nextRecvData;

      /// client data for multisend send done callback
      typedef struct SendCallbackData_t
      {
        Allreduce      *me;
        bool            isDone;
      } SendCallbackData;

      // send info - use AllreduceBase sendState
//         XMI_Request_t   *_sndReq;
//         SendCallbackData *_sndClientData;
//         CollHeaderData   *_sndInfo;
      unsigned         _sndInfoRequired; // Is the info field required on send()?
      int              _numActiveSends;
      unsigned         _sendConnectionID;

      // current phase information
      int          _curRcvPhase;    // Index into phaseVec -- should point to a receive phase
      unsigned     _curRcvChunk;    // Count of pipelined chunks received for _curSrcPeIndex
      unsigned     _curSrcPeIndex;  // Current index into srcPes (< numSrcPes)
      int          _startRcvPhase;

      // flag - don't call advance from callbacks.  We're probably already in advance().
      int         _delayAdvance;

      /// \brief Member function to be called by multisend cb_done static function
      inline void notifySendDone(const xmi_quad_t &info);
      /// \brief Member function to be called by postRecv cb_done static function
      inline void notifyRecv(unsigned src, const xmi_quad_t &info, char * buf, unsigned bytes);
      /// \brief Advance the [all]reduce progress
      inline void advance();
    public:

      /// \brief Setup receive structures for both postReceives and async notifyRecvHead
      inline void reset()
      {
        TRACE_FLOW((stderr,"<%#.8X>Executor::Allreduce::reset() enter\n",(int)this));
        _state->resetPhaseData ();
        _state->setDstBuf (&this->_dstbuf);

        CCMI_assert(_curRcvPhase == CCMI_KERNEL_EXECUTOR_ALLREDUCE_INITIAL_PHASE);
        CCMI_assert(_state->getBytes() > 0);

        _nextRecvData = 0;
        _state->resetReceives(true /*_sndInfoRequired*/);
      }

      /// \brief Post receives for all expected src pe's
      inline void postReceives();

      /// \brief Static function to be passed into the done of multisend send
      static void staticNotifySendDone (void *context, void *cd, xmi_result_t err)
      {
        TRACE_FLOW((stderr,"<%#.8X>Executor::Allreduce::staticNotifySendDone() enter\n",(int)((SendCallbackData *)cd)->me));
        ((SendCallbackData *)cd)->me->notifySendDone (*(xmi_quad_t *)cd);
        TRACE_FLOW((stderr,"<%#.8X>Executor::Allreduce::staticNotifySendDone() exit\n",(int)((SendCallbackData *)cd)->me));
      }

      /// \brief Static function to be passed into the done of multisend postRecv

      static void staticNotifyReceiveDone (void *ctxt, void *cd, xmi_result_t err)
      {
        RecvCallbackData * cdata = (RecvCallbackData *)cd;
        TRACE_FLOW((stderr,"<%#.8X>Executor::Allreduce::staticNotifyReceiveDone() enter\n",(int)cdata->allreduce));
        xmi_quad_t *info = (xmi_quad_t *)cd;

        cdata->allreduce->notifyRecv((unsigned)-1, *info, NULL, (unsigned)-1);
        TRACE_FLOW((stderr,"<%#.8X>Executor::Allreduce::staticNotifyReceiveDone() exit\n",(int)cdata->allreduce));
      }

      /// \brief Default Destructor
      virtual ~Allreduce ()
      {
        TRACE_ALERT((stderr,"<%#.8X>Executor::Allreduce::dtor() ALERT:\n",(int)this));
#ifdef CCMI_DEBUG
        _nextRecvData=0;
        _sndInfoRequired=1;
        _numActiveSends=0;
        _curRcvPhase=CCMI_KERNEL_EXECUTOR_ALLREDUCE_INITIAL_PHASE;
        _curRcvChunk=0;
        _curSrcPeIndex=0;
        _startRcvPhase=0;
        _delayAdvance=0;
#endif
      }
      /// \brief Default Constructor
      inline Allreduce () :
        AllreduceBase<T_Mcast, T_Sysdep, T_ConnectionManager>(),
      _state(&this->_astate),
      _nextRecvData(0),
      //_sState[].sndReq        uninitialzed opaque storage
      //_sState[].sndClientData initialized at start
      //_sState[].sndInfo       initialized before each send
      _sndInfoRequired(1),
      _numActiveSends(0),
      //_sendConnectionID initialized below
      _curRcvPhase(CCMI_KERNEL_EXECUTOR_ALLREDUCE_INITIAL_PHASE),
      _curRcvChunk(0),
      _curSrcPeIndex(0),
      _startRcvPhase(0),
      _delayAdvance(0)
      {
        TRACE_ALERT((stderr,"<%#.8X>Executor::Allreduce::ctor() ALERT:\n",(int)this));
        TRACE_FLOW((stderr,"<%#.8X>Executor::Allreduce::ctor(void) enter\n",(int)this));

        this->_sendCallbackHandler = staticNotifySendDone;
        this->_recvCallbackHandler = staticNotifyReceiveDone;

        TRACE_INIT((stderr, "<%#.8X>Executor::Allreduce::ctor(void) _sendCallbackHandler %#X, _recvCallbackHandler %#X\n",(int)this,
                    (int)this->_sendCallbackHandler,(int) this->_recvCallbackHandler));

        TRACE_FLOW((stderr,"<%#.8X>Executor::Allreduce::ctor(void) exit\n",(int)this));
      }


      /// \brief  Main constructor to initialize the executor
      ///
      /// \param[in] connmgr the connection manager to use
      /// \param[in] state   the persistent state/phase data
      /// \param[in] comm    the communicator id of the collective
      inline Allreduce(T_Sysdep *map,
                       T_ConnectionManager  * connmgr,
                       xmi_consistency_t                        consistency,
                       const unsigned                          commID,
                       unsigned                                iteration) :
        AllreduceBase<T_Mcast, T_Sysdep, T_ConnectionManager>(map,connmgr,consistency,commID,iteration,true),
      _state(&this->_astate),
      _nextRecvData(0),
      //_sState[].sndReq        uninitialzed opaque storage
      //_sState[].sndClientData initialized at start
      //_sState[].sndInfo       initialized before each send
      _sndInfoRequired(1),
      _numActiveSends(0),
      //_sendConnectionID initialized below
      _curRcvPhase(CCMI_KERNEL_EXECUTOR_ALLREDUCE_INITIAL_PHASE),
      _curRcvChunk(0),
      _curSrcPeIndex(0),
      _startRcvPhase(0),
      _delayAdvance(0)
      {
        TRACE_ALERT((stderr,"<%#.8X>Executor::Allreduce::ctor() ALERT:\n",(int)this));
        TRACE_FLOW((stderr,"<%#.8X>Executor::Allreduce::ctor() enter\n",(int)this));
#ifdef CCMI_DEBUG
        TRACE_MSG((stderr, "<%#.8X>Executor::Allreduce::ctor() "
                   "Allreduce %X, AllreduceBase %X, Executor %X\n",(int)this,
                   sizeof(CCMI::Executor::Allreduce<T_Mcast,T_Sysdep,T_ConnectionManager>),
                   sizeof(CCMI::Executor::AllreduceBase<T_Mcast, T_Sysdep, T_ConnectionManager>),
                   sizeof(CCMI::Executor::Executor)));
        TRACE_DATA(("this",(const char*)this, sizeof(*this)));
#endif

        this->_sendCallbackHandler = staticNotifySendDone;
        this->_recvCallbackHandler = staticNotifyReceiveDone;

        TRACE_INIT((stderr, "<%#.8X>Executor::Allreduce::ctor(void) _sendCallbackHandler %#X, _recvCallbackHandler %#X\n",(int)this,
                    (int)this->_sendCallbackHandler,(int) this->_recvCallbackHandler));
        /// \todo could be moved to state
        this->_sendConnectionID = this->_rconnmgr->getConnectionId(this->_commID,
                                                                   (unsigned)-1,
                                                                   (unsigned)-1,
                                                                   _curRcvPhase,
                                                                   (unsigned)-1);

        TRACE_FLOW((stderr,"<%#.8X>Executor::Allreduce::ctor() exit\n",(int)this));
      }

      /// \brief  Set the actual parameters for this [all]reduce operation
      ///         and calculate some member data based on them.
      ///
      /// \param[in]  pipelineWidth
      /// \param[in]  srcbuf
      /// \param[in]  dstbuf
      /// \param[in]  count
      /// \param[in]  sizeOfType
      /// \param[in]  func            reduce function
      inline void setDataFunc(unsigned         pipelineWidth,
                              const void     * srcbuf,
                              void           * dstbuf,
                              unsigned         count,
                              unsigned         sizeOfType,
                              coremath  func,
                              xmi_op          op = XMI_UNDEFINED_OP,
                              xmi_dt          dt = XMI_UNDEFINED_DT)
      {
        TRACE_FLOW((stderr,"<%#.8X>Executor::Allreduce::setDataFunc() enter\n",(int)this));
        CCMI_assert(_curRcvPhase == CCMI_KERNEL_EXECUTOR_ALLREDUCE_INITIAL_PHASE);
        CCMI_assert(pipelineWidth % sizeOfType == 0);

        this->_srcbuf        = (char *)srcbuf;
        this->_dstbuf        = (char *)dstbuf;
        this->_reduceFunc    = func;

        _state->setDataFunc(pipelineWidth,count,sizeOfType,op,dt);

        TRACE_DATA(("srcbuf",(const char*)srcbuf, count*sizeOfType));
        TRACE_DATA(("dstbuf",(const char*)dstbuf, count*sizeOfType));

        TRACE_FLOW((stderr,"<%#.8X>Executor::Allreduce::setDataFunc() exit\n",(int)this));
      }

      /// \brief Set the root for reduction
      ///
      /// \param[in]  root  default/-1 indicates allreduce, any other
      ///                   root indicates reduce
      inline void setRoot(int root=-1)
      {
        _state->setRoot(root);
      }

      /// \brief Register the schedule
      ///
      /// \param[in]  sched
      inline void setSchedule(Schedule::Schedule *sched)
      {
        TRACE_FLOW((stderr,"<%#.8X>Executor::Allreduce::setSchedule() enter\n",(int)this));
        CCMI_assert(_curRcvPhase == CCMI_KERNEL_EXECUTOR_ALLREDUCE_INITIAL_PHASE);

        _state->setSchedule(sched);
        //_state->resetPhaseData();
        //_state->setDstBuf(&_dstbuf);

      }

      /// \brief Register the multicast interface
      ///
      /// \param[in]  mf
      inline void setMulticastInterface(T_Mcast * mf)
      {
        CCMI_assert(_curRcvPhase == CCMI_KERNEL_EXECUTOR_ALLREDUCE_INITIAL_PHASE);
        this->_msendInterface = mf;
      }

      /// \brief Start [all]reduce
      inline void start();

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
      inline XMI_Request_t *   notifyRecvHead(const xmi_quad_t  * info,
                                               unsigned          count,
                                               unsigned          peer,
                                               unsigned          sndlen,
                                               unsigned          conn_id,
                                               void            * arg,
                                               unsigned        * rcvlen,
                                               char           ** rcvbuf,
                                               unsigned        * pipewidth,
                                               XMI_Callback_t * cb_done);
      static inline void _compile_time_assert_ ()
      {
        // Compile time assert
        // SendState array must must fit in a request
        COMPILE_TIME_ASSERT((sizeof(CCMI::Executor::AllreduceBase<T_Mcast, T_Sysdep, T_ConnectionManager>::SendState)*CCMI_KERNEL_EXECUTOR_ALLREDUCE_MAX_ACTIVE_SENDS) <= sizeof(XMI_CollectiveRequest_t));
      }
    }; // Allreduce
  };
};// CCMI::Executor


template<class T_Mcast, class T_Sysdep, class T_ConnectionManager>
inline XMI_Request_t *
CCMI::Executor::Allreduce<T_Mcast, T_Sysdep, T_ConnectionManager>::notifyRecvHead(const xmi_quad_t    * info,
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
  CCMI_assert(!_delayAdvance); /// \todo Don't expect to receive within send() processing but
  /// this could change in the future

  CCMI_assert(count == 1);
  CCMI_assert(info);
  CollHeaderData *cdata = (CollHeaderData*) info;
  TRACE_MSG((stderr,"<%#.8X>Executor::Allreduce::notifyRecvHead() count: %#X "
             "_nextRecvData:%#X connID:%#X phase:%#X startPhase:%#X sndlen:%#X "
             "_state->getPipelineWidth():%#X \n",
             (int)this,
             count,
             _nextRecvData,
             conn_id,
             cdata->_phase,
             (unsigned)_state->getStartPhase(),
             sndlen, //_state->getBytes(),
             _state->getPipelineWidth()));
  CCMI_assert(cdata->_comm == this->_commID);
  CCMI_assert(cdata->_root == (unsigned) _state->getRoot());
//  CCMI_assert((cdata->_phase >= (unsigned)_state->getStartPhase()) && (cdata->_phase <= (unsigned)_state->getEndPhase()));
  CCMI_assert(cdata->_phase >= (unsigned)_state->getStartPhase());

  CCMI_assert(peer != _state->getMyRank());
  CCMI_assert(conn_id == this->_rconnmgr->getRecvConnectionId(cdata->_comm, (unsigned)-1, peer, _curRcvPhase, (unsigned)-1));
  CCMI_assert(arg && rcvlen && rcvbuf && pipewidth && cb_done);

  // Schedule misbehavior patch:
  // src and dst phases *should* match but some schedules
  // have problems and we'll try to tolerate it
  // Here, if the sender is sending something past our end phase,
  // assume they meant end phase and see if it works out.
  if(cdata->_phase > (unsigned)_state->getEndPhase()) cdata->_phase = (unsigned)_state->getEndPhase();

  // Schedule misbehavior patch:
  // src and dst phases *should* match but some schedules
  // send (dst) in phase n and receive (src) in phase n+1.
  // Allow this, I guess, by looking for the next src phase
  // and assuming that's the one we want.  This only works if
  // there are no src's in the specified phase.  If there are,
  // then we expect a match.
  while((!_state->getPhaseNumSrcPes(cdata->_phase)) && (cdata->_phase < (unsigned)_state->getEndPhase()))
  {
    TRACE_MSG((stderr, "<%#.8X>Executor::Allreduce::notifyRecvHead no src in phase %#X\n",
               (int)this,cdata->_phase));
    cdata->_phase++;
  }
  CCMI_assert(_state->getPhaseNumSrcPes(cdata->_phase));

  // Find the matching srcPeIndex for this peer in this phase.  (Usually 0 but there could be multiple
  //  src pe's).
  for(_state->setRecvClientSrcPeIndex( _nextRecvData,0);
     (_state->getPhaseSrcPes(cdata->_phase,_state->getRecvClientSrcPeIndex( _nextRecvData)) != peer) &&
     (_state->getRecvClientSrcPeIndex( _nextRecvData) < _state->getPhaseNumSrcPes(cdata->_phase));
     _state->incrementRecvClientSrcPeIndex( _nextRecvData))
    TRACE_MSG((stderr,"<%#.8X>Executor::Allreduce::notifyRecvHead(%#X) srcPeIndex : %#X srcPe:%#X\n",
               (int)this,
               peer,
               _state->getRecvClientSrcPeIndex( _nextRecvData ),
               _state->getPhaseSrcPes(cdata->_phase,_state->getRecvClientSrcPeIndex( _nextRecvData))));

  // Assert we found a match in less than numSrcPes
  CCMI_assert(_state->getPhaseSrcPes(cdata->_phase,_state->getRecvClientSrcPeIndex( _nextRecvData )) == peer);
  CCMI_assert(_state->getRecvClientSrcPeIndex( _nextRecvData ) < _state->getPhaseNumSrcPes(cdata->_phase));


  _state->setRecvClientAllreduce( _nextRecvData, this);
  _state->setRecvClientPhase( _nextRecvData, cdata->_phase);

  *rcvlen    = sndlen; //_state->getBytes();
  *pipewidth = _state->getPipelineWidth();

  // Get start of this peer's receive buffer and offset to the next pipeline width
  *rcvbuf    = _state->getPhaseRecvBufs(cdata->_phase,_state->getRecvClientSrcPeIndex( _nextRecvData )) +
               (_state->getPhaseChunksRcvd(cdata->_phase,_state->getRecvClientSrcPeIndex( _nextRecvData )) * (*pipewidth));

  cb_done->function   = staticNotifyReceiveDone;
  cb_done->clientdata = _state->getRecvClient(_nextRecvData);

  TRACE_MSG((stderr,"<%#.8X>Executor::Allreduce::notifyRecvHead() count: %#X _nextRecvData:%#X connID:%#X phase:%#X srcPeIndex :%#X chunks:%#X buf:%08X sndlen:%#X "
             "pipelineWidth:%#X hints:%#X\n",(int)this,
             count,
             _nextRecvData,
             conn_id,
             cdata->_phase,
             _state->getRecvClientSrcPeIndex( _nextRecvData ),
             _state->getPhaseChunksRcvd(cdata->_phase,_state->getRecvClientSrcPeIndex( _nextRecvData )),
             (unsigned)*rcvbuf,
             sndlen, //_state->getBytes(),
             _state->getPipelineWidth(),
             _state->getPhaseSrcHints(cdata->_phase,_state->getRecvClientSrcPeIndex( _nextRecvData ))));
  return _state->getRecvReq()+_nextRecvData++;
};

template<class T_Mcast, class T_Sysdep, class T_ConnectionManager>
inline void CCMI::Executor::Allreduce<T_Mcast, T_Sysdep, T_ConnectionManager>::postReceives()
{
  // If receives are posted, sends do not require the info field.
  _sndInfoRequired = 0;

  // Setup our buffers, allocate them if necessary
  //resetReceives();

  // post receives for each expected incoming message
  XMI_Callback_t cb_done;
  cb_done.function   = staticNotifyReceiveDone;
  for(int i = _state->getStartPhase(); i <= _state->getEndPhase(); i++)
  {
    for(unsigned j = 0; j < _state->getPhaseNumSrcPes(i); j++)
    {
      unsigned   srcrank = _state->getPhaseSrcPes(i,j);
      unsigned   connID  = this->_rconnmgr->getRecvConnectionId(this->_commID,
                                                                (unsigned)-1,
                                                                srcrank,
                                                                _curRcvPhase,
                                                                (unsigned)-1);
      char     * buf     = _state->getPhaseRecvBufs(i,j);

      _state->setRecvClientAllreduce( _nextRecvData, this);
      _state->setRecvClientPhase( _nextRecvData, i);
      _state->setRecvClientSrcPeIndex( _nextRecvData, j);
      cb_done.clientdata = _state->getRecvClient(_nextRecvData);

      this->_msendInterface->postRecv(_state->getRecvReq()+_nextRecvData,
                                      &cb_done,
                                      connID,
                                      buf,
                                      _state->getBytes(),
                                      _state->getPipelineWidth(),
                                      _state->getPhaseSrcHints(i,j),
                                      _state->getOp(), _state->getDt());

      TRACE_INIT((stderr,"<%#.8X>Executor::Allreduce::postReceives() connID:%#X phase:%#X srcPeIndex :%#X buf:%#.8X bytes:%#X "
                  "pipelineWidth:%#X hints:%#X\n",(int)this,
                  connID,i,j,(unsigned)buf,_state->getBytes(),_state->getPipelineWidth(),_state->getPhaseSrcHints(i,j)));
      _nextRecvData++;
    }
  }
  TRACE_FLOW((stderr,"<%#.8X>Executor::Allreduce::postReceives() exit\n",(int)this));
}

template<class T_Mcast, class T_Sysdep, class T_ConnectionManager>
inline void CCMI::Executor::Allreduce<T_Mcast, T_Sysdep, T_ConnectionManager>::start()
{
  TRACE_FLOW((stderr,"<%#.8X>Executor::Allreduce::start() enter\n",(int)this));
  CCMI_assert(_state->getSchedule());
  CCMI_assert(_state->getBytes() > 0);
  CCMI_assert(_curRcvPhase == CCMI_KERNEL_EXECUTOR_ALLREDUCE_INITIAL_PHASE);

  // reset send info
  for(int i = 0; i < CCMI_KERNEL_EXECUTOR_ALLREDUCE_MAX_ACTIVE_SENDS; i++)
  {
    this->_sState[i].sndClientData.me        = this;
    this->_sState[i].sndClientData.isDone    = true;
  }

  _numActiveSends   = 0;

  // The start phase should actually do something, otherwise fix the schedule
  CCMI_assert(_state->getPhaseNumDstPes( _state->getStartPhase()) || _state->getPhaseNumSrcPes( _state->getStartPhase() ));

  // Kick off - send local contribution if there is one.
  if(_state->getPhaseNumDstPes( _state->getStartPhase()) > 0)
  {
    // We should send to EVERY destination that needs it. So look through the schedule for
    // all phases with a dstPe and no srcPe (no new chunk).  Combine them into one
    // multisend.
    int nextSrcPhase = _state->getStartPhase();
    while((nextSrcPhase < _state->getEndPhase()) &&
          (_state->getPhaseNumSrcPes(nextSrcPhase) == 0))
    {
      // Since phase actions are send first and then receive, an empty
      // src in phase (x) means send phase (x+1):
      // Phase  numSrcPes numDstPes   Combined
      // 3      0         1           y
      // 4      0         0           y
      // 5      0         2           y
      // 6      1         1           y (send before receiving)
      // 7      0         1           n (not sent until 6 is received)
      //
      if(_state->getPhaseNumDstPes(nextSrcPhase+1) != 0)
      {
        TRACE_INIT((stderr, "<%#.8X>Executor::Allreduce::start() compressing phase %#X, phase %#X\n",
                    (int)this,_state->getStartPhase(),nextSrcPhase+1));
        // Move this DstPes back to nextActivePhase by manipulating numDstPes.
        /// \todo move to state?
        _state->compressPhaseNumDstPes(_state->getStartPhase(),nextSrcPhase+1);
      }
      ++nextSrcPhase;
    }

    XMI_Callback_t cb_done;
    unsigned sndIndex  = CCMI_KERNEL_EXECUTOR_ALLREDUCE_INITIAL_SEND_INDEX;

    cb_done.function   = staticNotifySendDone;
    cb_done.clientdata = &this->_sState[sndIndex].sndClientData;

    this->_sState[sndIndex].sndClientData.isDone = false;

    if(_sndInfoRequired)
    {
      // Setup the unexpected message header in case it's needed.
      this->_sState[sndIndex].sndInfo._comm = this->_commID;
      this->_sState[sndIndex].sndInfo._count  = _state->getCount();
      this->_sState[sndIndex].sndInfo._phase= _state->getStartPhase();
      this->_sState[sndIndex].sndInfo._dt = _state->getDt();
      this->_sState[sndIndex].sndInfo._op = _state->getOp();
      this->_sState[sndIndex].sndInfo._iteration = _state->getIteration();
      this->_sState[sndIndex].sndInfo._root = _state->getRoot();
    }

    CCMI_assert(!_delayAdvance);
    _delayAdvance = 1;  // Don't let send callback all the way back into advance()

    _numActiveSends++;
    TRACE_MSG((stderr,"<%#.8X>Executor::Allreduce::start() SEND <%#.8X> curphase:%#X curChunk:%#X "
               "numActiveSends:%#X bytes:%#X connID:%#X numDstPE:%#X dstPEs:%#X/%#X/%#X/%#X dstHints:%#X/%#X/%#X/%#X consistencey %X _sState[].sndInfo %#.8X "
               "comm %#X, dt %#X, op %#X, count %#X, iteration %#X, phase %#X, root %#X, "
               "_sState[].sndClientData %#.8X\n",
               (int)this,
               (int)&(this->_sState[sndIndex].sndReq),
               this->_curRcvPhase,this->_curRcvChunk,
               this->_numActiveSends,
               this->_state->getBytes(), this->_sendConnectionID,
               this->_state->getPhaseNumDstPes(this->_state->getStartPhase()),
               this->_state->getPhaseDstPes(this->_state->getStartPhase(),0),
               this->_state->getPhaseNumDstPes(this->_state->getStartPhase()) > 1? this->_state->getPhaseDstPes(this->_state->getStartPhase(),1):-1,
               this->_state->getPhaseNumDstPes(this->_state->getStartPhase()) > 2? this->_state->getPhaseDstPes(this->_state->getStartPhase(),2):-1,
               this->_state->getPhaseNumDstPes(this->_state->getStartPhase()) > 3? this->_state->getPhaseDstPes(this->_state->getStartPhase(),3):-1,
               this->_state->getPhaseDstHints(this->_state->getStartPhase(),0),
               this->_state->getPhaseNumDstPes(this->_state->getStartPhase()) > 1? this->_state->getPhaseDstHints(this->_state->getStartPhase(),1):-1,
               this->_state->getPhaseNumDstPes(this->_state->getStartPhase()) > 2? this->_state->getPhaseDstHints(this->_state->getStartPhase(),2):-1,
               this->_state->getPhaseNumDstPes(this->_state->getStartPhase()) > 3? this->_state->getPhaseDstHints(this->_state->getStartPhase(),3):-1,
               this->_consistency,
               (int)(this->_sndInfoRequired?&this->_sState[sndIndex].sndInfo:NULL),
               (this->_sndInfoRequired?this->_sState[sndIndex].sndInfo._comm:-1),
               (this->_sndInfoRequired?this->_sState[sndIndex].sndInfo._dt:-1),
               (this->_sndInfoRequired?this->_sState[sndIndex].sndInfo._op:-1),
               (this->_sndInfoRequired?this->_sState[sndIndex].sndInfo._count:-1),
               (this->_sndInfoRequired?this->_sState[sndIndex].sndInfo._iteration:-1),
               (this->_sndInfoRequired?this->_sState[sndIndex].sndInfo._phase:-1),
               (this->_sndInfoRequired?this->_sState[sndIndex].sndInfo._root:-1),
               (int)&(this->_sState[sndIndex].sndClientData)));

    this->_msendInterface->send(&this->_sState[sndIndex].sndReq,
                          &cb_done,
                          this->_consistency,
                          (xmi_quad_t *)(void *)(_sndInfoRequired?&this->_sState[sndIndex].sndInfo:NULL),
                          1,
                          this->_sendConnectionID,
                          this->_srcbuf,
                          this->_state->getBytes(),
                          this->_state->getPhaseDstHints(_state->getStartPhase()),
                          this->_state->getPhaseDstPes(_state->getStartPhase()),
                          this->_state->getPhaseNumDstPes(_state->getStartPhase()),
                          this->_state->getOp(), _state->getDt());
    _delayAdvance = 0;  // advance() again

  }

  /// \todo move to state?
  // Set current receive phase - find the first src pe
  _curRcvPhase    = _state->getStartPhase();
  while(!_state->getPhaseNumSrcPes( _curRcvPhase) && _curRcvPhase <= _state->getEndPhase()) _curRcvPhase++;
  _startRcvPhase = _curRcvPhase;

  // Initialize some member data
  _curRcvChunk    = 0;
  _curSrcPeIndex = 0;

  // Advance now in case the notifyRecv()'s are already done.  They
  // would have no-op'd before the start phase so they need a kick start now.  There
  // might not be another cb_done if we don't and it would stall.
  //
  // Also, we might have delayed a send done callback advance (_delayAdvance) so
  // always advance now.
  advance();

  TRACE_FLOW((stderr,"<%#.8X>Executor::Allreduce::start() exit\n",(int)this));
}

template<class T_Mcast, class T_Sysdep, class T_ConnectionManager>
void CCMI::Executor::Allreduce<T_Mcast, T_Sysdep, T_ConnectionManager>::notifyRecv(unsigned src, const xmi_quad_t & info, char * buf, unsigned bytes)
{
  RecvCallbackData * cdata = (RecvCallbackData *)(&info);
  unsigned rphase = cdata->phase;
  unsigned srcPeIndex  = cdata->srcPeIndex;
  TRACE_FLOW((stderr,"<%#.8X>Executor::Allreduce::notifyRecv() enter\n",(int)this));

  // update state - another chunk received
  _state->incrementPhaseChunksRcvd(rphase,srcPeIndex);
  TRACE_MSG((stderr,"<%#.8X>Executor::Allreduce::notifyRecv() rphase:%#X srcPeIndex :%#X pwidth:%#X "
             "chunksRcvd:%#X curSndIndex :%#X cur_cData:%#.8X\n",(int)this,
             rphase, srcPeIndex, _state->getPipelineWidth(), _state->getPhaseChunksRcvd(rphase,srcPeIndex),
             _curRcvChunk % (CCMI_KERNEL_EXECUTOR_ALLREDUCE_MAX_PIPELINED_SENDS),
             (unsigned)(&this->_sState[_curRcvChunk%(CCMI_KERNEL_EXECUTOR_ALLREDUCE_MAX_PIPELINED_SENDS)].sndClientData)));
  CCMI_assert((_curRcvPhase == CCMI_KERNEL_EXECUTOR_ALLREDUCE_INITIAL_PHASE) || (rphase >= (unsigned)_curRcvPhase));
  TRACE_DATA(("received buf",_state->getPhaseRecvBufs(rphase,srcPeIndex) + (_state->getPhaseChunksRcvd(rphase,srcPeIndex) * _state->getPipelineWidth()), (_state->getPhaseChunksRcvd(rphase,srcPeIndex) <  _state->getLastChunk()  ? _state->getFullChunkCount() : _state->getLastChunkCount())*_state->getSizeOfType()));
  advance();
  TRACE_FLOW((stderr,"<%#.8X>Executor::Allreduce::notifyRecv() exit\n",(int)this));
}

template<class T_Mcast, class T_Sysdep, class T_ConnectionManager>
void CCMI::Executor::Allreduce<T_Mcast, T_Sysdep, T_ConnectionManager>::notifySendDone(const xmi_quad_t & info)
{
  TRACE_FLOW((stderr,"<%#.8X>Executor::Allreduce::notifySendDone() enter\n",(int)this));

  // update state - this send is done
  SendCallbackData * cdata = (SendCallbackData *)(&info);
  cdata->isDone = true;
  _numActiveSends--;

  TRACE_MSG((stderr,"<%#.8X>Executor::Allreduce::notifySendDone() curphase:%#X curChunk:%#X "
             "curReduceIndex :%#X cdata:%#.8X numActiveSends:%#X "
             "curSndIndex :%#X cur_cData:%#.8X\n",(int)this,
             _curRcvPhase,_curRcvChunk,_curSrcPeIndex, (unsigned)cdata, _numActiveSends,
             _curRcvChunk % (CCMI_KERNEL_EXECUTOR_ALLREDUCE_MAX_PIPELINED_SENDS),
             (unsigned)(&this->_sState[_curRcvChunk%(CCMI_KERNEL_EXECUTOR_ALLREDUCE_MAX_PIPELINED_SENDS)].sndClientData)));

  advance();
  TRACE_FLOW((stderr,"<%#.8X>Executor::Allreduce::notifySendDone() exit\n",(int)this));
}

template<class T_Mcast, class T_Sysdep, class T_ConnectionManager>
void CCMI::Executor::Allreduce<T_Mcast, T_Sysdep, T_ConnectionManager>::advance()
{
  // Don't advance.  Probably in advance up the stack and doing a callback.  Delay it.
  if(_delayAdvance)
  {
    TRACE_ADVANCE((stderr,"<%#.8X>Executor::Allreduce::advance() delay\n",(int)this));
    return;
  }

  // Don't advance until the phases have been start()'d.  (We might receive data before start().)
  if(_curRcvPhase == CCMI_KERNEL_EXECUTOR_ALLREDUCE_INITIAL_PHASE)
  {
    TRACE_ADVANCE((stderr,"<%#.8X>Executor::Allreduce::advance() not started\n",(int)this));
    return;
  }


  // Don't advance (reduce) MPI_IN_PLACE until any initial send completes or we may reduce right
  // over what we're (still) sending.
  if((this->_srcbuf ==this->_dstbuf) &&                      // MPI_IN_PLACE
     !(this->_sState[CCMI_KERNEL_EXECUTOR_ALLREDUCE_INITIAL_SEND_INDEX].sndClientData.isDone) // initial send is not done
    )
  {
    TRACE_ADVANCE((stderr,"<%#.8X>Executor::Allreduce::advance() MPI_IN_PLACE send not done\n",(int)this));
    return;
  }

  TRACE_FLOW((stderr,"<%#.8X>Executor::Allreduce::advance() enter\n",(int)this));

  // We just modulo our way through our array of send data structures.  Max active sends
  // is just an arbitrary limit of how many sends we want at one time.   One of the sends is
  // reserved for the initial send in start(), so we use -1 on the max.
  unsigned curSndIndex = _curRcvChunk % CCMI_KERNEL_EXECUTOR_ALLREDUCE_MAX_PIPELINED_SENDS;

  TRACE_ADVANCE((stderr,"<%#.8X>Executor::Allreduce::advance() curSndIndex %#X, isDone %#X,"
                 " curRcvPhase %#X _state->getEndPhase() %#X"
                 " _curSrcPeIndex %#X numSrcPes %#X chunksRcvd %#X > _curRcvChunk %#X\n",(int)this,
                 curSndIndex,
                 this->_sState[curSndIndex].sndClientData.isDone,
                 _curRcvPhase,
                 _state->getEndPhase(),
                 _curSrcPeIndex,
                 _state->getPhaseNumSrcPes(_curRcvPhase),
                 (_curRcvPhase <= _state->getEndPhase()) && _state->getPhaseNumSrcPes(_curRcvPhase)? _state->getPhaseChunksRcvd(_curRcvPhase,_curSrcPeIndex): -1,
                 _curRcvChunk));
  // If our current active send is done and our current receive phase received some data, then
  // do reduction and maybe another send
  while(this->_sState[curSndIndex].sndClientData.isDone &&
        _curRcvPhase <= _state->getEndPhase() &&
        _state->getPhaseChunksRcvd(_curRcvPhase,_curSrcPeIndex) > _curRcvChunk)
  {
    unsigned count     = _curRcvChunk <  _state->getLastChunk()  ? _state->getFullChunkCount() : _state->getLastChunkCount();
    unsigned bufOffset = _curRcvChunk * _state->getPipelineWidth();
    unsigned numSrc    = _state->getPhaseNumSrcPes(_curRcvPhase);
    // Don't touch the dstbuf unless we're the root (or an allreduce)
    char * reduceBuf   = ((_state->getRoot() == -1) | (_state->getRoot() == (int)_state->getMyRank()))?
                         this->_dstbuf : _state->getTempBuf();

    // Reduce, if the schedule tells us to, otherwise we're just receiving or maybe multisend
    // did the reduce for us.
    if(_state->getPhaseSrcHints(_curRcvPhase,_curSrcPeIndex) == XMI_COMBINE_SUBTASK)
    {
      //char * localbuf    = _curRcvPhase == _startRcvPhase ? (char*)_srcbuf : reduceBuf;
      //Suggested change from BRC as a fix for ShortRectangle.h (SK 07/29/08)
      char * localbuf    = ((_curRcvPhase == _startRcvPhase) && (_curSrcPeIndex == 0)) ?
                           (char*)this->_srcbuf : reduceBuf;

      TRACE_REDUCEOP((stderr,"<%#.8X>Executor::Allreduce::advance() OP curphase:%#X curChunk:%#X "
                      "curSrcIndex :%#X func(%#X), count = %#X\n",(int)this,
                      _curRcvPhase, _curRcvChunk, _curSrcPeIndex,
                      (int)this->_reduceFunc,count));
      TRACE_DATA(("localbuf",localbuf  + bufOffset, count*_state->getSizeOfType()));
      TRACE_DATA(("input buf",_state->getPhaseRecvBufs(_curRcvPhase,_curSrcPeIndex) + bufOffset, count*_state->getSizeOfType()));
      TRACE_DATA(("reduceBuf",reduceBuf + bufOffset, 1));  // Just to trace the input pointer
      void *bufs[2] = { localbuf + bufOffset,
        _state->getPhaseRecvBufs(_curRcvPhase,_curSrcPeIndex) + bufOffset,
      };
      this->_reduceFunc(reduceBuf + bufOffset,
                  bufs, 2,
                  count);
      TRACE_DATA(("reduceBuf",reduceBuf + bufOffset, count*_state->getSizeOfType()));
      TRACE_REDUCEOP((stderr,"<%#.8X>Executor::Allreduce::advance() OP curphase:%#X curChunk:%#X "
                      "curSrcIndex :%#X reduceBuf[0]:%#X\n",(int)this,
                      _curRcvPhase, _curRcvChunk, _curSrcPeIndex, *(unsigned *)(reduceBuf+bufOffset)));
    } //(_state->getPhaseSrcHints(_curRcvPhase,0) == CCMI::Schedule::COMBINE_SUBTASK)

    // Move to the next src in this phase
    _curSrcPeIndex++;

    // if current chunk is completed for all srcPe's this phase, move to the next chunk.
    if(_curSrcPeIndex >= numSrc)
    {
      _curSrcPeIndex = 0;

      // Skip empty phases, see what's next to do
      int nextActivePhase = _curRcvPhase + 1;
      while((nextActivePhase <= _state->getEndPhase()) &&
            (_state->getPhaseNumSrcPes(nextActivePhase) == 0) &&
            (_state->getPhaseNumDstPes(nextActivePhase) == 0))
        nextActivePhase++;

      // Send chunk to the next destination(s) if we found one in the next active phase
      if((nextActivePhase <= _state->getEndPhase()) && (_state->getPhaseNumDstPes(nextActivePhase) > 0))
      {
        // Since we are (partially) driven by receiving "chunks", we need to send this
        // chunk to EVERY destination that needs it. So look through the schedule for
        // all phases with a dstPe and no srcPe (no new chunk).  Combine them into one
        // multisend.
        int nextSrcPhase = nextActivePhase;
        while((nextSrcPhase < _state->getEndPhase()) &&
              (_state->getPhaseNumSrcPes(nextSrcPhase) == 0))
        {
          // Since phase actions are send first and then receive, an empty
          // src in phase (x) means send phase (x+1):
          // Phase  numSrcPes numDstPes   Combined
          // 3      0         1           y
          // 4      0         0           y
          // 5      0         2           y
          // 6      1         1           y (send before receiving)
          // 7      0         1           n (not sent until 6 is received)
          //
          if(_state->getPhaseNumDstPes(nextSrcPhase+1) != 0)
          {
            // Move this DstPes back to nextActivePhase by manipulating numDstPes.
            /// \todo move to state?
            TRACE_INIT((stderr, "<%#.8X>Executor::Allreduce::advance() compressing phase %#X, phase %#X\n",
                        (int)this,nextActivePhase, nextSrcPhase+1));
            _state->compressPhaseNumDstPes(nextActivePhase, nextSrcPhase+1);
          }
          ++nextSrcPhase;
        }
        // nextSrcPhase should point to a src phase or be > endphase. Never point
        // to a valid non-src phase.  i.e. handle this:
        // Phase  numSrcPes numDstPes   Combined
        // 6      0         1           y
        // 7      0         1           y
        //
        // _state->getEndPhase() = 7.  nextSrcPhase = 7 from the loop above.  Increment it.
        if((nextSrcPhase == _state->getEndPhase()) && (_state->getPhaseNumSrcPes(nextSrcPhase) == 0))
          ++nextSrcPhase;
        CCMI_assert((nextSrcPhase > _state->getEndPhase()) || _state->getPhaseNumSrcPes(nextSrcPhase));

        if(_sndInfoRequired)
        {
          this->_sState[curSndIndex].sndInfo._comm  = this->_commID;
          this->_sState[curSndIndex].sndInfo._count   =this-> _state->getCount();
          this->_sState[curSndIndex].sndInfo._phase = nextActivePhase;
          this->_sState[curSndIndex].sndInfo._dt    = this->_state->getDt();
          this->_sState[curSndIndex].sndInfo._op    = this->_state->getOp();
          this->_sState[curSndIndex].sndInfo._iteration = this->_state->getIteration();
          this->_sState[curSndIndex].sndInfo._root  = this->_state->getRoot();
        }

        XMI_Callback_t cb_done;
        this->_sState[curSndIndex].sndClientData.isDone = false;
        cb_done.function   = staticNotifySendDone;
        cb_done.clientdata =  &(this->_sState[curSndIndex].sndClientData);

        CCMI_assert(!_delayAdvance);
        _delayAdvance = 1;  // Don't let send callback all the way back into advance()

        TRACE_MSG((stderr,"<%#.8X>Executor::Allreduce::advance() SEND <%#.8X>  phase:%#X curChunk:%#X curSndIndex :%#X "
                   "numActiveSends:%#X bytes:%#X connID:%#X numDstPE:%#X dstPEs:%#X/%#X/%#X/%#X dstHints:%#X/%#X/%#X/%#X "
                   "consistencey %X _sState.sndInfo %#.8X "
                   "comm %#X, dt %#X, op %#X, count %#X, iteration %#X, phase %#X, root %#X, "
                   "_sState[].sndClientData %#.8X\n",
                   (int)this,
                   (int)&this->_sState[curSndIndex].sndReq,
                   nextActivePhase, _curRcvChunk, curSndIndex, _numActiveSends,
                   count * _state->getSizeOfType(), _sendConnectionID,
                   _state->getPhaseNumDstPes(nextActivePhase),
                   _state->getPhaseDstPes(nextActivePhase,0),
                   _state->getPhaseNumDstPes(nextActivePhase) > 1? _state->getPhaseDstPes(nextActivePhase,1):-1,
                   _state->getPhaseNumDstPes(nextActivePhase) > 2? _state->getPhaseDstPes(nextActivePhase,2):-1,
                   _state->getPhaseNumDstPes(nextActivePhase) > 3? _state->getPhaseDstPes(nextActivePhase,3):-1,
                   _state->getPhaseDstHints(nextActivePhase,0),
                   _state->getPhaseNumDstPes(nextActivePhase) > 1? _state->getPhaseDstHints(nextActivePhase,1):-1,
                   _state->getPhaseNumDstPes(nextActivePhase) > 2? _state->getPhaseDstHints(nextActivePhase,2):-1,
                   _state->getPhaseNumDstPes(nextActivePhase) > 3? _state->getPhaseDstHints(nextActivePhase,3):-1,
                   this->_consistency,
                   (int)(_sndInfoRequired?&this->_sState[curSndIndex].sndInfo:NULL),
                   (_sndInfoRequired?this->_sState[curSndIndex].sndInfo._comm:-1),
                   (_sndInfoRequired?this->_sState[curSndIndex].sndInfo._dt:-1),
                   (_sndInfoRequired?this->_sState[curSndIndex].sndInfo._op:-1),
                   (_sndInfoRequired?this->_sState[curSndIndex].sndInfo._count:-1),
                   (_sndInfoRequired?this->_sState[curSndIndex].sndInfo._iteration:-1),
                   (_sndInfoRequired?this->_sState[curSndIndex].sndInfo._phase:-1),
                   (_sndInfoRequired?this->_sState[curSndIndex].sndInfo._root:-1),
                   (int)&this->_sState[curSndIndex].sndClientData));

        this->_msendInterface->send(&this->_sState[curSndIndex].sndReq,
                              &cb_done,
                              this->_consistency,
                              (xmi_quad_t *)(void *)(_sndInfoRequired?&this->_sState[curSndIndex].sndInfo:NULL),
                              1,
                              _sendConnectionID,
                              reduceBuf + bufOffset,
                              count * _state->getSizeOfType(),
                              _state->getPhaseDstHints(nextActivePhase),
                              _state->getPhaseDstPes(nextActivePhase),
                              _state->getPhaseNumDstPes(nextActivePhase),
                              _state->getOp(), _state->getDt());

        _numActiveSends++;
        nextActivePhase = nextSrcPhase;

        _delayAdvance = 0;  // advance() again

      } //((nextActivePhase <= _state->getEndPhase()) && (_state->getPhaseNumDstPes(nextActivePhase) > 0))
      _curRcvChunk++;

      // Remember, we send before we receive in a phase.  So this phase must be complete if
      // we received all our chunks in this phase.  Enter the next phase
      if(_curRcvChunk > _state->getLastChunk())
      {
        _curRcvPhase = nextActivePhase;
        _curRcvChunk = 0;
      }
      curSndIndex = _curRcvChunk % CCMI_KERNEL_EXECUTOR_ALLREDUCE_MAX_PIPELINED_SENDS;
    } // (_curSrcPeIndex >= numSrc)

    TRACE_ADVANCE((stderr,"<%#.8X>Executor::Allreduce::advance() curSndIndex %#X, isDone %#X,"
                   " curRcvPhase %#X _state->getEndPhase() %#X"
                   " _curSrcPeIndex %#X chunksRcvd %#X > _curRcvChunk %#X\n",(int)this,
                   curSndIndex,
                   this->_sState[curSndIndex].sndClientData.isDone,
                   _curRcvPhase,
                   _state->getEndPhase(),
                   _curSrcPeIndex,
                   (_curRcvPhase <= _state->getEndPhase()) && _state->getPhaseNumSrcPes(_curRcvPhase)? _state->getPhaseChunksRcvd(_curRcvPhase,_curSrcPeIndex): -1,
                   _curRcvChunk));
  } //while(_sState[curSndIndex].sndClientData.isDone && _curRcvPhase <= _state->getEndPhase() && ...


  // If we have nothing to receive this phase (we must have completed a send),
  // then find our next active receive phase or determine if our contribution is done
  if(_curRcvPhase <= _state->getEndPhase() && _state->getPhaseNumSrcPes(_curRcvPhase) == 0)
    while((++_curRcvPhase <= _state->getEndPhase()) && (_state->getPhaseNumSrcPes(_curRcvPhase) == 0))
      // We shouldn't find a send since it was handled in the loop above.
      CCMI_assert_debug(_state->getPhaseNumDstPes(_curRcvPhase) == 0);


  TRACE_ADVANCE((stderr,"<%#.8X>Executor::Allreduce::advance() curphase:%#X curChunk:%#X "
                 "_curSrcPeIndex :%#X curSndIndex :%#X\n",(int)this,
                 _curRcvPhase,_curRcvChunk,_curSrcPeIndex,curSndIndex));

  // Are we done with both sends and receives?
  if(_numActiveSends == 0 && _curRcvPhase > _state->getEndPhase())
  {
    TRACE_ADVANCE((stderr,"<%#.8X>Executor::Allreduce::advance() DONE %#X/%#X\n",(int)this,
                   (int)this->_cb_done, (int)this->_clientdata));
    if(this->_cb_done) (*this->_cb_done)(NULL, this->_clientdata, XMI_SUCCESS);
    _curRcvPhase = CCMI_KERNEL_EXECUTOR_ALLREDUCE_INITIAL_PHASE; // executer is done
    if((_state->getRoot() == -1) | (_state->getRoot() == (int)_state->getMyRank()))
      TRACE_DATA(("_dstbuf",this->_dstbuf, _state->getBytes()));
    return;
  }

  TRACE_FLOW((stderr,"<%#.8X>Executor::Allreduce::advance() exit\n",(int)this));
}

#endif /* __allreduce_executor_h__ */
