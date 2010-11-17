/*
 * \file algorithms/executor/Barrier.h
 * \brief ???
 */

#ifndef __algorithms_executor_Barrier_h__
#define __algorithms_executor_Barrier_h__

#include "algorithms/interfaces/Schedule.h"
#include "algorithms/interfaces/Executor.h"
#include "algorithms/interfaces/NativeInterface.h"

#include "algorithms/executor/ScheduleCache.h"

#define CCMI_BARRIER_MAXPHASES  20

namespace CCMI
{
  namespace Executor
  {
    class BarrierExec : public Interfaces::Executor
    {
      public:
        /// pointer to the multicast interface to send messages
        Interfaces::NativeInterface    * _native;
        bool                 _senddone;  /// has send finished or not?
        unsigned             _start;     /// Start phase (don't assume 0)
        unsigned             _phase;     /// Which phase am I in ?
        unsigned             _nphases;   /// Number of phases
        unsigned             _connid;    ///Connection id for multisend
        unsigned             _iteration: 1; ///The Red or black iteration

        CollHeaderData                 _cdata;
        pami_multicast_t                _minfo;

        ///\brief A red/black vector for each neigbor which is incremented
        ///when the neighbor's message arrives
        char                 _phasevec[CCMI_BARRIER_MAXPHASES][2];

        ///\brief A cache of the barrier schedule
        ScheduleCache         _cache;

        PAMI::Topology         _srctopology;

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

          for (unsigned count = _start; count < (_start + _nphases); count ++)
            {
              _phasevec[count][_iteration] -= _cache.getSrcTopology(count)->size();

              TRACE_FLOW((stderr, "<%p>Executor::BarrierExec::decrementVector phase %d, nranks %zu, vec %d\n",
                          this, count, _cache.getSrcTopology(count)->size(),  _phasevec[count][_iteration]));
            }

          _senddone  =   false;
        }

        /// Static function to be passed into the done of multisend
        static void staticNotifySendDone(pami_context_t context, void *cd, pami_result_t err)
        {
          TRACE_FLOW((stderr, "<%p>Executor::BarrierExec::staticNotifySendDone\n", cd));

          BarrierExec *barrier = (BarrierExec *) cd;
          barrier->internalNotifySendDone();
        }

      public:

        /// Default Constructor
        BarrierExec() : Interfaces::Executor()
        {
          _start    = 0;
          _phase    = 0;
          _nphases  = 0;
        }

        /// Main constructor to initialize the executor
        BarrierExec(unsigned nranks, unsigned *ranks, unsigned comm,
                    unsigned connid,
                    Interfaces::NativeInterface *ninterface):
            Interfaces::Executor(),
            _native(ninterface),
            _connid(connid),
            _srctopology(ninterface->myrank())
        {
          TRACE_FLOW((stderr, "<%p>Executor::BarrierExec::::ctor(nranks %d,comm %X,connid %d)\n",
                      this, nranks, comm, connid));
          _start          =  0;
          _phase          =  0;
          _nphases        =  0;
          _senddone       =  false;
          _cdata._phase   = 0;
          _cdata._comm    = comm;
          MEMSET(_phasevec, 0, sizeof(_phasevec));

          _minfo.msginfo       = (pami_quad_t *)(void *) & _cdata;
          _minfo.msgcount      = 1;
          _minfo.src           = NULL;
          _minfo.dst           = NULL;
          _minfo.bytes         = 0;
          _minfo.roles         = -1U;
          _minfo.dst_participants  = NULL;
          _minfo.src_participants  = (pami_topology_t *) & _srctopology;

          _iteration           = 0;
        }

        void setCommSchedule(Interfaces::Schedule *sch)
        {
          //_cache.init (sch);
          _cache.setSchedule (sch);
          _cache.setRoot (-1);
          _cache.init (BARRIER_OP);

          _start     =   _cache.getStartPhase();
          _nphases   =   _cache.getNumPhases();
          _phase     =   _start + _nphases;    //so that the decrementVector assert passes
          CCMI_assert(_start + _nphases  <=  CCMI_BARRIER_MAXPHASES);

          for (unsigned count = _start; count < _start + _nphases; count ++)
            {
              _phasevec[count][0] = (char) _cache.getSrcTopology(count)->size();
              _phasevec[count][1] = (char) _cache.getSrcTopology(count)->size();
            }
        }

        /**
         * \brief notify when a message has been recived
         * \param src : source of the message
         * \param buf : address of the pipeworkqueue to produce incoming message
         * \param cb_done: completion callback
         */
        virtual void   notifyRecv     (unsigned             src,
                                       const pami_quad_t   & info,
                                       PAMI::PipeWorkQueue ** pwq,
                                       pami_callback_t      * cb_done);


        void internalNotifySendDone();

        /// Start sending barrier operation
        virtual void start();

    };  //-- BarrierExec
  };
};  // namespace CCMI::Executor

