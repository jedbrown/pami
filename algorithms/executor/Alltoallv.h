/**
 * \file algorithms/executor/Alltoallv.h
 * \brief ???
 */
#ifndef __algorithms_executor_Alltoallv_h__
#define __algorithms_executor_Alltoallv_h__


#include "algorithms/interfaces/Schedule.h"
#include "algorithms/interfaces/Executor.h"
#include "algorithms/connmgr/ConnectionManager.h"
#include "algorithms/interfaces/NativeInterface.h"

#define MAX_CONCURRENT 32
#define MAX_PARALLEL 20

namespace CCMI
{
  namespace Executor
  {
    /*
     * Implements a alltoallv strategy which uses one network link.
     */

      template <class T_Alltoall_type>
      struct AlltoallVecType
      {
         // COMPILE_TIME_ASSERT(0==1);
      };

      template<>
      struct AlltoallVecType<pami_alltoall_t>
      {
         typedef int base_type;
      };

      template<>
      struct AlltoallVecType<pami_alltoallv_t> {
         typedef size_t base_type;
      };

      template<>
      struct AlltoallVecType<pami_alltoallv_int_t> {
         typedef int base_type;
      };

      template <class T_Alltoall_type>
      inline void setAlltoallVec(T_Alltoall_type *xfer, void *sbuf, void *scounts, void *sdisps,
                                   void *rbuf, void *rcounts, void *rdisps)
      {
         COMPILE_TIME_ASSERT(0==1);
      }

      template <>
      inline void setAlltoallVec<pami_alltoall_t> (pami_alltoall_t *xfer, void *sbuf, void *scounts, void *sdisps,                                   void *rbuf, void *rcounts, void *rdisps)
      {
         *((char **)rbuf)   = xfer->rcvbuf;
         *((char **)sbuf)   = xfer->sndbuf;
         return;
      }

      template <>
      inline void setAlltoallVec<pami_alltoallv_t> (pami_alltoallv_t *xfer,
           void *sbuf, void *scounts, void *sdisps, void *rbuf, void *rcounts, void *rdisps)
      {
        *((char **)sbuf)      = xfer->sndbuf;
        *((size_t **)sdisps)  = xfer->sdispls;
        *((size_t **)scounts) = xfer->stypecounts;
        *((char **)rbuf)      = xfer->rcvbuf;
        *((size_t **)rdisps)  = xfer->rdispls;
        *((size_t **)rcounts) = xfer->rtypecounts;
        return;
      }

      template <>
      inline void setAlltoallVec<pami_alltoallv_int_t> (pami_alltoallv_int_t *xfer,
            void *sbuf, void *scounts, void *sdisps, void *rbuf, void *rcounts, void *rdisps)
      {
        *((char **)sbuf)   = xfer->sndbuf;
        *((int **)sdisps)  = xfer->sdispls;
        *((int **)scounts) = xfer->stypecounts;
        *((char **)rbuf)   = xfer->rcvbuf;
        *((int **)rdisps)  = xfer->rdispls;
        *((int **)rcounts) = xfer->rtypecounts;
        return;
      }

    template<class T_ConnMgr, class T_Type>
    class AlltoallvExec : public Interfaces::Executor
    {
      public:

      protected:
        Interfaces::Schedule           * _comm_schedule;
        Interfaces::NativeInterface    * _native;
        T_ConnMgr                      * _connmgr;

        int                 _comm;
        int                 _buflen;
        char                *_sbuf;
        char                *_rbuf;

        PAMI::PipeWorkQueue _pwq;
        PAMI::PipeWorkQueue _rpwq [MAX_PARALLEL];

        int                 _curphase;
        int                 _nphases;
        int                 _startphase;
        int                 _lphase;
        int                 _rphase [MAX_PARALLEL];

        int                 _maxsrcs;

        unsigned            _parindex;

        int                 _senddone;
        int                 _recvdone [MAX_PARALLEL];
        PAMI::Topology      _partopology;
        PAMI::Topology      _selftopology;
        PAMI::Topology      *_gtopology;

