/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/tspcoll/NBCallbacks.h
 * \brief ???
 */

#ifndef __algorithms_protocols_tspcoll_NBCallbacks_h__
#define __algorithms_protocols_tspcoll_NBCallbacks_h__

#include "components/devices/generic/Device.h"


// This is the "Unexpected Collective" class
// This class uses an external device (in this case the generic device)
// to retry unexpected collective messages
// This may be less optimal than checking the queue
// at the collective posting time, but this is simpler
// and should be faster than the common case
// We should expect this kind of barrier to be used only
// for geometry creates.

template<class T_NI>
class UnexpectedColl
{
    static pami_result_t wf(pami_context_t context, void *cookie)
    {
      UnexpectedColl *c = (UnexpectedColl *)cookie;
      c->deliverCallback();
      return PAMI_SUCCESS;
    }
  public:
    UnexpectedColl():
        _work_fn(wf),
        _cookie(this),
        _work(PAMI::Device::Generic::GenericThread(_work_fn, _cookie))
    {
    }

    void initialize(TSPColl::NBCollManager<T_NI> *mc,
                    pami_dispatch_p2p_function    fn,
                    pami_context_t                context,
                    void                         *cookie,
                    const void                   *header_addr,
                    size_t                        header_size,
                    const void                   *pipe_addr,
                    size_t                        data_size,
                    pami_endpoint_t               origin,
                    pami_recv_t                  *recv)
    {
      if (data_size)
        CCMI_FATALERROR (-1, "Collective Fatal: Only short(0-byte data) UE messages allowed");

      _mc             = mc;
      _ue_dispatch_fn = fn;

      _ue_context     = context;
      _ue_cookie      = cookie;
      pami_result_t prc;
      prc = __global.heap_mm->memalign((void **)&_ue_header_addr, 0, header_size);
      PAMI_assertf(prc == PAMI_SUCCESS, "alloc of _ue_header_addr failed");
      _ue_header_size = header_size;
      memcpy(_ue_header_addr, header_addr, header_size);

      _ue_pipe_addr   = NULL;
      _ue_data_size   = 0;

      _ue_origin      = origin;
      _ue_recv        = NULL;

      _work.setStatus(PAMI::Device::OneShot);
      _mc->postWork(&_work);

    }

    void repost()
    {
      _work.setStatus(PAMI::Device::OneShot);
      _mc->postWork(&_work);
    }

    void deliverCallback()
    {
      _ue_dispatch_fn(_ue_context,
                      _ue_cookie,
                      _ue_header_addr,
                      _ue_header_size,
                      _ue_pipe_addr,
                      _ue_data_size,
                      _ue_origin,
                      _ue_recv);
    }
    void finalize()
    {
      __global.heap_mm->free(_ue_header_addr);
    }

    // Work function parameters
    pami_work_function                    _work_fn;
    void                                 *_cookie;
    PAMI::Device::Generic::GenericThread  _work;

    // cb_incoming parameters

    TSPColl::NBCollManager<T_NI>         *_mc;
    pami_dispatch_p2p_function            _ue_dispatch_fn;
    pami_context_t                        _ue_context;
    void                                 *_ue_cookie;
    void                                 *_ue_header_addr;
    size_t                                _ue_header_size;
    void                                 *_ue_pipe_addr;
    size_t                                _ue_data_size;
    pami_endpoint_t                       _ue_origin;
    pami_recv_t                          *_ue_recv;
};



/* *********************************************************************** */
/*                   incoming active message                               */
/* *********************************************************************** */
template<class T_NI>
inline void TSPColl::CollExchange<T_NI>::cb_incoming_ue(pami_context_t    context,
                                                        void            * cookie,
                                                        const void      * header_addr,
                                                        size_t            header_size,
                                                        const void      * pipe_addr,
                                                        size_t            data_size,
                                                        pami_endpoint_t   origin,
                                                        pami_recv_t     * recv)
{
  struct AMHeader * header = (struct AMHeader *) header_addr;
  NBCollManager<T_NI> *mc = (NBCollManager<T_NI>*) cookie;
  void * base0 = mc->find (header->tag, header->id);
  void * base1 = mc->find_ue(header->tag, header->id);

  if (base0 == NULL)
    {
      // Unexpected collective case
      if (base1 == NULL)
        {
          TRACE((stderr, "Incoming, ALLOCATE(tag=%d, id=%d):  ctxt:%p cookie:%p header:%p h_sz=%zd "
                 "p_addr=%p d_size=%zd orig=%zd recv=%p\n",
                 header->tag, header->id,
                 context, cookie, header_addr, header_size,
                 pipe_addr, data_size,(size_t)origin, recv));
          UnexpectedColl<T_NI> *uecoll;
	  pami_result_t prc;
	  prc = __global.heap_mm->memalign((void **)&uecoll, 0, sizeof(*uecoll));
	  PAMI_assertf(prc == PAMI_SUCCESS, "alloc of UnexpectedColl<T_NI> failed");
          new(uecoll) UnexpectedColl<T_NI>();
          uecoll->initialize(mc,
                             cb_incoming_ue,
                             context,
                             cookie,
                             header_addr,
                             header_size,
                             pipe_addr,
                             data_size,
                             origin,
                             recv);
          mc->allocate_ue(header->tag, header->id, (void*)uecoll);
        }
      else
        {
          TRACE((stderr, "Incoming, REPOST(tag=%d, id=%d):  ctxt:%p cookie:%p header:%p h_sz=%zd "
                 "p_addr=%p d_size=%zd orig=%zd recv=%p\n",
                 header->tag, header->id,
                 context, cookie, header_addr, header_size,
                 pipe_addr, data_size, (size_t)origin, recv));
          UnexpectedColl<T_NI> *uecoll = (UnexpectedColl<T_NI> *)base1;
          uecoll->repost();
          return;
        }

      return;
      CCMI_FATALERROR (-1, "incoming: cannot find coll=<%d,%d>",
                       header->tag, header->id);
    }

  if (base1)
    mc->delete_ue(header->tag, header->id);


  CollExchange<T_NI> * b = (CollExchange<T_NI>* ) ((char *)base0 + header->offset);
  TRACE((stderr, "INC  tag=%d id=%d ctr=%d phase=%d nphases=%d "
         "msgctr=%d msgphase=%d\n",
         header->tag, header->id, b->_counter,
         b->_phase, b->_numphases,
         header->counter, header->phase));

  PAMI_assert(b->_header[0].id == header->id);
  PAMI_assert(b->_numphases > 0);

  if (b->_strict)
    {
      if (header->counter != b->_counter || b->_phase >= b->_numphases)
        b->internalerror (header, __LINE__);
    }

  b->_cmplt[header->phase].counter = header->counter;

  if (pipe_addr)
    memcpy(b->_rbuf[header->phase], pipe_addr, data_size);
  else if (recv)
    {
      recv->cookie        = &b->_cmplt[header->phase];
      recv->local_fn      = CollExchange<T_NI>::cb_recvcomplete;
      recv->addr          = (char*)b->_rbuf[header->phase];
      recv->type          = PAMI_BYTE;
      recv->offset        = 0;
      recv->data_fn       = PAMI_DATA_COPY;
      recv->data_cookie   = (void*)NULL;
      return;
    }

  CollExchange<T_NI>::cb_recvcomplete(context, &b->_cmplt[header->phase], PAMI_SUCCESS);
}

