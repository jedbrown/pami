/**
 * \file algorithms/executor/Gather.h
 * \brief ???
 */
#ifndef __algorithms_executor_Gather_h__
#define __algorithms_executor_Gather_h__


#include "algorithms/interfaces/Schedule.h"
#include "algorithms/interfaces/Executor.h"
#include "algorithms/connmgr/ConnectionManager.h"
#include "algorithms/interfaces/NativeInterface.h"

#define MAX_CONCURRENT 32
#define MAX_PARALLEL 20

#define EXECUTOR_DEBUG(x) // fprintf x

namespace CCMI
{
  namespace Executor
  {
    /*
     * Implements a gather strategy which uses one network link.
     */

    template <class T_Gather_type>
    struct GatherVecType
    {
      // COMPILE_TIME_ASSERT(0==1);
    };

    template<>
    struct GatherVecType<pami_gather_t>
    {
      typedef int base_type;
    };

    template<>
    struct GatherVecType<pami_gatherv_t>
    {
      typedef size_t base_type;
    };

    template<>
    struct GatherVecType<pami_gatherv_int_t>
    {
      typedef int base_type;
    };

    template <class T_Gather_type>
    void setGatherVectors(T_Gather_type *xfer, void *disps, void *sndcounts)
    {
    }

    template<>
    void setGatherVectors<pami_gather_t> (pami_gather_t *xfer, void *disps, void * rcvcounts)
    {
    }

    template<>
    void setGatherVectors<pami_gatherv_t> (pami_gatherv_t *xfer, void *disps, void *rcvcounts)
    {
      *((size_t **)disps)     = xfer->rdispls;
      *((size_t **)rcvcounts) = xfer->rtypecounts;
    }

    template<>
    void setGatherVectors<pami_gatherv_int_t> (pami_gatherv_int_t *xfer, void *disps, void *rcvcounts)
    {
      *((int **)disps)     = xfer->rdispls;
      *((int **)rcvcounts) = xfer->rtypecounts;
    }


    template<class T_ConnMgr, class T_Schedule, typename T_Gather_type>
    class GatherExec : public Interfaces::Executor
    {
      public:

        struct RecvStruct
        {
          int                 subsize;
          PAMI::PipeWorkQueue pwq;
          GatherExec         *exec;
        };

      protected:
        T_Schedule                     * _comm_schedule;
        Interfaces::NativeInterface    * _native;
        T_ConnMgr                      * _connmgr;

        int                 _comm;
        unsigned            _root;
        int                 _buflen;
        int                 _totallen;
        char                *_sbuf;
        char                *_rbuf;
        char                *_tmpbuf;

        unsigned            _myindex;
        unsigned            _rootindex;

        PAMI::PipeWorkQueue _pwq;
        RecvStruct          *_mrecvstr;

        int                 _curphase;
        int                 _nphases;
        int                 _startphase;
        int                 _donecount;
        int                 _mynphases;

        int                 _maxsrcs;
        pami_task_t         _srcranks [MAX_CONCURRENT];
        unsigned            _srclens  [MAX_CONCURRENT];
        PAMI::Topology      _dsttopology;
        PAMI::Topology      _selftopology;
        PAMI::Topology      *_gtopology;

        CollHeaderData      _mdata;
        pami_multicast_t    _msend;

        int                 *_disps;
        int                 *_rcvcounts;

        //Private method
        void             sendNext ();

      public:
        GatherExec () :
            Interfaces::Executor (),
            _comm_schedule(NULL),
            _comm(-1),
            _sbuf(NULL),
            _rbuf(NULL),
            _tmpbuf(NULL),
            _curphase(0),
            _nphases(0),
            _startphase(0),
            _donecount(-1),
            _mynphases(0),
            _disps(NULL),
            _rcvcounts(NULL)
        {
          TRACE_ADAPTOR((stderr, "<%p>Executor::GatherExec()\n", this));
        }

