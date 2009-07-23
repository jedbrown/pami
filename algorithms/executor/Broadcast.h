/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/executor/Broadcast.h
 * \brief ???
 */

#ifndef   __broadcast_executor_h__
#define   __broadcast_executor_h__

//#define TRACE_FLOW(x)  fprintf x

#include "algorithms/schedule/Schedule.h"
#include "algorithms/executor/Executor.h"
#include "interface/MultiSend.h"
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
    class Broadcast : public Executor
    {
    protected:

      CollectiveMapping                                   * _mapping;
      Schedule::Schedule                        * _comm_schedule;
      MultiSend::OldMulticastInterface             * _mInterface;
      MultiSend::CCMI_OldMulticast_t         _msend;

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

      //CM_Callback_t           _msend_cb;
      CM_Request_t            _send_request __attribute__((__aligned__(16)));   /// send request
      CM_Request_t            _recv_request __attribute__((__aligned__(16)));   /// recv request

      CollHeaderData           _mdata;
      ConnectionManager::ConnectionManager     * _connmgr;
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

          TRACE_FLOW ((stderr, "destpe %d phase %d", _destpes[_nmessages], count+_startphase));

          CCMI_assert(_nmessages + ndest < MAX_PARALLEL);
          _nmessages += ndest;
        }
      }


    public:

      static void staticSendDone (void *clientdata, CM_Error_t *err)
      {
        CMQuad * info = NULL;
        Broadcast *bcast = (Broadcast *) clientdata;
        bcast->notifySendDone( *info );
      }

      inline Broadcast () :
      Executor (),
      _comm_schedule(NULL),
      _comm(-1),
      _startphase (-1),
      _nphases (-1),
      _curlen(-1)
      {
      }

      inline Broadcast (CollectiveMapping *map, unsigned comm,
                        ConnectionManager::ConnectionManager *connmgr,
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

        _mapping             =   map;

        _msend.setRequestBuffer(&_send_request);
        _msend.setConsistency (CCMI_MATCH_CONSISTENCY);

        CMQuad *info = (_postReceives)?(NULL):(CMQuad*)((void*)&_mdata);
        _msend.setInfo(info,  1);
        _msend.setFlags (MultiSend::CCMI_FLAGS_UNSET);

        //	  _msend.setSendData (NULL, 0);
        // _msend.setRanks (&_destrank, 1);
        _msend.setOpcodes((CCMI_Subtask *) &_hints);

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

      inline void setMulticastInterface (MultiSend::OldMulticastInterface *mf)
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
	_msend.setConnectionId (connid);      
      }

      inline void setPipelineWidth (int pwidth) {
	_pipelinewidth = pwidth;
      }

      inline CM_Request_t * getRecvRequest ()
      {
        return & _recv_request;
      }
      inline CM_Request_t * getSendRequest ()
      {
        return & _send_request;
      }

      //------------------------------------------
      // -- Executor Virtual Methods
      //------------------------------------------

      virtual void   start          ();
      virtual void   notifySendDone ( const CMQuad &info );
      virtual void   notifyRecv     (unsigned src,  const CMQuad &info,
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
      inline CollectiveMapping       *getMapping ()
      {
        return _mapping;
      }

      int           startphase() { return _startphase; }
      int           nphases()    { return _nphases; }

    };  //-- Broadcast
  };   //-- Executor
};  //-- CCMI

///
/// \brief start sending broadcast data. Only active on the root node
///
inline void  CCMI::Executor::Broadcast :: start ()
{
  TRACE_FLOW ((stderr, "In Start with phase %d, num total phases %d\n", _startphase, _nphases));

  // Nothing to broadcast? We're done.
  if((_buflen == 0) && _cb_done)
    _cb_done (_clientdata, NULL);

  else if(_mapping->rank() == _root)
  {
    _bytesrecvd = _buflen;
    sendNext ();
  }
}

inline void  CCMI::Executor::Broadcast :: sendNext ()
{

  TRACE_FLOW ((stderr, "In Send Next %d, %d, %d\n", _startphase, _nphases, _nmessages));

  //Leaf node who does not have to send
  if(_startphase == _nphases || _nmessages == 0)
  {
    if(_bytesrecvd == _buflen && _cb_done)
      _cb_done (_clientdata, NULL);
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
      _cb_done (_clientdata, NULL);
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
    TRACE_FLOW ((stderr, "Calling multisend to %d for size %d\n", _destpes[dcount], _curlen));

  //Moved to setInfo call
  //unsigned connid = 
  //_connmgr->getConnectionId(_comm, _root, _color, (unsigned)-1, (unsigned)-1);
  //_msend.setConnectionId (connid);      

  if(_bytessent > 0)
    _msend.setFlags (MultiSend::CCMI_PERSISTENT_MESSAGE);
  
  _msend.setRanks    (_destpes, _nmessages);
  _msend.setSendData (_buf + _bytessent, _curlen);
  _mInterface->send(&_msend);

}

inline void  CCMI::Executor::Broadcast :: notifySendDone ( const CMQuad & info )
{
  TRACE_FLOW((stderr, "In notify send done %d\n", _nmessages));

  _bytessent += _curlen;
  _curlen = 0;
  CCMI_assert (_bytessent <= _buflen);

  sendNext ();
}


inline void  CCMI::Executor::Broadcast::notifyRecv
(unsigned        src,
 const CMQuad  & info,
 char          * buf,
 unsigned        bytes)
{
  TRACE_FLOW((stderr, "In notify recv for %d\n", bytes));

  //buffer has changed
  if(buf != NULL)
    _buf = buf;

  _bytesrecvd += bytes;

  if(_bytesrecvd > _buflen)
    _bytesrecvd = _buflen;

  sendNext ();
}


#endif
