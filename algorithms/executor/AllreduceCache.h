/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/executor/AllreduceCache.h
 * \brief ???
 */
#ifndef __algorithms_executor_AllreduceCache_h__
#define __algorithms_executor_AllreduceCache_h__

#include <pami.h>
#include "algorithms/interfaces/Schedule.h"
#include "algorithms/interfaces/Executor.h"
#include "algorithms/ccmi.h"
#include "util/ccmi_util.h"
#include "util/common.h"
#include "util/ccmi_debug.h"
#include "math/math_coremath.h"
#include "algorithms/executor/ScheduleCache.h"

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

    /// client data for multisend receive done callback
    typedef struct AC_RecvCallbackData_t
    {
      Interfaces::Executor  * allreduce;   //Allreduce state needs to work with
      //multiple allreduce executors SK 11/01
      unsigned    phase;
      unsigned    srcPeIndex;
    } AC_RecvCallbackData;

    /// client data for multisend send done callback
    typedef struct AC_SendCallbackData_t
    {
      Interfaces::Executor       * me;   //Allreduce state needs to work with
      //multiple allreduce executors SK 11/01
      bool             isDone;
    } AC_SendCallbackData;

    template<class T_Conn> class AllreduceCache
    {
      protected:
        struct AllreduceParams
        {
          unsigned          _count;         /// total count of datatypes on operation
          unsigned          _bytes;         /// # of bytes (not count of datatypes)
          unsigned          _sizeOfType;    /// Size of the data type
          pami_op            _op;            /// Arithmatic operation to be performed
          pami_dt            _dt;            /// Data type
          unsigned          _pipewidth;     /// Pipeline width of the allreduce

          AllreduceParams():
              _count(0),
              _bytes(0),
              _sizeOfType(0),
              _op(PAMI_OP_COUNT),
              _dt(PAMI_DT_COUNT),
              _pipewidth(0)
          {
          }

        };

        struct PhaseState
        {
          unsigned     totalChunksRcvd;  // # of chunks received in this phase
          char      ** recvBufs;     // receive buffers for this phase
          PAMI::PipeWorkQueue    * pwqs;
          PAMI::PipeWorkQueue      dpwq; // Destination pipe work queue
          unsigned  *  chunksRcvd;   // # of chunks received from each srcrank
          unsigned     chunksSent;   // # chunks sent in this phase
          unsigned     sconnId;      // # sender connection id for this phase
          pami_multicast_t   *mrecv;

          inline PhaseState() {}
        };

        ////////////// Parameters //////////////
        AllreduceParams     _pcache;
        unsigned            _lastChunk;
        unsigned            _lastChunkCount;
        unsigned            _fullChunkCount;
        unsigned            _sizeOfBuffers;
        int                 _dstPhase;

        /////// Utilities /////////////////
        Interfaces::Executor * _executor;
        unsigned            _iteration;
        unsigned            _protocol;
        unsigned            _myrank;
        unsigned            _commid;   /// Communicator identifier
        unsigned            _color;    /// Color of the collective
        T_Conn            * _rconnmgr;  /// the connection manager for reductions
        T_Conn            * _bconnmgr;  /// the connection manager for broadcast

        /// dynamically allocated buffer space (for received data)
        void              * _scheduleAllocation;
        unsigned            _scheduleAllocationSize;
        void              * _receiveAllocation;
        unsigned            _receiveAllocationSize;

        /// pointers into the receive allocation
        char              * _tempBuf;
        char              * _bufs;
        PAMI_Request_t    * _recvReq;
        AC_RecvCallbackData  * _recvClientData;

        /// buffer space for all phases (pointers into schedule allocation)
        char               ** _all_recvBufs;
        unsigned            * _all_chunks  ;
        PAMI::PipeWorkQueue  * _all_pwqs;
        pami_multicast_t     * _all_mrecvs;

        /// global state information
        PhaseState       * _phaseVec; // pointer into schedule allocation

        bool                  _isConfigChanged;
        ScheduleCache       * _scache;

      public:
        AllreduceCache(ScheduleCache *cache, unsigned myrank) :
            _pcache(),
            _lastChunk(0),
            _lastChunkCount(0),
            _fullChunkCount(0),
            _sizeOfBuffers(0),
            _dstPhase(PAMI_UNDEFINED_PHASE),
            _iteration((unsigned) - 1),
            _myrank(myrank),
            _commid ((unsigned) - 1),
            _color  ((unsigned) - 1),
            _rconnmgr (NULL),
            _bconnmgr (NULL),
            _scheduleAllocation(NULL),
            _scheduleAllocationSize(0),
            _receiveAllocation(NULL),
            _receiveAllocationSize(0),
            _tempBuf(NULL),
            _bufs(NULL),
            _recvReq(NULL),
            _recvClientData(NULL),
            _phaseVec(NULL),
            _isConfigChanged(true),
            _scache(cache)
        {
        }

        virtual ~AllreduceCache() { freeAllocations(); }

        void setIteration(unsigned iteration) { _iteration = iteration; }

        void init(unsigned         count,
                  unsigned         sizeOfType,
                  pami_op          op,
                  pami_dt          dt,
                  unsigned         pipelineWidth)
        {
          TRACE_FORMAT("count %u, size %u, op %u, dt %u, pipelineWidth %u",
                        count, sizeOfType, op, dt, pipelineWidth);
                        
          if ((_pcache._pipewidth == pipelineWidth) &&
              (_pcache._sizeOfType    == sizeOfType) &&
              (_pcache._bytes         == count * sizeOfType))
            return;

          _pcache._op         = op;
          _pcache._dt         = dt;
          _pcache._count      = count;
          _pcache._sizeOfType = sizeOfType;
          _pcache._bytes      = count * sizeOfType;
          updatePipelineWidth(pipelineWidth);

          //printf ("In AllreduceCache::init bytes = %x", _pcache._bytes);
        }

        void updatePipelineWidth (unsigned pw);

        /////  Query Allreduce Parameters //////
        unsigned getCount()
        {
          return _pcache._count;
        }

        unsigned getBytes()
        {
          return _pcache._bytes;
        }

        unsigned getSizeOfType()
        {
          return _pcache._sizeOfType;
        }

        pami_op getOp()
        {
          return _pcache._op;
        }

        pami_dt getDt()
        {
          return _pcache._dt;
        }

        unsigned getPipelineWidth()
        {
          return _pcache._pipewidth;
        }

        unsigned getLastChunkCount()
        {
          return _lastChunkCount;
        }

        unsigned getFullChunkCount()
        {
          return _fullChunkCount;
        }
        unsigned getLastChunk()
        {
          return _lastChunk;
        }

        /// Query Receive Buffers ////
        char* getTempBuf()
        {
          return _tempBuf;
        }

        char      *           getPhaseRecvBufs(unsigned index, unsigned jindex)
        {
          return  _phaseVec[index].recvBufs[jindex]  ;
        }

        unsigned              getPhaseChunksRcvd(unsigned index, unsigned jindex)
        {
          return  _phaseVec[index].chunksRcvd[jindex];
        }
        unsigned              getPhaseTotalChunksRcvd(unsigned index)
        {
          return  _phaseVec[index].totalChunksRcvd;
        }
        unsigned              getPhaseChunksSent(unsigned index)
        {
          return  _phaseVec[index].chunksSent;
        }
        unsigned               getPhaseSendConnectionId (unsigned index)
        {
          return  _phaseVec[index].sconnId;
        }
        PAMI::PipeWorkQueue  *getPhasePipeWorkQueues (unsigned index, unsigned jindex)
        {
          return  &_phaseVec[index].pwqs[jindex];
        }

        PAMI::PipeWorkQueue  *getPhaseDstPipeWorkQueue (unsigned index)
        {
          return  &_phaseVec[index].dpwq;
        }

        void                 incrementPhaseChunksRcvd(unsigned index, unsigned jindex, unsigned val = 1)
        {
          TRACE_FN_ENTER();
          _phaseVec[index].chunksRcvd[jindex] += val;
          _phaseVec[index].totalChunksRcvd += val;

          TRACE_FORMAT( "Incrementing chunk recvd for phase %d srcidx %d final val %d",
                      index, jindex,
                      _phaseVec[index].chunksRcvd[jindex]);
          TRACE_FN_EXIT();
        }
        void                  incrementPhaseChunksSent(unsigned index, unsigned val = 1)
        {
          _phaseVec[index].chunksSent += val;
        }

        AC_RecvCallbackData * getRecvClient(unsigned index)
        {
          return _recvClientData + index;
        }
        Interfaces::Executor * getRecvClientAllreduce(unsigned index)
        {
          return _recvClientData[index].allreduce;
        }
        unsigned    getRecvClientPhase(unsigned index)
        {
          return _recvClientData[index].phase;
        }
        unsigned    getRecvClientSrcPeIndex(unsigned index)
        {
          return _recvClientData[index].srcPeIndex;
        }

        PAMI_Request_t   * getRecvReq()
        {
          return  _recvReq;
        }

        unsigned getIteration() { return _iteration; }

        void setRecvClientAllreduce(unsigned index, Interfaces::Executor * value)
        {
          _recvClientData[index].allreduce = value;
        }

        void setRecvClientPhase(unsigned index, unsigned value)
        {
          _recvClientData[index].phase = value;
        }
        void setRecvClientSrcPeIndex(unsigned index, unsigned value)
        {
          _recvClientData[index].srcPeIndex = value;
        }
        void incrementRecvClientSrcPeIndex(unsigned index)
        {
          ++_recvClientData[index].srcPeIndex;
        }

        /// Utilities /////
        void setProtocol(unsigned protocol)
        {
          if (_protocol != protocol)
            {
              _isConfigChanged = true;
            }

          _protocol = protocol;
          return ;
        }
        unsigned getProtocol()
        {
          return _protocol;
        }

        /// \brief set the Connection manager
        void setReduceConnectionManager (T_Conn  *connmgr)
        {
          _rconnmgr = connmgr;
        }

        /// \brief set the Connection manager
        void setBroadcastConnectionManager (T_Conn  *connmgr)
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

        void setExecutor (Interfaces::Executor *exec)
        {
          _executor = exec;
        }

        void reset(bool rflag, bool infoRequired)
        {
          TRACE_FN_ENTER();
          /////////////////*****  Reset Pipe Work Queues  **********///////////////
          TRACE_FORMAT( "<%p>", this);
          CCMI_assert(_pcache._bytes > 0);

          //Hard reset the cache
          if (rflag) _isConfigChanged = true;

          // Do minimal setup if the config hasn't changed.
          if (!_isConfigChanged)
            {
              unsigned idx = 0;
              //Make xlc happy as it thinks _all_chunks may overwrite the this
              //pointer
              unsigned *chunks = _all_chunks;

              for (idx = 0; idx < _scache->getNumTotalSrcRanks(); idx++)
                chunks [idx] = 0;

              for (idx = _scache->getStartPhase(); idx <= _scache->getEndPhase(); idx++)
                {
                  _phaseVec[idx].chunksSent = 0;
                  _phaseVec[idx].totalChunksRcvd = 0;
                }
            }
          else    //The configuration has changed
            {
              constructPhaseData();
              setupReceives(infoRequired);
            }
          TRACE_FN_EXIT();
        }

        ///
        ///  \brief Move the code for reading phase state to an
        ///  un-inlined function. This will help improve compiler
        ///  performance and possibly reduce I-cache misses
        ///
        void constructPhaseData ();

        /// \ brief Set the final receive buffer to be the dstbuf, if appropriate.
        ///
        /// \param[in]  pdstbuf pointer to the destination buffer pointer.
        void setDstBuf(char** pdstbuf)
        {
          TRACE_FN_ENTER();
          if (_dstPhase != (int) PAMI_UNDEFINED_PHASE)
            {
              // We only use the destination buffer if we're a root or it's allreduce.  Otherwise we use
              // a temporary buffer.
              _phaseVec[_dstPhase].recvBufs = ((_scache->getRoot() == -1) |
                                               (_scache->getRoot() == (int)_myrank)) ?
                                              pdstbuf :  // Our target is the final buffer
                                              &_tempBuf; // Our target is a temp buffer

              // The mrecv structure has to match, but we assume this only works for 1 src pe?  We
              // only have a dst phase, not a dst src pe index (so mrecv[0]).  How would a final multi-
              // src receive phase work?  Probably not an issue, but assert anyway.
              PAMI_assert(_scache->getNumSrcRanks(_dstPhase) == 1);
              _phaseVec[_dstPhase].mrecv[0].dst = (pami_pipeworkqueue_t *) & _phaseVec[_dstPhase].pwqs[0];

              TRACE_FORMAT( "<%p>dstbuf(%p) dstPhase(%#X) _phaseVec[_dstPhase].recvBufs(%p)",
                          this,
                          *pdstbuf,
                          _dstPhase,
                          *_phaseVec[_dstPhase].recvBufs
                         );
            }
          TRACE_FN_EXIT();
        }

        /// \brief Setup receive structures and allocate buffers
        ///
        /// Call resetPhaseData() before setupReceives() to setup the schedule and phase
        /// structures.
        void setupReceives(bool infoRequired);

        pami_multicast_t  *   getPhaseMcastRecv(unsigned index, unsigned jindex)
        {
          return &_phaseVec[index].mrecv[jindex];
        }


        /// \brief Free all allocations if over the limit
        void freeAllocations(unsigned limit = 0)
        {
          TRACE_FN_ENTER();
          if ((_receiveAllocationSize) > limit)
            {
              TRACE_FORMAT( "<%p>limit(%#.8X) ALERT: Allocation freed, %p(%#X bytes), %p(%#X bytes)", this,
                          limit, _scheduleAllocation, _scheduleAllocationSize, _receiveAllocation, _receiveAllocationSize);

#ifdef CCMI_DEBUG
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

              _pcache._count         = 0;
              _pcache._pipewidth     = 0;
              _pcache._sizeOfType    = 0;
              _pcache._bytes         = 0;
              _lastChunk             = 0;
              _lastChunkCount        = 0;
              _fullChunkCount        = 0;
              _sizeOfBuffers         = 0;

              _all_recvBufs  = NULL;
              _all_chunks    = NULL;
              _all_mrecvs    = NULL;

              _phaseVec      = NULL;
              _tempBuf       = NULL;
              _bufs          = NULL;
              _recvReq       = NULL;
              _recvClientData = NULL;
              _dstPhase      = PAMI_UNDEFINED_PHASE;
#if 0

              if (_nextActivePhase)
                {
                  __global.heap_mm->free(_nextActivePhase);
                  _nextActivePhase = NULL;
                }

              _all_srcPes    = (unsigned*)0xFFFFFFF0;
              _all_srcHints  = (unsigned*)0xFFFFFFF1;
              _all_recvBufs  = (char**)0xFFFFFFF2;
              _all_dstPes    = (unsigned*)0xFFFFFFF3;
              _all_dstHints  = (unsigned*)0xFFFFFFF4;
              _all_chunks    = (unsigned*)0xFFFFFFF5;
              _all_mrecvs    = (MultiSend::CCMI_MulticastRecv_t*)0xFFFFFFF6;

              _phaseVec      = (PhaseState*)0xFFFFFFF7;

              _tempBuf       = (char             *)0xFFFFFFF8;
              _bufs          = (char             *)0xFFFFFFF9;
              _recvReq       = (PAMI_Request_t   *)0xFFFFFFFa;
              _recvClientData = (RecvCallbackData *)0xFFFFFFFb;
#endif
            }
          TRACE_FN_EXIT();
        }
    };
  };
};

