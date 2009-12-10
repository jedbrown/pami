/*
 * \file algorithms/executor/Barrier.h
 * \brief ???
 */

#ifndef __algorithms_executor_Barrier_h__
#define __algorithms_executor_Barrier_h__

#include "algorithms/interfaces/Schedule.h"
#include "algorithms/executor/Executor.h"
#include "algorithms/interfaces/NativeInterface.h"
#include "algorithms/executor/OldBarrier.h"

#undef TRACE_ERR
//#define TRACE_ERR(x) fprintf x
#define TRACE_ERR(x)

#include "algorithms/executor/ScheduleCache.h"

namespace CCMI
{
  namespace Executor
  {
    class BarrierExec : public Executor
    {
    public:
      /// pointer to the multicast interface to send messages
      Interfaces::NativeInterface    * _native;
      bool                 _senddone;  /// has send finished or not?
      unsigned             _start;     /// Start phase (don't assume 0)
      unsigned             _phase;     /// Which phase am I in ?
      unsigned             _nphases;   /// Number of phases
      unsigned             _connid;    ///Connection id for multisend
      unsigned             _iteration:1; ///The Red or black iteration

      CollHeaderData                 _cdata;
      xmi_multicast_t                _minfo;

      ///\brief A red/black vector for each neigbor which is incremented
      ///when the neighbor's message arrives
      char                 _phasevec[CCMI_BARRIER_MAXPHASES][2];

      ///\brief A cache of the barrier schedule
      ScheduleCache         _cache;

      char                  *_request;  //A 1024 byte request

      XMI::Topology         _srctopology;

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
          _phasevec[count][_iteration] -= _cache.getSrcTopology(count)->size();

          TRACE_ERR((stderr,"<%X>Executor::BarrierExec::decrementVector phase %d, nranks %d, vec %d\n",
                     (int) this,count,_cache.getSrcTopology(count)->size(),  _phasevec[count][_iteration]));
        }

        _senddone  =   false;
      }

      /// Static function to be passed into the done of multisend
      static void staticNotifySendDone(xmi_context_t context, void *cd, xmi_result_t err)
      {
        xmi_quad_t * info = NULL;
        TRACE_ERR((stderr,"<%X>Executor::BarrierExec::staticNotifySendDone\n",(int)cd));

        BarrierExec *barrier = (BarrierExec *) cd;
        barrier->internalNotifySendDone( *info );
      }

    public:

      /// Default Constructor
      BarrierExec() : Executor()
      {
        _start    = 0;
        _phase    = 0;
        _nphases  = 0;
      }

      /// Main constructor to initialize the executor
      BarrierExec(unsigned nranks, unsigned *ranks,unsigned comm,
              unsigned connid,
              Interfaces::NativeInterface *ninterface):
      Executor(),
      _native(ninterface),
      _connid(connid),
      _srctopology(ninterface->myrank())
      {
        TRACE_ERR((stderr,"<%X>Executor::BarrierExec::::ctor(nranks %d,comm %X,connid %d)\n",
                   (int)this, nranks, comm, connid));
        _start          =  0;
        _phase          =  0;
        _nphases        =  0;
        _senddone       =  false;
        _cdata._phase   = 0;
        _cdata._comm    = comm;
        MEMSET(_phasevec, 0, sizeof(_phasevec));

	_minfo.msginfo       = (xmi_quad_t *)(void *) &_cdata;
	_minfo.msgcount      = 1;
	_minfo.src           = NULL;
	_minfo.dst           = NULL;
	_minfo.bytes         = 0;

	_request = (char *) malloc (16384); //Large buffer for request.
	_minfo.request       = (void *)&_request;
        //_minfo.connection_id = _connid;
        _minfo.roles         = -1U;
        _minfo.dst_participants  = NULL;
	_minfo.src_participants  = (xmi_topology_t *)&_srctopology;

        _iteration           = 0;
      }

      void setCommSchedule(Interfaces::Schedule *sch)
      {
        _cache.init (sch);

        _start     =   _cache.getStartPhase();
        _nphases   =   _cache.getNumPhases();
        _phase     =   _start + _nphases;    //so that the decrementVector assert passes
        CCMI_assert(_start + _nphases  <=  CCMI_BARRIER_MAXPHASES);

        for(unsigned count = _start; count < _start + _nphases; count ++)
        {
          _phasevec[count][0] = (char) _cache.getSrcTopology(count)->size();
          _phasevec[count][1] = (char) _cache.getSrcTopology(count)->size();
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

    };  //-- BarrierExec
  };
};  // namespace CCMI::Executor

