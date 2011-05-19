/**
 * \file algorithms/executor/Broadcast.h
 * \brief ???
 */
#ifndef __algorithms_executor_Broadcast_h__
#define __algorithms_executor_Broadcast_h__


#include "algorithms/interfaces/Schedule.h"
#include "algorithms/interfaces/Executor.h"
#include "algorithms/connmgr/ConnectionManager.h"
#include "algorithms/interfaces/NativeInterface.h"

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
        T                              * _connmgr;
        bool                             _postReceives;
        CollHeaderData                   _mdata;
        pami_multicast_t                 _msend;
        PAMI::PipeWorkQueue              _pwq;

        pami_task_t             _dstranks [MAX_PARALLEL];
        PAMI::Topology          _dsttopology;
        PAMI::Topology          _selftopology;
        PAMI::Topology          _roottopology;

        //Private method
        void             sendNext ();

      public:
        BroadcastExec () :
            Interfaces::Executor (),
            _comm_schedule(NULL)
	      //_comm(-1)
        {
          TRACE_ADAPTOR((stderr, "<%p>Executor::BroadcastExec()\n", this));
        }

        BroadcastExec (Interfaces::NativeInterface  * mf,
                       T                            * connmgr,
                       unsigned                       comm):
            Interfaces::Executor(),
            _comm_schedule (NULL),
            _native(mf),
	    _connmgr(connmgr),
	    _postReceives (false),
            _dsttopology((pami_task_t *)&_dstranks, MAX_PARALLEL),
	    _selftopology(mf->myrank())
        {
          TRACE_ADAPTOR((stderr, "<%p>Executor::BroadcastExec(...)\n", this));
          //_root              =  (unsigned) - 1;
          //_buflen            =  0;
	  pami_quad_t *info   =  (pami_quad_t*)((void*) & _mdata);
          _msend.msginfo     =  info;
          _msend.msgcount    =  1; 
          _msend.roles       = -1U;
	  _msend.src_participants  = (pami_topology_t *) & _selftopology;
	  _msend.dst_participants  = (pami_topology_t *) & _dsttopology;
	  _msend.src               = (pami_pipeworkqueue_t *) & _pwq;
	  _msend.dst               =   NULL;	  
	  _mdata._comm       =  comm;
	  _mdata._count = -1; // not used on broadcast
	  _mdata._phase = 0;
        }

        void setPostReceives ()
        {
          _postReceives    =  true;
          _msend.msginfo   =  NULL;
          _msend.msgcount  =  0; // only async exec needs metadata
        }

        //-----------------------------------------
        // --  Initialization routines
        //------------------------------------------

        void setSchedule (Interfaces::Schedule *ct, unsigned color)
        {
          TRACE_ADAPTOR((stderr, "<%p>Executor::BroadcastExec::setSchedule()\n", this));
          //_color = color;
          _comm_schedule = ct;
          int nph, phase;
          _comm_schedule->init (_mdata._root, BROADCAST_OP, phase, nph);
          CCMI_assert(_comm_schedule != NULL);
          _comm_schedule->getDstUnionTopology (&_dsttopology);

          if (_connmgr)
            _msend.connection_id = _connmgr->getConnectionId(_mdata._comm, _mdata._root, color, (unsigned) - 1, (unsigned) - 1);
        }

        void setConnectionID (unsigned cid)
        {
          //Override the connection id from the connection manager
          _msend.connection_id = cid;
        }

        void setRoot(unsigned root)
        {
          TRACE_ADAPTOR((stderr, "<%p>Executor::BroadcastExec::setRoot() root %u\n", this, root));
          _mdata._root = root;
          new (&_roottopology) PAMI::Topology(root);
        }

        void  setBuffers (char *src, char *dst, int len)
        {
          TRACE_ADAPTOR((stderr, "<%p>Executor::BroadcastExec::setBuffers() src %p, dst %p, len %d, _pwq %p\n", this, src, dst, len, &_pwq));
          _msend.bytes = len;

          //Setup pipework queue. This depends on setRoot so it better be correct
          size_t bufinit = 0;
          if (_native->myrank() == _mdata._root)
            bufinit = len;
          _pwq.configure (src, len, bufinit);
          _pwq.reset();
          TRACE_ADAPTOR((stderr, "<%p>Executor::BroadcastExec::setBuffers() _pwq %p, bytes available %zu/%zu\n", this, &_pwq,
                         _pwq.bytesAvailableToConsume(), _pwq.bytesAvailableToProduce()));
        }

        //------------------------------------------
        // -- Executor Virtual Methods
        //------------------------------------------
        virtual void   start          ();
        virtual void   notifyRecv     (unsigned             src,
                                       const pami_quad_t   & info,
                                       PAMI::PipeWorkQueue ** pwq,
                                       pami_callback_t      * cb_done);

        //-----------------------------------------
        //--  Query functions ---------------------
        //-----------------------------------------
        unsigned       getRoot   ()
        {
          return _mdata._root;
        }
        unsigned       getComm   ()
        {
          return _mdata._comm;
        }

        void postReceives ()
        {
          if (_native->myrank() == _mdata._root) return;

          if (_msend.bytes == 0) return; //we call callback in start

          pami_multicast_t mrecv;
          //memcpy (&mrecv, &_msend, sizeof(pami_multicast_t));
	  mrecv.msginfo  = _msend.msginfo;
	  mrecv.msgcount = _msend.msgcount;
	  mrecv.connection_id = _msend.connection_id;

          TRACE_MSG((stderr, "<%p>Executor::BroadcastExec::postReceives ndest %zu, bytes %zu, rank %u, root %u\n", this, _dsttopology.size(), _msend.bytes, _selftopology.index2Rank(0),_roottopology.index2Rank(0)));
          mrecv.src_participants   = (pami_topology_t *) & _roottopology; 
          mrecv.dst_participants   = (pami_topology_t *) & _selftopology;

          if (_dsttopology.size() == 0)
            {
              mrecv.cb_done.function   = _cb_done;
              mrecv.cb_done.clientdata = _clientdata;
            }
          else
            {
              mrecv.cb_done.function   = NULL;
              mrecv.cb_done.clientdata = NULL;
            }

          mrecv.dst    =  (pami_pipeworkqueue_t *) & _pwq;
          mrecv.src    =  NULL;
          mrecv.bytes  = _msend.bytes;
          _native->multicast(&mrecv);
        }
        static void notifyRecvDone( pami_context_t   context,
                                    void           * cookie,
                                    pami_result_t    result )
        {
          TRACE_MSG ((stderr, "<%p>Executor::BroadcastExec::notifyRecvDone()\n", cookie));
          BroadcastExec<T> *exec =  (BroadcastExec<T> *) cookie;
          exec->sendNext();
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
  TRACE_ADAPTOR((stderr, "<%p>Executor::BroadcastExec::start() count %zu\n", this, _msend.bytes));

  // Nothing to broadcast? We're done.
  if ((_msend.bytes == 0) && _cb_done)
    {
      _cb_done (NULL, _clientdata, PAMI_SUCCESS);
      return;
    }

  if (_native->myrank() == _mdata._root || _postReceives)
    {
      sendNext ();
    }
}