inline void CCMI::Executor::BarrierExec::sendNext()
{
  CCMI_assert(_phase <= (_start + _nphases));
  TRACE_FLOW((stderr, "<%p>Executor::BarrierExec::sendNext _phase %d, _start %d, _nphases %d\n",
              this, _phase, _start, _nphases));

  if (_phase == (_start + _nphases))
    {
      TRACE_FLOW((stderr, "<%p>Executor::BarrierExec::sendNext DONE _cb_done %p, _phase %d, _clientdata %p\n",
                  this, _cb_done, _phase, _clientdata));

      if (_cb_done) _cb_done(NULL, _clientdata, PAMI_SUCCESS);

      _senddone = false;

      return;
    }

  _senddone = false;
  PAMI::Topology *topology = _cache.getDstTopology(_phase);
  int ndest = topology->size();
  _minfo.dst_participants = (pami_topology_t *)topology;

  ///We can now send any number of messages in barrier
  if (ndest > 0)
    {
#ifdef CCMI_DEBUG
      pami_task_t *dstranks = NULL;
      topology->rankList(&dstranks);
      CCMI_assert (dstranks != NULL);

      TRACE_FLOW((stderr, "<%p>Executor::BarrierExec::sendNext dstranks %p\n", this, dstranks));

      for (int count = 0; count < ndest; count++)
        TRACE_FLOW((stderr, "<%p>Executor::BarrierExec::sendNext _phase %d, ndest %d, _dstranks[count] %u, _connid %d, _clientdata %p\n", this, _phase, ndest, dstranks[count], _connid, _clientdata));

      CCMI_assert (topology->type() == PAMI_LIST_TOPOLOGY);
#endif

      _minfo.connection_id = _phase; //set connection id to phase
      _cdata._phase     = _phase;
      _cdata._iteration = _iteration;  //Send the last bit of iteration

      //if last receive has arrived before the last send dont call executor notifySendDone rather app done callback
      if ( (_phase == (_start + _nphases - 1)) &&
           ((size_t)_phasevec[_phase][_iteration] >= _cache.getSrcTopology(_phase)->size()) )
        {
          TRACE_FLOW((stderr, "<%p>Executor::BarrierExec::sendNext set callback %p\n", this, _cb_done));
          _minfo.cb_done.function   = _cb_done;
          _minfo.cb_done.clientdata = _clientdata;
          _phase ++;
        }
      else
        {
          _minfo.cb_done.function   = staticNotifySendDone;
          _minfo.cb_done.clientdata = this;
        }

      ///Initiate multisend
      _native->multicast(&_minfo);
    }
  else
    {
      //nothing to do, skip this phase
      internalNotifySendDone();
    }
}

/// Entry function called to start the barrier
inline void  CCMI::Executor::BarrierExec::start()
{
  TRACE_FLOW((stderr, "<%p>Executor::BarrierExec::start\n", this));
  decrementVector();
  sendNext();
}


/**
 * \brief notify when a message has been recived
 * \param src : source of the message
 * \param buf : address of the pipeworkqueue to produce incoming message
 * \param cb_done: completion callback
 */
inline void  CCMI::Executor::BarrierExec::notifyRecv  (unsigned             src,
                                                       const pami_quad_t   & info,
                                                       PAMI::PipeWorkQueue ** pwq,
                                                       pami_callback_t      * cb_done)
{
  CollHeaderData *hdr = (CollHeaderData *) (& info);
  CCMI_assert (hdr->_iteration <= 1);
  //Process this message by incrementing the phase vec
  _phasevec[hdr->_phase][hdr->_iteration] ++;

  TRACE_FLOW((stderr, "<%p>Executor::BarrierExec::notifyRecv phase %d/%d(%d,%d), vec %d expected vec %zu\n", this,
              hdr->_phase, _phase, _start, _nphases, _phasevec[hdr->_phase][hdr->_iteration],  _cache.getSrcTopology(hdr->_phase)->size()));

  //Start has not been called, just record recv and return
  if (_phase == _start + _nphases)
    return;

  ///Check the current iteration's number of messages received. Have we received all messages
  if ((size_t)_phasevec[_phase][_iteration] >= _cache.getSrcTopology(_phase)->size())
    {
      if (_senddone)
        {
          _phase ++;
          sendNext();
        }
    }
}


///
/// \brief Entry function to indicate the send has finished
///
inline void CCMI::Executor::BarrierExec::internalNotifySendDone( )
{
  TRACE_FLOW((stderr, "<%p>Executor::BarrierExec::notifySendDone phase %d, vec %d\n", this, _phase, _phasevec[_phase][_iteration]));

  _senddone = true;

  //Message for that phase has been received
  if ((size_t)_phasevec[_phase][_iteration] >= _cache.getSrcTopology(_phase)->size())
    {
      _phase ++;
      sendNext();
    }
}

#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
