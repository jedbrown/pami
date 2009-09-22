/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/executor/AllreduceState.h
 * \brief The persistent state data for [all]reduce executor
 */
#ifndef __allreduce_state_h__
#define __allreduce_state_h__

#include "algorithms/schedule/Schedule.h"
#include "algorithms/executor/Executor.h"
#include "algorithms/connmgr/ConnectionManager.h"
#include "algorithms/ccmi.h"
#include "util/ccmi_util.h"
#include "util/ccmi_debug.h"
#include "util/common.h"

namespace CCMI
{
  namespace Executor
  {

/// Arbitrary maximum number PE's per phase.  (In practice it's < 4)
/// This is used for examining the PE lists from the schedule.  If a
/// schedule ever returns more than 16, we'll assert.
#define XMI_KERNEL_EXECUTOR_ALLREDUCE_MAX_PES_PER_PHASE 128

    /// client data for multisend receive done callback
    typedef struct RecvCallbackData_t
    {
      Executor  * allreduce;   //Allreduce state needs to work with
      //multiple allreduce executors SK 11/01
      unsigned    phase;
      unsigned    srcPeIndex;
    } RecvCallbackData;

    /// client data for multisend send done callback
    typedef struct SendCallbackData_t
    {
      Executor       * me;   //Allreduce state needs to work with
      //multiple allreduce executors SK 11/01
      bool             isDone;
    } SendCallbackData;

    ///
    /// \brief Allreduce (and Reduce) persistent state data
    /// This class allocates storage for receive buffers and schedule data.
    ///

    template<class T_mcastrecv, class T_Sysdep>
    class AllreduceState
    {
    public:
      /// per phase state
      /// Params have been re-organized for cache
      /// performance. For example numSrcPes and numDstPes are the
      /// most commonly used  SK 10/30
      typedef struct _phase_state    //XLC complains about anonymous structs SK 10/30
      {
        unsigned     numSrcPes;    // # of source ranks
        unsigned     numDstPes;    // # of destination ranks
        unsigned     totalChunksRcvd;  // # of chunks received in this phase
        unsigned  *  srcPes;       // source ranks
        unsigned  *  srcHints;     // hints, one per source
        char      ** recvBufs;     // receive buffers for this phase
        unsigned  *  chunksRcvd;   // # of chunks received from each srcrank
        unsigned     chunksSent;   // # chunks sent in this phase
        unsigned     sconnId;      // # sender connection id for this phase
        unsigned  *  dstPes;       // destination ranks
        unsigned  *  dstHints;     // hints, one per destination
        T_mcastrecv  *mrecv;
      } PhaseState __attribute__((__aligned__(16))); ///Achieve better cache blocking

    protected:

      int               _root;
      unsigned          _myRank;
      unsigned          _protocol;

      unsigned          _count;         /// total count of datatypes on operation
      unsigned          _bytes;         /// # of bytes (not count of datatypes)
      unsigned          _sizeOfType;    /// Size of the data type
      unsigned          _sizeOfBuffers; /// Size of allocated buffers (>= _bytes)

      /// my schedule
      Schedule::Schedule                   * _sched;

      /// dynamically allocated buffer space (for algorithms/schedule/phase data members)
      void             * _scheduleAllocation;
      unsigned           _scheduleAllocationSize;

      /// buffer space for all phases (pointers into schedule allocation)
      unsigned     *_all_srcPes  ;
      unsigned     *_all_srcHints;
      char        **_all_recvBufs;
      unsigned     *_all_dstPes  ;
      unsigned     *_all_dstHints;
      unsigned     *_all_chunks  ;
      unsigned     *_nextActivePhase;  //convert to dynamic vector
      T_mcastrecv  *_all_mrecvs;

      /// dynamically allocated buffer space (for received data)
      void             * _receiveAllocation;
      unsigned           _receiveAllocationSize;

      /// pointers into the receive allocation
      char             * _tempBuf;
      char             * _bufs;
      XMI_Request_t   * _recvReq;
      RecvCallbackData * _recvClientData;

