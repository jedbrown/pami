/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/executor/AllreduceState.cc
 * \brief ???
 */

//#include "AllreduceState.h"
#include "AllreduceBase.h"


void CCMI::Executor::AllreduceState::constructPhaseData()
{

  TRACE_ALERT((stderr,"<%#.8X>Executor::AllreduceState::constructPhaseData() ALERT: Phase data being reset\n",(int)this));
  TRACE_STATE((stderr,"<%#.8X>Executor::AllreduceState::constructPhaseData() enter\n",(int)this));

  _dstPhase = (int) CCMI_UNDEFINED_PHASE;

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

  CCMI_assert(nphases > 0);
  CCMI_assert(startphase >= 0);

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
    iSrcPes[CCMI_KERNEL_EXECUTOR_ALLREDUCE_MAX_PES_PER_PHASE], 
    iSrcHints[CCMI_KERNEL_EXECUTOR_ALLREDUCE_MAX_PES_PER_PHASE],
    iDstPes[CCMI_KERNEL_EXECUTOR_ALLREDUCE_MAX_PES_PER_PHASE], 
    iDstHints[CCMI_KERNEL_EXECUTOR_ALLREDUCE_MAX_PES_PER_PHASE];

    _sched->getSrcPeList(i, iSrcPes, iNumSrcPes, iSrcHints);
    _sched->getDstPeList(i, iDstPes, iNumDstPes, iDstHints);
    CCMI_assert(iNumSrcPes <= CCMI_KERNEL_EXECUTOR_ALLREDUCE_MAX_PES_PER_PHASE && 
                iNumDstPes <= CCMI_KERNEL_EXECUTOR_ALLREDUCE_MAX_PES_PER_PHASE); 
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
        if(iSrcHints [idx] == CCMI_COMBINE_SUBTASK ||
           iSrcHints [idx] == CCMI_REDUCE_RECV_STORE ||
           iSrcHints [idx] == CCMI_REDUCE_RECV_NOSTORE)
          _lastCombinePhase = i;

        ///Broadcast recv phase is the last non-combine recv phase.  
        ///This is an assumption that should hold across all schedules
      if(i > _startPhase)
      {
        for(idx = 0; idx < iNumSrcPes; idx++)
          if(iSrcHints [idx] != CCMI_COMBINE_SUBTASK &&
             iSrcHints [idx] != CCMI_REDUCE_RECV_STORE &&
             iSrcHints [idx] != CCMI_REDUCE_RECV_NOSTORE)
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
      iDstPes [CCMI_KERNEL_EXECUTOR_ALLREDUCE_MAX_PES_PER_PHASE], 
      iDstHints [CCMI_KERNEL_EXECUTOR_ALLREDUCE_MAX_PES_PER_PHASE];
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
    sizeof(MultiSend::CCMI_OldMulticastRecv_t))) + // _all_mrecvs
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

#ifdef CCMI_DEBUG
    memset(_scheduleAllocation, 0xFD, allocationNewSize);

    if(!_scheduleAllocation)
      fprintf(stderr,"<%#.8X>CCMI_Alloc failed<%#.8X> free'd %#X, malloc'd %#X\n",(int)this,
              (int)_scheduleAllocation,_scheduleAllocationSize, allocationNewSize);
#endif
    CCMI_assert(_scheduleAllocation);

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
  _all_mrecvs   = (MultiSend::CCMI_OldMulticastRecv_t *) ((char*)_all_dstHints   + (_numDstPes * sizeof(unsigned)));

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

          MultiSend::CCMI_OldMulticastRecv_t *mrecv = &(_phaseVec[i].mrecv[scount]); 
          mrecv->connection_id = connID;
          mrecv->bytes = _bytes;
          mrecv->pipelineWidth = _pipelineWidth;
          mrecv->opcode = (CCMI_Subtask) _phaseVec[i].srcHints[scount];
        }

        indexSrcPe += _phaseVec[i].numSrcPes;

        // If this is a non-combine phase, use the destination buffers.  We must be receiving the
        // final answer.  
        // Since the dstbuf changes external to this class, we will set the phase index and the 
        // executor will use this to setup the dstbuff appropriately.
        if(_phaseVec[i].srcHints[0] != CCMI_COMBINE_SUBTASK)
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

        ///CCMI_assert (_color > 0); /// Most protocols set color to undefined, which we should change in the long run

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
#ifdef CCMI_DEBUG_SCHEDULE
    for(unsigned j = 1; j < _phaseVec[i].numSrcPes; ++j)
      TRACE_SCHEDULE((stderr,"<%#.8X>Executor::AllreduceState::constructPhaseData() src[%#X]:%#X srchints:%#X\n",(int)this,j,
                      _phaseVec[i].srcPes[j],_phaseVec[i].srcHints[j]));

    for(unsigned j = 1; j < _phaseVec[i].numDstPes; ++j)
      TRACE_SCHEDULE((stderr,"<%#.8X>Executor::AllreduceState::constructPhaseData() dst[%#X]:%#X dsthints:%#X\n",(int)this,j,
                      _phaseVec[i].dstPes[j],_phaseVec[i].dstHints[j]));
#endif // CCMI_DEBUG_SCHEDULE
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


void  CCMI::Executor::AllreduceState::setupReceives(unsigned infoRequired)
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
  ((_numSrcPes * numRequests) * sizeof(CCMI_Request_t)) +         // _recvReq
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

#ifdef CCMI_DEBUG
    memset(_receiveAllocation, 0xFE, allocationNewSize);

    if(!_receiveAllocation)
      fprintf(stderr,"<%#.8X>CCMI_Alloc failed<%#.8X> free'd %#X, malloc'd %#X\n",(int)this,
              (int)_receiveAllocation,_receiveAllocationSize, allocationNewSize);
#endif
    CCMI_assert(_receiveAllocation);

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
    ((_numSrcPes * numRequests) * sizeof(CCMI_Request_t)) +         // _recvReq
    ((_numSrcPes * numRequests) * (sizeof(RecvCallbackData) + 4)) + // _recvClientData (padded to 16 bytes)
    (_numSrcPes * _sizeOfBuffers) +                                   // _bufs
    (((_root == -1) | (_root == (int)_myRank))? 0 : _sizeOfBuffers);  // We need a temp buffer on non-root nodes

    if(maxAllocationSize > _receiveAllocationSize)
      _sizeOfBuffers = alignedBytes;
  }

  _recvReq = (CCMI_Request_t *) _receiveAllocation;

  _recvClientData = (RecvCallbackData *) ((char*)_recvReq + ((_numSrcPes * numRequests) * sizeof(CCMI_Request_t)));

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
        CCMI_Request_t *request = getRecvReq() + nextRecvData;
        RecvCallbackData *rdata = getRecvClient(nextRecvData);
        rdata->allreduce        = _executor;
        rdata->phase            = p;
        rdata->srcPeIndex       = scount;

        _phaseVec[p].mrecv[scount].request = request;
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
