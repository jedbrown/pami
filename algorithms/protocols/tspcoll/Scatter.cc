/**
 * \file algorithms/protocols/tspcoll/Scatter.cc
 * \brief ???
 */
#include "algorithms/protocols/tspcoll/Scatter.h"
#include "algorithms/protocols/tspcoll/Team.h"


/* **************************************************************** */
/*      Start a new scatter. Old scatters needs to complete       */
/* **************************************************************** */
template<class T_NI>
void xlpgas::Scatter<T_NI>::reset (int root, const void * s, void * d,
		    TypeCode           * stype,
		    size_t               stypecount,
		    TypeCode           * rtype,
		    size_t               rtypecount)
{
  _root           = root;
  _rcvcount       = 0;
  _rbuf           = (char *)d;
  _sbuf           = (const char *)s;
  _len            = stype->GetDataSize() * stypecount;
  _pwq.configure((char *)this->_sbuf, this->_len, this->_len, stype, rtype);
  _pwq.reset();
}

/* **************************************************************** */
/*                   do all the sends in an alltoall                */
/* **************************************************************** */
template<class T_NI>
void xlpgas::Scatter<T_NI>::kick    (){
  if (_root == this->ordinal())
    {
      memcpy (_rbuf,
	      _sbuf + _root*_len,
	      _len);

      _rcvcount++;
      /* UNLOCK */

      for(int i=0;i<(int)this->_comm->size();++i){
	if(i != (int)_root){
	  xlpgas_endpoint_t dst = this->_comm->endpoint (i);
//	  ((AMHeader&)(_header->hdr)).dest_ctxt = dst.ctxt;
          pami_send_t p_send;
          pami_send_event_t   events;
          p_send.send.header.iov_base  = this->_header;
          p_send.send.header.iov_len   = sizeof(*this->_header);
          p_send.send.data.iov_base    = (char*)(this->_sbuf + i * this->_len);
          p_send.send.data.iov_len     = this->_len;
          p_send.send.dispatch         = -1;
          memset(&p_send.send.hints, 0, sizeof(p_send.send.hints));
          p_send.send.dest             = dst;
          events.cookie         = this;
          events.local_fn       = this->cb_senddone;
          events.remote_fn      = NULL;
          this->_p2p_iface->sendPWQ(this->_pami_ctxt, dst,sizeof(*this->_header),this->_header, this->_len, &_pwq, &events);
          //this->_p2p_iface->send(&p_send);
	}
      }
    }

  if (this->_cb_complete)
    this->_cb_complete (this->_pami_ctxt,this->_arg, PAMI_SUCCESS);

}

/* **************************************************************** */
/*                     check whether we are done                    */
/* **************************************************************** */
template<class T_NI>
bool xlpgas::Scatter<T_NI>::isdone () const
{
  return (_rcvcount >= 1);
}

/* **************************************************************** */
/*               send completion in scatter                         */
/* **************************************************************** */
template<class T_NI>
void xlpgas::Scatter<T_NI>::cb_senddone (void * ctxt, void * arg, pami_result_t res)
{
  Scatter<T_NI> * self = (Scatter<T_NI>*)arg;
  /* LOCK */
  TRACE((stderr, "%d: Scatter<T_NI>: <%d,%d> SENDDONE sndcount=%d\n",
         XLPGAS_MYNODE, hdr->tag, hdr->kind,
         self->_sndcount[self->_odd]));

  self->_rcvcount = 1;
  /* UNLOCK */
  if (self->_cb_complete)
    self->_cb_complete (self->_pami_ctxt, self->_arg, PAMI_SUCCESS);
}

/* **************************************************************** */
/*               reception header handler                           */
/* **************************************************************** */
template<class T_NI>
inline void xlpgas::Scatter<T_NI>::cb_incoming(pami_context_t    context,
                                               void            * cookie,
                                               const void      * hdr,
                                               size_t            header_size,
                                               const void      * pipe_addr,
                                               size_t            data_size,
                                               pami_endpoint_t   origin,
                                               pami_recv_t     * recv)
{
  struct AMHeader * header = (struct AMHeader *) hdr;

  //unused:  int ctxt = header->dest_ctxt;
  CollectiveManager<T_NI> *mc = (CollectiveManager<T_NI>*) cookie;
  void * base0 =  mc->find (header->kind, header->tag);

  if (base0 == NULL)
    xlpgas_fatalerror (-1, "%d: Scatter<T_NI>/v: <%d,%d> is undefined",
                         XLPGAS_MYNODE, header->tag, header->kind);
  Scatter<T_NI> * s = (Scatter<T_NI> * ) ((char *)base0 + header->offset);
  TRACE((stderr, "%d: Scatter<T_NI>: <%d,%d> INCOMING base=%p ptr=%p len=%d\n",
         XLPGAS_MYNODE, header->tag, header->kind, base0, s, s->_len));

  if (pipe_addr)
    memcpy(s->_rbuf, pipe_addr, data_size);
  else if (recv)
  {
    recv->cookie        = s;
    recv->local_fn      = &Scatter::cb_recvcomplete;
    recv->addr          = s->_rbuf;
    recv->type          = PAMI_TYPE_BYTE;
    recv->offset        = 0;
    recv->data_fn       = PAMI_DATA_COPY;
    recv->data_cookie   = (void*)NULL;
    TRACE((stderr, "SCATTER: <%d,%d> INCOMING RETURING base=%p ptr=%p\n",
           header->tag, header->id, base0, s));
    return;
  }

  Scatter::cb_recvcomplete(context, s, PAMI_SUCCESS);
  return;
}

/* **************************************************************** */
/*           active message reception complete                      */
/* **************************************************************** */
template<class T_NI>
void xlpgas::Scatter<T_NI>::cb_recvcomplete (void * unused, void * arg, pami_result_t result)
{
  Scatter<T_NI> * self = (Scatter<T_NI> *) arg;
  self->_rcvcount++;

  TRACE((stderr, "%d: Scatter<T_NI>: <%d,%d> RECVDONE\n",
         XLPGAS_MYNODE, self->_header.tag, self->_header.id));

  if (self->_rcvcount >= 1)
    if (self->_cb_complete)
      self->_cb_complete (self->_pami_ctxt, self->_arg, PAMI_SUCCESS);
}
