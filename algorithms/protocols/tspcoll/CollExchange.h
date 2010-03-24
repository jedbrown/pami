/* ************************************************************************* */
/*                            IBM Confidential                               */
/*                          OCO Source Materials                             */
/*                      IBM XL UPC Alpha Edition, V0.9                       */
/*                                                                           */
/*                      Copyright IBM Corp. 2009, 2010.                      */
/*                                                                           */
/* The source code for this program is not published or otherwise divested   */
/* of its trade secrets, irrespective of what has been deposited with the    */
/* U.S. Copyright Office.                                                    */
/* ************************************************************************* */
/**
 * \file algorithms/protocols/tspcoll/CollExchange.h
 * \brief ???
 */

#ifndef __algorithms_protocols_tspcoll_CollExchange_h__
#define __algorithms_protocols_tspcoll_CollExchange_h__

#include "util/ccmi_debug.h"
#include "algorithms/ccmi.h" //for request type
#include "algorithms/protocols/tspcoll/NBColl.h"

//#define DEBUG_TSPCOLL 1
#undef TRACE
#ifdef DEBUG_TSPCOLL
#define TRACE(x)  fprintf x;
#else
#define TRACE(x)
#endif

/* *********************************************************************** */
/*  Base class for implementing a non-blocking collective using amsends.   */
/* *********************************************************************** */
#define MAX_PHASES 64

namespace TSPColl
{

  template<class T_Mcast>
  class CollExchange: public NBColl<T_Mcast>
  {
  protected:
    //    static const int MAX_PHASES=64;
    typedef void (* cb_Coll_t) (CollExchange *, unsigned);

  public:
    /* ------------------------------ */
    /*  public API                    */
    /* ------------------------------ */
    virtual void  kick             (T_Mcast *mcast_iface);
    virtual bool  isdone           () const;
    static void   amsend_reg       (T_Mcast *mcast_iface, void *cd);
  protected:

    CollExchange                   (PAMI_GEOMETRY_CLASS *, NBTag,
                                    int id, int off, bool strict=true,
                                    pami_event_function cb_complete=NULL,
                                    void * arg = NULL);
    void          reset            (void);


  private:

    /* ------------------------------ */
    /*  local functions               */
    /* ------------------------------ */

    void          send                     (int phase,T_Mcast*mcast_iface);
    //static inline CCMI::MultiSend::DCMF_OldRecvMulticast cb_incoming;
    static inline pami_quad_t *cb_incoming(const pami_quad_t  * hdr,
                                          unsigned          count,
                                          unsigned          peer,
                                          unsigned          sndlen,
                                          unsigned          conn_id,
                                          void            * arg,
                                          unsigned        * rcvlen,
                                          char           ** rcvbuf,
                                          unsigned        * pipewidth,
                                          PAMI_Callback_t * cb_done);

    static void   cb_recvcomplete (pami_context_t context, void * arg, pami_result_t error);
    static void   cb_senddone     (pami_context_t, void*, pami_result_t);
  protected:
    /* ------------------------------ */
    /* static: set by constructor     */
    /* ------------------------------ */
    PAMI_Request_t                       _req[MAX_PHASES];
    PAMI_Request_t                       _rreq[MAX_PHASES];

    T_Mcast                            *_mcast_iface;

    int          _numphases;

    /* ------------------------------ */
    /* set by start()                 */
    /* ------------------------------ */

    int          _dest     [MAX_PHASES];    /* list of destination nodes     */
    void       * _sbuf     [MAX_PHASES];    /* list of source addresses      */
    void       * _rbuf     [MAX_PHASES];    /* list of destination addresses */
    size_t       _sbufln   [MAX_PHASES];    /* list of buffer lenghts        */
    cb_Coll_t    _cb_recv1 [MAX_PHASES];    /* immediate callback */
    cb_Coll_t    _cb_recv2 [MAX_PHASES];    /* callback to process buffer */

