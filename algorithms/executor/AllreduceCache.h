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
    template<class T_Conn> class AllreduceCache
    {
      protected:
        struct AllreduceParams
        {
          unsigned          _count;         /// total count of datatypes on operation
          unsigned          _bytes;         /// # of bytes (not count of datatypes)
          unsigned          _sizeOfType;    /// Size of data type
          pami_op           _op;            /// Arithmatic operation to be performed
          pami_dt           _dt;            /// Primitive data type
          TypeCode        * _stype;         /// Send data type
          TypeCode        * _rtype;         /// Recv data type
          unsigned          _pipewidth;     /// Pipeline width of the allreduce

          AllreduceParams():
              _count(0),
              _bytes(0),
              _sizeOfType(0),
              _op(PAMI_OP_COUNT),
              _dt(PAMI_DT_COUNT),
              _stype((TypeCode *)PAMI_TYPE_BYTE),
              _rtype((TypeCode *)PAMI_TYPE_BYTE),
              _pipewidth(0)
          {
          }

        };

        struct PhaseState
        {
          char                 ** recvBufs; // receive buffers for this phase
          PAMI::PipeWorkQueue   * pwqs;
          unsigned                sconnId;      // # sender connection id for this phase

          inline PhaseState() {}
        };

        ////////////// Parameters //////////////
        AllreduceParams     _pcache;
        unsigned            _lastChunk;
        unsigned            _lastChunkCount;
        unsigned            _fullChunkCount;
        unsigned            _sizeOfBuffers;

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
	char              * _dstbuf;
        char              * _bufs;

        /// buffer space for all phases (pointers into schedule allocation)
        char               ** _all_recvBufs;
        PAMI::PipeWorkQueue  * _all_pwqs;
	PAMI::PipeWorkQueue    _destpwq; // Destination pipe work queue
        PAMI::PipeWorkQueue    _bcastpwq;

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
            _phaseVec(NULL),
            _isConfigChanged(true),
            _scache(cache)
        {
        }

        virtual ~AllreduceCache() { freeAllocations(); }

        void setIteration(unsigned iteration) { _iteration = iteration; }

        void init(unsigned         count,
                  unsigned         sizeOfType,/*SSS: This has to be type->GetDataSize() not the size of primitive type*/
                  pami_op          op,
                  pami_dt          dt,
                  TypeCode        *stype,
                  TypeCode        *rtype,
                  unsigned         pipelineWidth)
        {
          TRACE_FORMAT("count %u, sizeOfType %u, op %u, dt %u, pipelineWidth %u",
                        count, sizeOfType, op, dt, pipelineWidth);
                        
	  //Early arrival packets only have op, dt, count info with them
          if ((_pcache._pipewidth == pipelineWidth) &&
              (_pcache._sizeOfType    == sizeOfType) &&
              (_pcache._bytes         == count * sizeOfType) &&
              (_pcache._op         == op) &&
              (_pcache._dt         == dt))
            return;

          _pcache._op          = op;
          _pcache._dt          = dt;
          _pcache._count       = count;
          _pcache._sizeOfType  = sizeOfType;
          _pcache._bytes       = count * sizeOfType;
          _pcache._stype       = stype;
          _pcache._rtype       = rtype;
          updatePipelineWidth(pipelineWidth);

          TRACE_FORMAT("In AllreduceCache::init bytes = %x\n", _pcache._bytes);
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

        TypeCode * getStype()
        {
          return _pcache._stype;
        }

        TypeCode * getRtype()
        {
          return _pcache._rtype;
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

        char      *getPhaseRecvBufs(unsigned index, unsigned jindex)
        {
          return  _phaseVec[index].recvBufs[jindex];
        }
	
	char     **getPhaseRecvBufsVec(unsigned index)
	{
	  return  _phaseVec[index].recvBufs;
	}

	bool  isPhaseRecvActive(unsigned phase) {
	  return (_phaseVec[phase].recvBufs != NULL);	  
	}

        unsigned               getPhaseSendConnectionId (unsigned index)
        {
          return  _phaseVec[index].sconnId;
        }

        PAMI::PipeWorkQueue  *getPhasePipeWorkQueues (unsigned index, unsigned jindex)
        {
	  return  &_phaseVec[index].pwqs[jindex];
        }

        PAMI::PipeWorkQueue  *getDestPipeWorkQueue ()
	{
	  return  &_destpwq; 
	}
	
	PAMI::PipeWorkQueue  *getBcastPipeWorkQueue () 
	{
	  return  &_bcastpwq;
        }

        unsigned getIteration() { return _iteration; }


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

	unsigned getCommID () { return _commid; }

        /// \brief set the communicator id
        void setCommID (unsigned  commid)
        {
          _commid = commid;
        }

	unsigned getColor () { return _color; }

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
          //CCMI_assert(_pcache._bytes > 0);

          //Hard reset the cache
          if (rflag) _isConfigChanged = true;

          // Do minimal setup if the config hasn't changed.
          if (_isConfigChanged)
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
        void setDstBuf(char* pdstbuf)
        {
          TRACE_FN_ENTER();
	  if ((_scache->getRoot() == -1) | (_scache->getRoot() == (int)_myrank))
	    _dstbuf = pdstbuf;
	  else
	    _dstbuf = _tempBuf;

          TRACE_FN_EXIT();
        }

        /// \brief Setup receive structures and allocate buffers
        ///
        /// Call resetPhaseData() before setupReceives() to setup the schedule and phase
        /// structures.
        void setupReceives(bool infoRequired);

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
              _phaseVec      = NULL;
              _tempBuf       = NULL;
              _bufs          = NULL;

#if 0
              _all_srcPes    = (unsigned*)0xFFFFFFF0;
              _all_srcHints  = (unsigned*)0xFFFFFFF1;
              _all_recvBufs  = (char**)0xFFFFFFF2;
              _all_dstPes    = (unsigned*)0xFFFFFFF3;
              _all_dstHints  = (unsigned*)0xFFFFFFF4;
              _phaseVec      = (PhaseState*)0xFFFFFFF7;

              _tempBuf       = (char             *)0xFFFFFFF8;
              _bufs          = (char             *)0xFFFFFFF9;
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

  _isConfigChanged = true;

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
    ((_scache->getEndPhase() + 1) * sizeof(PhaseState)) + // _phaseVec
    ( _scache->getNumTotalSrcRanks() *
      (sizeof(char*)  +  sizeof(PAMI::PipeWorkQueue)));   // _all_recvBufs
  
  /// \todo only grows, never shrinks?  runtime vs memory efficiency?
  if (allocationNewSize > _scheduleAllocationSize)
    {
      if (_scheduleAllocation)
        CCMI_Free(_scheduleAllocation);
      
      CCMI_Alloc(_scheduleAllocation, allocationNewSize);

      CCMI_assert(_scheduleAllocation);
      _scheduleAllocationSize = allocationNewSize;
    }

  // Set the pointers appropriately into the allocated storage.
  // Pipework queues start aligned to 64 bytes
  _all_pwqs = (PAMI::PipeWorkQueue *) _scheduleAllocation;  
  _phaseVec = (PhaseState *) (_all_pwqs + _scache->getNumTotalSrcRanks()); 
  _all_recvBufs = (char**) ((char*)_phaseVec  + ((_scache->getEndPhase() + 1) * sizeof(PhaseState)));
        
  PAMI::PipeWorkQueue *dpwq = &_destpwq;
  new (dpwq) PAMI::PipeWorkQueue();
  dpwq->configure (_dstbuf, _pcache._bytes, 0);

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
	  _phaseVec[i].pwqs       = NULL;
		    
          // If there was a src pe, increment our src index into the
          // allocated storage
          if (_scache->getNumSrcRanks(i) > 0)
            {
	      if (i <= _scache->getLastReducePhase())
		_phaseVec[i].pwqs       = _all_pwqs    + indexSrcPe;
	      else {
		CCMI_assert (_scache->getNumSrcRanks(i) == 1);
		_phaseVec[i].pwqs       = &_bcastpwq;
	      }	      

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
                }

              indexSrcPe += _scache->getNumSrcRanks(i);
            }
          else // No source/receive processing this phase.
            {
              _phaseVec[i].recvBufs   = NULL;
            }
        }
      else // Better be no source/receive processing this phase.
        {
          // setup src info
          _phaseVec[i].recvBufs   = NULL;
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

  // How many requests might we receive per srcPe?  "infoRequired" indicates we
  // are using recv head callback and need 1 per chunk per srcPE.  Otherwise we're doing postReceive
  // processing which means only one postReceive per srcPE.
    
  unsigned alignedBytes = ((_pcache._bytes + 15) / 16) * 16; // Buffers need to be 16 byte aligned

  /// \todo maybe one too many mallocs?  for the final non-combine receive buf?
  unsigned allocationNewSize =
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
          (_scache->getNumTotalSrcRanks() * _sizeOfBuffers) +                                   // _bufs
          (((_scache->getRoot() == -1) | (_scache->getRoot() == (int)_myrank)) ? 0 : _sizeOfBuffers); // We need a temp buffer on non-root nodes

        if (maxAllocationSize > _receiveAllocationSize)
          _sizeOfBuffers = alignedBytes;
      }

  _bufs = (char *)_receiveAllocation; 

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

  unsigned p = _scache->getStartPhase();
  //  unsigned pwidth = getPipelineWidth();
  //  unsigned bytes  = getBytes();

  for (p = _scache->getStartPhase(); p <= _scache->getEndPhase(); p++)
    {
      if (_scache->getNumSrcRanks(p) > 0)
        {
          for (unsigned scount = 0; scount < _scache->getNumSrcRanks(p); scount ++)
            {
              TRACE_FORMAT( "<%p>p:%u, scount: %u, _phaseVec[p].recvBufs[scount]:%p, _all_recvBufs[%u]: %p",
                          this, p, scount, _phaseVec[p].recvBufs[scount], p*scount, _all_recvBufs[p*scount]);
              //CCMI_assert (_pcache._bytes != 0);
              CCMI_assert (_phaseVec[p].recvBufs[scount] != NULL);
              PAMI::PipeWorkQueue *pwq = &_phaseVec[p].pwqs[scount];
              new (pwq) PAMI::PipeWorkQueue();
	      //early arrival callback does not have correct send/recv types
              pwq->configure (_phaseVec[p].recvBufs[scount],_pcache._bytes,0);
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
