/**
 * \file algorithms/executor/Scatter.h
 * \brief ???
 */
#ifndef __algorithms_executor_Scatter_h__
#define __algorithms_executor_Scatter_h__


#include "algorithms/interfaces/Schedule.h"
#include "algorithms/interfaces/Executor.h"
#include "algorithms/connmgr/ConnectionManager.h"
#include "algorithms/interfaces/NativeInterface.h"
#include "common/default/Topology.h"

#define MAX_CONCURRENT 32
#define MAX_PARALLEL 20

namespace CCMI
{
  namespace Executor
  {
    /*
     * Implements a scatter strategy which uses one network link.
     */

      template <class T_Scatter_type>
      struct ScatterVecType
      {
         // COMPILE_TIME_ASSERT(0==1);
      };

      template<>
      struct ScatterVecType<pami_scatter_t>
      {
         typedef int base_type;
      };

      template<>
      struct ScatterVecType<pami_scatterv_t> {
         typedef size_t base_type;
      };

      template<>
      struct ScatterVecType<pami_scatterv_int_t> {
         typedef int base_type;
      };

      template <class T_Scatter_type>
      void setScatterVectors(T_Scatter_type *xfer, void *disps, void *sndcounts)
      {
      }

      template<>
      void setScatterVectors<pami_scatter_t> (pami_scatter_t *xfer, void *disps, void * sndcounts)
      {
      }

      template<>
      void setScatterVectors<pami_scatterv_t> (pami_scatterv_t *xfer, void *disps, void * sndcounts)
      {
            *((size_t **)disps)     = xfer->sdispls;
            *((size_t **)sndcounts) = xfer->stypecounts;
      }

      template<>
      void setScatterVectors<pami_scatterv_int_t> (pami_scatterv_int_t *xfer, void *disps, void * sndcounts)
      {
           *((int **)disps)     = xfer->sdispls;
           *((int **)sndcounts) = xfer->stypecounts;
      }

    template<class T_ConnMgr, class T_Schedule, typename T_Scatter_type>
    class ScatterExec : public Interfaces::Executor
    {
      public:

        struct SendStruct {
          pami_multicast_t    msend;
          PAMI::PipeWorkQueue pwq;
          PAMI::Topology      dsttopology;
        } ;

      protected:
        T_Schedule                     * _comm_schedule;
        Interfaces::NativeInterface    * _native;
        T_ConnMgr                      * _connmgr;

        int                 _comm;
        unsigned            _root;

        unsigned            _rootindex;
        unsigned            _myindex;

        int                 _buflen;
        char                *_sbuf;
        char                *_rbuf;
        char                *_tmpbuf;

        PAMI::PipeWorkQueue _pwq;

        int                 _curphase;
        int                 _nphases;
        int                 _startphase;
        int                 _donecount;

        int                 _maxdsts;
        pami_task_t         _dstranks [MAX_CONCURRENT];
        unsigned            _dstlens  [MAX_CONCURRENT];
        PAMI::Topology      _selftopology;
        PAMI::Topology      *_gtopology;

        CollHeaderData      _mdata;
        SendStruct          *_msendstr;

        typedef typename ScatterVecType<T_Scatter_type>::base_type basetype;

        basetype            *_disps;
        basetype            *_sndcounts;

        //Private method
        void             sendNext ();

      public:
        ScatterExec () :
            Interfaces::Executor (),
            _comm_schedule(NULL),
            _comm(-1),
            _sbuf(NULL),
            _rbuf(NULL),
            _tmpbuf(NULL),
            _curphase(0),
            _nphases(0),
            _startphase(0),
            _disps(NULL),
            _sndcounts(NULL)
        {
          TRACE_ADAPTOR((stderr, "<%p>Executor::ScatterExec()\n", this));
        }

        ScatterExec (Interfaces::NativeInterface  * mf,
                       T_ConnMgr                    * connmgr,
                       unsigned                       comm,
                       PAMI::Topology               *gtopology) :
            Interfaces::Executor(),
            _comm_schedule (NULL),
            _native(mf),
            _connmgr(connmgr),
            _comm(comm),
            _sbuf(NULL),
            _rbuf(NULL),
            _tmpbuf(NULL),
            _curphase(0),
            _nphases(0),
            _startphase(0),
            _selftopology(mf->myrank()),
            _gtopology(gtopology),
            _msendstr(NULL),
            _disps(NULL),
            _sndcounts(NULL)
        {
          TRACE_ADAPTOR((stderr, "<%p>Executor::ScatterExec(...)\n", this));
          _clientdata        =  0;
          _root              =  (unsigned) - 1;
          _buflen            =  0;

          _donecount         =   0;

          _mdata._comm       = _comm;
          _mdata._root       = _root;
          _mdata._count      = -1;
          _mdata._phase      = 0;

        }

        virtual ~ScatterExec ()
        {
           /// Todo: convert this to allocator ?
           if (_maxdsts) free (_msendstr);
           if (_native->myrank() != _root || (_root != 0 && _native->numranks() != (unsigned)_nphases+1)) free (_tmpbuf);
        }