template <class T>
inline void  CCMI::Executor::BroadcastExec<T>::sendNext ()
{
  //CCMI_assert (_dsttopology.size() != 0); //We have nothing to send
  if (_dsttopology.size() == 0)
    {
      TRACE_MSG((stderr, "<%p>Executor::BroadcastExec::sendNext() bytes %zu, ndsts %zu bytes available to consume %zu\n",
                 this, _msend.bytes, _dsttopology.size(), _pwq.bytesAvailableToConsume()));
      //_cb_done(NULL, _clientdata, PAMI_SUCCESS);
      return;
    }

#ifdef CCMI_DEBUG
  char tbuf[1024];
  char sbuf[16384];
  sprintf(sbuf, "<%p>Executor::BroadcastExec::sendNext() from %zu: bytes %zu, ndsts %zu bytes available to consume %zu ",
          this,__global.mapping.task(), _msend.bytes, _dsttopology.size(), _pwq.bytesAvailableToConsume());

  for (unsigned i = 0; i < _dsttopology.size(); ++i)
    {
      sprintf(tbuf, " dstrank[%d] = %d/%d ", i, _dstranks[i], _dsttopology.index2Rank(i));
      strcat (sbuf, tbuf);
    }

  fprintf (stderr, " %s\n", sbuf);
#endif

  TRACE_MSG((stderr, "<%p>Executor::BroadcastExec::sendNext() bytes %zu, ndsts %zu bytes available to consume %zu\n",
             this, _msend.bytes, _dsttopology.size(), _pwq.bytesAvailableToConsume()));

  //Sending message header to setup receive of an async message
  //  _mdata._comm  = _comm;
  //_mdata._root  = _root;
  _msend.cb_done.function   = _cb_done;
  _msend.cb_done.clientdata = _clientdata;
  //_msend.bytes  = _msend.bytes;
  _native->multicast(&_msend);
}

template <class T>
inline void  CCMI::Executor::BroadcastExec<T>::notifyRecv
(unsigned             src,
 const pami_quad_t   & info,
 PAMI::PipeWorkQueue ** pwq,
 pami_callback_t      * cb_done)
{
  TRACE_MSG((stderr, "<%p> Executor::BroadcastExec::notifyRecv() from %d, dsttopology.size %zu\n", this, src, _dsttopology.size()));

  *pwq = &_pwq;

  if (_dsttopology.size() > 0)
    {
      TRACE_ADAPTOR((stderr, "<%p> Executor::BroadcastExec::notifyRecv() dsttopology.size %zu\n", this, _dsttopology.size()));
      /// \todo this sendNext() should have worked but MPI platform didn't support it (yet).
      //    cb_done->function = NULL;  //There is a send here that will notify completion
      //    sendNext ();
      cb_done->function = notifyRecvDone;
      cb_done->clientdata = this;
    }
  else
    {
      TRACE_ADAPTOR((stderr, "<%p> Executor::BroadcastExec::notifyRecv() Nothing to send, receive completion indicates completion\n", this));
      cb_done->function   = _cb_done;
      cb_done->clientdata = _clientdata;
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
