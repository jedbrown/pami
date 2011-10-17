/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/executor/AllreduceBaseExec.h
 * \brief ???
 */
#ifndef __algorithms_executor_AllreduceBaseExec_h__
#define __algorithms_executor_AllreduceBaseExec_h__

#include <pami.h>
#include "algorithms/interfaces/Schedule.h"
#include "algorithms/interfaces/Executor.h"
#include "algorithms/connmgr/ConnectionManager.h"
#include "algorithms/interfaces/NativeInterface.h"
#include "algorithms/executor/ScheduleCache.h"
#include "algorithms/executor/AllreduceCache.h"

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

namespace CCMI
{
  namespace Executor
  {
    template <class T_Conn>
    class AllreduceBaseExec : public Interfaces::Executor
    {
      public:
        struct SendState
        {
          CollHeaderData    sndInfo __attribute__((__aligned__(16)));
          AC_SendCallbackData  sndClientData;
        } __attribute__((__aligned__(16)));
      private:
        /// \brief Static function to be passed into the done of multisend send
        static void staticNotifySendDone (pami_context_t context, void *cd, pami_result_t err)
        {
          TRACE_FN_ENTER();
          AC_SendCallbackData * cdata = ( AC_SendCallbackData *)cd;
          pami_quad_t *info = (pami_quad_t *)cd;
          TRACE_FORMAT("<%p>", cdata->me);
          ((AllreduceBaseExec<T_Conn> *)(cdata->me))->AllreduceBaseExec<T_Conn>::notifySendDone(*info);
          TRACE_FN_EXIT();
        }

        /// \brief Static function to be passed into the done of multisend postRecv
        static void staticNotifyReceiveDone (pami_context_t context, void *cd, pami_result_t err)
        {
          TRACE_FN_ENTER();
          AC_RecvCallbackData * cdata = (AC_RecvCallbackData *)cd;
          TRACE_FORMAT("<%p>", cdata->allreduce);
          pami_quad_t *info = (pami_quad_t *)cd;

          ((AllreduceBaseExec<T_Conn> *)cdata->allreduce)->AllreduceBaseExec<T_Conn>::notifyRecv((unsigned) - 1, *info, NULL, NULL);
          TRACE_FN_EXIT();
        }

        static void short_recv_done (pami_context_t context, void *me, pami_result_t res)
        {
          TRACE_FN_ENTER();
          AllreduceBaseExec<T_Conn> *allreduce = (AllreduceBaseExec<T_Conn> *)me;
          TRACE_FORMAT("<%p>", allreduce);

          if (allreduce->_sState.sndClientData.isDone) //send has finished
            allreduce->advance();

          TRACE_FN_EXIT();
        }

      protected:
        void inline_math_isum (void *dst, void *src1, void *src2, unsigned count)
        {
          TRACE_FN_ENTER();
          int *idst  = (int *) dst;
          int *isrc1 = (int *) src1;
          int *isrc2 = (int *) src2;

          TRACE_HEXDATA((char*)src1, count*_acache.getSizeOfType());
          TRACE_HEXDATA((char*)src2, count*_acache.getSizeOfType());
          TRACE_HEXDATA((char*)dst, 1); // Just to trace the input pointer

          for (unsigned c = 0; c < count; c++)
            idst[c] = isrc1[c] + isrc2[c];

          TRACE_HEXDATA((char*)dst, count*_acache.getSizeOfType());
          TRACE_FN_EXIT();
        }

        void inline_math_dsum (void *dst, void *src1, void *src2, unsigned count)
        {
          TRACE_FN_ENTER();
          double *idst  = (double *) dst;
          double *isrc1 = (double *) src1;
          double *isrc2 = (double *) src2;

          TRACE_HEXDATA((char*)src1, count*_acache.getSizeOfType());
          TRACE_HEXDATA((char*)src2, count*_acache.getSizeOfType());
          TRACE_HEXDATA((char*)dst, 1); // Just to trace the input pointer

          for (unsigned c = 0; c < count; c++)
            idst[c] = isrc1[c] + isrc2[c];

          TRACE_HEXDATA((char*)dst, count*_acache.getSizeOfType());
          TRACE_FN_EXIT();
        }

