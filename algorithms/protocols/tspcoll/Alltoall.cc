/**
 * \file algorithms/protocols/tspcoll/Alltoall.cc
 * \brief ???
 */
#include "algorithms/protocols/tspcoll/Alltoall.h"
#include "algorithms/protocols/tspcoll/Team.h"

/* **************************************************************** */
/*      start a new alltoall. Old alltoall has to be complete       */
/* **************************************************************** */
template<class T_NI>
void xlpgas::Alltoall<T_NI>::reset (const void        * s,
                                          void        * d,
                                          TypeCode    * stype,
                                          size_t        stypecount,
                                          TypeCode    * rtype,
                                          size_t        rtypecount)
{
  MUTEX_LOCK(&this->_mutex);
  _odd            = (!_odd);
  _sndcount[_odd] = 0;
  _rcvcount[_odd] = 0;
  _rbuf           = (char *)d;
  _sbuf           = (const char *)s;
  _len            = stype->GetDataSize() * stypecount;
  _stype          = stype;
  _rtype          = rtype;
  MUTEX_UNLOCK(&this->_mutex);
}

/* **************************************************************** */
/*                   do all the sends in an alltoall                */
/* **************************************************************** */
template<class T_NI>
void xlpgas::Alltoall<T_NI>::kick    () {
  MUTEX_LOCK(&this->_mutex);
  for (int i=0; i < (int)this->_comm->size(); i++)
    if (i == (int)this->ordinal())
      {
	memcpy (_rbuf + i * _len,
		_sbuf + i* _len,
		_len);

	_sndcount[_odd]++;
	_rcvcount[_odd]++;

	/* UNLOCK */
	MUTEX_UNLOCK(&this->_mutex);
	if (this->_sndcount[this->_odd] >= (int)this->_comm->size() &&
	    this->_rcvcount[this->_odd] >= (int)this->_comm->size())
	  if (this->_cb_complete)
	    this->_cb_complete (this->_pami_ctxt,this->_arg,PAMI_SUCCESS);
      }
    else {
      MUTEX_UNLOCK(&this->_mutex);
//      _headers[i].dest_ctxt = _comm->endpoint(i).ctxt;
      pami_send_t p_send;/*This should go once we make sure sendPWQ works*/
	  pami_send_event_t   events;
      p_send.send.header.iov_base  = &(_headers[i]);
      p_send.send.header.iov_len   = sizeof(_headers[i]);
      p_send.send.data.iov_base    = (char*) _sbuf + i * _len;
      p_send.send.data.iov_len     = this->_len;
      p_send.send.dispatch         = -1;
      memset(&p_send.send.hints, 0, sizeof(p_send.send.hints));
      p_send.send.dest             = this->_comm->endpoint (i);
      events.cookie         = this;
      events.local_fn       = this->cb_senddone;
      events.remote_fn      = NULL;
      _pwq.configure((char *)_sbuf + i * _len, this->_len, this->_len, _stype, _rtype);
      _pwq.reset();
      this->_p2p_iface->sendPWQ(this->_pami_ctxt, this->_comm->endpoint (i), sizeof(_headers[i]),&_headers[i],this->_len, &_pwq, &events);
      //this->_p2p_iface->send(&p_send);
    }
}

/* **************************************************************** */
/*                     check whether we are done                    */
/* **************************************************************** */
template<class T_NI>
bool xlpgas::Alltoall<T_NI>::isdone () const
{
  return (this->_sndcount[this->_odd] >= (int)this->_comm->size() &&
          this->_rcvcount[this->_odd] >= (int)this->_comm->size());
}

/* **************************************************************** */
/*               send completion in alltoall                        */
/* **************************************************************** */
template<class T_NI>
void xlpgas::Alltoall<T_NI>::cb_senddone (void * ctxt, void * arg, pami_result_t res)
{
  Alltoall<T_NI> * self = (Alltoall<T_NI>*)arg;
  /* LOCK */
  MUTEX_LOCK(&self->_mutex);
  TRACE((stderr, "%d: ALLTOALL: <%d,%d> SENDDONE sndcount=%d\n",
         XLPGAS_MYNODE, hdr->tag, hdr->kind,
         self->_sndcount[self->_odd]));

  self->_sndcount[self->_odd]++;

  /* UNLOCK */
  MUTEX_UNLOCK(&self->_mutex);
  if (self->_sndcount[self->_odd] >= (int)self->_comm->size() &&
      self->_rcvcount[self->_odd] >= (int)self->_comm->size())
    if (self->_cb_complete)
      self->_cb_complete (self->_pami_ctxt, self->_arg, res);
}

/* **************************************************************** */
/*               reception header handler                           */
/* **************************************************************** */
template<class T_NI>
inline void xlpgas::Alltoall<T_NI>::cb_incoming(pami_context_t    context,
                                                void            * cookie,
                                                const void      * hdr,
                                                size_t            header_size,
                                                const void      * pipe_addr,
                                                size_t            data_size,
                                                pami_endpoint_t   origin,
                                                pami_recv_t     * recv)
{
  struct AMHeader * header = (struct AMHeader *) hdr;
  //unused:   int ctxt = header->dest_ctxt;
  CollectiveManager<T_NI> *mc = (CollectiveManager<T_NI>*) cookie;
  void * base0 =  mc->find (header->kind, header->tag);

  if (base0 == NULL)
    xlpgas_fatalerror (-1, "%d: Alltoall<T_NI>/v: <%d,%d> is undefined",
                         XLPGAS_MYNODE, header->tag, header->kind);
  Alltoall<T_NI> * s = (Alltoall<T_NI> * ) ((char *)base0 + header->offset);
  TRACE((stderr, "%d: ALLTOALL: <%d,%d> INCOMING base=%p ptr=%p len=%d\n",
         XLPGAS_MYNODE, header->tag, header->kind, base0, s, s->_len));

  char * rbuf = s->_rbuf + header->senderID * s->_len;
  if (pipe_addr)
    memcpy(rbuf, pipe_addr, data_size);
  else if (recv)
  {
    recv->cookie        = s;
    recv->local_fn      = Alltoall<T_NI>::cb_recvcomplete;
    recv->addr          = rbuf;
    recv->type          = PAMI_TYPE_BYTE;
    recv->offset        = 0;
    recv->data_fn       = PAMI_DATA_COPY;
    recv->data_cookie   = (void*)NULL;
    TRACE((stderr, "SCATTER: <%d,%d> INCOMING RETURING base=%p ptr=%p\n",
           header->tag, header->id, base0, s));
    return;
  }
  Alltoall::cb_recvcomplete(context, s, PAMI_SUCCESS);
  return;
}

/* **************************************************************** */
/*           active message reception complete                      */
/* **************************************************************** */
template<class T_NI>
void xlpgas::Alltoall<T_NI>::cb_recvcomplete (void * unused, void * arg, pami_result_t res)
{
  Alltoall<T_NI> * self = (Alltoall<T_NI> *) arg;
  MUTEX_LOCK(&self->_mutex);
  self->_rcvcount[self->_odd]++;
  MUTEX_UNLOCK(&self->_mutex);
  TRACE((stderr, "%d: ALLTOALL: <%d,%d> RECVDONE\n",
         XLPGAS_MYNODE, self->_header.tag, self->_header.id));

  if (self->_sndcount[self->_odd] >= (int)self->_comm->size() &&
      self->_rcvcount[self->_odd] >= (int)self->_comm->size())
    if (self->_cb_complete)
      self->_cb_complete (self->_pami_ctxt, self->_arg, res);
}
