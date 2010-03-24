/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/executor/OldBroadcast.h
 * \brief ???
 */

#ifndef __algorithms_executor_OldBroadcast_h__
#define __algorithms_executor_OldBroadcast_h__

//#define TRACE_FLOW(x)  fprintf x

#include "algorithms/schedule/Schedule.h"
#include "algorithms/executor/Executor.h"
#include "algorithms/connmgr/ConnectionManager.h"

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
    template <class T_Sysdep, class T_Mcast, class T_ConnectionManager>
    class OldBroadcast : public Executor
    {
    protected:
      T_Sysdep            * _sd;
      Schedule::Schedule  * _comm_schedule;
      T_Mcast             * _mInterface;
      pami_oldmulticast_t    _msend;

      int              _comm;
      unsigned         _root;
      int              _startphase;
      int              _nphases;
      int              _nmessages;  //number of messages to send

      int              _buflen;
      int              _pipelinewidth;
      int              _curlen;

      const char     * _buf;
      int              _bytesrecvd;
      int              _bytessent;

      unsigned         _destpes    [MAX_PARALLEL];
      unsigned         _hints      [MAX_PARALLEL];

      //PAMI_Callback_t           _msend_cb;
      PAMI_Request_t            _send_request __attribute__((__aligned__(16)));   /// send request
      PAMI_Request_t            _recv_request __attribute__((__aligned__(16)));   /// recv request

      CollHeaderData           _mdata;
      T_ConnectionManager    * _connmgr;
      unsigned                  _color;
      bool                      _postReceives;

      //Private method
      void             sendNext ();

      inline void setPhase (int phase)
      {
        _startphase = phase;
        CCMI_assert(_comm_schedule != NULL);
        _nmessages = 0;
        for(int count = 0; count < _nphases - phase; count ++)
        {
          unsigned ndest = 0;
          _comm_schedule->getDstPeList (phase+count, _destpes+_nmessages,
                                        ndest, _hints+_nmessages);

          TRACE_FLOW ((stderr, "<%p>Executor::OldBroadcast::setPhase() destpe %d phase %d",this, _destpes[_nmessages], count+_startphase));

          CCMI_assert(_nmessages + ndest < MAX_PARALLEL);
          _nmessages += ndest;
        }
      }


    public:

      static void staticSendDone (pami_context_t context, void *clientdata, pami_result_t err)
      {
        pami_quad_t * info = NULL;
        OldBroadcast *bcast = (OldBroadcast *) clientdata;
        bcast->notifySendDone( *info );
      }

      inline OldBroadcast () :
      Executor (),
      _comm_schedule(NULL),
      _comm(-1),
      _startphase (-1),
      _nphases (-1),
      _curlen(-1)
      {
      }

      inline OldBroadcast (T_Sysdep *sd, unsigned comm,
                        T_ConnectionManager *connmgr,
                        unsigned color, bool post_recvs = false):
      Executor(),
      _comm_schedule (NULL),
      _comm(comm),
      _connmgr(connmgr),
      _color(color),
      _postReceives (post_recvs)
      {
        _startphase     =   0;
        _nphases        =  -1;
        _clientdata     =   0;
        _bytesrecvd     =   0;
        _bytessent      =   0;
        _buf            =   0;
        _buflen         =  -1;
        _root           =  (unsigned)-1;
        _pipelinewidth  =  -1;
        _curlen         =   0;
        _nmessages      =   0;
        _msend.cb_done.function   =   staticSendDone;
        _msend.cb_done.clientdata =   this;
        _sd           =   sd;
        _msend.request     = &_send_request[0];
        pami_quad_t *info   = (_postReceives)?(NULL):(pami_quad_t*)((void*)&_mdata);
        _msend.msginfo     = info;
        _msend.count       = 1;
        _msend.flags       = 0;  //FLAGS_UNSET
        _msend.opcodes     = (pami_subtask_t*)&_hints;
      }

      inline unsigned bytesrecvd ()
      {
        return _bytesrecvd;
      }

      //-----------------------------------------
      // --  Initialization routines
      //------------------------------------------

      inline void setSchedule (Schedule::Schedule *ct)
      {
        _comm_schedule = ct;

        int nph, phase, nmessages;
        _comm_schedule->init (_root, BROADCAST_OP, phase, nph, nmessages);
        _nphases = phase + nph;

        setPhase (phase);
      }

      inline void setMulticastInterface (T_Mcast *mf)
      {
        _mInterface = mf;
      }

      inline void  setInfo (int root, int pwidth, const char *buf, int len)
      {
        _root           =  root;
        _pipelinewidth  =  pwidth;
        _buflen         =  len;
        _buf            =  buf;

	unsigned connid =
	  _connmgr->getConnectionId(_comm, _root, _color, (unsigned)-1, (unsigned)-1);
	_msend.connection_id = connid;
      }

      inline void setPipelineWidth (int pwidth) {
	_pipelinewidth = pwidth;
      }

      inline PAMI_Request_t * getRecvRequest ()
      {
        return & _recv_request;
      }
      inline PAMI_Request_t * getSendRequest ()
      {
        return & _send_request;
      }

      //------------------------------------------
      // -- Executor Virtual Methods
      //------------------------------------------

      virtual void   start          ();
      virtual void   notifySendDone ( const pami_quad_t &info );
      virtual void   notifyRecv     (unsigned src,  const pami_quad_t &info,
                                     char     *buf, unsigned bytes);

      //-----------------------------------------
      //--  Query functions ---------------------
      //-----------------------------------------

      inline unsigned       getRoot   ()
      {
        return _root;
      }
      inline unsigned       getComm   ()
      {
        return _comm;
      }
      inline unsigned       getPwidth ()
      {
        return _pipelinewidth;
      }
      inline T_Sysdep       *getSysdep ()
      {
        return _sd;
      }

      int           startphase() { return _startphase; }
      int           nphases()    { return _nphases; }

    };  //-- OldBroadcast
  };   //-- Executor
};  //-- CCMI