        void inline_math_dmin (void *dst, void *src1, void *src2, unsigned count)
        {
          TRACE_FN_ENTER();
          double *idst  = (double *) dst;
          double *isrc1 = (double *) src1;
          double *isrc2 = (double *) src2;

          TRACE_HEXDATA((char*)src1, count*_acache.getSizeOfType());
          TRACE_HEXDATA((char*)src2, count*_acache.getSizeOfType());
          TRACE_HEXDATA((char*)dst, 1); // Just to trace the input pointer

          for (unsigned c = 0; c < count; c++)
            idst[c] = (isrc1[c] < isrc2[c]) ? isrc1[c] : isrc2[c];

          TRACE_HEXDATA((char*)dst, count*_acache.getSizeOfType());
          TRACE_FN_EXIT();
        }

        void inline_math_dmax (void *dst, void *src1, void *src2, unsigned count)
        {
          TRACE_FN_ENTER();
          double *idst  = (double *) dst;
          double *isrc1 = (double *) src1;
          double *isrc2 = (double *) src2;

          TRACE_HEXDATA((char*)src1, count*_acache.getSizeOfType());
          TRACE_HEXDATA((char*)src2, count*_acache.getSizeOfType());
          TRACE_HEXDATA((char*)dst, 1); // Just to trace the input pointer

          for (unsigned c = 0; c < count; c++)
            idst[c] = (isrc1[c] > isrc2[c]) ? isrc1[c] : isrc2[c];

          TRACE_HEXDATA((char*)dst, count*_acache.getSizeOfType());
          TRACE_FN_EXIT();
        }

        void advance ();

