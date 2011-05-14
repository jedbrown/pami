#include "algorithms/protocols/tspcoll/Permute.h"
#include "algorithms/protocols/tspcoll/Team.h"


/* **************************************************************** */
/*      start a new permute.                                        */
/* **************************************************************** */


/* TO be fixed when copy in place; race condition */
template <class T_NI>
void xlpgas::Permute<T_NI>::reset (int dest, const void * s, void * d, unsigned l)
{
  _dest           = dest;
  _rcvcount       = 0;
  _rbuf           = (char *)d;
  _sbuf           = (const char *)s;
  _len            = l;
}

/* **************************************************************** */
/*                   do all the sends in an alltoall                */
/* **************************************************************** */
template <class T_NI>
void xlpgas::Permute<T_NI>::kick    () {
  if (this->_dest == this->_comm->ordinal())
    {
      memcpy (this->_rbuf ,
	      this->_sbuf ,
	      this->_len);

      this->_rcvcount++;
      /* UNLOCK */

      if (this->_cb_complete)
	this->_cb_complete (this->_pami_ctxt,this->_arg, PAMI_SUCCESS);
    }
  else {
    xlpgas_endpoint_t dst = this->_comm->endpoint (this->_dest);
//    ((AMHeader&)(_header->hdr)).dest_ctxt = dst.ctxt;
    pami_send_t p_send;
    p_send.send.header.iov_base  = this->_header;
    p_send.send.header.iov_len   = sizeof(*this->_header);
    p_send.send.data.iov_base    = (char*)this->_sbuf;
    p_send.send.data.iov_len     = this->_len;
    p_send.send.dispatch         = -1;
    memset(&p_send.send.hints, 0, sizeof(p_send.send.hints));
    p_send.send.dest             = dst;
    p_send.events.cookie         = this;
    p_send.events.local_fn       = this->cb_senddone;
    p_send.events.remote_fn      = NULL;
    this->_p2p_iface->send(&p_send);
    }
}

/* **************************************************************** */
/*                     check whether we are done                    */
/* **************************************************************** */
template <class T_NI>
bool xlpgas::Permute<T_NI>::isdone () const
{
  return (_rcvcount >= 1);
}

/* **************************************************************** */
/*               send completion in permute                         */
/* **************************************************************** */
template <class T_NI>
void xlpgas::Permute<T_NI>::cb_senddone (void * ctxt, void * arg, pami_result_t res)
{
  Permute * self = (Permute*)arg;
  /* LOCK */
  TRACE((stderr, "%d: Permute: <%d,%d> SENDDONE sndcount=%d\n",
         XLPGAS_MYNODE, hdr->tag, hdr->kind,
         self->_sndcount[self->_odd]));

  /* UNLOCK */
  if (self->_cb_complete)
    self->_cb_complete (self->_pami_ctxt, self->_arg, res);
}

/* **************************************************************** */
/*               reception header handler                           */
/* **************************************************************** */

template<class T_NI>
inline void xlpgas::Permute<T_NI>::cb_incoming(pami_context_t    context,
                                               void            * cookie,
                                               const void      * hdr,
                                               size_t            header_size,
                                               const void      * pipe_addr,
                                               size_t            data_size,
                                               pami_endpoint_t   origin,
                                               pami_recv_t     * recv)
{
  struct AMHeader * header = (struct AMHeader *) hdr;

  int ctxt = header->dest_ctxt;
  CollectiveManager<T_NI> *mc = (CollectiveManager<T_NI>*) cookie;
  void * base0 =  mc->find (header->kind, header->tag);

  if (base0 == NULL)
    xlpgas_fatalerror (-1, "%d: Permute/v: <%d,%d> is undefined",
                         XLPGAS_MYNODE, header->tag, header->kind);
  Permute * s = (Permute * ) ((char *)base0 + header->offset);
  TRACE((stderr, "%d: Permute: <%d,%d> INCOMING base=%p ptr=%p len=%d\n",
         XLPGAS_MYNODE, header->tag, header->kind, base0, s, s->_len));

  char * rbuf = s->_rbuf;
  if (pipe_addr)
    memcpy(rbuf, pipe_addr, data_size);
  else if (recv)
  {
    recv->cookie        = s;
    recv->local_fn      = Permute<T_NI>::cb_recvcomplete;
    recv->addr          = rbuf;
    recv->type          = PAMI_TYPE_BYTE;
    recv->offset        = 0;
    recv->data_fn       = PAMI_DATA_COPY;
    recv->data_cookie   = (void*)NULL;
    TRACE((stderr, "PERMUTE: <%d,%d> INCOMING RETURING base=%p ptr=%p\n",
           header->tag, header->id, base0, s));
    return;
  }
  Permute::cb_recvcomplete(context, s, PAMI_SUCCESS);
  return;
}

/* **************************************************************** */
/*           active message reception complete                      */
/* **************************************************************** */
template <class T_NI>
void xlpgas::Permute<T_NI>::cb_recvcomplete (void * unused, void * arg, pami_result_t res)
{
  Permute * self = (Permute *) arg;
  self->_rcvcount++;

  TRACE((stderr, "%d: Permute: <%d,%d> RECVDONE\n",
         XLPGAS_MYNODE, self->_header.tag, self->_header.id));

  if (self->_rcvcount >= 1)
    if (self->_cb_complete)
      self->_cb_complete (self->_pami_ctxt, self->_arg, res);
}