    /* --------------------------------- */
    /* STATE: changes during execution   */
    /* --------------------------------- */

  protected:
    int          _phase;                    /* phase in current execution    */
    int          _counter;                  /* how many times been reset     */
    int          _sendstarted;
    int          _sendcomplete;             /* #sends complete               */
    int          _recvcomplete[MAX_PHASES]; /* #recv complete in each phase  */
    int          _cbcomplete  [MAX_PHASES]; /* #callbacks complete           */
    bool         _strict;                   /* early incoming msgs not perm. */

  private:

    /* ------------------------------ */
    /*      active message helpers    */
    /* ------------------------------ */

    struct AMHeader
    {
      NBTag               tag;
      int                 id;
      int                 offset;
      int                 counter;
      int                 phase;
    }
    _header [MAX_PHASES] __attribute__((__aligned__(16)));

    /* --------------------------------- */
    /* send & receive completion helper  */
    /* --------------------------------- */

    struct CompleteHelper
    {
      int                phase;
      int                counter;
      CollExchange     * base;
    }
    _cmplt [MAX_PHASES];

    DECL_MUTEX(_mutex);

    void internalerror (AMHeader *, int);
  };
};

/* *********************************************************************** */
/*                  register collexchange                                  */
/* *********************************************************************** */
template <class T_Mcast>
inline void TSPColl::CollExchange<T_Mcast>::amsend_reg  (T_Mcast *mcast_iface, void* cd)
{

  mcast_iface->setCallback(cb_incoming, cd);
  // __pgasrt_tsp_amsend_reg (PGASRT_TSP_AMSEND_COLLEXCHANGE, cb_incoming);
}



/* *********************************************************************** */
/*                  CollExchange constructor                               */
/* *********************************************************************** */
template <class T_Mcast>
inline TSPColl::CollExchange<T_Mcast>::
CollExchange (PAMI_GEOMETRY_CLASS * comm,
                       NBTag tag, int id, int offset,
                       bool strict, pami_event_function cb_complete, void *arg):
NBColl<T_Mcast> (comm, tag, id, cb_complete, arg), _strict(strict)
{
  _counter         = 0;
  _numphases       = -100 * tag;
  _phase           = _numphases+1;
  for (int i=0; i<MAX_PHASES; i++)
    {
      _sbuf[i]                 = NULL;
      _rbuf[i]                 = NULL;
      _sbufln[i]               = 0;
      _cb_recv1[i]             = NULL;
      _cb_recv2[i]             = NULL;
      _cmplt[i].phase          = i;
      _cmplt[i].base           = this;
      _cbcomplete[i]           = 0;
      _recvcomplete[i]         = 0;
      _header[i].tag           = tag;
      _header[i].id            = id;
      _header[i].offset        = offset;
      _header[i].phase         = i;
      _header[i].counter       = 0;
    }

  _sendstarted = _sendcomplete = 0;
  MUTEX_INIT(&_mutex);
}

/* *********************************************************************** */
/*    reinitialize the state machine for another collective execution      */
/* *********************************************************************** */
template <class T_Mcast>
inline void TSPColl::CollExchange<T_Mcast>::reset()
{
  _sendstarted = _sendcomplete = 0;
  _counter++;
  _phase = 0;
}

