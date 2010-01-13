/**
 * \file algorithms/executor/Broadcast.h
 * \brief ???
 */
#ifndef __algorithms_executor_Broadcast_h__
#define __algorithms_executor_Broadcast_h__

#define TRACE_FLOW(x)  //fprintf x

#include "algorithms/interfaces/Schedule.h"
#include "algorithms/interfaces/Executor.h"
#include "algorithms/connmgr/ConnectionManager.h"
#include "algorithms/interfaces/NativeInterface.h"
#include "algorithms/executor/OldBroadcast.h"

#define MAX_PARALLEL 20

namespace CCMI
{
  namespace Executor
  {
    /*
     * Implements a broadcast strategy which uses one network
     * link. With rectangular schedule it will lead to a one color
     * broadcast. Also implements pipelining.
     */
    template<class T>
    class BroadcastExec : public Interfaces::Executor
    {
    protected:
      Interfaces::Schedule           * _comm_schedule;
      Interfaces::NativeInterface    * _native;
      xmi_multicast_t                 _msend;

      int              _comm;
      unsigned         _root;
      int              _buflen;

      XMI::PipeWorkQueue     _pwq;
      xmi_task_t             _dstranks [MAX_PARALLEL];
      XMI::Topology          _dsttopology;
      XMI::Topology          _selftopology;

      XMI_Request_t          _request __attribute__((__aligned__(16)));   /// send request

      CollHeaderData                               _mdata;
      T                                         *  _connmgr;
      unsigned                                     _color;
      bool                                         _postReceives;
      bool                                         _mactive;

      //Private method
      void             sendNext ();

    public:
      BroadcastExec () :
      Interfaces::Executor (),
      _comm_schedule(NULL),
      _comm(-1)
      {
      }

      BroadcastExec (Interfaces::NativeInterface  * mf,
		     unsigned                       comm,
		     T                            * connmgr,
		     unsigned                       color,
		     bool                           post_recvs = false):
      Interfaces::Executor(),
      _comm_schedule (NULL),
      _native(mf),
      _comm(comm),
      _connmgr(connmgr),
      _color(color),
      _postReceives (post_recvs),
      _dsttopology((xmi_task_t *)&_dstranks, MAX_PARALLEL),
      _selftopology(mf->myrank())
      {
        _clientdata        =  0;
        _root              =  (unsigned)-1;
	_buflen            =  0;
        _msend.request     =  &_request;
        xmi_quad_t *info   =  (_postReceives)?(NULL):(xmi_quad_t*)((void*)&_mdata);
        _msend.msginfo     =  info;
        _msend.msgcount    =  1;
	_msend.roles       = -1U;
	_mactive           =  false;
      }

      //-----------------------------------------
      // --  Initialization routines
      //------------------------------------------

      void setSchedule (Interfaces::Schedule *ct)
      {
        _comm_schedule = ct;
        int nph, phase;
        _comm_schedule->init (_root, BROADCAST_OP, phase, nph);
        CCMI_assert(_comm_schedule != NULL);
	_comm_schedule->getDstUnionTopology (&_dsttopology);
      }

      void  setInfo (int root, char *buf, int len)
      {
        _root           =  root;
	unsigned connid =  _connmgr->getConnectionId(_comm, _root, _color, (unsigned)-1, (unsigned)-1);
	_msend.connection_id = connid;

	//Setup pipework queue
	_pwq.configure (NULL, buf, len, 0);
      }

      XMI_Request_t * getRequest ()
      {
        return & _request;
      }

      //------------------------------------------
      // -- Executor Virtual Methods
      //------------------------------------------
      virtual void   start          ();
      virtual void   notifyRecv     (unsigned             src,
				     const xmi_quad_t   & info,
				     XMI::PipeWorkQueue ** pwq,
				     xmi_callback_t      * cb_done);

      //-----------------------------------------
      //--  Query functions ---------------------
      //-----------------------------------------
      unsigned       getRoot   ()
      {
        return _root;
      }
      unsigned       getComm   ()
      {
        return _comm;
      }

      void postReceives () {
	if(_native->myrank() == _root) return;

	xmi_multicast_t mrecv;
	memcpy (&mrecv, &_msend, sizeof(xmi_multicast_t));

	mrecv.src_participants   = NULL; //current mechanism to identify a non-root node
	mrecv.dst_participants   = (xmi_topology_t *)&_selftopology;
	mrecv.cb_done.function   = _cb_done;
	mrecv.cb_done.clientdata = _clientdata;
	mrecv.dst    =  (xmi_pipeworkqueue_t *)&_pwq;
	mrecv.src    =  NULL;
	mrecv.bytes  = _buflen;
	_native->multicast(&mrecv);
      }

    };  //-- BroadcastExec
  };   //-- Executor
};  //-- CCMI

///
/// \brief start sending broadcast data. Only active on the root node
///
template <class T>
inline void  CCMI::Executor::BroadcastExec<T>::start ()
{
  TRACE_FLOW ((stderr, "<%#.8X>Executor::BroadcastExec::start()\n",(int)this));

  _mactive = false;
  // Nothing to broadcast? We're done.
  if((_buflen == 0) && _cb_done)
    _cb_done (NULL, _clientdata, XMI_SUCCESS);
  else if(_native->myrank() == _root)
  {
    _pwq.produceBytes (_buflen);
    sendNext ();
  }
}

template <class T>
inline void  CCMI::Executor::BroadcastExec<T>::sendNext ()
{
  CCMI_assert (_dsttopology.size() != 0); //We have nothing to send
  //_mactive = true;  //not sure if I need this flag??

  TRACE_FLOW ((stderr, "<%#.8X>Executor::BroadcastExec::sendNext()\n",(int)this));

  //for(int dcount = 0; dcount < _nmessages; dcount++)
  //TRACE_FLOW ((stderr, "<%#.8X>Executor::BroadcastExec::sendNext() send to %d for size %d\n",(int)this, _dstranks[dcount], _curlen));

  //Sending message header to setup receive of an async message
  _mdata._comm  = _comm;
  _mdata._root  = _root;
  _mdata._count = -1; // not used on broadcast
  _mdata._phase = 0;
  _msend.src_participants  = (xmi_topology_t *)&_selftopology;
  _msend.dst_participants  = (xmi_topology_t *)&_dsttopology;
  _msend.cb_done.function   = _cb_done;
  _msend.cb_done.clientdata = _clientdata;
  _msend.dst   =   NULL;
  _msend.src    = (xmi_pipeworkqueue_t *)&_pwq;
  _msend.bytes  = _buflen;
  _native->multicast(&_msend);
}

template <class T>
inline void  CCMI::Executor::BroadcastExec<T>::notifyRecv
(unsigned             src,
 const xmi_quad_t   & info,
 XMI::PipeWorkQueue ** pwq,
 xmi_callback_t      * cb_done)
{
  TRACE_FLOW ((stderr, "<%#.8X>Executor::BroadcastExec::notifyRecv() from %d\n",(int)this, src));

  *pwq = &_pwq;
  if (_dsttopology.size() > 0) {
    cb_done->function = NULL;  //There is a send here that will notify completion
    sendNext ();
  }
  else {
    cb_done->function   = _cb_done;
    cb_done->clientdata = _clientdata;
  }
}


#endif