        /// NOTE: This is required to make "C" programs link successfully with virtual destructors
        void operator delete(void * p)
        {
        }

        // --  Initialization routines
        //------------------------------------------

        void setSchedule (T_Schedule *ct)
        {
          TRACE_ADAPTOR((stderr, "<%p>Executor::ScatterExec::setSchedule()\n", this));
          _comm_schedule = ct;
          _comm_schedule->init (_root, CCMI::Schedule::SCATTER, _startphase, _nphases, _maxdsts);
          CCMI_assert(_maxdsts <= MAX_CONCURRENT);
          // overwrite _nphase since we only care about my own number of phases
          _nphases = _comm_schedule->getMyNumPhases();

          //fprintf(stderr, "_nphases = %d, _startphase = %d, _maxdsts = %d\n", _nphases, _startphase, _maxdsts);

          //CCMI_assert(_comm_schedule != NULL);
          //_comm_schedule->getDstUnionTopology (&_dsttopology);

          _myindex    = _gtopology->rank2Index(_native->myrank());
          _rootindex  = _gtopology->rank2Index(_root);

          unsigned connection_id = (unsigned) -1;
          if (_connmgr)
            connection_id = _connmgr->getConnectionId(_comm, _root, 0, (unsigned) - 1, (unsigned) - 1);

          _msendstr = NULL;
          if (_maxdsts)
          {
            _msendstr = (SendStruct *) malloc (_maxdsts * sizeof(SendStruct)) ;

            pami_quad_t *info      =  (pami_quad_t*)((void*) & _mdata);
            for (int i = 0; i <_maxdsts; ++i)
            {
              _msendstr[i].msend.msginfo       =  info;
              _msendstr[i].msend.msgcount      =  1;
              _msendstr[i].msend.roles         = -1U;
              _msendstr[i].msend.connection_id = connection_id;
            }
          }
        }

        void setConnectionID (unsigned cid)
        {

          CCMI_assert(_comm_schedule != NULL);

          //Override the connection id from the connection manager
          for (int i = 0; i <_maxdsts; ++i) _msendstr[i].msend.connection_id = cid;

        }

        void setRoot(unsigned root)
        {
          TRACE_ADAPTOR((stderr, "<%p>Executor::ScatterExec::setRoot()\n", this));
          _root        = root;
          _mdata._root = root;
        }

        void  setBuffers (char *src, char *dst, int len)
        {

          _buflen = len;
          _sbuf = src;
          _rbuf = dst;

          // ship data length info in the header for async protocols
          _mdata._count = len;

          CCMI_assert(_comm_schedule != NULL);
          // setup PWQ
          if (_native->myrank() == _root)
          {
            if ((unsigned)_nphases == _native->numranks()-1 || _root == 0)
              _tmpbuf = src;
            else  // allocate temporary buffer and reshuffle the data
            {
              size_t buflen = _native->numranks() * len;
              _tmpbuf = (char *) malloc(buflen);
              memcpy (_tmpbuf, src+_myindex*len, (_native->numranks() - _myindex)*len);
              memcpy (_tmpbuf+(_native->numranks() - _myindex)*len  ,src, _myindex * len);
            }
          }
          else if (_nphases > 1)
          {
            // schedule's getLList() method can be used for an accurate buffer size
            size_t  buflen = _native->numranks() * len;
            _tmpbuf = (char *)malloc(buflen);
            _pwq.configure (NULL, _tmpbuf, buflen, 0);
            _pwq.reset();
          }
          else
          {
            _pwq.configure (NULL, dst, len, 0);
            _pwq.reset();
          }

        }

        void setVectors(T_Scatter_type *xfer)
        {

           if (_native->myrank() == _root) {
             setScatterVectors<T_Scatter_type>(xfer, (void *)&_disps, (void *)&_sndcounts);
           }
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
          return _root;
        }
        unsigned       getComm   ()
        {
          return _comm;
        }

        static void notifySendDone (pami_context_t context, void *cookie, pami_result_t result)
        {
          TRACE_MSG ((stderr, "<%p>Executor::ScatterExec::notifySendDone()\n", cookie));
          ScatterExec<T_ConnMgr, T_Schedule, T_Scatter_type> *exec =  (ScatterExec<T_ConnMgr, T_Schedule, T_Scatter_type> *) cookie;
          exec->_donecount --;
          exec->_curphase  ++;
          exec->sendNext();
        }

        static void notifyRecvDone( pami_context_t   context,
                                    void           * cookie,
                                    pami_result_t    result )
        {
          TRACE_MSG ((stderr, "<%p>Executor::ScatterExec::notifyRecvDone()\n", cookie));
          ScatterExec<T_ConnMgr, T_Schedule, T_Scatter_type> *exec =  (ScatterExec<T_ConnMgr, T_Schedule, T_Scatter_type> *) cookie;
          exec->_curphase      =  exec->_startphase+1;
          exec->sendNext();
        }


    };  //-- ScatterExec
  };   //-- Executor
};  //-- CCMI