/* *********************************************************************** */
/*                   kick the state machine (make progress)                */
/* *********************************************************************** */
template <class T_Mcast>
inline void TSPColl::CollExchange<T_Mcast>::kick(T_Mcast *mcast_iface)
{
  /* continued ATOMIC (code should be entered with mutex already locked */
  _mcast_iface = mcast_iface;
  for (; _phase < _numphases; _phase++)
    {
      /* ---------------------------------------------------- */
      /* deal with sending what we have to send in this phase */
      /* ---------------------------------------------------- */

      if (_sendstarted <= _phase)
        {
          _sendstarted++;
          if (_sbuf[_phase])
            {
              int phase = _phase;
              MUTEX_UNLOCK(&_mutex);
              send(phase,mcast_iface);
              return;
            }
          else
            _sendcomplete++;
        }

      /* ------------------------------------------------------------ */
      /* reception and callback : all complete? advance to next phase */
      /* ------------------------------------------------------------ */

      if (_cbcomplete[_phase] >= _counter)
        {
          continue;
        }

      /* ------------------------------------------------------- */
      /* special case: nothing to receive in this phase; advance */
      /* ------------------------------------------------------- */

      if (_rbuf[_phase] == NULL)
        {
          assert (_cb_recv2[_phase] == NULL);
          _recvcomplete[_phase]++;          /* no receive, no callback */
          _cbcomplete[_phase]++;
          assert (_recvcomplete[_phase] <= _counter);
          assert (_cbcomplete[_phase] <= _counter);
          continue;
        }

      /* ---------------------------------------------------- */
      /* reception not complete - NOT advancing to next phase */
      /* ---------------------------------------------------- */

      if (_recvcomplete[_phase] < _counter) goto the_end;

      /* -------------------------------------------------------- */
      /* reception complete - no callback - advance to next phase */
      /* -------------------------------------------------------- */

      if (_cb_recv2[_phase] == NULL) /* no cb */
        {
          TRACE((stderr, "NOCB tag=%d ctr=%d phase=%d\n",
                 _tag, _counter, _phase));
          _cbcomplete[_phase]++;
          continue;
        }

      /* -------------------------------------------------------- */
      /* cannot run callback until send in this phase is complete */
      /* -------------------------------------------------------- */

      if (_sendcomplete <= _phase) { TRACE((stderr, "*\n")); goto the_end; }

      /* ------------ */
      /* run callback */
      /* ------------ */

      TRACE((stderr, "CBCK tag=%d ctr=%d phase=%d\n",
             _tag, _counter, _phase));
      _cb_recv2[_phase] (this, _phase);
      _cbcomplete[_phase]++;
    }

  TRACE((stderr, "FINI tag=%d ctr=%d phase=%d/%d sendcmplt=%d cb_complete=%p\n",
         _tag, _counter,
         _phase, _numphases,
         _sendcomplete,
         this->_cb_complete));

  if (this->_cb_complete)
      if (_phase == _numphases)
          {
              _phase++;
              TRACE((stderr, "Delivering user done callback fcn=%p arg=%p\n",
                     this->_cb_complete, this->_arg));
              this->_cb_complete (NULL, this->_arg, PAMI_SUCCESS);
          }

 the_end:
  ;
  /* END ATOMIC */
  MUTEX_UNLOCK(&_mutex);
}

/* *********************************************************************** */
/*    advance the progress engine                                          */
/* *********************************************************************** */
template <class T_Mcast>
inline bool TSPColl::CollExchange<T_Mcast>::isdone() const
{
    TRACE((stderr, "Is done:  _phase=%d sendcomplete=%d, numphase=%d\n",
           _phase, _sendcomplete, _numphases));
  return (_phase >= _numphases && _sendcomplete >= _numphases);
}