        GatherExec (Interfaces::NativeInterface  * mf,
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
            _mrecvstr(NULL),
            _curphase(0),
            _nphases(0),
            _startphase(0),
            _donecount (-1),
            _mynphases(0),
            _dsttopology(),
            _selftopology(mf->myrank()),
            _gtopology(gtopology),
            _disps(NULL),
            _rcvcounts(NULL)
        {
          TRACE_ADAPTOR((stderr, "<%p>Executor::GatherExec(...)\n", this));
          _clientdata        =  0;
          _root              =  (unsigned) - 1;
          _buflen            =  0;

          _mdata._comm       = _comm;
          _mdata._root       = _root;
          _mdata._count      = -1;
          _mdata._phase      = 0;

          pami_quad_t *info    =  (pami_quad_t*)((void*) & _mdata);
          _msend.msginfo       =  info;
          _msend.msgcount      =  1;
          _msend.roles         = -1U;

        }

        virtual ~GatherExec ()
        {
          /// Todo: convert this to allocator ?
          if (_maxsrcs) free (_mrecvstr);

          if (!(_disps && _rcvcounts)) free (_tmpbuf);
        }

        /// NOTE: This is required to make "C" programs link successfully with virtual destructors
        void operator delete(void * p)
        {
        }

        // --  Initialization routines
        //------------------------------------------

        void setSchedule (T_Schedule *ct)
        {
          TRACE_ADAPTOR((stderr, "<%p>Executor::GatherExec::setSchedule()\n", this));
          _comm_schedule = ct;
          _comm_schedule->init (_root, CCMI::Schedule::GATHER, _startphase, _nphases, _maxsrcs);
          CCMI_assert(_maxsrcs <= MAX_CONCURRENT);

          _mynphases = _comm_schedule->getMyNumPhases();
          CCMI_assert(_mynphases <= MAX_PARALLEL);

          _myindex    = _gtopology->rank2Index(_native->myrank());
          _rootindex  = _gtopology->rank2Index(_root);

          unsigned connection_id = (unsigned) - 1;

          if (_connmgr)
            connection_id = _connmgr->getConnectionId(_comm, _root, 0, (unsigned) - 1, (unsigned) - 1);

          _msend.connection_id = connection_id;

          // todo: this is clearly not scalable, need to use rendezvous similar to
          // that in allgather
          if (_maxsrcs)
            _mrecvstr = (RecvStruct *) malloc (_maxsrcs * _mynphases * sizeof(RecvStruct)) ;
        }

        void setConnectionID (unsigned cid)
        {

          CCMI_assert(_comm_schedule != NULL);

          //Override the connection id from the connection manager
          _msend.connection_id = cid;

        }

        void setRoot(unsigned root)
        {
          TRACE_ADAPTOR((stderr, "<%p>Executor::GatherExec::setRoot()\n", this));
          _root              = root;
          _mdata._root       = _root;
        }

        void  updateBuffers(char *src, char *dst, int len)
        {
          _buflen = len;
          _sbuf   = src;
          _rbuf   = dst;
        }

        void updatePWQ()
        {
          _pwq.configure (NULL, _sbuf, _buflen, 0);
          _pwq.reset();
          _pwq.produceBytes(_buflen);
        }

        void  setBuffers (char *src, char *dst, int len)
        {
          TRACE_ADAPTOR((stderr, "<%p>Executor::GatherExec::setInfo() src %p, dst %p, len %d, _pwq %p\n", this, src, dst, len, &_pwq));

          _buflen   = len;
          _sbuf     = src;
          _rbuf     = dst;

          // ship data length info in the header for async protocols
          _mdata._count = len;

          CCMI_assert(_comm_schedule != NULL);

          if (_native->myrank() == _root)
            {
              _donecount = _native->numranks();
              size_t buflen = 0;

              if (_disps && _rcvcounts)
                {
                  for (unsigned i = 0; i < _native->numranks() ; ++i)
                    {
                      buflen += _rcvcounts[i];

                      if (_rcvcounts[i] == 0 && i != _rootindex) _donecount--;
                    }

                  _buflen = buflen;
                  _tmpbuf = _rbuf;
                }
              else
                {
                  buflen = _native->numranks() * len;
                  _tmpbuf = (char *) malloc(buflen);
                }
            }
          else // setup PWQ
            {

              unsigned ndst;
              _comm_schedule->getLList(_startphase, &_srcranks[0], ndst, &_srclens[0]);
              CCMI_assert(ndst == 1);

              //new (&_dsttopology) PAMI::Topology(_gtopology->index2Rank(_srcranks[0]));
              new (&_dsttopology) PAMI::Topology(_srcranks[0]);

              _donecount        = _srclens[0];
              size_t  buflen    = _srclens[0]  * _buflen;

              if (_mynphases > 1)
                {
                  _tmpbuf = (char *)malloc(buflen);
                  _pwq.configure (NULL, _tmpbuf, buflen, 0);
                }
              else
                {
                  _pwq.configure (NULL, src, buflen, 0);
                }

              _pwq.reset();
              _pwq.produceBytes(buflen);

              _totallen = _srclens[0];

            }

          //TRACE_ADAPTOR((stderr, "<%p>Executor::GatherExec::setInfo() _pwq %p, bytes available %zu/%zu\n", this, &_pwq, _pwq.bytesAvailableToConsume(), _pwq.bytesAvailableToProduce()));

        }

        void setVectors(T_Gather_type *xfer)
        {

          if (_native->myrank() == _root)
            {
              setGatherVectors<T_Gather_type>(xfer, (void *)&_disps, (void *)&_rcvcounts);
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

        static void notifyRecvDone( pami_context_t   context,
                                    void           * cookie,
                                    pami_result_t    result )
        {
          TRACE_MSG ((stderr, "<%p>Executor::GatherExec::notifyRecvDone()\n", cookie));
          RecvStruct    *mrecv = (RecvStruct *) cookie;
          GatherExec<T_ConnMgr, T_Schedule, T_Gather_type> *exec =  mrecv->exec;

          EXECUTOR_DEBUG((stderr, "GatherExec::notifyRecvDone, donecount = %d, subsize = %d\n", exec->_donecount, mrecv->subsize);)
          exec->_donecount -= mrecv->subsize;

          if (exec->_donecount == 0) exec->sendNext();
        }


    };  //-- GatherExec
  };   //-- Executor
};  //-- CCMI

///
/// \brief start sending gather data. Only active on the root node
///
template <class T_ConnMgr, class T_Schedule, typename T_Gather_type>
inline void  CCMI::Executor::GatherExec<T_ConnMgr, T_Schedule, T_Gather_type>::start ()
{
  TRACE_ADAPTOR((stderr, "<%p>Executor::GatherExec::start() count%d\n", this, _buflen));

  EXECUTOR_DEBUG((stderr, "GatherExec::start, mynphases = %d, buflen = %d, donecount = %d\n",
                  _mynphases, _buflen, _donecount);)

  // Nothing to gather? We're done. What if in Gatherv ?
  if ((_buflen == 0) && _cb_done)
    {
      _cb_done (NULL, _clientdata, PAMI_SUCCESS);
      return;
    }

  _curphase  = _startphase;

  if (_native->myrank() == _root)
    {
      if (_disps && _rcvcounts)
        memcpy(_rbuf + _disps[_rootindex], _sbuf, _rcvcounts[_rootindex]);
      else
        memcpy(_rbuf + _rootindex * _buflen, _sbuf, _buflen);
    }
  else if (_mynphases > 1)  memcpy(_tmpbuf, _sbuf, _buflen);

  --_donecount;

  if (_donecount == 0) sendNext();
}

template <class T_ConnMgr, class T_Schedule, typename T_Gather_type>
inline void  CCMI::Executor::GatherExec<T_ConnMgr, T_Schedule, T_Gather_type>::sendNext ()
{
  CCMI_assert(_comm_schedule != NULL);
  CCMI_assert(_donecount  == 0);

  // TODO: needs to add noncontiguous datatype handling
  if (_native->myrank() == _root)
    {

      if (!(_disps && _rcvcounts))
        {
          if (_rootindex != 0)
            {
              memcpy (_rbuf + ((_myindex + 1) % _native->numranks())* _buflen, _tmpbuf + _buflen, (_native->numranks() - _myindex - 1)*_buflen);
              memcpy (_rbuf, _tmpbuf + (_native->numranks() - _myindex)*_buflen, _myindex * _buflen);
            }
          else
            {
              memcpy (_rbuf, _tmpbuf, _myindex * _buflen);
            }
        }

      if (_cb_done) _cb_done (NULL, _clientdata, PAMI_SUCCESS);

      return;
    }

  _mdata._phase             = _startphase;
  _msend.src_participants   = (pami_topology_t *) & _selftopology;
  _msend.dst_participants   = (pami_topology_t *) & _dsttopology;
  _msend.cb_done.function   = _cb_done;
  _msend.cb_done.clientdata = _clientdata;
  _msend.src                = (pami_pipeworkqueue_t *) & _pwq;
  _msend.dst                = NULL;
  _msend.bytes              = _totallen * _buflen;

  EXECUTOR_DEBUG((stderr, "GatherExec::sendNext() to %d, totalcnt = %d, buflen = %d, multicast address %p, %p\n", _dsttopology.index2Rank(0), _totallen, _buflen, &_msend, &_mdata);)
  _native->multicast(&_msend);
}

template <class T_ConnMgr, class T_Schedule, typename T_Gather_type >
inline void  CCMI::Executor::GatherExec<T_ConnMgr, T_Schedule, T_Gather_type>::notifyRecv
(unsigned             src,
 const pami_quad_t   & info,
 PAMI::PipeWorkQueue ** pwq,
 pami_callback_t      * cb_done)
{

  CollHeaderData *cdata = (CollHeaderData*) & info;

  unsigned i;
  unsigned nsrcs;
  _comm_schedule->getRList(cdata->_phase, &_srcranks[0], nsrcs, &_srclens[0]);

  for (i = 0; i < nsrcs; ++i)

    //if (_srcranks[i] == _gtopology->rank2Index(src)) break;
    if (_srcranks[i] == src) break;

  CCMI_assert(i < nsrcs);

  unsigned srcindex = _gtopology->rank2Index(_srcranks[i]);
  unsigned size     = _gtopology->size();

  size_t      buflen;
  unsigned    offset;

  if (_disps && _rcvcounts)
    {
      CCMI_assert(_native->myrank() == _root);
      _srclens[i] = 1;
      buflen    =  _rcvcounts[srcindex];
      offset    =  _disps[srcindex];
    }
  else if (0 && (unsigned)_mynphases == _native->numranks() - 1)
    {
      _srclens[i] = 1;
      buflen   = _buflen;
      offset   = srcindex * _buflen;
    }
  else
    {
      buflen   = _srclens[i] * _buflen;
      offset   = ((srcindex + size - _myindex) % size) * _buflen; // will root be affected by this ?
    }

  // CCMI_assert (buflen == cdata->_count);

  char    *tmpbuf   = _tmpbuf + offset;
  unsigned ind      = cdata->_phase * _maxsrcs + i;
  *pwq = &_mrecvstr[ind].pwq;
  (*pwq)->configure (NULL, tmpbuf, buflen, 0);
  (*pwq)->reset();
  // (*pwq)->produceBytes(buflen);
  _mrecvstr[ind].subsize  = _srclens[i];
  _mrecvstr[ind].exec     = this;

  EXECUTOR_DEBUG((stderr, "GatherExec::notifyRecv - src = %d, offset = %d, lenth = %d, phase = %d\n", src, offset, buflen, cdata->_phase);)

  cb_done->function    = notifyRecvDone;
  cb_done->clientdata  = &_mrecvstr[ind];
}

#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