      // global state information
      PhaseState  *_phaseVec; // pointer into schedule allocation
      int          _startPhase;
      int          _endPhase;
      int          _dstPhase;  /// Phase in which the dstbuf is the receive destination
      int          _numSrcPes;
      int          _numDstPes;

      // "Chunk"s are pipelined data blocks
      unsigned     _pipelineWidth;
      unsigned     _lastChunk;
      unsigned     _fullChunkCount;
      unsigned     _lastChunkCount;

      // configuration
      bool _isConfigChanged;

      ///
      /// \brief The state module now auto-detects a broadcast
      /// embedded in an allreduce  SK 11/01
      ///
      int           _lastReducePhase;    //last phase before broadcast
      int           _bcastRecvPhase;     //first broadcast phase
      int           _bcastSendPhase;     //first broadcast send phase
      int           _lastCombinePhase;   //last phases where arithmatic needs to be done

      ConnectionManager::ConnectionManager<T_Sysdep>  * _rconnmgr;  /// the connection manager for reductions
      ConnectionManager::ConnectionManager<T_Sysdep>  * _bconnmgr;  /// the connection manager for broadcast

      unsigned                                _commid;   /// Communicator identifier
      unsigned                                _color;    /// Color of the collective

      xmi_op                                 _op;         /// allreduce operation
      xmi_dt                                 _dt;         /// allreduce datatype
      unsigned                                _iteration;   /// allreduce async iteration
      Executor                              * _executor;   /// Pointer to executor which is needed to
      /// set up the receive callback data objects

    public:


#ifdef XMI_DEBUG
      inline void checkCorruption()
      {
        if(_receiveAllocation)
        {
          XMI_ADAPTOR_DEBUG_trace_data("CHECK RECEIVE ALLOCATION CORRUPTION",(((char*)_receiveAllocation)-8), 16);
        }
        else XMI_ADAPTOR_DEBUG_trace_data("CHECK RECEIVE ALLOCATION CORRUPTION NULL",NULL , 0);
        if(_scheduleAllocation)
        {
          XMI_ADAPTOR_DEBUG_trace_data("CHECK SCHEDULE ALLOCATION CORRUPTION",(((char*)_scheduleAllocation)-8), 16);
          XMI_ADAPTOR_DEBUG_trace_data("CHECK PHASE VECTOR CORRUPTION",(char*)_phaseVec, 128);
        }
        else XMI_ADAPTOR_DEBUG_trace_data("CHECK SCHEDULE ALLOCATION CORRUPTION NULL",NULL , 0);
      }
#endif
      inline xmi_op getOp()
      {
        return _op;
      }
      inline xmi_dt getDt()
      {
        return _dt;
      }
      inline int getIteration()
      {
        return _iteration;
      }
      inline void setIteration(unsigned i)
      {
        _iteration = i;
        return ;
      }
      inline int getStartPhase()
      {
        return _startPhase;
      }
      inline unsigned getCount()
      {
        return _count;
      }
      inline unsigned getBytes()
      {
        return _bytes;
      }
      inline int getEndPhase()
      {
        return _endPhase;
      }

      inline int getLastReducePhase ()
      {
        return _lastReducePhase;
      }

      inline int getLastCombinePhase ()
      {
        return _lastCombinePhase;
      }

      inline int getBcastRecvPhase ()
      {
        return _bcastRecvPhase;
      }

      inline int getBcastSendPhase ()
      {
        return _bcastSendPhase;
      }

      inline unsigned getBcastNumDstPes ()
      {
        if(_bcastSendPhase > 0)
          return _phaseVec[_bcastSendPhase].numDstPes;
        else
          return 0;
      }

      inline unsigned *getBcastDstPes ()
      {
        if(_bcastSendPhase > 0)
          return _phaseVec[_bcastSendPhase].dstPes;
        else
          return NULL;
      }