/* *********************************************************************** */
/*                     send an active message                              */
/* *********************************************************************** */
template <class T_Mcast>
inline void TSPColl::CollExchange<T_Mcast>::send (int phase, T_Mcast *mcast_iface)
{
  TRACE((stderr, "SEND tag=%d ctr=%d phase=%d tgt=%d nbytes=%d, mcast_iface=%p\n",
         _tag, _counter, phase,
         _dest[phase], _sbufln[phase],mcast_iface));
  _header[phase].counter = _counter;
  assert (_dest[phase] != -1);
 #if 0
  void * r = __pgasrt_tsp_amsend (_dest[phase],
                                  & _header[phase].hdr,
                                  (__pgasrt_local_addr_t) _sbuf[phase],
                                  _sbufln[phase],
                                  CollExchange::cb_senddone,
                                  &_cmplt[phase]);
#endif
  unsigned        hints   = PAMI_PT_TO_PT_SUBTASK;
  unsigned        ranks   = _dest[phase];
  PAMI_Callback_t cb_done;
  cb_done.function   = CollExchange::cb_senddone;
  cb_done.clientdata = &_cmplt[phase];
  void *r = NULL;
  TRACE((stderr, "SEND MCAST_IFACE %p: tag=%d id=%d,hdr=%p count=%d\n",
         ((int*)mcast_iface)[0],
         _header[phase].tag,
         _header[phase].id,
         &_header[phase],
         PAMIQuad_sizeof(_header[phase])));

  mcast_iface->send (&_req[phase],
                     &cb_done,
                     PAMI_MATCH_CONSISTENCY,
                     (pami_quad_t*)& _header[phase],
                     PAMIQuad_sizeof(_header[phase]),
                     phase,
                     (char*)_sbuf[phase],
                     (unsigned)_sbufln[phase],
                     &hints,
                     &ranks,
                     1);
  TRACE((stderr, "SEND finished\n"));

  assert (r == NULL);
}

/* *********************************************************************** */
/*                             send complete                               */
/* *********************************************************************** */
template <class T_Mcast>
inline void TSPColl::CollExchange<T_Mcast>::cb_senddone (pami_context_t context, void * arg, pami_result_t err)
{
  CollExchange * base  = ((CompleteHelper *) arg)->base;
  MUTEX_LOCK(&base->_mutex);
  /* BEGIN ATOMIC */
  base->_sendcomplete++;
  TRACE((stderr,
         "SENT tag=%d ctr=%d phase=%d/%d tgt=%d nbyt=%d cplt=%d\n",
         base->_tag, base->_counter,
         base->_phase, base->_numphases,
         base->_dest[base->_phase], base->_sbufln[base->_phase],
         base->_sendcomplete));
  base->kick(base->_mcast_iface);
}


/* *********************************************************************** */
/*                  active message reception complete                      */
/* *********************************************************************** */
template <class T_Mcast>
inline void
TSPColl::CollExchange<T_Mcast>::cb_recvcomplete (pami_context_t context, void * arg, pami_result_t error)
{
  CollExchange * base  = ((CompleteHelper *) arg)->base;
  unsigned  phase = ((CompleteHelper *) arg)->phase;
  // int  counter = ((CompleteHelper *)arg)->counter;
  if (base->_strict)
    if (base->_recvcomplete[phase] > base->_counter)
      base->internalerror (NULL, __LINE__);
  /* BEGIN ATOMIC */
  MUTEX_LOCK(&base->_mutex);
  base->_recvcomplete[phase]++;
  TRACE((stderr, "IN_D tag=%d ctr=%d phase=%d msgphase=%d cplt=%d\n",
         base->_tag,
         base->_counter, base->_phase, phase, base->_recvcomplete[phase]));
  if (base->_cb_recv1[phase]) base->_cb_recv1[phase](base, phase);
  base->kick(base->_mcast_iface);
}

/* *********************************************************************** */
/*      something bad happened. We print the state as best as we can.      */
/* *********************************************************************** */
template <class T_Mcast>
inline void
TSPColl::CollExchange<T_Mcast>::internalerror (AMHeader * header, int lineno)
{
  if (header)
    fprintf (stderr, "CollExchange internal: line=%d "
             "tag=%d id=%d phase=%d/%d ctr=%d "
             "header: tag=%d id=%d phase=%d ctr=%d\n",
             lineno,
             NBColl<T_Mcast>::_tag, NBColl<T_Mcast>::_instID,
             _phase, _numphases, _counter,
             header->tag, header->id, header->phase,
             header->counter);
  else
    fprintf (stderr, "CollExchange internal: line=%d "
             "tag=%d id=%d phase=%d/%d ctr=%d\n",
             lineno,
             NBColl<T_Mcast>::_tag, NBColl<T_Mcast>::_instID,
             _phase, _numphases, _counter);
  abort();
}






#undef TRACE
#endif