template<class T_Conn>
inline void CCMI::Executor::AllreduceCache<T_Conn>::updatePipelineWidth
(unsigned pwidth)
{
  TRACE_FN_ENTER();
  _pcache._pipewidth = pwidth;
  unsigned bytes = _pcache._bytes;

  // Setup our "chunk" member data based on the pipeline width
  if (_pcache._pipewidth >= bytes)
    {
      _lastChunk      = 0;
      _fullChunkCount = 0;
      _lastChunkCount = bytes / _pcache._sizeOfType;
    }
  else
    {
      unsigned lastChunk = bytes / _pcache._pipewidth + (bytes % _pcache._pipewidth != 0) - 1;

      if (lastChunk > _lastChunk)
        {
          TRACE_FORMAT( "<%p>Pipelining grew %#X > %#X",
                      this, lastChunk, _lastChunk);
          _isConfigChanged = true;
        }

      _lastChunk = lastChunk;
      _fullChunkCount = _pcache._pipewidth / _pcache._sizeOfType;

      if (bytes % _pcache._pipewidth == 0)
        _lastChunkCount = _fullChunkCount;
      else
        _lastChunkCount = (bytes % _pcache._pipewidth) / _pcache._sizeOfType;
    }

  TRACE_FORMAT( "<%p>bytes:%#X "
              "pwidth:%#X lastChunk:%#X fullCount:%#X lastCount:%#X ", this,
              bytes, _pcache._pipewidth, _lastChunk, _fullChunkCount,
              _lastChunkCount);

  // We can reuse our existing buffer allocations if we aren't > the allocated size.
  if (bytes > _sizeOfBuffers)
    {
      TRACE_FORMAT( "<%p>Buffers too small %#X < %#X",
                  this, _sizeOfBuffers, bytes);
      _isConfigChanged = true;
    }

  ///Recreate multisend recv data structures
  if (!_isConfigChanged)
    {
      for (unsigned phase = _scache->getStartPhase(); phase <= _scache->getEndPhase(); phase++)
        for (unsigned scount = 0; scount < _scache->getNumSrcRanks(phase); scount ++)
          {
            pami_multicast_t *mrecv = &(_phaseVec[phase].mrecv[scount]);
            mrecv->bytes = _pcache._bytes;
            //mrecv->pipelineWidth = _pcache._pipewidth;
          }
    }

  TRACE_FORMAT( "<%p>", this);
  TRACE_FN_EXIT();
}