      inline unsigned *getBcastDstHints ()
      {
        if(_bcastSendPhase > 0)
          return _phaseVec[_bcastSendPhase].dstHints;
        else
          return NULL;
      }

      inline int getRoot()
      {
        return _root;
      }
      inline unsigned getMyRank()
      {
        return _myRank;
      }

      inline int getNextActivePhase (int phase)
      {
        XMI_assert (phase >= _startPhase);
        XMI_assert (phase <= _endPhase);
        return _nextActivePhase [phase];
      }

      inline void setProtocol(unsigned protocol)
      {
        if(_protocol != protocol)
        {
          _isConfigChanged = true;
        }
        _protocol = protocol;
        return ;
      }
      inline unsigned getProtocol()
      {
        return _protocol;
      }
      inline char* getTempBuf()
      {
        return _tempBuf;
      }
      inline unsigned getPipelineWidth()
      {
        return _pipelineWidth;
      }
      inline unsigned getLastChunkCount()
      {
        return _lastChunkCount;
      }
      inline unsigned getFullChunkCount()
      {
        return _fullChunkCount;
      }
      inline unsigned getLastChunk()
      {
        return _lastChunk;
      }
      inline unsigned getSizeOfType()
      {
        return _sizeOfType;
      }


      inline  unsigned              getPhaseSrcPes(unsigned index,unsigned jindex)
      {
        return _phaseVec[index].srcPes[jindex];
      }
      inline  unsigned              getPhaseSrcHints(unsigned index,unsigned jindex)
      {
        return _phaseVec[index].srcHints[jindex]  ;
      }
      inline  char      *           getPhaseRecvBufs(unsigned index,unsigned jindex)
      {
        return _phaseVec[index].recvBufs[jindex]  ;
      }
      inline  unsigned              getPhaseChunksRcvd(unsigned index,unsigned jindex)
      {
        return _phaseVec[index].chunksRcvd[jindex];
      }
      inline  unsigned              getPhaseTotalChunksRcvd(unsigned index)
      {
        return _phaseVec[index].totalChunksRcvd;
      }
      inline T_mcastrecv  *   getPhaseMcastRecv(unsigned index,unsigned jindex)
      {
        TRACE_STATE((stderr,"<%#.8X>Executor::AllreduceState::getPhaseMcastRecv _phaseVec[%#X].recvBufs[%#X]=%#X _phaseVec[%#X].mrecv[%#X].rcvbuf=%#X\n",
                     (int)this,
                     index,
                     jindex,
                     (int)_phaseVec[index].recvBufs[jindex],
                     index,
                     jindex,
                     (int)_phaseVec[index].mrecv[jindex].rcvbuf
                    ));
        return &_phaseVec[index].mrecv[jindex];
      }
      inline  unsigned              getPhaseDstPes(unsigned index,unsigned jindex)
      {
        return _phaseVec[index].dstPes[jindex]    ;
      }
      inline  unsigned              getPhaseDstHints(unsigned index,unsigned jindex)
      {
        return _phaseVec[index].dstHints[jindex]  ;
      }
      inline  unsigned  *           getPhaseDstPes(unsigned index)
      {
        return _phaseVec[index].dstPes;
      }
      inline  unsigned  *           getPhaseDstHints(unsigned index)
      {
        return _phaseVec[index].dstHints;
      }
      inline  unsigned              getPhaseChunksSent(unsigned index)
      {
        return _phaseVec[index].chunksSent;
      }
      inline unsigned               getPhaseSendConnectionId (unsigned index)
      {
        return _phaseVec[index].sconnId;
      }
      inline  unsigned              getPhaseNumSrcPes(unsigned index)
      {
        return _phaseVec[index].numSrcPes;
      }
      inline  unsigned              getPhaseNumDstPes(unsigned index)
      {
        return _phaseVec[index].numDstPes;
      }
      inline  void              compressPhaseNumDstPes(unsigned index, unsigned jindex)
      {
        TRACE_STATE((stderr,"<%#.8X>Executor::AllreduceState::compressPhaseNumDstPes() phase[%#X].numDstPes %#X <- phase[%#X].numDstPes %#X\n",
                     (int)this, index, _phaseVec[index].numDstPes, jindex, _phaseVec[jindex].numDstPes));
        _phaseVec[index].numDstPes += _phaseVec[jindex].numDstPes;
        _phaseVec[jindex].numDstPes = 0;
      }