///
/// \brief start sending scatter data. Only active on the root node
///
template <class T_ConnMgr, class T_Schedule, typename T_Scatter_type>
inline void  CCMI::Executor::ScatterExec<T_ConnMgr, T_Schedule, T_Scatter_type>::start ()
{
  TRACE_ADAPTOR((stderr, "<%p>Executor::ScatterExec::start() count%d\n", this, _buflen));

  // Nothing to scatter? We're done.
  if ((_buflen == 0) && _cb_done && !(_disps && _sndcounts))
    {
      _cb_done (NULL, _clientdata, PAMI_SUCCESS);
      return;
    }

  _curphase  = 0;

  if (_native->myrank() == _root)
    {
      sendNext ();
    }
}

template <class T_ConnMgr, class T_Schedule, typename T_Scatter_type>
inline void  CCMI::Executor::ScatterExec<T_ConnMgr, T_Schedule, T_Scatter_type>::sendNext ()
{
  unsigned ndst = 0;
  unsigned size     = _gtopology->size();

  CCMI_assert(_comm_schedule != NULL);
  CCMI_assert(_curphase >= _startphase);
  if (_curphase == _startphase + _nphases)
  {
     // all parents copy from send buffer to application destination buffer
     if (_disps && _sndcounts)
       memcpy(_rbuf, _sbuf+_disps[_myindex], _buflen);
     else if (_native->myrank() == _root)
       memcpy(_rbuf, _sbuf+_myindex * _buflen, _buflen);
     else if (_nphases > 1)
       memcpy(_rbuf, _tmpbuf, _buflen);

     if (_cb_done) _cb_done (NULL, _clientdata, PAMI_SUCCESS);
     return;
  }

  _comm_schedule->getRList(_curphase, &_dstranks[0], ndst, &_dstlens[0]);

  CCMI_assert(_donecount  == 0);
  _donecount = ndst;

  for (unsigned i = 0; i < ndst; ++i)
  {

    SendStruct *sendstr = &(_msendstr[i]);
    pami_multicast_t *msend = &sendstr->msend;
    //new (&sendstr->dsttopology) PAMI::Topology(_gtopology->index2Rank(_dstranks[i]));
    new (&sendstr->dsttopology) PAMI::Topology(_dstranks[i]);

    unsigned dstindex = _gtopology->rank2Index(_dstranks[i]);
    size_t buflen;
    unsigned offset;
    if (_disps && _sndcounts)
    {
      CCMI_assert(_native->myrank() == _root);
      CCMI_assert(ndst == 1);
      buflen   =  _sndcounts[dstindex];
      offset   =  _disps[dstindex];
      _mdata._count = buflen;
    }
    else if ((unsigned)_nphases == _native->numranks() - 1)
    {
      buflen   = _buflen;
      offset   = dstindex * _buflen;
    }
    else
    {
      buflen   = _dstlens[i] * _buflen;
      offset   = ((dstindex + size - _myindex)% size) * _buflen;
    }

    char    *tmpbuf   = _tmpbuf + offset;
    sendstr->pwq.configure (NULL, tmpbuf, buflen, 0);
    sendstr->pwq.reset();
    sendstr->pwq.produceBytes(buflen);

    msend->src_participants   = (pami_topology_t *) & _selftopology;
    msend->dst_participants   = (pami_topology_t *) & sendstr->dsttopology;
    msend->cb_done.function   = notifySendDone;
    msend->cb_done.clientdata = this;
    msend->src                = (pami_pipeworkqueue_t *) & sendstr->pwq;
    msend->dst                = NULL;
    msend->bytes              = buflen;
    _native->multicast(&_msendstr[i].msend);
  }
}

template <class T_ConnMgr, class T_Schedule, typename T_Scatter_type>
inline void  CCMI::Executor::ScatterExec<T_ConnMgr, T_Schedule, T_Scatter_type>::notifyRecv
(unsigned             src,
 const pami_quad_t   & info,
 PAMI::PipeWorkQueue ** pwq,
 pami_callback_t      * cb_done)
{
  TRACE_MSG((stderr, "<%p> Executor::ScatterExec::notifyRecv() from %d, dsttopology.size %zu\n", this, src, _dsttopology.size()));

  *pwq = &_pwq;

  if (_nphases > 1)
    {
      TRACE_ADAPTOR((stderr, "<%p> Executor::ScatterExec::notifyRecv() dsttopology.size %zu\n", this, _dsttopology.size()));
      /// \todo this sendNext() should have worked but MPI platform didn't support it (yet).
      //    cb_done->function = NULL;  //There is a send here that will notify completion
      //    sendNext ();
      cb_done->function = notifyRecvDone;
      cb_done->clientdata = this;
    }
  else
    {
      TRACE_ADAPTOR((stderr, "<%p> Executor::ScatterExec::notifyRecv() Nothing to send, receive completion indicates completion\n", this));
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
