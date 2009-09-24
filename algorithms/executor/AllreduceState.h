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

    template<class T_ConnectionManager>
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
        xmi_oldmulticast_recv_t *mrecv;
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
      xmi_oldmulticast_recv_t *_all_mrecvs;

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

      T_ConnectionManager  * _rconnmgr;  /// the connection manager for reductions
      T_ConnectionManager  * _bconnmgr;  /// the connection manager for broadcast

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
      inline xmi_oldmulticast_recv_t  *   getPhaseMcastRecv(unsigned index,unsigned jindex)
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
//      inline void resetPhaseData();

      ///
      ///  \brief Move the code for reading phase state to an
      ///  un-inlined function. This will help improve compiler
      ///  performance and possibly reduce I-cache misses
      ///

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
//      void resetReceives(unsigned infoRequired);

      /// \brief Setup receive structures and allocate buffers
      ///
      /// Call resetPhaseData() before setupReceives() to setup the schedule and phase
      /// structures.


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
      void setReduceConnectionManager (T_ConnectionManager  *connmgr)
        {
          _rconnmgr = connmgr;
        }

      /// \brief set the Connection manager
      void setBroadcastConnectionManager (T_ConnectionManager  *connmgr)
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
                        xmi_oldmulticast_recv_t *mrecv = &(_phaseVec[phase].mrecv[scount]);
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


      void constructPhaseData()
        {

          TRACE_ALERT((stderr,"<%#.8X>Executor::AllreduceState::constructPhaseData() ALERT: Phase data being reset\n",(int)this));
          TRACE_STATE((stderr,"<%#.8X>Executor::AllreduceState::constructPhaseData() enter\n",(int)this));

          _dstPhase = (int) XMI_UNDEFINED_PHASE;

          int startphase, nphases, maxranks;

          // Initialize the schedule for allreduce (root == -1) or reduce.  This returns
          // the start phase, number of phases, and maximum number of ranks to which we
          // need to communicate.  Note, however, that max ranks is combined srcPes and dstPes,
          // which really isn't completely useful.
          _sched->init(_root,
                       _root == -1 ? ALLREDUCE_OP : REDUCE_OP,
                       startphase,
                       nphases,
                       maxranks);

          TRACE_STATE((stderr,"<%#.8X>Executor::AllreduceState::constructPhaseData() root:%#X "
                       "startphase:%#X nphases:%#X maxranks:%#X\n",(int)this,
                       _root,startphase,nphases,maxranks));

          XMI_assert(nphases > 0);
          XMI_assert(startphase >= 0);

          _startPhase = startphase;
          _endPhase   = _startPhase + nphases - 1;

          ///We can now auto detect broadcasts. The last reduce phase is the
          ///phase before the broadcast message is recieved
          ///(bcastRecvPhase-1). The first broadcast phase is the phase just
          ///after the broadcast message is received (bcastRecvPhase + 1). On
          ///the root of the collective the bcastrecvphase is always -1 as
          ///broadcast does not need any special handling. On non root nodes
          ///the broadcast needs to be handled seperately as pipelining of the
          ///reduce should be independent of the pipelining of the broadcast.

          _lastReducePhase  =  _endPhase;
          _lastCombinePhase =  _endPhase;
          _bcastRecvPhase   = -1;  //should we set it to UNDEFINED_PHASE
          _bcastSendPhase   = -1;

          // maxranks is useless to us, loop through the phases and count numDstPe's and numSrcPe's.
          _numSrcPes = _numDstPes = 0;
          for(int i = _startPhase; i <= _endPhase; i++)
              {
                unsigned iNumSrcPes, iNumDstPes,
                  iSrcPes[XMI_KERNEL_EXECUTOR_ALLREDUCE_MAX_PES_PER_PHASE],
                  iSrcHints[XMI_KERNEL_EXECUTOR_ALLREDUCE_MAX_PES_PER_PHASE],
                  iDstPes[XMI_KERNEL_EXECUTOR_ALLREDUCE_MAX_PES_PER_PHASE],
                  iDstHints[XMI_KERNEL_EXECUTOR_ALLREDUCE_MAX_PES_PER_PHASE];

                _sched->getSrcPeList(i, iSrcPes, iNumSrcPes, iSrcHints);
                _sched->getDstPeList(i, iDstPes, iNumDstPes, iDstHints);
                XMI_assert(iNumSrcPes <= XMI_KERNEL_EXECUTOR_ALLREDUCE_MAX_PES_PER_PHASE &&
                           iNumDstPes <= XMI_KERNEL_EXECUTOR_ALLREDUCE_MAX_PES_PER_PHASE);
                _numSrcPes += iNumSrcPes;
                _numDstPes += iNumDstPes;

                ///SK 11/01
                ///Does the phase have a local combine recv. We dont handle the
                ///behaviour when a phase has both a combine recv and a
                ///non-combine recv.
                if(_root == -1)  //allreduce and not reduce
                    {
                      unsigned idx = 0;
                      ///find the last combine phase
                      for(idx = 0; idx < iNumSrcPes; idx++)
                        if(iSrcHints [idx] == XMI_COMBINE_SUBTASK ||
                           iSrcHints [idx] == XMI_REDUCE_RECV_STORE ||
                           iSrcHints [idx] == XMI_REDUCE_RECV_NOSTORE)
                          _lastCombinePhase = i;

                      ///Broadcast recv phase is the last non-combine recv phase.
                      ///This is an assumption that should hold across all schedules
                      if(i > _startPhase)
                          {
                            for(idx = 0; idx < iNumSrcPes; idx++)
                              if(iSrcHints [idx] != XMI_COMBINE_SUBTASK &&
                                 iSrcHints [idx] != XMI_REDUCE_RECV_STORE &&
                                 iSrcHints [idx] != XMI_REDUCE_RECV_NOSTORE)
                                  {
                                    _bcastRecvPhase = i;
                                    break;
                                  }
                          }
                    }
              }
          if(_bcastRecvPhase > 0)
              {
                _lastReducePhase = _bcastRecvPhase - 1;
                if(_lastReducePhase < _startPhase)
                  _lastReducePhase = _startPhase;
              }

          if(_bcastRecvPhase > 0)
              {
                _bcastSendPhase = _bcastRecvPhase + 1;

                // Find the first broadcast send phase, starting the phase after
                // the last reduce phase
                for(; _bcastSendPhase <= _endPhase; _bcastSendPhase++)
                    {
                      unsigned iNumDstPes,
                        iDstPes [XMI_KERNEL_EXECUTOR_ALLREDUCE_MAX_PES_PER_PHASE],
                        iDstHints [XMI_KERNEL_EXECUTOR_ALLREDUCE_MAX_PES_PER_PHASE];
                      _sched->getDstPeList(_bcastSendPhase, iDstPes, iNumDstPes, iDstHints);
                      if(iNumDstPes > 0) break;
                    }

                if(_bcastSendPhase > _endPhase)
                  _bcastSendPhase = -1;   //we can do only one check for
                //_bcastSendPhase < 0 while getting
                //bcast destintations and hints
              }

          /// \todo skip phases before start phase?  This means changing the allocation below
          ///  to nphases AND changing the indexing throughout the code to _phaseVec[phase-_startPhase]
          ///  instead of _phaseVec[phase]

          /// Calculate how much storage we need for all our algorithms/schedule/phase data.
          unsigned allocationNewSize =
            ((_endPhase + 1) * sizeof(PhaseState)) +      // _phaseVec
            (_numSrcPes *                                 // src pe data structures:
             (sizeof(unsigned) +                          // _all_srcPes
              sizeof(unsigned) +                          // _all_srcHints
              sizeof(unsigned) +                          // _all_chunks
              sizeof(char*)  +                            // _all_recvBufs
              sizeof(xmi_oldmulticast_recv_t))) + // _all_mrecvs
            (_numDstPes *                                 // dst pe data structures:
             (sizeof(unsigned) +                          // _all_dstPes
              sizeof(unsigned)));                         // _all_dstHints

/// \todo only grows, never shrinks?  runtime vs memory efficiency?
          if(allocationNewSize > _scheduleAllocationSize)
              {
                TRACE_ALERT((stderr,"<%#.8X>Executor::AllreduceState::constructPhaseData() ALERT: Phase data being reallocated (%#X < %#X), endPhase %#X, numSrcPes %#X, numDstPes %#X\n",(int)this,
                             _scheduleAllocationSize, allocationNewSize, _endPhase, _numSrcPes, _numDstPes));
                TRACE_STATE((stderr,"<%#.8X>Executor::AllreduceState::constructPhaseData() allocate new size<%#.8X> old size<%#.8X>\n",(int)this,
                             allocationNewSize, _scheduleAllocationSize));

                if(_scheduleAllocation)
                  CCMI_Free(_scheduleAllocation);
                _scheduleAllocation = CCMI_Alloc(allocationNewSize);

#ifdef XMI_DEBUG
                memset(_scheduleAllocation, 0xFD, allocationNewSize);

                if(!_scheduleAllocation)
                  fprintf(stderr,"<%#.8X>CCMI_Alloc failed<%#.8X> free'd %#X, malloc'd %#X\n",(int)this,
                          (int)_scheduleAllocation,_scheduleAllocationSize, allocationNewSize);
#endif
                XMI_assert(_scheduleAllocation);

                _scheduleAllocationSize = allocationNewSize;
              }

          /// Set the pointers appropriately into the allocated storage.
          _phaseVec = (PhaseState*) _scheduleAllocation;
          _all_recvBufs = (char**)   ((char*)_phaseVec     + ((_endPhase + 1) * sizeof(PhaseState)));
          _all_srcPes   = (unsigned*)((char*)_all_recvBufs + (_numSrcPes * sizeof(char*)));
          _all_srcHints = (unsigned*)((char*)_all_srcPes   + (_numSrcPes * sizeof(unsigned)));
          _all_chunks   = (unsigned*)((char*)_all_srcHints + (_numSrcPes * sizeof(unsigned)));
          _all_dstPes   = (unsigned*)((char*)_all_chunks   + (_numSrcPes * sizeof(unsigned)));
          _all_dstHints = (unsigned*)((char*)_all_dstPes   + (_numDstPes * sizeof(unsigned)));
          _all_mrecvs   = (xmi_oldmulticast_recv_t *) ((char*)_all_dstHints   + (_numDstPes * sizeof(unsigned)));

          TRACE_STATE((stderr,"<%#.8X>Executor::AllreduceState::constructPhaseData() allocation<%#.8X> _phaseVec<%#.8X> _all_recvBufs<%#.8X> _all_srcPes<%#.8X> _all_srcHints<%#.8X> _all_chunks<%#.8X> _all_dstPes<%#.8X> _all_dstHints<%#.8X>\n",
                       (int)this,(int)_scheduleAllocation,
                       (int)_phaseVec,
                       (int)_all_recvBufs,
                       (int)_all_srcPes  ,
                       (int)_all_srcHints,
                       (int)_all_chunks  ,
                       (int)_all_dstPes  ,
                       (int)_all_dstHints));

          // configure per phase state info structures
          /// \todo How about some data layout diagrams?
          int indexSrcPe = 0, indexDstPe = 0;
          for(int i = _startPhase; i <= _endPhase; i++)
              {
                // Don't index past our allocation
                if(indexSrcPe < _numSrcPes)
                    {
                      // setup src info
                      _phaseVec[i].srcPes     = _all_srcPes   + indexSrcPe;
                      _phaseVec[i].recvBufs   = &_all_recvBufs[indexSrcPe];
                      _phaseVec[i].chunksRcvd = _all_chunks   + indexSrcPe;
                      _phaseVec[i].srcHints   = _all_srcHints  + indexSrcPe;
                      _sched->getSrcPeList(i, _phaseVec[i].srcPes,
                                           _phaseVec[i].numSrcPes, _phaseVec[i].srcHints);
                      _phaseVec[i].mrecv      = _all_mrecvs + indexSrcPe;

                      // If there was a src pe, increment our src index into the allocated storage
                      if(_phaseVec[i].numSrcPes)
                          {
                            for(unsigned scount = 0; scount < _phaseVec[i].numSrcPes; scount ++)
                                {
                                  unsigned srcrank  =  _phaseVec[i].srcPes[scount];
                                  unsigned connID   =  (unsigned) -1;
                                  if(i <= _lastReducePhase)
                                    connID = _rconnmgr->getRecvConnectionId (_commid, _root, srcrank, i, _color);
                                  else
                                    connID = _bconnmgr->getRecvConnectionId (_commid, _root, srcrank, i, _color);

                                  xmi_oldmulticast_recv_t *mrecv = &(_phaseVec[i].mrecv[scount]);
                                  mrecv->connection_id = connID;
                                  mrecv->bytes = _bytes;
                                  mrecv->pipelineWidth = _pipelineWidth;
                                  mrecv->opcode = (xmi_subtask_t) _phaseVec[i].srcHints[scount];
                                }

                            indexSrcPe += _phaseVec[i].numSrcPes;

                            // If this is a non-combine phase, use the destination buffers.  We must be receiving the
                            // final answer.
                            // Since the dstbuf changes external to this class, we will set the phase index and the
                            // executor will use this to setup the dstbuff appropriately.
                            if(_phaseVec[i].srcHints[0] != XMI_COMBINE_SUBTASK)
                              _dstPhase = i;  // No combine? Our target buffer will be the final dstbuf.
                            else; // Leave it as previously set to: &_all_recvBufs[indexSrcPe];
                          }
                      else // No source/receive processing this phase.
                          {
                            _phaseVec[i].srcPes     = NULL;
                            _phaseVec[i].recvBufs   = NULL;
                            _phaseVec[i].chunksRcvd = NULL;
                            _phaseVec[i].srcHints   = NULL;
                            _phaseVec[i].mrecv      = NULL;
                          }
                      TRACE_STATE((stderr,"<%#.8X>Executor::AllreduceState::constructPhaseData() phaseVec[%#X]<%#.8X> srcPes<%#.8X> recvBufs<%#.8X> chunksRcvd<%#.8X> srcHints<%#.8X>\n",
                                   (int)this,i,
                                   (int)&_phaseVec[i],
                                   (int)_phaseVec[i].srcPes,
                                   (int)_phaseVec[i].recvBufs,
                                   (int)_phaseVec[i].chunksRcvd,
                                   (int)_phaseVec[i].srcHints));
                    }
                else // Better be no source/receive processing this phase.
                    {
                      // setup src info
                      _phaseVec[i].numSrcPes  = 0;
                      _phaseVec[i].srcPes     = NULL;
                      _phaseVec[i].recvBufs   = NULL;
                      _phaseVec[i].chunksRcvd = NULL;
                      _phaseVec[i].srcHints   = NULL;
                      _phaseVec[i].mrecv      = NULL;
                    }

                _phaseVec[i].sconnId = (unsigned) -1;

                // Don't index past our allocation
                if(indexDstPe < _numDstPes)
                    {
                      // setup dst info
                      _phaseVec[i].dstPes    = _all_dstPes + indexDstPe;
                      _phaseVec[i].dstHints  = _all_dstHints  + indexDstPe;
                      _sched->getDstPeList(i,
                                           _phaseVec[i].dstPes,
                                           _phaseVec[i].numDstPes,
                                           _phaseVec[i].dstHints);

                      if(_phaseVec[i].numDstPes)
                          {
                            indexDstPe += _phaseVec[i].numDstPes;

                            ///When there is more than one destination we assume they share
                            ///the connection id of dstpes[0] This is a temporary solution
                            ///till we switch multisend multicast to have a different
                            ///connection id for each message
                            bool bcast_phase = false;
                            if(_bcastSendPhase > 0)
                                {
                                  if(i >= _bcastSendPhase)
                                    bcast_phase = true;
                                }
                            else if(_root == -1 && _bcastRecvPhase == -1)
                                {
                                  if(i > _lastCombinePhase) ///There is a send on this node after
                                    ///the last combine phase but no
                                    ///broadcast to receive, must be root!
                                    bcast_phase = true;
                                }

                            ///XMI_assert (_color > 0); /// Most protocols set color to undefined, which we should change in the long run

                            if(bcast_phase)
                              //Use the broadcast connection manager
                              _phaseVec[i].sconnId = _bconnmgr->getConnectionId (_commid, _root, _color, i, _phaseVec[i].dstPes[0]);
                            else
                              //Use the reduction connection manager
                              _phaseVec[i].sconnId = _rconnmgr->getConnectionId (_commid, _root, _color, i, _phaseVec[i].dstPes[0]);
                          }
                      else // No destination/send processing this phase.
                          {
                            // setup dst info
                            _phaseVec[i].dstPes    = NULL;
                            _phaseVec[i].dstHints  = NULL;
                          }

                      TRACE_STATE ((stderr,"<%#.8X>Executor::AllreduceState::constructPhaseData() phaseVec[%#X]<%#.8X>:<%#.8X><%#.8X>, connid = %d\n",
                                    (int)this, i,
                                    (int)&_phaseVec[i],
                                    (int)_phaseVec[i].dstPes,
                                    (int)_phaseVec[i].dstHints,
                                    (int)_phaseVec[i].sconnId));
                    }
                else // Better be no destination/send processing this phase.
                    {
                      // setup dst info
                      _phaseVec[i].numDstPes = 0;
                      _phaseVec[i].dstPes    = NULL;
                      _phaseVec[i].dstHints  = NULL;
                    }

                TRACE_SCHEDULE((stderr,"<%#.8X>Executor::AllreduceState::constructPhaseData() i:%#X numsrc:%#X "
                                "src[0]:%#X srchints:%#X numdst:%#X dst[0]:%#X dsthints:%#X\n",(int)this,
                                i,
                                _phaseVec[i].numSrcPes ,
                                _phaseVec[i].numSrcPes ? _phaseVec[i].srcPes[0]:-1,
                                _phaseVec[i].numSrcPes ? _phaseVec[i].srcHints[0]:-1,
                                _phaseVec[i].numDstPes ,
                                _phaseVec[i].numDstPes ? _phaseVec[i].dstPes[0]:-1,
                                _phaseVec[i].numDstPes ? _phaseVec[i].dstHints[0]:-1));
#ifdef XMI_DEBUG_SCHEDULE
                for(unsigned j = 1; j < _phaseVec[i].numSrcPes; ++j)
                  TRACE_SCHEDULE((stderr,"<%#.8X>Executor::AllreduceState::constructPhaseData() src[%#X]:%#X srchints:%#X\n",(int)this,j,
                                  _phaseVec[i].srcPes[j],_phaseVec[i].srcHints[j]));

                for(unsigned j = 1; j < _phaseVec[i].numDstPes; ++j)
                  TRACE_SCHEDULE((stderr,"<%#.8X>Executor::AllreduceState::constructPhaseData() dst[%#X]:%#X dsthints:%#X\n",(int)this,j,
                                  _phaseVec[i].dstPes[j],_phaseVec[i].dstHints[j]));
#endif // XMI_DEBUG_SCHEDULE
              } // for(int i = _startPhase; i <= _endPhase; i++)


          if(_bcastSendPhase > 0)
              {
                // Combine all broadcast sends into one phase
                for(int i = _bcastSendPhase + 1; i <= _endPhase; i++)
                    {
                      TRACE_STATE((stderr,"<%#.8X>Executor::AllreduceState::constructPhaseData() _lastReducePhase %#X, _bcastRecvPhase %#X,bcastSendPhase %#X, i %#X\n",
                                   (int)this,_lastReducePhase, _bcastRecvPhase, _bcastSendPhase, i));
                      compressPhaseNumDstPes (_bcastSendPhase, i);
                    }
              }

          if(_bcastRecvPhase == -1 && _root == -1)
              {
                int i = 0;
                for(i = _lastCombinePhase+1; i <= _endPhase; i++)
                    {
                      if(_phaseVec[i].numDstPes)
                        break;
                    }
                int first_send_phase = i;
                i++;
                for(; i <= _endPhase; i++)
                    {
                      compressPhaseNumDstPes (first_send_phase, i);
                    }
              }

          if (_nextActivePhase)
            CCMI_Free(_nextActivePhase);

          _nextActivePhase = (unsigned *) CCMI_Alloc (sizeof(unsigned) * (_endPhase + 1));

          //the next active phase after endphase is a dummy place holder
          int cur_active_phase = _endPhase + 1;

          for (int p = _endPhase; p >= _startPhase; p --) {
            _nextActivePhase[p] = cur_active_phase;
            if ((getPhaseNumSrcPes(p) > 0) || (getPhaseNumDstPes(p) > 0)) {
              cur_active_phase = p;
            }
          }

          TRACE_STATE((stderr,"<%#.8X>Executor::AllreduceState::constructPhaseData() exit\n",(int)this));
        }

      void  setupReceives(unsigned infoRequired)
        {

          TRACE_ALERT((stderr,"<%#.8X>Executor::AllreduceState::setupReceives ALERT: Receive data being reset\n",(int)this));

          // setup/allocate receive request objects and bufs

          // How many requests might we receive per srcPe?  "infoRequired" indicates we
          // are using recv head callback and need 1 per chunk per srcPE.  Otherwise we're doing postReceive
          // processing which means only one postReceive per srcPE.
          /// \todo we have over allocated callback mode when we start reusing these:
          ///         _phaseVec[p].mrecv[scount].request = request;
          ///         _phaseVec[p].mrecv[scount].cb_done.clientdata = rdata;

          unsigned numRequests = infoRequired? (_lastChunk + 1) : 1;

          unsigned alignedBytes = ((_bytes + 15)/16) * 16; // Buffers need to be 16 byte aligned

          /// \todo maybe one too many mallocs?  for the final non-combine receive buf?
          unsigned allocationNewSize =
            ((_numSrcPes * numRequests) * sizeof(XMI_Request_t)) +         // _recvReq
            ((_numSrcPes * numRequests) * (sizeof(RecvCallbackData) + 4)) + // _recvClientData (padded to 16 bytes)
            (_numSrcPes * alignedBytes) +                                   // _bufs
            (((_root == -1) | (_root == (int)_myRank))? 0 : alignedBytes);  // We need a temp buffer on non-root nodes


          TRACE_STATE((stderr,"<%#.8X>Executor::AllreduceState::setupReceives() _numSrcPes<%#.8X> new size<%#.8X> old size<%#.8X> numRequests<%#.8X> alignedBytes<%#.8X> sizeOfBuffers<%#.8X>\n",(int)this,
                       _numSrcPes, allocationNewSize, _receiveAllocationSize, numRequests, alignedBytes, _sizeOfBuffers));
          /// \todo only grows, never shrinks?  runtime vs memory efficiency?
          if(allocationNewSize > _receiveAllocationSize)
              {
                TRACE_ALERT((stderr,"<%#.8X>Executor::AllreduceState::setupReceives ALERT: Receive data being reallocated %#X < %#X, _numSrcPes %#X\n",(int)this,
                             _receiveAllocationSize, allocationNewSize, _numSrcPes));
                TRACE_STATE((stderr,"<%#.8X>Executor::AllreduceState::setupReceives() allocate new size<%#.8X> old size<%#.8X>\n",(int)this,
                             allocationNewSize, _receiveAllocationSize));

                if(_receiveAllocation) CCMI_Free(_receiveAllocation);
                _receiveAllocation = CCMI_Alloc(allocationNewSize);

#ifdef XMI_DEBUG
                memset(_receiveAllocation, 0xFE, allocationNewSize);

                if(!_receiveAllocation)
                  fprintf(stderr,"<%#.8X>CCMI_Alloc failed<%#.8X> free'd %#X, malloc'd %#X\n",(int)this,
                          (int)_receiveAllocation,_receiveAllocationSize, allocationNewSize);
#endif
                XMI_assert(_receiveAllocation);

                _receiveAllocationSize = allocationNewSize;
                _sizeOfBuffers = alignedBytes;
              }
          else
            // We don't want to change (shrink) the sizeOfBuffers unless we have to.  So the next two if's are
            // a little weird, but necessary.  They handle being here for schedule changes that may affect
            // how many buffers we have carved the allocation into.
            if(_sizeOfBuffers <= alignedBytes)
              // Need bigger buffers but not a bigger allocation?  Must have shrunk _numSrcPes.  Readjust size of buffers.
              _sizeOfBuffers = alignedBytes;
            else // Allocated larger buffers than needed, see if that's still ok... // Size of buffers may have changed (shrunk) while numSrcPe's may have grown.
                {
                  // See if the current allocation supports the buffer size.  If not, adjust buffer size.
                  unsigned maxAllocationSize =
                    ((_numSrcPes * numRequests) * sizeof(XMI_Request_t)) +         // _recvReq
                    ((_numSrcPes * numRequests) * (sizeof(RecvCallbackData) + 4)) + // _recvClientData (padded to 16 bytes)
                    (_numSrcPes * _sizeOfBuffers) +                                   // _bufs
                    (((_root == -1) | (_root == (int)_myRank))? 0 : _sizeOfBuffers);  // We need a temp buffer on non-root nodes

                  if(maxAllocationSize > _receiveAllocationSize)
                    _sizeOfBuffers = alignedBytes;
                }

          _recvReq = (XMI_Request_t *) _receiveAllocation;

          _recvClientData = (RecvCallbackData *) ((char*)_recvReq + ((_numSrcPes * numRequests) * sizeof(XMI_Request_t)));

          _bufs = (char *) _recvClientData + ((_numSrcPes * numRequests) * (sizeof(RecvCallbackData) + 4));

          // We allocated a temp buffer only on non-root nodes
          if((_root == -1) | (_root == (int)_myRank))
            _tempBuf = NULL;
          else
            _tempBuf = (char*) _bufs + (_numSrcPes * _sizeOfBuffers);

          for(int i = 0, offset = 0; i < _numSrcPes; i++, offset += _sizeOfBuffers)
            _all_recvBufs[ i ] = _bufs + offset;

          TRACE_STATE((stderr,"<%#.8X>Executor::AllreduceState::setupReceives() _bufs:%08X all[0]:%08X all[1]:%08X all[2]:%08X all[3]:%08X all[4]:%08X tempbuf:%08X\n",(int)this,
                       (unsigned)_bufs,(unsigned)_all_recvBufs[0],
                       (unsigned)_all_recvBufs[1],(unsigned)_all_recvBufs[2],
                       (unsigned)_all_recvBufs[3], (unsigned)_all_recvBufs[4],
                       (unsigned) _tempBuf));

          _isConfigChanged = false;

          // clear received chunk counters
          memset(_all_chunks, 0, _numSrcPes * sizeof(unsigned));

          for(int idx = _startPhase; idx <= _endPhase; idx++)
              {
                _phaseVec[idx].chunksSent = 0;
                _phaseVec[idx].totalChunksRcvd = 0;
              }

          unsigned nextRecvData = 0;
          int p = _startPhase;
          unsigned pwidth = getPipelineWidth();
          unsigned bytes  = getBytes();

          for(p = _startPhase; p <= _endPhase; p++)
              {
                if(_phaseVec[p].numSrcPes > 0)
                    {
                      for(unsigned scount = 0; scount < _phaseVec[p].numSrcPes; scount ++)
                          {
                            XMI_Request_t *request = getRecvReq() + nextRecvData;
                            RecvCallbackData *rdata = getRecvClient(nextRecvData);
                            rdata->allreduce        = _executor;
                            rdata->phase            = p;
                            rdata->srcPeIndex       = scount;

                            _phaseVec[p].mrecv[scount].request = (xmi_quad_t*)request;
                            _phaseVec[p].mrecv[scount].cb_done.clientdata = rdata;
                            _phaseVec[p].mrecv[scount].op = _op;
                            _phaseVec[p].mrecv[scount].dt = _dt;
                            _phaseVec[p].mrecv[scount].rcvbuf = _phaseVec[p].recvBufs[scount];
                            _phaseVec[p].mrecv[scount].bytes  = bytes;
                            _phaseVec[p].mrecv[scount].pipelineWidth = pwidth;

                            nextRecvData ++;
                          }
                    }
              }
        }
    }; // Allreduce
  };
};// CCMI::Executor







#endif /* __allreduce_state_h__ */