      inline void                  incrementPhaseChunksRcvd(unsigned index,unsigned jindex,unsigned val=1)
      {
        _phaseVec[index].chunksRcvd[jindex] += val;
        _phaseVec[index].totalChunksRcvd += val;
      }
      inline void                  incrementPhaseChunksSent(unsigned index,unsigned val=1)
      {
        _phaseVec[index].chunksSent += val;
      }

      inline RecvCallbackData * getRecvClient(unsigned index)
      {
        return _recvClientData + index;
      }
      inline Executor * getRecvClientAllreduce(unsigned index)
      {
        return _recvClientData[index].allreduce;
      }
      inline unsigned    getRecvClientPhase(unsigned index)
      {
        return _recvClientData[index].phase;
      }
      inline unsigned    getRecvClientSrcPeIndex(unsigned index)
      {
        return _recvClientData[index].srcPeIndex;
      }
      inline void setRecvClientAllreduce(unsigned index, Executor * value)
      {
        _recvClientData[index].allreduce = value;
      }
      inline void setRecvClientPhase(unsigned index, unsigned value)
      {
        _recvClientData[index].phase = value;
      }
      inline void setRecvClientSrcPeIndex(unsigned index, unsigned value)
      {
        _recvClientData[index].srcPeIndex = value;
      }
      inline void incrementRecvClientSrcPeIndex(unsigned index)
      {
        ++_recvClientData[index].srcPeIndex;
      }

      inline XMI_Request_t   * getRecvReq()
      {
        return  _recvReq;
      }
      /// \brief Set the root for reduction. Will set _isConfigChanged if the root changes.
      ///
      /// \param[in]  root  default/-1 indicates allreduce, any other
      ///                   root indicates reduce
      ///
      inline void setRoot(int root=-1)
      {
        TRACE_STATE((stderr,"<%#.8X>Executor::AllreduceState::setRoot(%#X) enter\n",(int)this, root));
        if(_root != root)
        {
          TRACE_ALERT((stderr,"<%#.8X>Executor::AllreduceState::setRoot ALERT: Root change %#X != %#X\n",(int)this, _root, root));
          _isConfigChanged = true; /// \todo not all root changes are significant!
        }
        _root = root;
      }

      /// \brief Register the schedule
      ///
      /// \param[in]  sched
      inline void setSchedule(Schedule::Schedule *sched)
      {
        TRACE_STATE((stderr,"<%#.8X>Executor::AllreduceState::setSchedule(%#X) enter\n",(int)this, (int)sched));
        _sched = sched;
      }
      inline Schedule::Schedule * getSchedule()
      {
        return _sched;
      }

      /// \brief Reset our phase state data based on changes to the schedule.
      ///
      inline void resetPhaseData();

      ///
      ///  \brief Move the code for reading phase state to an
      ///  un-inlined function. This will help improve compiler
      ///  performance and possibly reduce I-cache misses
      ///
      void constructPhaseData ();