        CollHeaderData      _mldata;
        CollHeaderData      _mrdata;
        pami_multicast_t    _mlsend;
        pami_multicast_t    _mrsend;

        typedef typename AlltoallVecType<T_Type>::base_type basetype;

        basetype            *_sdisps;
        basetype            *_scounts;
        basetype            *_rdisps;
        basetype            *_rcounts;

        //Private method
        void             sendNext ();

      public:
        AlltoallvExec () :
            Interfaces::Executor (),
            _comm_schedule(NULL),
            _comm(-1),
            _sbuf(NULL),
            _rbuf(NULL),
            _curphase(0),
            _nphases(0),
            _startphase(0),
            _sdisps(NULL),
            _scounts(NULL),
            _rdisps(NULL),
            _rcounts(NULL)
        {
          TRACE_ADAPTOR((stderr, "<%p>Executor::AlltoallvExec()\n", this));
        }

        AlltoallvExec (Interfaces::NativeInterface  * mf,
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
            _curphase(0),
            _nphases(0),
            _startphase(0),
            _partopology(),
            _selftopology(mf->myrank()),
            _gtopology(gtopology),
            _sdisps(NULL),
            _scounts(NULL),
            _rdisps(NULL),
            _rcounts(NULL)
        {
          TRACE_ADAPTOR((stderr, "<%p>Executor::AlltoallvExec(...)\n", this));
          _clientdata        =  0;
          _buflen            =  0;

          _senddone          =  0;
          for (int i = 0; i < MAX_PARALLEL; ++i) _recvdone[i] = 0;

          _mldata._comm       = _comm;
          _mldata._root       = -1;
          _mldata._count      = -1; // indicating this is only a sync message
          _mldata._phase      = 0;

          pami_quad_t *info    =  (pami_quad_t*)((void*) & _mldata);
          _mlsend.msginfo       =  info;
          _mlsend.msgcount      =  1;
          _mlsend.roles         = -1U;

          _mrdata._comm       = _comm;
          _mrdata._root       = -1;
          _mrdata._count      = -1;
          _mrdata._phase      = 0;

          info    =  (pami_quad_t*)((void*) & _mrdata);
          _mrsend.msginfo       =  info;
          _mrsend.msgcount      =  1;
          _mrsend.roles         = -1U;

        }

        virtual ~AlltoallvExec ()
        {
        }

        /// NOTE: This is required to make "C" programs link successfully with virtual destructors
        void operator delete(void * p)
        {
        }

        // --  Initialization routines
        //------------------------------------------

        void setSchedule (Interfaces::Schedule *ct)
        {
          TRACE_ADAPTOR((stderr, "<%p>Executor::AlltoallvExec::setSchedule()\n", this));
          _comm_schedule = ct;

          _nphases    = _native->numranks() - 1;
          _startphase = 0;
          _curphase   = 0;
          _lphase     = 0;
          for (int i = 0; i < MAX_PARALLEL; ++i) _rphase[i]     = 0;

          unsigned connection_id = (unsigned) -1;
          if (_connmgr)
            connection_id = _connmgr->getConnectionId(_comm, (unsigned)-1, 0, (unsigned) - 1, (unsigned) - 1);
        }

        void setConnectionID (unsigned cid)
        {

          //Override the connection id from the connection manager
          _mlsend.connection_id = cid;
          _mrsend.connection_id = cid;

        }

        void  setBuffers (char *src, char *dst, int len)
        {
          TRACE_ADAPTOR((stderr, "<%p>Executor::AlltoallvExec::setInfo() src %p, dst %p, len %d, _pwq %p\n", this, src, dst, len, &_pwq));

          _buflen = len;
          _sbuf = src;
          _rbuf = dst;
        }

        void setVectors(T_Type *xfer)
        {
           setAlltoallVec<T_Type> (xfer, _sbuf, _sdisps, _scounts, _rbuf, _rdisps, _rcounts);
        }

        void  updateVectors(T_Type *xfer)
        {
           setAlltoallVec<T_Type> (xfer, _sbuf, _sdisps, _scounts, _rbuf, _rdisps, _rcounts);
        }

        /// \todo: this should be moved to the schedule
        unsigned getPartnerIndex(unsigned uphase)
        {

          int phase   = (int)uphase;
          int tasks   = _native->numranks();
          int myindex = _native->myrank();

          int partner;
          if(tasks & 1)
          {
             partner = (tasks + 2*phase -myindex) % tasks;
             if (partner == myindex)  partner = (unsigned) -1;
           } else {
             tasks--;
             if (myindex == tasks)
               partner = phase;
             else
               partner = (tasks + 2*phase - myindex) % tasks;
             if (partner == myindex)
               partner = tasks;
           }
	  return partner;
        }

        size_t getSendLength(unsigned index)
        {
           return (_scounts) ? _scounts[index] : _buflen;
        }

        size_t getRecvLength(unsigned index)
        {
           return (_rcounts) ? _rcounts[index] : _buflen;
        }

        size_t getSendDisp(unsigned index)
        {
           return (_sdisps) ? _sdisps[index] : index * _buflen;
        }

        size_t getRecvDisp(unsigned index)
        {
           return (_rdisps) ? _rdisps[index] : index * _buflen;
        }

        PAMI::PipeWorkQueue *getSendPWQ(unsigned index)
        {
          size_t sleng = getSendLength(index);
          size_t sdisp = getSendDisp(index);
          _pwq.configure (NULL, _rbuf + sdisp, sleng, 0);
          _pwq.reset();
          _pwq.produceBytes(sleng);
          return &_pwq;
        }

        PAMI::PipeWorkQueue *getRecvPWQ(unsigned index, int phase)
        {
          size_t rleng = getRecvLength(index);
          size_t rdisp = getRecvDisp(index);
          _rpwq[phase - _curphase].configure (NULL, _rbuf + rdisp, rleng, 0);
          _rpwq[phase - _curphase].reset();
          return &_rpwq[phase - _curphase];
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
        /*
        unsigned       getRoot   ()
        {
          return _root;
        }
        */
        unsigned       getComm   ()
        {
          return _comm;
        }

        static void notifySendDone (pami_context_t context, void *cookie, pami_result_t result)
        {
          TRACE_MSG ((stderr, "<%p>Executor::AlltoallvExec::notifySendDone()\n", cookie));
          AlltoallvExec<T_ConnMgr, T_Type> *exec =  (AlltoallvExec<T_ConnMgr, T_Type> *) cookie;
          exec->_senddone = 1;
          if (exec->_recvdone[exec->_curphase % MAX_PARALLEL] == 1) {
            exec->_recvdone[exec->_curphase % MAX_PARALLEL] = 0;
            exec->_senddone = 0;
            exec->_curphase ++;
            exec->sendNext();
          }
        }

        static void notifyRecvDone( pami_context_t   context,
                                    void           * cookie,
                                    pami_result_t    result )
        {
          TRACE_MSG ((stderr, "<%p>Executor::AlltoallvExec::notifyRecvDone()\n", cookie));
          AlltoallvExec<T_ConnMgr, T_Type> *exec =  (AlltoallvExec<T_ConnMgr, T_Type> *) cookie;
          exec->_recvdone[exec->_curphase % MAX_PARALLEL] = 1;
          if (exec->_senddone == 1) {
            exec->_recvdone[exec->_curphase % MAX_PARALLEL] = 0;
            exec->_senddone = 0;
            exec->_curphase ++;
            exec->sendNext();
          }
        }

        static void notifyAvailRecvDone( pami_context_t   context,
                                    void           * cookie,
                                    pami_result_t    result )
        {
          TRACE_MSG ((stderr, "<%p>Executor::AlltoallvExec::notifyRecvDone()\n", cookie));
          AlltoallvExec<T_ConnMgr, T_Type> *exec =  (AlltoallvExec<T_ConnMgr, T_Type> *) cookie;
          exec->sendNext();
        }


    };  //-- AlltoallvExec
  };   //-- Executor
};  //-- CCMI

///
/// \brief start sending alltoallv data. Only active on the root node
///
template <class T_ConnMgr, class T_Type>
inline void  CCMI::Executor::AlltoallvExec<T_ConnMgr,T_Type>::start ()
{
  TRACE_ADAPTOR((stderr, "<%p>Executor::AlltoallvExec::start() count%d\n", this, _buflen));

  _curphase  = _startphase;
  _lphase    = _curphase + 1;

  // what is myrank ? in world geometry or in this geometry ?
  unsigned myindex = _native->myrank();
  memcpy(_rbuf + getRecvDisp(myindex), _sbuf + getSendDisp(myindex), getRecvLength(myindex));

  sendNext ();
}

template <class T_ConnMgr, class T_Type>
inline void  CCMI::Executor::AlltoallvExec<T_ConnMgr, T_Type>::sendNext ()
{
  if (_curphase == _startphase + _nphases) {
    if (_cb_done) _cb_done (NULL, _clientdata, PAMI_SUCCESS);
    return;
  }

  // setup destination topology
  _parindex         = getPartnerIndex(_curphase);
  if (_parindex == (unsigned)-1) { // skip this phase
    _lphase   ++;
    CCMI_assert(_rphase[_curphase % MAX_PARALLEL] == 0);
    _curphase ++;
    if (_curphase == _startphase + _nphases) {
      if (_cb_done) _cb_done (NULL, _clientdata, PAMI_SUCCESS);
      return;
    }
    _parindex         = getPartnerIndex(_curphase);
  }
  new (&_partopology) PAMI::Topology(_gtopology->index2Rank(_parindex));

  // send buffer available msg to left neighbor
  if (_lphase == _curphase+1) {
    _lphase ++;
    _mldata._phase             = _curphase+1;
    _mlsend.src_participants   = (pami_topology_t *) & _selftopology;
    _mlsend.dst_participants   = (pami_topology_t *) & _partopology;
    _mlsend.cb_done.function   = NULL;
    _mlsend.cb_done.clientdata = 0;
    _mlsend.src                = NULL;
    _mlsend.dst                = NULL;
    _mlsend.bytes              = 0;
    _native->multicast(&_mlsend);
  }

  if (_rphase[_curphase % MAX_PARALLEL] == _curphase+1) { // buffer available at the right neighbor
    _rphase[_curphase % MAX_PARALLEL] = 0;
    _mrdata._phase             = _curphase;
    _mrdata._count             = 0; // indicating this is data message
    _mrsend.src_participants   = (pami_topology_t *) & _selftopology;
    _mrsend.dst_participants   = (pami_topology_t *) & _partopology;
    _mrsend.cb_done.function   = notifySendDone;
    _mrsend.cb_done.clientdata = this;
    _mrsend.src                = (pami_pipeworkqueue_t *)getSendPWQ(_parindex);
    _mrsend.dst                = NULL;
    _mrsend.bytes              = getSendLength(_parindex);
    _native->multicast(&_mrsend);
  }

  return;
}

template <class T_ConnMgr, class T_Type>
inline void  CCMI::Executor::AlltoallvExec<T_ConnMgr, T_Type>::notifyRecv
(unsigned             src,
 const pami_quad_t   & info,
 PAMI::PipeWorkQueue ** pwq,
 pami_callback_t      * cb_done)
{

  CollHeaderData *cdata = (CollHeaderData*) &info;

  if ((int)cdata->_count == -1) {
    CCMI_assert(cdata->_phase - _curphase < MAX_PARALLEL);
    unsigned pindex;
    pindex = getPartnerIndex(cdata->_phase);
    CCMI_assert(pindex != (unsigned) -1);
    CCMI_assert(src == _gtopology->index2Rank(pindex));
    _rphase[cdata->_phase % MAX_PARALLEL] = cdata->_phase;
    *pwq = NULL;
    cb_done->function   = notifyAvailRecvDone;
    cb_done->clientdata = this;
  } else {
    CCMI_assert(cdata->_count == 0);
    CCMI_assert(src == _gtopology->index2Rank(_parindex));
    CCMI_assert(cdata->_phase == _curphase);
    *pwq = getRecvPWQ(_parindex, _curphase);
    cb_done->function   = notifyRecvDone;
    cb_done->clientdata = this;
  }

  return;
}

#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