template<class T_Conn>
inline void CCMI::Executor::AllreduceCache<T_Conn>::constructPhaseData()
{
  TRACE_FN_ENTER();
  /// Calculate how much storage we need for all our schedule/phase data.
  unsigned allocationNewSize =
    ((_scache->getEndPhase() + 1) * sizeof(PhaseState)) +      // _phaseVec
    ( _scache->getNumTotalSrcRanks() *
      (sizeof(unsigned) +                          // _all_chunks
       sizeof(char*)  +                            // _all_recvBufs
       sizeof(pami_multicast_t) +
       sizeof(PAMI::PipeWorkQueue) ));    // _all_mrecvs

  /// \todo only grows, never shrinks?  runtime vs memory efficiency?
  if (allocationNewSize > _scheduleAllocationSize)
    {
      if (_scheduleAllocation)
        CCMI_Free(_scheduleAllocation);

      CCMI_Alloc(_scheduleAllocation, allocationNewSize);

      CCMI_assert(_scheduleAllocation);
      _scheduleAllocationSize = allocationNewSize;
    }

  /// Set the pointers appropriately into the allocated storage.
  _phaseVec = (PhaseState*) _scheduleAllocation;
  _all_recvBufs = (char**)   ((char*)_phaseVec     + ((_scache->getEndPhase() + 1) * sizeof(PhaseState)));
  _all_chunks   = (unsigned*)((char*)_all_recvBufs + (_scache->getNumTotalSrcRanks() * sizeof(char*)));
  _all_mrecvs   = (pami_multicast_t *) ((char*)_all_chunks   + (_scache->getNumTotalSrcRanks() * sizeof(unsigned)));
  _all_pwqs     = (PAMI::PipeWorkQueue *) ((char*)_all_mrecvs + (_scache->getNumTotalSrcRanks() * sizeof(pami_multicast_t)));

  // configure per phase state info structures
  /// \todo How about some data layout diagrams?
  unsigned indexSrcPe = 0;

  for (unsigned i = _scache->getStartPhase(); i <= _scache->getEndPhase(); i++)
    {
      unsigned connID   =  (unsigned) - 1;

      // Don't index past our allocation
      if (indexSrcPe < _scache->getNumTotalSrcRanks())
        {
          // setup src info
          TRACE_FORMAT( "<%p>_phaseVec[%u].recvBufs %p",
                      this, i, _phaseVec[i].recvBufs);
          _phaseVec[i].recvBufs   = &_all_recvBufs[indexSrcPe];
          _phaseVec[i].chunksRcvd = _all_chunks  + indexSrcPe;
          _phaseVec[i].mrecv      = _all_mrecvs  + indexSrcPe;
          _phaseVec[i].pwqs       = _all_pwqs    + indexSrcPe;

          // If there was a src pe, increment our src index into the allocated storage
          if (_scache->getNumSrcRanks(i) > 0)
            {
              for (unsigned scount = 0; scount < _scache->getNumSrcRanks(i); scount ++)
                {
                  PAMI::Topology *topology = _scache->getSrcTopology(i);
                  pami_task_t *ranks = NULL;
                  topology->rankList(&ranks);

                  unsigned srcrank  =  ranks[scount];

                  if (i <= _scache->getLastReducePhase())
                    connID = _rconnmgr->getRecvConnectionId (_commid, _scache->getRoot(), srcrank, i, _color);
                  else
                    connID = _bconnmgr->getRecvConnectionId (_commid, _scache->getRoot(), srcrank, i, _color);

                  pami_multicast_t *mrecv = &(_phaseVec[i].mrecv[scount]);
                  mrecv->connection_id = connID;
                  mrecv->bytes = _pcache._bytes;
                  //mrecv->pipelineWidth = _pcache._pipewidth;
                  //mrecv->opcode = (CCMI_Subtask) _scache->getSrcSubtasks(i)[scount];
                }

              indexSrcPe += _scache->getNumSrcRanks(i);

              PAMI::PipeWorkQueue *dpwq = &_phaseVec[i].dpwq;
              new (dpwq) PAMI::PipeWorkQueue();
              dpwq->configure (_tempBuf, _pcache._bytes, 0);
              dpwq->reset();
            }
          else // No source/receive processing this phase.
            {
              _phaseVec[i].recvBufs   = NULL;
              _phaseVec[i].chunksRcvd = NULL;
              _phaseVec[i].mrecv      = NULL;
            }
        }
      else // Better be no source/receive processing this phase.
        {
          // setup src info
          _phaseVec[i].recvBufs   = NULL;
          _phaseVec[i].chunksRcvd = NULL;
          _phaseVec[i].mrecv      = NULL;
        }

      if (_scache->getNumDstRanks(i) > 0)
        {
          PAMI::Topology * topology = _scache->getDstTopology(i);
          pami_task_t *dstranks = NULL;
          topology->rankList(&dstranks);

          if (i <= _scache->getLastReducePhase())
            //Use the broadcast connection manager
            _phaseVec[i].sconnId = _rconnmgr->getConnectionId (_commid, _scache->getRoot(), _color, i,
                                                               dstranks[0]);
          else
            //Use the reduction connection manager
            _phaseVec[i].sconnId = _bconnmgr->getConnectionId (_commid, _scache->getRoot(), _color, i,
                                                               dstranks[0]);
        }
    }
  TRACE_FN_EXIT();
}

