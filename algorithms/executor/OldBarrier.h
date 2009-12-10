/*
 * \file algorithms/executor/OldBarrier.h
 * \brief ???
 */

#ifndef __algorithms_executor_OldBarrier_h__
#define __algorithms_executor_OldBarrier_h__

#include "algorithms/schedule/Schedule.h"
#include "algorithms/executor/Executor.h"

#undef TRACE_ERR
//#define TRACE_ERR(x) fprintf x
#define TRACE_ERR(x)

#define CCMI_BARRIER_MAXPHASES  20
#define CCMI_BARRIER_MAXRANKS   128

// Old, deprecated, interfaces for use during transition from OldMulticast to Multisync

namespace CCMI
{
  namespace Executor
  {
    template <class T_Mcast>
    class OldBarrier : public Executor
    {
      ///\brief The ScheduleCache caches the schedule so that it does
      ///not have to be read in every phase. It only stores the number
      ///of sources that send data to a particular processor in every
      ///phase. It stores all the destinations that a processor sends to
      ///in a given phase. The total number of messages that can be sent
      ///by the executor has a ceiling of CCMI_BARRIER_MAXPHASES to save space.

      class ScheduleCache
      {
      protected:
        unsigned          _start;      //Start phase
        unsigned          _nphases;    //Number of phases

        ///Combined list of all destinations
        unsigned          _dstranks    [CCMI_BARRIER_MAXPHASES];
        ///Combined list of all subtasks
        unsigned          _dstsubtasks [CCMI_BARRIER_MAXPHASES];

        ///Number or sources that send data in each phase
        unsigned   char   _nsrcranks   [CCMI_BARRIER_MAXPHASES];
        ///Number or destinations we send data to in each phase
        unsigned   char   _ndstranks   [CCMI_BARRIER_MAXPHASES];
        ///Where are my destinations and subtasks stored
        unsigned   char   _dstoffsets  [CCMI_BARRIER_MAXPHASES];

      public:
        ScheduleCache ()
        {
        }

        void init(Schedule::Schedule *schedule)
        {
          int start, nph, nmessages = 0;
          schedule->init (-1, BARRIER_OP, start, nph, nmessages);
          _start = start;
          _nphases = nph;

          TRACE_ERR((stderr,"<%X>Executor::Barrier::ScheduleCache::init _start %d, nph %d, nmessages %d\n",
                     (int) this,_start, _nphases, nmessages));

          CCMI_assert(_start + _nphases  <=  CCMI_BARRIER_MAXPHASES);
          CCMI_assert(nmessages <=  CCMI_BARRIER_MAXRANKS);

          unsigned dstindex = 0;
          for(unsigned count = _start; count < (_start + _nphases); count ++)
          {
            unsigned srcranks[CCMI_BARRIER_MAXRANKS], nsrc=0, dstranks[CCMI_BARRIER_MAXRANKS], ndst=0, subtasks[CCMI_BARRIER_MAXRANKS];
            schedule->getSrcPeList(count, srcranks, nsrc, subtasks);

            //CCMI_assert(nsrc <= 1);
            _nsrcranks [count]    = nsrc;

            schedule->getDstPeList(count, dstranks, ndst, subtasks);
            CCMI_assert (dstindex + ndst < CCMI_BARRIER_MAXPHASES);

            MEMCPY(&_dstranks[dstindex], dstranks, ndst *sizeof(int));
            MEMCPY(&_dstsubtasks[dstindex], subtasks, ndst *sizeof(int));

            _ndstranks[count]  = ndst;
            _dstoffsets[count] = dstindex;
            dstindex += ndst;
          }
        }

        unsigned  getSrcNumRanks (unsigned phase)
        {
          CCMI_assert ( (phase >= _start) && (phase < _start + _nphases));
          return _nsrcranks[phase];
        }

        unsigned  getDstNumRanks (unsigned phase)
        {
          CCMI_assert ( (phase >= _start) && (phase < _start + _nphases));
          return _ndstranks[phase];
        }

        unsigned  *getDstRanks (unsigned phase)
        {
          CCMI_assert ( (phase >= _start) && (phase < _start + _nphases));
          return &_dstranks[_dstoffsets[phase]];
        }

        unsigned  *getDstSubtasks (unsigned phase)
        {
          CCMI_assert ( (phase >= _start) && (phase < _start + _nphases));
          return &_dstsubtasks[_dstoffsets[phase]];
        }

        unsigned  getStartPhase()
        {
          return _start;
        }
        unsigned  getNumPhases()
        {
          return _nphases;
        }
      };

    public:

      /// pointer to the multicast interface to send messages
      T_Mcast            * _mcastInterface;
      bool                 _senddone;  /// has send finished or not?
      unsigned             _start;     /// Start phase (don't assume 0)
      unsigned             _phase;     /// Which phase am I in ?
      unsigned             _nphases;   /// Number of phases
      unsigned             _connid;    ///Connection id for multisend
      unsigned             _iteration:1; ///The Red or black iteration

      CollHeaderData       _cdata;
      xmi_oldmulticast_t   _minfo;

      ///\brief A red/black vector for each neigbor which is incremented
      ///when the neighbor's message arrives
      char                 _phasevec[CCMI_BARRIER_MAXPHASES][2];

      ///\brief A cache of the barrier schedule
      ScheduleCache         _cache;

      /// \brief Request for the multisend msg
      XMI_Request_t        _request __attribute__((__aligned__(16)));

      ///
      /// \brief core internal function to initiate the next phase
      ///
      void             sendNext();

      ///
      /// \brief : Internal function to decrement entries in the
      /// phase vector table. It is called at the start of each
      /// phase.
      ///
      void decrementVector()
      {
        CCMI_assert(_phase == _start + _nphases);
        _phase     =   _start;
        _iteration ++;  //1 bit itertation count that can wrap

        for(unsigned count = _start; count < (_start + _nphases); count ++)
        {
          _phasevec[count][_iteration] -= _cache.getSrcNumRanks (count);

          TRACE_ERR((stderr,"<%X>Executor::Barrier::decrementVector phase %d, nranks %d, vec %d\n",
                     (int) this,count,_cache.getSrcNumRanks(count),  _phasevec[count][_iteration]));
        }

        _senddone  =   false;
      }

      /// Static function to be passed into the done of multisend
      static void staticNotifySendDone(xmi_context_t context, void *cd, xmi_result_t err)
      {
        xmi_quad_t * info = NULL;
        TRACE_ERR((stderr,"<%X>Executor::Barrier::staticNotifySendDone\n",(int)cd));

        OldBarrier *barrier = (OldBarrier *) cd;
        barrier->internalNotifySendDone( *info );
      }

    public:

      /// Default Constructor
      OldBarrier() : Executor()
      {
        _start    = 0;
        _phase    = 0;
        _nphases  = 0;
      }

      /// Main constructor to initialize the executor
      OldBarrier(unsigned  nranks,
                 unsigned *ranks,
                 unsigned  comm,
                 unsigned  connid,
                 T_Mcast  *minterface):
      Executor(),
      _mcastInterface(minterface),
      _connid(connid)
      {
        TRACE_ERR((stderr,"<%X>Executor::OldBarrier::::ctor(nranks %d,comm %X,connid %d)\n",
                   (int)this, nranks, comm, connid));
        _start          =  0;
        _phase          =  0;
        _nphases        =  0;
        _senddone       =  false;
        _cdata._phase   = 0;
        _cdata._comm    = comm;
        MEMSET(_phasevec, 0, sizeof(_phasevec));

//	_minfo.setRequestBuffer(& _request);
//	_minfo.setConsistency (CCMI_MATCH_CONSISTENCY);
//      _minfo.setInfo((xmi_quad_t *)((void *) &_cdata),  1);
//      _minfo.setConnectionId(_connid);
//	_minfo.setSendData (NULL, 0);
//	_minfo.setRanks (NULL, 0);
//	_minfo.setOpcodes(NULL);

        _minfo.request=(xmi_quad_t*)&_request;
        _minfo.msginfo=(xmi_quad_t *)((void *) &_cdata);
        _minfo.count  =1;
        _minfo.bytes  =0;
        _minfo.src    =NULL;
        _minfo.tasks  =NULL;
        _minfo.ntasks=0;
        _minfo.opcodes=(xmi_subtask_t*)NULL;
        _iteration     = 0;
      }

      void setCommSchedule(Schedule::Schedule *sch)
      {
        _cache.init (sch);

        _start     =   _cache.getStartPhase();
        _nphases   =   _cache.getNumPhases();
        _phase     =   _start + _nphases;    //so that the decrementVector assert passes
        CCMI_assert(_start + _nphases  <=  CCMI_BARRIER_MAXPHASES);

        for(unsigned count = _start; count < _start + _nphases; count ++)
        {
          _phasevec[count][0] = (char) _cache.getSrcNumRanks (count);
          _phasevec[count][1] = (char) _cache.getSrcNumRanks (count);
        }
      }

      /// Entry function to notify message has arrived
      virtual void notifyRecv(unsigned src, const xmi_quad_t &info, char *buf, unsigned len);

      /// Entry function to declare that Message has been sent
      virtual void notifySendDone( const xmi_quad_t & info )
      {
        internalNotifySendDone(info);
      }

      void internalNotifySendDone( const xmi_quad_t & info );

      /// Start sending barrier operation
      virtual void start();

    };  //-- OldBarrier
  };
};  // namespace CCMI::Executor