      /// \ brief Set the final receive buffer to be the dstbuf, if appropriate.
      ///
      /// \param[in]  pdstbuf pointer to the destination buffer pointer.
      inline void setDstBuf(char** pdstbuf)
      {
        XMI_assert(_scheduleAllocationSize);
        if(_dstPhase != (int) XMI_UNDEFINED_PHASE)
        {
          // We only use the destination buffer if we're a root or it's allreduce.  Otherwise we use
          // a temporary buffer.
          _phaseVec[_dstPhase].recvBufs = ((_root == -1) | (_root == (int)_myRank))?
                                          pdstbuf :  // Our target is the final buffer
                                          &_tempBuf; // Our target is a temp buffer

          // The mrecv structure has to match, but we assume this only works for 1 src pe?  We
          // only have a dst phase, not a dst src pe index (so mrecv[0]).  How would a final multi-
          // src receive phase work?  Probably not an issue, but assert anyway.
          XMI_assert(_phaseVec[_dstPhase].numSrcPes == 1);
          _phaseVec[_dstPhase].mrecv[0].rcvbuf = *_phaseVec[_dstPhase].recvBufs;

          TRACE_STATE((stderr,"<%#.8X>Executor::AllreduceState::setDstBuf(%#X) dstPhase(%#X) _phaseVec[_dstPhase].recvBufs(%#X)\n",
                       (int)this,
                       (int)*pdstbuf,
                       _dstPhase,
                       (int)*_phaseVec[_dstPhase].recvBufs
                      ));
        }
      }

      /// \brief Setup receive structures and allocate buffers if
      /// the configuration has changed.
      ///
      /// Call resetPhaseData() before resetReceives() to setup the
      void resetReceives(unsigned infoRequired);

      /// \brief Setup receive structures and allocate buffers
      ///
      /// Call resetPhaseData() before setupReceives() to setup the schedule and phase
      /// structures.
      void setupReceives(unsigned infoRequired);

      /// \brief Free all allocations if over the limit
      inline void freeAllocations(unsigned limit = 0)
      {
        if((_scheduleAllocationSize+_receiveAllocationSize) > limit)
        {
          TRACE_ALERT((stderr,"<%#.8X>Executor::AllreduceState::freeAllocations(%#.8X) ALERT: Allocation freed, %#X(%#X bytes), %#X(%#X bytes)\n",(int)this,
                       limit, (int)_scheduleAllocation, _scheduleAllocationSize, (int)_receiveAllocation, _receiveAllocationSize));
#ifdef XMI_DEBUG
          memset(_scheduleAllocation, 0xFB, _scheduleAllocationSize);
          memset(_receiveAllocation, 0xFC, _receiveAllocationSize);
#endif
          _isConfigChanged = true;
          CCMI_Free(_scheduleAllocation);
          _scheduleAllocation = NULL;
          _scheduleAllocationSize = 0;
          CCMI_Free(_receiveAllocation);
          _receiveAllocation = NULL;
          _receiveAllocationSize = 0;

          _count         = 0;
          _pipelineWidth = 0;
          _sizeOfType    = 0;
          _bytes         = 0;
          _lastChunk     = 0;
          _lastChunkCount= 0;
          _fullChunkCount= 0;
          _sizeOfBuffers = 0;

          _all_srcPes    = NULL;
          _all_srcHints  = NULL;
          _all_recvBufs  = NULL;
          _all_dstPes    = NULL;
          _all_dstHints  = NULL;
          _all_chunks    = NULL;
          _all_mrecvs    = NULL;

          _phaseVec      = NULL;
          _tempBuf       = NULL;
          _bufs          = NULL;
          _recvReq       = NULL;
          _recvClientData= NULL;

          _dstPhase = XMI_UNDEFINED_PHASE;
          if (_nextActivePhase)
          {
            CCMI_Free(_nextActivePhase);
            _nextActivePhase = NULL;
          }
#ifdef XMI_DEBUG
          _all_srcPes    = (unsigned*)0xFFFFFFF0;
          _all_srcHints  = (unsigned*)0xFFFFFFF1;
          _all_recvBufs  = (char**)0xFFFFFFF2;
          _all_dstPes    = (unsigned*)0xFFFFFFF3;
          _all_dstHints  = (unsigned*)0xFFFFFFF4;
          _all_chunks    = (unsigned*)0xFFFFFFF5;
          _all_mrecvs    = (MultiSend::XMI_OldMulticastRecv_t*)0xFFFFFFF6;

          _phaseVec      = (PhaseState*)0xFFFFFFF7;

          _tempBuf       = (char             *)0xFFFFFFF8;
          _bufs          = (char             *)0xFFFFFFF9;
          _recvReq       = (XMI_Request_t   *)0xFFFFFFFa;
          _recvClientData= (RecvCallbackData *)0xFFFFFFFb;
#endif
        }
      }