inline void CCMI::Executor::BarrierExec::sendNext()
{
  CCMI_assert(_phase <= (_start + _nphases));

  if(_phase == (_start + _nphases))
  {
    TRACE_ERR((stderr,"<%X>Executor::BarrierExec::sendNext DONE _cb_done %X, _phase %d, _clientdata %X\n",
               (int) this, (int)_cb_done, _phase, (int)_clientdata));
    if(_cb_done) _cb_done(NULL, _clientdata, XMI_SUCCESS);
    _senddone = false;

    return;
  }

  _senddone = false;
  XMI::Topology *topology = _cache.getDstTopology(_phase);
  int ndest = topology->size();
  _minfo.dst_participants = (xmi_topology_t *)topology;

  ///We can now send any number of messages in barrier
  if(ndest > 0)
  {
#if 0
    size_t *dstranks = NULL;
    topology->rankList(&dstranks);
    CCMI_assert (dstranks != NULL);

    TRACE_ERR((stderr,"Executor::BarrierExec::sendNext dstranks %p\n", dstranks));

    for (int count = 0; count < ndest; count++)
      TRACE_ERR((stderr,"<%X>Executor::BarrierExec::sendNext _phase %d, ndest %zd, _dstranks[count] %d, _connid %d, _clientdata %X\n", (int) this,_phase, ndest, dstranks[count], _connid, (int)_clientdata));
    CCMI_assert (topology->type() == XMI_LIST_TOPOLOGY);
#endif

    _minfo.connection_id = _phase; //set connection id to phase
    _cdata._phase     = _phase;
    _cdata._iteration = _iteration;  //Send the last bit of iteration

    //if last receive has arrived before the last send dont call executor notifySendDone rather app done callback
    if( (_phase == (_start + _nphases - 1)) &&
        (_phasevec[_phase][_iteration] >= _cache.getSrcTopology(_phase)->size()) )
    {
      TRACE_ERR((stderr,"<%X>Executor::BarrierExec::sendNext set callback %X\n",(int)this, (int)_cb_done));
      _minfo.cb_done.function   = _cb_done;
      _minfo.cb_done.clientdata = _clientdata;
      _phase ++;
    }
    else {
      _minfo.cb_done.function   = staticNotifySendDone;
      _minfo.cb_done.clientdata = this;
    }

    ///Initiate multisend
    _native->multicast(&_minfo);
  }
  else
  {
    xmi_quad_t * info = NULL;
    //nothing to do, skip this phase
    notifySendDone( *info );
  }
}

/// Entry function called to start the barrier
inline void  CCMI::Executor::BarrierExec::start()
{
  TRACE_ERR((stderr,"<%X>Executor::BarrierExec::start\n",(int) this));
  decrementVector();
  sendNext();
}


///
/// \brief grab the info of the message
///
inline void CCMI::Executor::BarrierExec::notifyRecv(unsigned          src,
						const xmi_quad_t  & info,
						char            * buf,
						unsigned          size)
{
  CollHeaderData *hdr = (CollHeaderData *) (& info);
  CCMI_assert (hdr->_iteration <= 1);
  //Process this message by incrementing the phase vec
  _phasevec[hdr->_phase][hdr->_iteration] ++;

  TRACE_ERR((stderr,"<%X>Executor::BarrierExec::notifyRecv phase %d, vec %d\n",(int)this,
             hdr->_phase, _phasevec[hdr->_phase][hdr->_iteration]));

  //Start has not been called, just record recv and return
  if(_phase == _start + _nphases)
    return;

  ///Check the current iteration's number of messages received. Have we received all messages
  if(_phasevec[_phase][_iteration] >= _cache.getSrcTopology(_phase)->size())
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
inline void CCMI::Executor::BarrierExec::internalNotifySendDone( const xmi_quad_t & info )
{
  TRACE_ERR((stderr,"<%X>Executor::BarrierExec::notifySendDone phase %d, vec %d\n",(int) this,_phase, _phasevec[_phase][_iteration]));

  _senddone = true;

  //Message for that phase has been received
  if( _phasevec[_phase][_iteration] >= _cache.getSrcTopology(_phase)->size())
  {
    _phase ++;
    sendNext();
  }
}

#endif
