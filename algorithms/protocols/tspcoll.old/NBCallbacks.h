/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/tspcoll.old/NBCallbacks.h
 * \brief ???
 */

#ifndef __algorithms_protocols_tspcoll_old_NBCallbacks_h__
#define __algorithms_protocols_tspcoll_old_NBCallbacks_h__

#include "components/devices/generic/Device.h"

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
      recv->type          = PAMI_TYPE_BYTE;
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
      recv->type          = PAMI_TYPE_BYTE;
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