///
/// \brief start sending broadcast data. Only active on the root node
///
template <class T_Sysdep, class T_Mcast, class T_ConnectionManager>
inline void  CCMI::Executor::OldBroadcast<T_Sysdep, T_Mcast, T_ConnectionManager> :: start ()
{
  TRACE_FLOW ((stderr, "<%p>Executor::OldBroadcast::start() phase %d, num total phases %d\n",this, _startphase, _nphases));

  // Nothing to broadcast? We're done.
  if((_buflen == 0) && _cb_done)
    _cb_done (NULL, _clientdata, PAMI_SUCCESS);

  else if(__global.mapping.task() == _root)
  {
    _bytesrecvd = _buflen;
    sendNext ();
  }
}
template <class T_Sysdep, class T_Mcast, class T_ConnectionManager>
inline void  CCMI::Executor::OldBroadcast<T_Sysdep, T_Mcast, T_ConnectionManager> :: sendNext ()
{

  TRACE_FLOW ((stderr, "<%p>Executor::OldBroadcast::sendNext() startphase %d, nphases %d, nmessages %d\n",this,_startphase, _nphases, _nmessages));

  //Leaf node who does not have to send
  if(_startphase == _nphases || _nmessages == 0)
  {
    if(_bytesrecvd == _buflen && _cb_done)
      _cb_done (NULL, _clientdata, PAMI_SUCCESS);
    return;
  }

  //------------------------------------------------------
  //----  Root or intermediate nodes, who have to send ---
  //------------------------------------------------------

  //Message send in progress
  if(_curlen > 0)
    return;

  //Finished sending call done handler
  if(_bytessent == _buflen)
  {
    if(_cb_done)
      _cb_done (NULL, _clientdata, PAMI_SUCCESS);
    return;
  }

  //-------------------------------------------
  //---- Not received enough yet -------------
  //-------------------------------------------
  if((_bytesrecvd < _bytessent + _pipelinewidth) && (_bytesrecvd < _buflen))
    return;

  //-----------------------------------------------
  //---  We have something to send ----------------
  //-----------------------------------------------
  _curlen = _bytesrecvd - _bytessent;

  //Sending message header to setup receive of an async message
  _mdata._comm  = _comm;
  _mdata._root  = _root;
  _mdata._count = -1; // not used on broadcast
  _mdata._phase = 0;

  for(int dcount = 0; dcount < _nmessages; dcount++)
    TRACE_FLOW ((stderr, "<%p>Executor::OldBroadcast::sendNext() send to %d for size %d\n",this, _destpes[dcount], _curlen));

  //Moved to setInfo call
  //unsigned connid =
  //_connmgr->getConnectionId(_comm, _root, _color, (unsigned)-1, (unsigned)-1);
  //_msend.setConnectionId (connid);

  if(_bytessent > 0)
    _msend.flags = 1; //PERSISTENT_MESSAGE);

  _msend.tasks  = _destpes;
  _msend.ntasks = _nmessages;

  _msend.src    = _buf + _bytessent;
  _msend.bytes  = _curlen;
  _mInterface->send(&_msend);

}
template <class T_Sysdep, class T_Mcast, class T_ConnectionManager>
inline void  CCMI::Executor::OldBroadcast<T_Sysdep, T_Mcast, T_ConnectionManager> :: notifySendDone ( const pami_quad_t & info )
{
  TRACE_FLOW ((stderr, "<%p>Executor::OldBroadcast::notifySendDone() nmessages %d\n",this, _nmessages));

  _bytessent += _curlen;
  _curlen = 0;
  CCMI_assert (_bytessent <= _buflen);

  sendNext ();
}

template <class T_Sysdep, class T_Mcast, class T_ConnectionManager>
inline void  CCMI::Executor::OldBroadcast<T_Sysdep, T_Mcast, T_ConnectionManager>::notifyRecv
(unsigned        src,
 const pami_quad_t  & info,
 char          * buf,
 unsigned        bytes)
{
  TRACE_FLOW ((stderr, "<%p>Executor::OldBroadcast::notifyRecv() bytes %d\n",this, bytes));

  //buffer has changed
  if(buf != NULL)
    _buf = buf;

  _bytesrecvd += bytes;

  if(_bytesrecvd > _buflen)
    _bytesrecvd = _buflen;

  sendNext ();
}


#endif