      /// \brief set the Connection manager
      void setReduceConnectionManager (ConnectionManager::ConnectionManager<T_Sysdep>  *connmgr)
      {
        _rconnmgr = connmgr;
      }

      /// \brief set the Connection manager
      void setBroadcastConnectionManager (ConnectionManager::ConnectionManager<T_Sysdep>  *connmgr)
      {
        _bconnmgr = connmgr;
      }

      /// \brief set the communicator id
      void setCommID (unsigned  commid)
      {
        _commid = commid;
      }

      /// \brief set the color of the collective
      void setColor (unsigned  color)
      {
        _color = color;
      }

      /// \brief Default Constructor
      AllreduceState (unsigned iteration, unsigned rank) :
      _root(-1),
      _myRank(rank),
      _count(0),
      _bytes(0),
      _sizeOfType(0),
      _sizeOfBuffers(0),
      _sched(NULL),
      _scheduleAllocation(NULL),
      _scheduleAllocationSize(0),
      _receiveAllocation(NULL),
      _receiveAllocationSize(0),
      _bufs(NULL),
      _recvReq(NULL),
      _recvClientData(NULL),
      _phaseVec(NULL),
      _startPhase(-1),
      _endPhase(-1),
      _dstPhase(XMI_UNDEFINED_PHASE),
      _numSrcPes(0),
      _numDstPes(0),
      _pipelineWidth(0),
      _lastChunk(0),
      _fullChunkCount(0),
      _lastChunkCount(0),
      _isConfigChanged(true),
      _rconnmgr (NULL),
      _bconnmgr (NULL),
      _commid ((unsigned)-1),
      _color  ((unsigned)-1),
      _op(XMI_UNDEFINED_OP),
      _dt(XMI_UNDEFINED_DT),
      _iteration(iteration)
      {
        TRACE_STATE((stderr,"<%#.8X>Executor::AllreduceState::ctor(void) enter\n",(int)this));
        TRACE_ALERT((stderr,"<%#.8X>Executor::AllreduceState::ctor ALERT: Constructor\n",(int)this));
        TRACE_STATE((stderr,"<%#.8X>Executor::AllreduceState::ctor(void) exit\n",(int)this));
        _nextActivePhase = 0;
      }
      /// Default Destructor
      virtual ~AllreduceState ()
      {
        TRACE_ALERT((stderr,"<%#.8X>Executor::AllreduceState::dtor ALERT: Destructor\n",(int)this));
        freeAllocations();
      }

      /// NOTE: This is required to make "C" programs link successfully with virtual destructors
      inline void operator delete(void * p)
      {
        XMI_abort();
      }

      inline void setExecutor (Executor *exe)
      {
        _executor = exe;
      }