template<class T_Conn>
inline void  CCMI::Executor::AllreduceCache<T_Conn>::setupReceives(bool infoRequired)
{
  TRACE_FN_ENTER();
  TRACE_FORMAT( "<%p>Receive data being reset", this);

  // setup/allocate receive request objects and bufs

  /// \todo not sure this alignment is needed anymore but leaving it.
  COMPILE_TIME_ASSERT((sizeof(PAMI_Request_t) % 16) == 0);     // Need 16 byte alignment?
  /// \todo not sure this alignment is needed anymore, and it's removed because it doesn't compile in 64 bit
//  COMPILE_TIME_ASSERT(((sizeof(AC_RecvCallbackData) + 4)%16)==0); // Need 16 byte alignment?

  // How many requests might we receive per srcPe?  "infoRequired" indicates we
  // are using recv head callback and need 1 per chunk per srcPE.  Otherwise we're doing postReceive
  // processing which means only one postReceive per srcPE.
  /// \todo we have over allocated callback mode when we start reusing these:
  ///         _phaseVec[p].mrecv[scount].request = request;
  ///         _phaseVec[p].mrecv[scount].cb_done.clientdata = rdata;

  unsigned numRequests = infoRequired ? (_lastChunk + 1) : 1;
  unsigned alignedBytes = ((_pcache._bytes + 15) / 16) * 16; // Buffers need to be 16 byte aligned

  /// \todo maybe one too many mallocs?  for the final non-combine receive buf?
  unsigned allocationNewSize =
    ((_scache->getNumTotalSrcRanks() * numRequests) * sizeof(PAMI_Request_t)) +         // _recvReq
    ((_scache->getNumTotalSrcRanks() * numRequests) * (sizeof(AC_RecvCallbackData) + 4)) + // _recvClientData (padded to 16 bytes)
    (_scache->getNumTotalSrcRanks() * alignedBytes) +                                   // _bufs
    (((_scache->getRoot() == -1) | (_scache->getRoot() == (int)_myrank)) ? 0 : alignedBytes); // We need a temp buffer on non-root nodes


  /// \todo only grows, never shrinks?  runtime vs memory efficiency?
  if (allocationNewSize > _receiveAllocationSize)
    {
      if (_receiveAllocation) CCMI_Free(_receiveAllocation);

      CCMI_Alloc(_receiveAllocation, allocationNewSize);

      CCMI_assert(_receiveAllocation);

      _receiveAllocationSize = allocationNewSize;
      _sizeOfBuffers = alignedBytes;
    }
  else

    // We don't want to change (shrink) the sizeOfBuffers unless we have to.  So the next two if's are
    // a little weird, but necessary.  They handle being here for schedule changes that may affect
    // how many buffers we have carved the allocation into.
    if (_sizeOfBuffers <= alignedBytes)
      // Need bigger buffers but not a bigger allocation?  Must have shrunk _scache->getNumTotalSrcRanks().  Readjust size of buffers.
      _sizeOfBuffers = alignedBytes;
    else // Allocated larger buffers than needed, see if that's still ok... // Size of buffers may have changed (shrunk) while numSrcPe's may have grown.
      {
        // See if the current allocation supports the buffer size.  If not, adjust buffer size.
        unsigned maxAllocationSize =
          ((_scache->getNumTotalSrcRanks() * numRequests) * sizeof(PAMI_Request_t)) +         // _recvReq
          ((_scache->getNumTotalSrcRanks() * numRequests) * (sizeof(AC_RecvCallbackData) + 4)) + // _recvClientData (padded to 16 bytes)
          (_scache->getNumTotalSrcRanks() * _sizeOfBuffers) +                                   // _bufs
          (((_scache->getRoot() == -1) | (_scache->getRoot() == (int)_myrank)) ? 0 : _sizeOfBuffers); // We need a temp buffer on non-root nodes

        if (maxAllocationSize > _receiveAllocationSize)
          _sizeOfBuffers = alignedBytes;
      }

  _recvReq = (PAMI_Request_t *) _receiveAllocation;

  _recvClientData = (AC_RecvCallbackData *) ((char*)_recvReq + ((_scache->getNumTotalSrcRanks() * numRequests) * sizeof(PAMI_Request_t)));

  _bufs = (char *) _recvClientData + ((_scache->getNumTotalSrcRanks() * numRequests) * (sizeof(AC_RecvCallbackData) + 4));

  // We allocated a temp buffer only on non-root nodes
  if ((_scache->getRoot() == -1) | (_scache->getRoot() == (int)_myrank))
    _tempBuf = NULL;
  else
    _tempBuf = (char*) _bufs + (_scache->getNumTotalSrcRanks() * _sizeOfBuffers);

  for (unsigned i = 0, offset = 0; i < _scache->getNumTotalSrcRanks(); i++, offset += _sizeOfBuffers)
    {
      _all_recvBufs[ i ] = _bufs + offset;
    }

  TRACE_FORMAT( "<%p>_bufs:%p all[0]:%p all[1]:%p all[2]:%p all[3]:%p all[4]:%p tempbuf:%p",
              this,
              _bufs, _all_recvBufs[0],
              _all_recvBufs[1], _all_recvBufs[2],
              _all_recvBufs[3], _all_recvBufs[4],
              _tempBuf);

  _isConfigChanged = false;

  // clear received chunk counters
  memset(_all_chunks, 0, _scache->getNumTotalSrcRanks() * sizeof(unsigned));

  for (unsigned idx = _scache->getStartPhase(); idx <= _scache->getEndPhase(); idx++)
    {
      _phaseVec[idx].chunksSent = 0;
      _phaseVec[idx].totalChunksRcvd = 0;
    }

  unsigned nextRecvData = 0;
  unsigned p = _scache->getStartPhase();
//  unsigned pwidth = getPipelineWidth();
  unsigned bytes  = getBytes();

  for (p = _scache->getStartPhase(); p <= _scache->getEndPhase(); p++)
    {
      if (_scache->getNumSrcRanks(p) > 0)
        {
          for (unsigned scount = 0; scount < _scache->getNumSrcRanks(p); scount ++)
            {
              TRACE_FORMAT( "<%p>p:%u, scount: %u, _phaseVec[p].recvBufs[scount]:%p, _all_recvBufs[%u]: %p",
                          this, p, scount, _phaseVec[p].recvBufs[scount], p*scount, _all_recvBufs[p*scount]);
//        PAMI_Request_t *request = getRecvReq() + nextRecvData;
              AC_RecvCallbackData *rdata = getRecvClient(nextRecvData);
              rdata->allreduce        = _executor;
              rdata->phase            = p;
              rdata->srcPeIndex       = scount;

              //_phaseVec[p].mrecv[scount].request = request;
              _phaseVec[p].mrecv[scount].cb_done.clientdata = rdata;
              /* Multicast doesnt take in op and dt arguments !!!! We need to add multicombine support*/
              //_phaseVec[p].mrecv[scount].op  = _pcache._op;
              //_phaseVec[p].mrecv[scount].dt  = _pcache._dt;
              _phaseVec[p].mrecv[scount].src = NULL;
              _phaseVec[p].mrecv[scount].dst = (pami_pipeworkqueue_t *) & _phaseVec[p].pwqs[scount];
              _phaseVec[p].mrecv[scount].bytes  = bytes;
              //        _phaseVec[p].mrecv[scount].pipelineWidth = pwidth;

              nextRecvData ++;

              CCMI_assert (_pcache._bytes != 0);
              CCMI_assert (_phaseVec[p].recvBufs[scount] != NULL);
              PAMI::PipeWorkQueue *pwq = &_phaseVec[p].pwqs[scount];
              new (pwq) PAMI::PipeWorkQueue();
              pwq->configure (_phaseVec[p].recvBufs[scount], _pcache._bytes, 0);
              pwq->reset();
              CCMI_assert (pwq->bufferToProduce() != NULL);

              TRACE_FORMAT( "<%p>Buffer for phase %d index %d is %p", this, p, scount, pwq->bufferToProduce());
            }
        }
    }
  TRACE_FN_EXIT();
}

#undef  DO_TRACE_ENTEREXIT
#undef  DO_TRACE_DEBUG

#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