template <class T_Mcast>
inline void CCMI::Executor::OldBarrier<T_Mcast>::sendNext()
{
  CCMI_assert(_phase <= (_start + _nphases));
  if(_phase == (_start + _nphases))
  {
    TRACE_ERR((stderr,"<%X>Executor::OldBarrier::sendNext DONE _cb_done %X, _phase %d, _clientdata %X\n",
               (int) this, (int)_cb_done, _phase, (int)_clientdata));
    if(_cb_done)
      _cb_done(NULL, _clientdata, XMI_SUCCESS);
    _senddone = false;

    return;
  }

  _senddone = false;
  unsigned ndest     = _cache.getDstNumRanks(_phase);
  unsigned *dstranks = _cache.getDstRanks(_phase);
  unsigned *subtasks = _cache.getDstSubtasks(_phase);

  TRACE_ERR((stderr,"<%X>Executor::OldBarrier::sendNext _phase %d, ndest %zd, _destrank %zd, _connid %d, _clientdata %X\n",
             (int) this,_phase, ndest, dstranks[0], _connid, (int)_clientdata));

  ///We can now send any number of messages in barrier
  if(ndest > 0)
  {
    _cdata._phase     = _phase;
    _cdata._iteration = _iteration;  //Send the last bit of iteration

    //if last receive has arrived before the last send dont call executor notifySendDone rather app done callback
    if( (_phase == (_start + _nphases - 1)) &&
        ((size_t)_phasevec[_phase][_iteration] >= _cache.getSrcNumRanks(_phase)) )
    {
      TRACE_ERR((stderr,"<%X>Executor::OldBarrier::sendNext set callback %X\n",(int)this, (int)_cb_done));
      //_minfo.setCallback (_cb_done, _clientdata);
      _minfo.cb_done.function  =_cb_done;
      _minfo.cb_done.clientdata=_clientdata;
      _phase ++;
    }
    else
        {
//          _minfo.setCallback (staticNotifySendDone, this);
          _minfo.cb_done.function  =staticNotifySendDone;
          _minfo.cb_done.clientdata=this;
        }
//    _minfo.setOpcodes((CCMI_Subtask *)subtasks);
//    _minfo.setRanks(dstranks, ndest);
    _minfo.opcodes=(xmi_subtask_t*) subtasks;
    _minfo.tasks=dstranks;
    _minfo.ntasks=ndest;
    ///Initiate multisend
    _mcastInterface->send (&_minfo);
//    sleep(1);
  }
  else
  {
    xmi_quad_t * info = NULL;
    //nothing to do, skip this phase
    notifySendDone( *info );
  }
}

/// Entry function called to start the barrier
template <class T_Mcast>
inline void  CCMI::Executor::OldBarrier<T_Mcast>::start()
{
  TRACE_ERR((stderr,"<%X>Executor::OldBarrier::start\n",(int) this));
  decrementVector();
  sendNext();
}


///
/// \brief grab the info of the message
///
template <class T_Mcast>
inline void CCMI::Executor::OldBarrier<T_Mcast>::notifyRecv(unsigned          src,
                                                const xmi_quad_t  & info,
                                                char            * buf,
                                                unsigned          size)
{
  CollHeaderData *hdr = (CollHeaderData *) (& info);
  CCMI_assert (hdr->_iteration <= 1);
  //Process this message by incrementing the phase vec
  _phasevec[hdr->_phase][hdr->_iteration] ++;

  TRACE_ERR((stderr,"<%X>Executor::OldBarrier::notifyRecv phase %d, vec %d\n",(int)this,
             hdr->_phase, _phasevec[hdr->_phase][hdr->_iteration]));

  //Start has not been called, just record recv and return
  if(_phase == _start + _nphases)
    return;

  ///Check the current iteration's number of messages received. Have we received all messages
  if((size_t)_phasevec[_phase][_iteration] >= _cache.getSrcNumRanks(_phase))
  {
    if(_senddone)
    {
      _phase ++;
      sendNext();
    }
  }
}


///
/// \brief Entry function to indicate the send has finished
///
template <class T_Mcast>
inline void CCMI::Executor::OldBarrier<T_Mcast>::internalNotifySendDone( const xmi_quad_t & info )
{
  TRACE_ERR((stderr,"<%X>Executor::OldBarrier::notifySendDone phase %d, vec %d\n",(int) this,_phase, _phasevec[_phase][_iteration]));

  _senddone = true;

  //Message for that phase has been received
  if( (size_t)_phasevec[_phase][_iteration] >= _cache.getSrcNumRanks(_phase) )
  {
    _phase ++;
    sendNext();
  }
}
//////////////////////////////////////////////////////////////////////////////////////

#endif
