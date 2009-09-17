/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/executor/Barrier.h
 * \brief The basic/general barrier executor
 */

#ifndef __barrier_executor_h__
#define __barrier_executor_h__

#include "algorithms/schedule/Schedule.h"
#include "algorithms/executor/Executor.h"
#include "interface/MultiSend.h"
#include "interface/Topology.h"

#undef TRACE_ERR
//#define TRACE_ERR(x) fprintf x
#define TRACE_ERR(x)

#define MAX_PHASES  20
#define MAX_RANKS   128

namespace CCMI
{
  namespace Executor
  {
    class Barrier : public Executor
    {
      ///\brief The ScheduleCache caches the schedule so that it does
      ///not have to be read in every phase. It only stores the number
      ///of sources that send data to a particular processor in every
      ///phase. It stores all the destinations that a processor sends to
      ///in a given phase. The total number of messages that can be sent
      ///by the executor has a ceiling of MAX_PHASES to save space.

      class ScheduleCache
      {
      protected:
        unsigned          _start;      //Start phase
        unsigned          _nphases;    //Number of phases

        ///Combined list of all destinations
        size_t          _dstranks    [MAX_PHASES];
        ///Combined list of all subtasks
        size_t          _dstsubtasks [MAX_PHASES];

        ///Number or sources that send data in each phase
        unsigned   char   _nsrcranks   [MAX_PHASES];
        ///Number or destinations we send data to in each phase
        unsigned   char   _ndstranks   [MAX_PHASES];
        ///Where are my destinations and subtasks stored
        unsigned   char   _dstoffsets  [MAX_PHASES];

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

          CCMI_assert(_start + _nphases  <=  MAX_PHASES);
          CCMI_assert(nmessages <=  MAX_RANKS);