        void sendMessage (const char         * buffer,
                          unsigned             size,
                          PAMI::Topology      * dst_topology,
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

        SendState           _sState;
        const char        * _srcbuf;
        char              * _dstbuf;

        coremath            _reduceFunc;

        //void (*_sendCallbackHandler) (void*, PAMI_Error_t *);
        //void (*_recvCallbackHandler) (void*, PAMI_Error_t *);

        pami_event_function         _sendCallbackHandler;
        pami_event_function         _recvCallbackHandler;

        T_Conn * _rconnmgr;  ///Reduce connection manager
        T_Conn * _bconnmgr;  ///Broadcast connction manager

        Interfaces::NativeInterface    * _native;
        pami_multicast_t                  _msend;
        PAMI::Topology                    _selftopology;

        ScheduleCache                          _scache;
        AllreduceCache<T_Conn>                 _acache;

      public:

        /// Default Destructor
        virtual ~AllreduceBaseExec ()
        {
          TRACE_FN_ENTER();
#ifdef CCMI_DEBUG
          _curPhase = (unsigned) - 1;
          _curIdx = (unsigned) - 1;
          _startPhase = (unsigned) - 1;
          _endPhase = (unsigned) - 1;
          _firstCombinePhase = (unsigned) - 1;
          _nAsyncRcvd = 0;
          _initialized = false;
          _postReceives = false;
          _enablePipelining = false;
          _srcbuf = NULL;
          _dstbuf = NULL;
          _reduceFunc = NULL;
          _native = NULL;
          _rconnmgr = NULL;
          _bconnmgr = NULL;
#endif
          TRACE_FORMAT("<%p>", this);
          TRACE_FN_EXIT();
        }

        /// Default Constructor
        AllreduceBaseExec () :
            Interfaces::Executor (),
            _curPhase ((unsigned) - 1),
            _curIdx ((unsigned) - 1),
            _firstCombinePhase((unsigned) - 1),
            _nAsyncRcvd (0),
            _initialized (false), _postReceives (false),
            _enablePipelining (false),
            _srcbuf (NULL), _dstbuf (NULL),
            _reduceFunc (NULL),
            _rconnmgr (NULL),
            _bconnmgr(NULL),
            _native (NULL),
            _msend(),
            _selftopology(),
            _scache(),
            _acache(NULL, (unsigned) - 1)
        {
          TRACE_FN_ENTER();
          TRACE_FORMAT("<%p>", this);
          TRACE_FN_EXIT();
        }

        ///  Main constructor to initialize the executor
        ///  By default it only needs one connection manager
        AllreduceBaseExec(Interfaces::NativeInterface    * native,
                          T_Conn                         * connmgr,
                          const unsigned                   commID,
                          bool                             enable_pipe = false):
            Interfaces::Executor(),
            _commID (commID), _curPhase ((unsigned) - 1), _curIdx ((unsigned) - 1),
            _nAsyncRcvd (0),
            _initialized (false), _postReceives (false),
            _enablePipelining (enable_pipe),
            _srcbuf (NULL), _dstbuf (NULL),
            _reduceFunc (NULL),
            _rconnmgr (connmgr),
            _bconnmgr(connmgr),
            _native (native),
            _msend(),
            _selftopology(native->myrank()),
            _scache(),
            _acache(&_scache, native->myrank())
        {
          TRACE_FN_ENTER();
          TRACE_FORMAT("<%p>", this);

          _msend.msginfo     = NULL;
          _msend.msgcount    = 0;
          _msend.roles       = -1U;

          _acache.setReduceConnectionManager (connmgr);
          _acache.setBroadcastConnectionManager (connmgr);

          _sendCallbackHandler = staticNotifySendDone;
          _recvCallbackHandler = staticNotifyReceiveDone;

          _acache.setExecutor (this);
          _acache.setCommID (commID);

          _msend.cb_done.function = _sendCallbackHandler;
          _msend.cb_done.clientdata = &_sState.sndClientData;
          TRACE_FN_EXIT();
        }

        void setIteration (unsigned iteration)
        {
          _acache.setIteration(iteration);
        }

        void setSchedule (Interfaces::Schedule *schedule, unsigned color = 0)
        {
          _scache.setSchedule(schedule);
          _acache.setColor(color);
        }

        void setReduceConnectionManager (T_Conn *cmgr)
        {
          _rconnmgr = cmgr;
          _acache.setReduceConnectionManager(cmgr);
        }

        void setBroadcastConnectionManager (T_Conn *cmgr)
        {
          _bconnmgr = cmgr;
          _acache.setBroadcastConnectionManager(cmgr);
        }

        ///entry method
        virtual void notifySendDone( const pami_quad_t &info );

        ///entry method
        virtual void   notifyRecv  ( unsigned             src,
                                     const pami_quad_t   & info,
                                     PAMI::PipeWorkQueue ** pwq,
                                     pami_callback_t      * cb_done );

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
        virtual PAMI_Request_t *   notifyRecvHead(const pami_quad_t     * info,
                                                  unsigned               count,
                                                  unsigned               conn_id,
                                                  unsigned               peer,
                                                  unsigned               sndlen,
                                                  void                 * arg,
                                                  size_t               * rcvlen,
                                                  pami_pipeworkqueue_t ** recvpwq,
                                                  PAMI_Callback_t       * cb_done);

#if 0
        ///
        ///  \fast callback for short allreduce operations
        ///  This callback does not return a request
        ///
        void notifyRecvShort           (unsigned          phase,
                                        unsigned          sndlen,
                                        unsigned          srcindex,
                                        unsigned        * rcvlen,
                                        char           ** rcvbuf,
                                        CCMI_Callback_t * cb_done,
                                        unsigned          order);
#endif

        ///
        /// \brief Set the buffer info for the allreduce collective
        ///        These members are always set.
        /// \param[in]  srcbuf
        /// \param[in]  dstbuf
        /// Note setDataInfo should be called before setReduceInfo!!
        ///

        void setBuffers ( void           * srcbuf,
                          void           * dstbuf,
                          unsigned         bytes,
                          TypeCode       * stype,
                          TypeCode       * rtype)
        {
          //printf ("%d: srcbuf = %x, dstbuf %x", _native->myrank(), srcbuf, dstbuf);
          _srcbuf        = (char *) srcbuf;
          _dstbuf        = (char *) dstbuf;
          /// SSS: bytes, stype, rtype are not needed here..We just need this signature for compiling
        }

        /// \brief Set the parameters related to the reduce.  When
        ///  parameters change the allreduce state changes.
        ///
        /// \param[in]  count
        /// \param[in]  pipelineWidth
        /// \param[in]  sizeOfType
        /// \param[in]  func            reduce function

        void setReduceInfo( unsigned         count,
                            unsigned         pipelineWidth,
                            unsigned         sizeOfType,
                            coremath         func,
                            TypeCode       * stype,
                            TypeCode       * rtype,
                            pami_op          op = PAMI_OP_COUNT,
                            pami_dt          dt = PAMI_DT_COUNT)
        {
          TRACE_FN_ENTER();
          TRACE_FORMAT("<%p> count %#X, pipelineWidth %#X, sizeOfType %#X, func %p, op %#X, dt %#X",
                      this, count, pipelineWidth, sizeOfType, func, op, dt);
          CCMI_assert (pipelineWidth % sizeOfType == 0);

          _reduceFunc    = func;

          //override the pipeline width as we do not support
          //pipelining in this simple executor.
          if (!_enablePipelining)
            pipelineWidth = count * sizeOfType;

          _acache.init(count, sizeOfType, op, dt, stype, rtype, pipelineWidth);//sizeOfType is based on stype/rtype size on dt size

          TRACE_FN_EXIT();
        }

        pami_event_function  getRecvCallbackHandler ()
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
        void setRoot(int root = -1)
        {
          _scache.setRoot(root);
        }

        /// \brief Get the root for reduction
        ///
        /// \return  root -1 indicates allreduce, any other
        ///                   root indicates reduce root
        int getRoot()
        {
          return _scache.getRoot();
        }

        /// \brief Get the async iteration id
        ///
        /// \return  iteration id
        unsigned getIteration()
        {
          return _acache.getIteration();
        }

        void postReceives ();
        //void postRecv (unsigned p, unsigned next);

        /// \brief Register the multicast interface
        ///
        /// \param[in]  mf
        void setNativeInterface(Interfaces::NativeInterface * mf)
        {
          _native = mf;
        }

        Interfaces::NativeInterface *getMulticastInterface()
        {
          return _native;
        }

        virtual void reset ()
        {
          _acache.setDstBuf (&_dstbuf);

          bool flag;
          if(_scache.getRoot() == -1)
            flag = _scache.init(ALLREDUCE_OP);
          else
            flag = _scache.init(REDUCE_OP);

          _acache.reset (flag, false);

          _startPhase = _curPhase = _scache.getStartPhase();
          _curIdx = 0;
          _endPhase = _scache.getEndPhase();
          _initialized = false;
          _nAsyncRcvd = 0;

          _firstCombinePhase = _scache.getStartPhase();

          while (_firstCombinePhase <= _scache.getEndPhase() && !_scache.getNumSrcRanks(_firstCombinePhase))
            _firstCombinePhase++;

          //Reduce might not have a combine phase (on non-roots)
          if ((_scache.getRoot() != -1) && (_firstCombinePhase > _scache.getEndPhase()))
            _firstCombinePhase = _scache.getStartPhase();

          CCMI_assert (_firstCombinePhase <= _scache.getEndPhase());
        }

        void operator delete(void * p)
        {
          CCMI_abort();
        }

        pami_op    getOp ()
        {
          return  _acache.getOp();
        }

        pami_dt    getDt ()
        {
          return  _acache.getDt();
        }

        unsigned   getCount ()
        {
          return  _acache.getCount();
        }

        TypeCode * getStype()
        {
          return _acache.getStype();
        }

        TypeCode * getRtype()
        {
          return _acache.getRtype();
        }

        static void _compile_time_assert_ ()
        {
          // Compile time assert
          // SendState storage must must fit in a request
          COMPILE_TIME_ASSERT(sizeof(CCMI::Executor::AllreduceBaseExec<T_Conn>::SendState) <= sizeof(PAMI_CollectiveRequest_t));
        }
    }; // AllreduceBaseExec
  } // Executor
} // CCMI

/////////////////////////////////////////////
///   Protected Methods
/////////////////////////////////////////////
template <class T_Conn>
inline void CCMI::Executor::AllreduceBaseExec<T_Conn>::advance ()
{
  TRACE_FN_ENTER();
  TRACE_FORMAT("<%p>_curPhase %d,_endPhase %d, lastReducePhase %d, _curIdx %d, nsrcranks %u", this,
                  _curPhase, _scache.getEndPhase(),_scache.getLastReducePhase(), _curIdx, _scache.getNumSrcRanks(_curPhase));

  CCMI_assert_debug (_initialized);
  CCMI_assert_debug (_sState.sndClientData.isDone == true);

  char * reducebuf = NULL;

  //Allreduce or I am the root
  if ((_scache.getRoot() == -1) || (_scache.getRoot() == (int)_native->myrank()))
    reducebuf = _dstbuf;
  else
    //Reduce operation and I am not the root
    reducebuf = _acache.getTempBuf();

  while (_curPhase <= _scache.getEndPhase())
    {
      unsigned nsrcranks = _scache.getNumSrcRanks(_curPhase);
      void *src1 = (void *) (((_curIdx == 0) && (_curPhase == _firstCombinePhase)) ? _srcbuf : reducebuf);
      void *src2 = NULL;

      pami_op op = _acache.getOp();
      pami_dt dt = _acache.getDt();
      unsigned count = _acache.getCount();
      TRACE_FORMAT("<%p>op %u, dt %u, count %u", this,
                      op, dt, count);

      if (_curPhase <= _scache.getLastReducePhase())
        {
          if (op == PAMI_SUM && dt == PAMI_SIGNED_INT)
            {
              for (; (_curIdx < nsrcranks) && (_acache.getPhaseChunksRcvd(_curPhase, _curIdx) > 0); _curIdx ++)
                {
                  src2 = _acache.getPhaseRecvBufs (_curPhase, _curIdx);
                  inline_math_isum (reducebuf, src1, src2, count);
                  src1 = reducebuf;
                }
            }
          else if (op == PAMI_SUM && dt == PAMI_DOUBLE)
            {
              for (; (_curIdx < nsrcranks) && (_acache.getPhaseChunksRcvd(_curPhase, _curIdx) > 0); _curIdx ++)
                {
                  src2 = _acache.getPhaseRecvBufs (_curPhase, _curIdx);
                  inline_math_dsum (reducebuf, src1, src2, count);
                  src1 = reducebuf;
                }
            }
          else if (op == PAMI_MIN && dt == PAMI_DOUBLE)
            {
              for (; (_curIdx < nsrcranks) && (_acache.getPhaseChunksRcvd(_curPhase, _curIdx) > 0); _curIdx ++)
                {
                  src2 = _acache.getPhaseRecvBufs (_curPhase, _curIdx);
                  inline_math_dmin (reducebuf, src1, src2, count);
                  src1 = reducebuf;
                }
            }
          else if (op == PAMI_MAX && dt == PAMI_DOUBLE)
            {
              for (; (_curIdx < nsrcranks) && (_acache.getPhaseChunksRcvd(_curPhase, _curIdx) > 0); _curIdx ++)
                {
                  src2 = _acache.getPhaseRecvBufs (_curPhase, _curIdx);
                  inline_math_dmax (reducebuf, src1, src2, count);
                  src1 = reducebuf;
                }
            }
          else
            {
              for (; (_curIdx < nsrcranks) && (_acache.getPhaseChunksRcvd(_curPhase, _curIdx) > 0); _curIdx ++)
                {
                  src2 =  _acache.getPhaseRecvBufs (_curPhase, _curIdx);
                  void * bufs[2];
                  bufs[0] = src1;
                  bufs[1] = src2;
                  TRACE_HEXDATA((char*)bufs[0], count*_acache.getSizeOfType());
                  TRACE_HEXDATA((char*)bufs[1], count*_acache.getSizeOfType());
                  TRACE_HEXDATA((char*)reducebuf, 1); // Just to trace the input pointer
                  _reduceFunc( reducebuf, bufs, 2, count);

                  TRACE_HEXDATA((char*)reducebuf, count*_acache.getSizeOfType());
                  src1 = reducebuf;
                }
            }
        }
      else if ((_curIdx < nsrcranks) && (_acache.getPhaseChunksRcvd(_curPhase, _curIdx) > 0))
      {
        CCMI_assert(nsrcranks == 1);
        src2 =  _acache.getPhaseRecvBufs (_curPhase, _curIdx);
        TRACE_HEXDATA((char*)src1,      count*_acache.getSizeOfType());
        TRACE_HEXDATA((char*)src2,      count*_acache.getSizeOfType());
        TRACE_HEXDATA((char*)reducebuf, count*_acache.getSizeOfType());
        memcpy(reducebuf, src2, count*_acache.getSizeOfType());
        _curIdx = nsrcranks;
      }

      TRACE_FORMAT("<%p>_curPhase %d,_endPhase %d,_curIdx %d nsrcranks %d phase chunks rcvd %d", this, _curPhase, _scache.getEndPhase(), _curIdx, nsrcranks, (_curIdx < nsrcranks)?_acache.getPhaseChunksRcvd(_curPhase, _curIdx):-1);

      if (_curIdx != nsrcranks) //we are waiting for more data
        break;

      if (_curPhase == _scache.getEndPhase())
        {
          // Call application done callback
          if (_cb_done)
            _cb_done (NULL, _clientdata, PAMI_SUCCESS);

          break;
        }

      _curIdx = 0; //Start the next phase
      _curPhase ++;

      unsigned ndstranks = _scache.getNumDstRanks (_curPhase);

      if (ndstranks > 0)
        {
          PAMI::Topology *dst_topology   = _scache.getDstTopology(_curPhase);
          sendMessage (reducebuf, _acache.getBytes(), dst_topology,
                       _curPhase, &_sState);
          //wait for send to finish
          break;
        }
    }
  TRACE_FN_EXIT();
}


///
///  \brief Send the next message by calling the msend interface
///
template <class T_Conn>
inline void CCMI::Executor::AllreduceBaseExec<T_Conn>::sendMessage
(const char             * buf,
 unsigned                 bytes,
 PAMI::Topology          * dst_topology,
 unsigned                 sphase,
 SendState              * s_state)
{
  TRACE_FN_ENTER();
  //Request buffer and callback set in setSendState !!
  CCMI_assert (dst_topology->size() > 0);

  _msend.connection_id = _acache.getPhaseSendConnectionId (sphase);
  _msend.bytes         = bytes;

  PAMI::PipeWorkQueue *pwq = _acache.getPhaseDstPipeWorkQueue(sphase);
  pwq->configure((char *)buf, bytes, 0, getStype(), getRtype());
  pwq->reset();
  pwq->produceBytes(bytes);

  _msend.src              = (pami_pipeworkqueue_t *) pwq;
  _msend.dst              = NULL;
  _msend.src_participants = (pami_topology_t *) & _selftopology;
  _msend.dst_participants = (pami_topology_t *) _scache.getDstTopology(sphase);

  //the message has been sent and send state slot is unavailable
  s_state->sndClientData.isDone = false;

  if (!_postReceives)
    {
      s_state->sndInfo._comm      = _commID;
      s_state->sndInfo._count     = _acache.getCount();
      s_state->sndInfo._phase     = sphase;
      s_state->sndInfo._dt        = _acache.getDt();
      s_state->sndInfo._op        = _acache.getOp();
      s_state->sndInfo._iteration = _acache.getIteration();
      s_state->sndInfo._root      = (unsigned)_scache.getRoot();
      _msend.msginfo  =  (pami_quad_t *) & s_state->sndInfo;
      _msend.msgcount = 1;
    }

  pami_task_t *dstranks = NULL;
  dst_topology->rankList(&dstranks);

  TRACE_FORMAT("<%p>connid %#X curphase:%#X "
              "bytes:%#X destPe:%#X ndst %zu cData:%p ",
              this,
              _acache.getPhaseSendConnectionId (sphase),
              sphase, bytes, dstranks[0], dst_topology->size(),
              &s_state->sndInfo);

  _native->multicast (&_msend);
  TRACE_FN_EXIT();
}


////////////////////////////////////////////////////////
///
///  Public methods that can be called externally
///
////////////////////////////////////////////////////////

template <class T_Conn>
inline void CCMI::Executor::AllreduceBaseExec<T_Conn>::start()
{
  TRACE_FN_ENTER();
  TRACE_FORMAT("<%p>", this);

  _initialized = true;
  _sState.sndClientData.me        = this;
  _sState.sndClientData.isDone    = true;

  CCMI_assert_debug (_startPhase != (unsigned) - 1);
  //CCMI_assert_debug (_curPhase != (unsigned) -1);

  // Skip bogus initial phase(s)
  while (!_scache.getNumDstRanks (_curPhase) && !_scache.getNumSrcRanks (_curPhase))
    ++ _curPhase;

  unsigned ndstranks = _scache.getNumDstRanks (_curPhase);

  if (ndstranks)
    {
      PAMI::Topology *dst_topology   = _scache.getDstTopology(_curPhase);
      sendMessage (_srcbuf, _acache.getBytes(), dst_topology, _curPhase, &_sState);
    }
  else
    advance ();
  TRACE_FN_EXIT();
}

template <class T_Conn>
inline void CCMI::Executor::AllreduceBaseExec<T_Conn>::notifyRecv
( unsigned              src,
  const pami_quad_t    & info,
  PAMI::PipeWorkQueue ** p,
  pami_callback_t      * cb_done )
{
  TRACE_FN_ENTER();
  AC_RecvCallbackData * cdata = (AC_RecvCallbackData *)(&info);

  TRACE_FORMAT("<%p> %#X, %#X",
              this, cdata->phase, cdata->srcPeIndex);

  // update state  (we dont support multiple sources per phase yet)
  _acache.incrementPhaseChunksRcvd(cdata->phase, cdata->srcPeIndex);
  PAMI::PipeWorkQueue *pwq = _acache.getPhasePipeWorkQueues(cdata->phase,
                                                            cdata->srcPeIndex);
  pwq->consumeBytes (_acache.getBytes());

  //The send for the previous phase has finished and we have received
  //data for the current phase
  if (cdata->phase == _curPhase      && //got the data for my phase
      _sState.sndClientData.isDone  &&  //send has finished
      _initialized)
    advance();
  TRACE_FN_EXIT();
}

template <class T_Conn>
inline void CCMI::Executor::AllreduceBaseExec<T_Conn>::notifySendDone
( const pami_quad_t & info)
{
  TRACE_FN_ENTER();
  // update state
  TRACE_FORMAT("<%p>cur phase %#X",
             this, _curPhase);

  AC_SendCallbackData * cdata = (AC_SendCallbackData *)(&info);
  cdata->isDone = true;

  //Call the appropriate advance
  advance ();
  TRACE_FN_EXIT();
}

template <class T_Conn>
inline PAMI_Request_t *
CCMI::Executor::AllreduceBaseExec<T_Conn>::notifyRecvHead
(const pami_quad_t     * info,
 unsigned               count,
 unsigned               conn_id,
 unsigned               peer,
 unsigned               sndlen,
 void                 * arg,
 size_t               * rcvlen,
 pami_pipeworkqueue_t ** rcvpwq,
 PAMI_Callback_t       * cb_done)
{
  TRACE_FN_ENTER();
  CCMI_assert_debug(count == 1);
  CCMI_assert_debug(info);
  CollHeaderData *cdata = (CollHeaderData*) info;

  TRACE_FORMAT("<%p>count: %u "
             "connID:%u phase:%u root:%u local root:%u src %u sndlen %u,_state->getBytes():%u "
             "_state->getPipelineWidth():%u",
             this,
             count,
             conn_id,
             cdata->_phase,
             (int) cdata->_root,
             _scache.getRoot(),
             peer,
             sndlen,
             _acache.getBytes(),
             _acache.getPipelineWidth());

  CCMI_assert_debug(cdata->_comm == _commID);
  CCMI_assert_debug(cdata->_root == (unsigned) _scache.getRoot());

  CCMI_assert_debug(cdata->_phase >= (unsigned)_scache.getStartPhase());
  //CCMI_assert_debug(conn_id == _rconnmgr->getRecvConnectionId(cdata->_comm, (unsigned)-1, peer, cdata->_phase, (unsigned) -1));

  //CCMI_assert(arg && rcvlen && rcvbuf && pipewidth && cb_done);
  //CCMI_assert (cdata->_phase <= _scache.getEndPhase());

  if (cdata->_phase > _scache.getEndPhase())
    cdata->_phase = _scache.getEndPhase();

  if (_scache.getNumSrcRanks(cdata->_phase) > 0)
    {
      // Schedule misbehavior patch:
      // src and dst phases *should* match but some schedules
      // send (dst) in phase n and receive (src) in phase n+1.
      // Allow this, I guess, by looking for the next src phase
      // and assuming that's the one we want.  This only works if
      // there are no src's in the specified phase.  If there are,
      // then we expect a match.
      while ((_scache.getNumSrcRanks(cdata->_phase) == 0) &&
             (cdata->_phase < (unsigned)_scache.getEndPhase()))
        {
          TRACE_FORMAT("<%p>no src in phase %#X",
                     this, cdata->_phase);
          cdata->_phase++;
        }
    }

  CCMI_assert(_scache.getNumSrcRanks(cdata->_phase) > 0);

  TRACE_FORMAT("<%p>phase %#X, numsrcranks %#X",
              this, cdata->_phase, _scache.getNumSrcRanks(cdata->_phase));

  int srcPeIndex = -1;
  int idx = 0;
  int nsrc = _scache.getNumSrcRanks(cdata->_phase);

  for (idx = 0; idx < nsrc; idx ++)
    {
      PAMI::Topology *srctopology = _scache.getSrcTopology(cdata->_phase);
      pami_task_t *srcranks = NULL;
      srctopology->rankList(&srcranks);

      if (srcranks[idx] == peer)
        {
          srcPeIndex = idx;
          break;
        }
    }

  CCMI_assert (srcPeIndex >= 0);

  unsigned index = 0;
  unsigned nchunks = 0;

  //This is the fist message from that source
  if ((nchunks = _acache.getPhaseChunksRcvd (cdata->_phase, srcPeIndex)) == 0)
    index = _nAsyncRcvd;
  else
    {
      for (index = 0; index < _nAsyncRcvd; index ++)

        // match phase and src rank
        if (_acache.getRecvClientPhase(index) == cdata->_phase &&
            _acache.getRecvClientSrcPeIndex(index) == (unsigned)srcPeIndex)
          break;

      CCMI_assert (index < _nAsyncRcvd);
    }

  AC_RecvCallbackData *rdata = _acache.getRecvClient(index);

  if (index == _nAsyncRcvd)
    {
      rdata->allreduce =  this;
      rdata->phase     =  cdata->_phase;
      rdata->srcPeIndex = srcPeIndex;

      _nAsyncRcvd ++;
    }

  *rcvlen    = sndlen;
  PAMI::PipeWorkQueue *pwq = _acache.getPhasePipeWorkQueues(cdata->_phase, srcPeIndex);
  pwq->reset();
  //char * buf = pwq->bufferToProduce();
  //CCMI_assert (buf != NULL);
  //printf ("%d: PWQ buffer 0x%x for phase %d index %d", _native->myrank(), buf, cdata->_phase, srcPeIndex);

  * rcvpwq    = (pami_pipeworkqueue_t *) pwq;
  cb_done->function   = _recvCallbackHandler;
  cb_done->clientdata = rdata;

  TRACE_FN_EXIT();
  return _acache.getRecvReq() + index;
}

#if 0

template <class T_Conn>
inline void CCMI::Executor::AllreduceBaseExec<T_Conn>::postReceives ()
{
  TRACE_FN_ENTER();
  // post receives for each expected incoming message
  _postReceives = true;

  for (unsigned p = _scache.getStartPhase(); p <= _scache.getEndPhase(); p++)
    {
      if (_scache.getNumSrcRanks(p) > 0)
        {
          pami_multicast_t *recv = _acache.getPhaseMcastRecv (p, 0);
          recv->cb_done.function = _recvCallbackHandler;
          _native->multicast(recv);
        }
    }

  TRACE_FORMAT("<%p>",
              this);
  TRACE_FN_EXIT();
}

#endif

#if 0
///
///  \fast callback for short allreduce operations
///  This callback does not return a request
///
template <class T_Conn>
inline void
CCMI::Executor::AllreduceBaseExec<T_Conn>::notifyRecvShort
(unsigned          phase,
 unsigned          sndlen,
 unsigned          srcindex,
 unsigned        * rcvlen,
 char           ** rcvbuf,
 PAMI_Callback_t * cb_done,
 unsigned          order)
{
  TRACE_FN_ENTER();
  *rcvbuf = _acache.getPhaseRecvBufs (phase, srcindex);
  *rcvlen = sndlen;

  ///This is a short callback and at the end of it the data is
  ///guarangeed to be copied in
  CCMI_assert(_acache.getPhaseChunksRcvd(phase, srcindex) == 0);
  _acache.incrementPhaseChunksRcvd(phase, srcindex);

  unsigned nsrcranks = _scache.getNumSrcRanks(phase);

  TRACE_FORMAT("phase = %d, srcindex = %d, nsrcranks = %d, totalchunksrcvd %d, recvbuf = %#X, len = %d",
               phase, srcindex, nsrcranks, _acache.getPhaseTotalChunksRcvd(_curPhase),
               (int)*rcvbuf,
               *rcvlen);

  cb_done->function = NULL;

  ///Trigger advance on the last packet
  if (_acache.getPhaseTotalChunksRcvd(_curPhase) == nsrcranks)
    {
      cb_done->function   = short_recv_done; //short recv callback
      cb_done->clientdata = this;
    }
  TRACE_FN_EXIT();
}

#endif

#undef  DO_TRACE_ENTEREXIT
#undef  DO_TRACE_DEBUG

#endif /* __simple_allreduce_executor_h__ */
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