      /// \brief  Set the actual parameters for this [all]reduce operation
      ///         and calculate some member data based on them.
      ///         Will set _isConfigChanged if necessary.
      ///
      /// \param[in]  pipelineWidth
      /// \param[in]  count
      /// \param[in]  sizeOfType
      /// \param[in]  reduce operation
      /// \param[in]  reduce datatype
      inline void setDataFunc(unsigned         pipelineWidth,
                              unsigned         count,
                              unsigned         sizeOfType,
                              xmi_op          op,
                              xmi_dt          dt)
      {
        TRACE_STATE((stderr,"<%#.8X>Executor::AllreduceState::setDataFunc() enter\n",(int)this));
        XMI_assert(pipelineWidth % sizeOfType == 0);

        _op = op;
        _dt = dt;

        unsigned bytes = count * sizeOfType;
        _count = count;

        /// If these values haven't changed, we don't need to recalculate/reset anything
        if((_pipelineWidth == pipelineWidth) &&
           (_sizeOfType    == sizeOfType) &&
           (_bytes         == bytes))
          return;

        _pipelineWidth = pipelineWidth;
        _sizeOfType    = sizeOfType;

        // Setup our "chunk" member data based on the pipeline width
        if(_pipelineWidth >= bytes)
        {
          _lastChunk      = 0;
          _fullChunkCount = 0;
          _lastChunkCount = bytes / sizeOfType;
        }
        else
        {
          unsigned lastChunk = bytes / _pipelineWidth + (bytes % _pipelineWidth != 0) - 1;
          if(lastChunk > _lastChunk)
          {
            TRACE_ALERT((stderr,"<%#.8X>Executor::AllreduceState::setDataFunc ALERT: Pipelining grew %#X > %#X\n",(int)this, lastChunk, _lastChunk));
            _isConfigChanged = true;
          }
          _lastChunk = lastChunk;
          _fullChunkCount = _pipelineWidth / sizeOfType;
          if(bytes % _pipelineWidth == 0)
            _lastChunkCount = _fullChunkCount;
          else
            _lastChunkCount = (bytes % _pipelineWidth) / sizeOfType;
        }

        TRACE_STATE((stderr,"<%#.8X>Executor::AllreduceState::setDataFunc() bytes:%#X "
                     "pwidth:%#X lastChunk:%#X fullCount:%#X lastCount:%#X \n",(int)this,
                     bytes,_pipelineWidth,_lastChunk,_fullChunkCount,
                     _lastChunkCount));

        _bytes = bytes;

        // We can reuse our existing buffer allocations if we aren't > the allocated size.
        if(bytes > _sizeOfBuffers)
        {
          TRACE_ALERT((stderr,"<%#.8X>Executor::AllreduceState::setDataFunc ALERT: Buffers too small %#X < %#X\n",(int)this, _sizeOfBuffers, bytes));
          _isConfigChanged = true;
        }

        ///Recreate multisend recv data structures
        if(!_isConfigChanged)
        {
          for(int phase = _startPhase; phase <= _endPhase; phase++)
            for(unsigned scount = 0; scount < _phaseVec[phase].numSrcPes; scount ++)
            {
              T_mcastrecv *mrecv = &(_phaseVec[phase].mrecv[scount]);
              mrecv->bytes = _bytes;
              mrecv->pipelineWidth = _pipelineWidth;
              //mrecv->opcode = (XMI_Subtask) _phaseVec[phase].srcHints[scount];
            }
        }

        TRACE_STATE((stderr,"<%#.8X>Executor::AllreduceState::setDataFunc() exit\n",(int)this));
      }

      inline void resetPhaseData()
        {
          // Do nothing if the config hasn't changed.
          if(!_isConfigChanged) return;

          constructPhaseData ();
        }

      inline void resetReceives(unsigned infoRequired)
        {
          TRACE_STATE((stderr,"<%#.8X>Executor::AllreduceState::resetReceives() enter\n",(int)this));
          //  XMI_assert(_curRcvPhase == XMI_KERNEL_EXECUTOR_ALLREDUCE_INITIAL_PHASE);
          XMI_assert(_bytes > 0);
          XMI_assert(_sched);

          // Do minimal setup if the config hasn't changed.
          if(!_isConfigChanged)
              {
                int idx = 0;
                //Make xlc happy as it thinks _all_chunks may overwrite the this
                //pointer
                unsigned *chunks = _all_chunks;
                for(idx = 0; idx < _numSrcPes; idx++)
                  chunks [idx] = 0;

                for(idx = _startPhase; idx <= _endPhase; idx++)
                    {
                      _phaseVec[idx].chunksSent = 0;
                      _phaseVec[idx].totalChunksRcvd = 0;
                    }
                return;
              }

          setupReceives (infoRequired);
          return;
        }
    }; // Allreduce
  };
};// CCMI::Executor







#endif /* __allreduce_state_h__ */