          unsigned dstindex = 0;
          for(unsigned count = _start; count < (_start + _nphases); count ++)
          {
            unsigned srcranks[MAX_RANKS], nsrc=0, dstranks[MAX_RANKS], ndst=0, subtasks[MAX_RANKS];
            schedule->getSrcPeList(count, srcranks, nsrc, subtasks);

            //CCMI_assert(nsrc <= 1);
            _nsrcranks [count]    = nsrc;

            schedule->getDstPeList(count, dstranks, ndst, subtasks);
            CCMI_assert (dstindex + ndst < MAX_PHASES);

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

        size_t getDstNumRanks (unsigned phase)
        {
          CCMI_assert ( (phase >= _start) && (phase < _start + _nphases));
          return _ndstranks[phase];
        }

        size_t  *getDstRanks (unsigned phase)
        {
          CCMI_assert ( (phase >= _start) && (phase < _start + _nphases));
          return &_dstranks[_dstoffsets[phase]];
        }

        size_t  *getDstSubtasks (unsigned phase)
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
      MultiSend::MultisyncInterface   * _msyncInterface;

      bool                 _senddone;  /// has send finished or not?
      unsigned             _start;     /// Start phase (don't assume 0)
      unsigned             _phase;     /// Which phase am I in ?
      unsigned             _nphases;   /// Number of phases
      unsigned             _connid;    ///Connection id for multisend
      unsigned             _iteration:1; ///The Red or black iteration

      CollHeaderData                 _cdata;
      MultiSend::CCMI_Multisync_t    _minfo;

      ///\brief A red/black vector for each neigbor which is incremented
      ///when the neighbor's message arrives
      char                 _phasevec[MAX_PHASES][2];

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
      static void staticNotifySendDone(void *cd, XMI_Error_t *err)
      {
        XMIQuad * info = NULL;
        TRACE_ERR((stderr,"<%X>Executor::Barrier::staticNotifySendDone\n",(int)cd));

        Barrier *barrier = (Barrier *) cd;
        barrier->internalNotifySendDone( *info );
      }

    public:

      /// Default Constructor
      Barrier() : Executor()
      {
        _start    = 0;
        _phase    = 0;
        _nphases  = 0;
      }

      /// Main constructor to initialize the executor
      Barrier(unsigned nranks, unsigned *ranks,unsigned comm,
              unsigned connid,
              MultiSend::MultisyncInterface   * minterface):
      Executor(),
      _msyncInterface(minterface),
      _connid(connid)
      {
        TRACE_ERR((stderr,"<%X>Executor::Barrier::::ctor(nranks %d,comm %X,connid %d)\n",
                   (int)this, nranks, comm, connid));
        _start          =  0;
        _phase          =  0;
        _nphases        =  0;
        _senddone       =  false;
        _cdata._phase   = 0;
        _cdata._comm    = comm;

        MEMSET(_phasevec, 0, sizeof(_phasevec));

        _minfo.setRequestBuffer(& _request, sizeof(_request));
        //_minfo.setInfo((XMIQuad *)((void *) &_cdata),  1);
        _minfo.setConnectionId(_connid);
        _minfo.setRoles((unsigned)-1);
        _minfo.setRanks(NULL);

        _iteration     = 0;
      }

      void setCommSchedule(Schedule::Schedule *sch)
      {
        _cache.init (sch);

        _start     =   _cache.getStartPhase();
        _nphases   =   _cache.getNumPhases();
        _phase     =   _start + _nphases;    //so that the decrementVector assert passes
        CCMI_assert(_start + _nphases  <=  MAX_PHASES);

        for(unsigned count = _start; count < _start + _nphases; count ++)
        {
          _phasevec[count][0] = (char) _cache.getSrcNumRanks (count);
          _phasevec[count][1] = (char) _cache.getSrcNumRanks (count);
        }
      }

      /// Entry function to notify message has arrived
      virtual void notifyRecv(unsigned src, const XMIQuad &info, char *buf, unsigned len);

      /// Entry function to declare that Message has been sent
      virtual void notifySendDone( const XMIQuad & info )
      {
        internalNotifySendDone(info);
      }

      void internalNotifySendDone( const XMIQuad & info );

      /// Start sending barrier operation
      virtual void start();

    };  //-- Barrier
  };
};  // namespace CCMI::Executor


inline void CCMI::Executor::Barrier::sendNext()
{
  CCMI_assert(_phase <= (_start + _nphases));

  if(_phase == (_start + _nphases))
  {
    TRACE_ERR((stderr,"<%X>Executor::Barrier::sendNext DONE _cb_done %X, _phase %d, _clientdata %X\n",
               (int) this, (int)_cb_done, _phase, (int)_clientdata));
    if(_cb_done)
      _cb_done(_clientdata, NULL);
    _senddone = false;

    return;
  }

  _senddone = false;
  size_t ndest     = _cache.getDstNumRanks(_phase);
  size_t *dstranks = _cache.getDstRanks(_phase);
  XMI_Topology_t topo;
  //unsigned *subtasks = _cache.getDstSubtasks(_phase);

  TRACE_ERR((stderr,"<%X>Executor::Barrier::sendNext _phase %d, ndest %zd, _destrank %zd, _connid %d, _clientdata %X\n",
             (int) this,_phase, ndest, dstranks[0], _connid, (int)_clientdata));

  ///We can now send any number of messages in barrier
  if(ndest > 0)
  {
    _cdata._phase     = _phase;
    _cdata._iteration = _iteration;  //Send the last bit of iteration

    //if last receive has arrived before the last send dont call executor notifySendDone rather app done callback
    if( (_phase == (_start + _nphases - 1)) &&
        (_phasevec[_phase][_iteration] >= _cache.getSrcNumRanks(_phase)) )
    {
      TRACE_ERR((stderr,"<%X>Executor::Barrier::sendNext set callback %X\n",(int)this, (int)_cb_done));
      _minfo.setCallback (_cb_done, _clientdata);
      _phase ++;
    }
    else
      _minfo.setCallback (staticNotifySendDone, this);

    new (&topo) XMI::Topology(dstranks, ndest);
    _minfo.setRanks(&topo);

    ///Initiate multisend
    _msyncInterface->generate(&_minfo);
  }
  else
  {
    XMIQuad * info = NULL;
    //nothing to do, skip this phase
    notifySendDone( *info );
  }
}

/// Entry function called to start the barrier
inline void  CCMI::Executor::Barrier::start()
{
  TRACE_ERR((stderr,"<%X>Executor::Barrier::start\n",(int) this));
  decrementVector();
  sendNext();
}


///
/// \brief grab the info of the message
///
inline void CCMI::Executor::Barrier::notifyRecv(unsigned          src,
                                                const XMIQuad  & info,
                                                char            * buf,
                                                unsigned          size)
{
  CollHeaderData *hdr = (CollHeaderData *) (& info);
  CCMI_assert (hdr->_iteration <= 1);
  //Process this message by incrementing the phase vec
  _phasevec[hdr->_phase][hdr->_iteration] ++;

  TRACE_ERR((stderr,"<%X>Executor::Barrier::notifyRecv phase %d, vec %d\n",(int)this,
             hdr->_phase, _phasevec[hdr->_phase][hdr->_iteration]));

  //Start has not been called, just record recv and return
  if(_phase == _start + _nphases)
    return;

  ///Check the current iteration's number of messages received. Have we received all messages
  if(_phasevec[_phase][_iteration] >= _cache.getSrcNumRanks(_phase))
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
inline void CCMI::Executor::Barrier::internalNotifySendDone( const XMIQuad & info )
{
  TRACE_ERR((stderr,"<%X>Executor::Barrier::notifySendDone phase %d, vec %d\n",(int) this,_phase, _phasevec[_phase][_iteration]));

  _senddone = true;

  //Message for that phase has been received
  if( _phasevec[_phase][_iteration] >= _cache.getSrcNumRanks(_phase) )
  {
    _phase ++;
    sendNext();
  }
}

// Old, deprecated, interfaces for use during transition from OldMulticast to Multisync

namespace CCMI
{
  namespace Executor
  {
    class OldBarrier : public Executor
    {
      ///\brief The ScheduleCache caches the schedule so that it does
      ///not have to be read in every phase. It only stores the number
      ///of sources that send data to a particular processor in every
      ///phase. It stores all the destinations that a processor sends to
      ///in a given phase. The total number of messages that can be sent
      ///by the executor has a ceiling of MAX_PHASES to save space.

      class ScheduleCache
      {
      protected:
        unsigned          _start;      //Start phase
        unsigned          _nphases;    //Number of phases

        ///Combined list of all destinations
        unsigned          _dstranks    [MAX_PHASES];
        ///Combined list of all subtasks
        unsigned          _dstsubtasks [MAX_PHASES];

        ///Number or sources that send data in each phase
        unsigned   char   _nsrcranks   [MAX_PHASES];
        ///Number or destinations we send data to in each phase
        unsigned   char   _ndstranks   [MAX_PHASES];
        ///Where are my destinations and subtasks stored
        unsigned   char   _dstoffsets  [MAX_PHASES];

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

          CCMI_assert(_start + _nphases  <=  MAX_PHASES);
          CCMI_assert(nmessages <=  MAX_RANKS);

          unsigned dstindex = 0;
          for(unsigned count = _start; count < (_start + _nphases); count ++)
          {
            unsigned srcranks[MAX_RANKS], nsrc=0, dstranks[MAX_RANKS], ndst=0, subtasks[MAX_RANKS];
            schedule->getSrcPeList(count, srcranks, nsrc, subtasks);

            //CCMI_assert(nsrc <= 1);
            _nsrcranks [count]    = nsrc;

            schedule->getDstPeList(count, dstranks, ndst, subtasks);
            CCMI_assert (dstindex + ndst < MAX_PHASES);

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
      MultiSend::OldMulticastInterface   * _mcastInterface;

      bool                 _senddone;  /// has send finished or not?
      unsigned             _start;     /// Start phase (don't assume 0)
      unsigned             _phase;     /// Which phase am I in ?
      unsigned             _nphases;   /// Number of phases
      unsigned             _connid;    ///Connection id for multisend
      unsigned             _iteration:1; ///The Red or black iteration

      CollHeaderData                 _cdata;
      MultiSend::CCMI_OldMulticast_t    _minfo;

      ///\brief A red/black vector for each neigbor which is incremented
      ///when the neighbor's message arrives
      char                 _phasevec[MAX_PHASES][2];

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
      static void staticNotifySendDone(void *cd, XMI_Error_t *err)
      {
        XMIQuad * info = NULL;
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
      OldBarrier(unsigned nranks, unsigned *ranks,unsigned comm,
              unsigned connid,
              MultiSend::OldMulticastInterface   * minterface):
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

	_minfo.setRequestBuffer(& _request);
	_minfo.setConsistency (CCMI_MATCH_CONSISTENCY);
        _minfo.setInfo((XMIQuad *)((void *) &_cdata),  1);
        _minfo.setConnectionId(_connid);
	_minfo.setSendData (NULL, 0);
	_minfo.setRanks (NULL, 0);
	_minfo.setOpcodes(NULL);

        _iteration     = 0;
      }

      void setCommSchedule(Schedule::Schedule *sch)
      {
        _cache.init (sch);

        _start     =   _cache.getStartPhase();
        _nphases   =   _cache.getNumPhases();
        _phase     =   _start + _nphases;    //so that the decrementVector assert passes
        CCMI_assert(_start + _nphases  <=  MAX_PHASES);

        for(unsigned count = _start; count < _start + _nphases; count ++)
        {
          _phasevec[count][0] = (char) _cache.getSrcNumRanks (count);
          _phasevec[count][1] = (char) _cache.getSrcNumRanks (count);
        }
      }

      /// Entry function to notify message has arrived
      virtual void notifyRecv(unsigned src, const XMIQuad &info, char *buf, unsigned len);

      /// Entry function to declare that Message has been sent
      virtual void notifySendDone( const XMIQuad & info )
      {
        internalNotifySendDone(info);
      }

      void internalNotifySendDone( const XMIQuad & info );

      /// Start sending barrier operation
      virtual void start();

    };  //-- OldBarrier
  };
};  // namespace CCMI::Executor


inline void CCMI::Executor::OldBarrier::sendNext()
{
  CCMI_assert(_phase <= (_start + _nphases));

  if(_phase == (_start + _nphases))
  {
    TRACE_ERR((stderr,"<%X>Executor::OldBarrier::sendNext DONE _cb_done %X, _phase %d, _clientdata %X\n",
               (int) this, (int)_cb_done, _phase, (int)_clientdata));
    if(_cb_done)
      _cb_done(_clientdata, NULL);
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
        (_phasevec[_phase][_iteration] >= _cache.getSrcNumRanks(_phase)) )
    {
      TRACE_ERR((stderr,"<%X>Executor::OldBarrier::sendNext set callback %X\n",(int)this, (int)_cb_done));
      _minfo.setCallback (_cb_done, _clientdata);
      _phase ++;
    }
    else
      _minfo.setCallback (staticNotifySendDone, this);

    _minfo.setOpcodes((CCMI_Subtask *)subtasks);
    _minfo.setRanks(dstranks, ndest);

    ///Initiate multisend
    _mcastInterface->send (&_minfo);
  }
  else
  {
    XMIQuad * info = NULL;
    //nothing to do, skip this phase
    notifySendDone( *info );
  }
}

/// Entry function called to start the barrier
inline void  CCMI::Executor::OldBarrier::start()
{
  TRACE_ERR((stderr,"<%X>Executor::OldBarrier::start\n",(int) this));
  decrementVector();
  sendNext();
}


///
/// \brief grab the info of the message
///
inline void CCMI::Executor::OldBarrier::notifyRecv(unsigned          src,
                                                const XMIQuad  & info,
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
  if(_phasevec[_phase][_iteration] >= _cache.getSrcNumRanks(_phase))
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
inline void CCMI::Executor::OldBarrier::internalNotifySendDone( const XMIQuad & info )
{
  TRACE_ERR((stderr,"<%X>Executor::OldBarrier::notifySendDone phase %d, vec %d\n",(int) this,_phase, _phasevec[_phase][_iteration]));

  _senddone = true;

  //Message for that phase has been received
  if( _phasevec[_phase][_iteration] >= _cache.getSrcNumRanks(_phase) )
  {
    _phase ++;
    sendNext();
  }
}
//////////////////////////////////////////////////////////////////////////////////////

#endif