/* *********************************************************************** */
/*                   incoming active message                               */
/* *********************************************************************** */
template<class T_NI>
inline void TSPColl::CollExchange<T_NI>::cb_incoming(pami_context_t    context,
                                                     void            * cookie,
                                                     const void      * header_addr,
                                                     size_t            header_size,
                                                     const void      * pipe_addr,
                                                     size_t            data_size,
                                                     pami_endpoint_t   origin,
                                                     pami_recv_t     * recv)
{
  struct AMHeader * header = (struct AMHeader *) header_addr;
  NBCollManager<T_NI> *mc = (NBCollManager<T_NI>*) cookie;
  void * base0 =  mc->find (header->tag, header->id);

  if (base0 == NULL)
    CCMI_FATALERROR (-1, "incoming: cannot find coll=<%d,%d>",
                     header->tag, header->id);

  CollExchange * b = (CollExchange * ) ((char *)base0 + header->offset);
  TRACE((stderr, "INC  tag=%d id=%d ctr=%d phase=%d nphases=%d "
         "msgctr=%d msgphase=%d\n",
         header->tag, header->id, b->_counter,
         b->_phase, b->_numphases,
         header->counter, header->phase));

  PAMI_assert(b->_header[0].id == header->id);
  PAMI_assert(b->_numphases > 0);

  if (b->_strict)
    {
      if (header->counter != b->_counter || b->_phase >= b->_numphases)
        b->internalerror (header, __LINE__);
    }

  b->_cmplt[header->phase].counter = header->counter;

  if (pipe_addr)
    memcpy(b->_rbuf[header->phase], pipe_addr, data_size);
  else if (recv)
    {
      recv->cookie        = &b->_cmplt[header->phase];
      recv->local_fn      = CollExchange::cb_recvcomplete;
      recv->addr          = (char*)b->_rbuf[header->phase];
      recv->type          = PAMI_BYTE;
      recv->offset        = 0;
      recv->data_fn       = PAMI_DATA_COPY;
      recv->data_cookie   = (void*)NULL;
      return;
    }

  CollExchange::cb_recvcomplete(context, &b->_cmplt[header->phase], PAMI_SUCCESS);
}



/* **************************************************************** */
/*            incoming active message                               */
/* **************************************************************** */
template<class T_NI>
void TSPColl::Scatter<T_NI>::cb_incoming(pami_context_t    context,
                                         void            * cookie,
                                         const void      * header_addr,
                                         size_t            header_size,
                                         const void      * pipe_addr,
                                         size_t            data_size,
                                         pami_endpoint_t   origin,
                                         pami_recv_t     * recv)
{
  struct scatter_header * header = (struct scatter_header *) header_addr;
  NBCollManager<T_NI> *mc = (NBCollManager<T_NI>*) cookie;
  void * base0 =  mc->find (header->tag, header->id);

  if (base0 == NULL)
    CCMI_FATALERROR (-1, "Scatter/v: <%d,%d> is undefined",
                     header->tag, header->id);

  Scatter * s = (Scatter * ) ((char *)base0 + header->tagoff);
  TRACE((stderr, "SCATTER/v: <%d,%d> INCOMING base=%p ptr=%p\n",
         header->tag, header->id, base0, s));

  if (pipe_addr)
    memcpy(s->_rbuf, pipe_addr, data_size);
  else if (recv)
    {
      recv->cookie        = s;
      recv->local_fn      = Scatter::cb_recvcomplete;
      recv->addr          = s->_rbuf;
      recv->type          = PAMI_BYTE;
      recv->offset        = 0;
      recv->data_fn       = PAMI_DATA_COPY;
      recv->data_cookie   = (void*)NULL;
      TRACE((stderr, "SCATTER/v: <%d,%d> INCOMING RETURING base=%p ptr=%p\n",
             header->tag, header->id, base0, s));
      return;
    }

  Scatter::cb_recvcomplete(context, s, PAMI_SUCCESS);
}




#endif
