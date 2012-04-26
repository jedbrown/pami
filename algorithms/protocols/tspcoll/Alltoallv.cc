/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* This is an automatically generated copyright prolog.             */
/* After initializing,  DO NOT MODIFY OR MOVE                       */
/*  --------------------------------------------------------------- */
/* Licensed Materials - Property of IBM                             */
/* Blue Gene/Q 5765-PER 5765-PRP                                    */
/*                                                                  */
/* (C) Copyright IBM Corp. 2011, 2012 All Rights Reserved           */
/* US Government Users Restricted Rights -                          */
/* Use, duplication, or disclosure restricted                       */
/* by GSA ADP Schedule Contract with IBM Corp.                      */
/*                                                                  */
/*  --------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/tspcoll/Alltoallv.cc
 * \brief ???
 */
#include "algorithms/protocols/tspcoll/Alltoallv.h"
#include "algorithms/protocols/tspcoll/Team.h"

/* **************************************************************** */
/*      start a new alltoallv. Old alltoallv has to be complete       */
/* **************************************************************** */
template<class T_NI, class CntType>
void xlpgas::Alltoallv<T_NI,CntType>::reset (const void * s, void * d,
			       TypeCode     *stype,
			       const CntType *scnts,
			       const CntType *sdispls,
			       TypeCode     *rtype,
			       const CntType *rcnts,
			       const CntType *rdispls)
{
  MUTEX_LOCK(&this->_mutex);
  //alltoall common
  this->_odd = (!this->_odd);
  this->_sndcount[this->_odd] = 0;
  this->_sndstartedcount[this->_odd] = 0;
  this->_rcvcount[this->_odd] = 0;
  this->_rbuf           = (char *)d;
  this->_sbuf           = (const char *)s;
  this->_stype          = stype;
  this->_rtype          = rtype;
  this->_current        = this->ordinal();
  //altoallv specific
  this->_scnts          = scnts;
  this->_sdispls        = sdispls;
  this->_rcnts          = rcnts;
  this->_rdispls        = rdispls;
  MUTEX_UNLOCK(&this->_mutex);
}

/* **************************************************************** */
/*                   do all the sends in an alltoall                */
/* **************************************************************** */
template<class T_NI, class CntType>
void xlpgas::Alltoallv<T_NI,CntType>::kick_internal    () {
  MUTEX_LOCK(&this->_mutex);
  size_t sdatawidth = this->_stype->GetDataSize();
  size_t rdatawidth = this->_rtype->GetDataSize();
  int j = this->_sndstartedcount[this->_odd];
  for (; j < (int)this->_comm->size(); j++) {
    //if the buffer is full then we give the system some time to
    //complete pending sends
    if (this->buffer_full()) {
      MUTEX_UNLOCK(&this->_mutex);
      break;
    }

    if (this->_current == this->ordinal()) {
	memcpy (this->_rbuf + this->_rdispls[this->_current] * rdatawidth,
		this->_sbuf + this->_sdispls[this->_current] * sdatawidth,
		this->_scnts[this->_current]*sdatawidth);

	this->_sndcount[this->_odd]++;
	this->_sndstartedcount[this->_odd]++;
	this->_rcvcount[this->_odd]++;

	/* UNLOCK */
	MUTEX_UNLOCK(&this->_mutex);
	if (this->_sndcount[this->_odd] >= (int)this->_comm->size() &&
	    this->_rcvcount[this->_odd] >= (int)this->_comm->size())
	  if (this->_cb_complete)
	    this->_cb_complete (this->_pami_ctxt,this->_arg, PAMI_SUCCESS);
    }
    else {
      MUTEX_UNLOCK(&this->_mutex);
      this->_sndstartedcount[this->_odd]++;
      pami_send_t p_send;
      pami_send_event_t   events;
      p_send.send.header.iov_base  = &(this->_header);
      p_send.send.header.iov_len   = sizeof(this->_header);
      p_send.send.data.iov_base    = (char*) this->_sbuf + this->_sdispls[this->_current] * sdatawidth;
      p_send.send.data.iov_len     = this->_scnts[this->_current] * sdatawidth;
      p_send.send.dispatch         = -1;
      memset(&p_send.send.hints, 0, sizeof(p_send.send.hints));
      p_send.send.dest             = this->_comm->index2Endpoint (this->_current);
      events.cookie         = this;
      events.local_fn       = this->cb_senddone;
      events.remote_fn      = NULL;
      this->_pwq.configure((char*) this->_sbuf + this->_sdispls[this->_current] * sdatawidth, this->_scnts[this->_current] * sdatawidth, this->_scnts[this->_current] * sdatawidth, this->_stype, this->_rtype);
      this->_p2p_iface->sendPWQ(this->_pami_ctxt, this->_comm->index2Endpoint (this->_current), sizeof(this->_header),&this->_header,this->_scnts[this->_current] * sdatawidth, &this->_pwq, &events);
      //this->_p2p_iface->send(&p_send);
    }

    // increment current wrapping arround
    this->_current += 1;
    if(this->_current == this->_comm->size())
      this->_current = 0;

  }
}

/*
  Alltoall pushes a certain number of messages and waits
 */
template<class T_NI, class CntType>
pami_result_t repost_all2allv_function (pami_context_t context, void *cookie) {
  xlpgas::Alltoallv<T_NI,CntType>* coll = (xlpgas::Alltoallv<T_NI,CntType>*)cookie;
  coll->kick_internal();
  if( coll->all_sent() ) {
    return PAMI_SUCCESS;
  }
  else {
    return PAMI_EAGAIN;
  }
}

template<class T_NI, class CntType>
void xlpgas::Alltoallv<T_NI,CntType>::kick    () {
  this->kick_internal();
  if( ! this->all_sent() ) {
    //repost if not all messages sent due to buffer full
    PAMI::Device::Generic::GenericThread *work = new ((void*)(&(this->_work_pami))) PAMI::Device::Generic::GenericThread(repost_all2allv_function<T_NI, CntType>, (void*)this);
    this->_dev[0].postThread(work);
  }
}


/* **************************************************************** */
/*               reception header handler                           */
/* **************************************************************** */
template<class T_NI, class CntType>
inline void xlpgas::Alltoallv<T_NI,CntType>::cb_incoming_v(pami_context_t    context,
                                                   void            * cookie,
                                                   const void      * hdr,
                                                   size_t            header_size,
                                                   const void      * pipe_addr,
                                                   size_t            data_size,
                                                   pami_endpoint_t   origin,
                                                   pami_recv_t     * recv)
{
  struct Alltoall<T_NI>::AMHeader * header = (struct Alltoall<T_NI>::AMHeader *) hdr;
  //unused:   int ctxt = header->dest_ctxt;
  CollectiveManager<T_NI> *mc = (CollectiveManager<T_NI>*) cookie;
  void * base0 =  mc->find (header->kind, header->tag);

  if (base0 == NULL)
    xlpgas_fatalerror (-1, "%d: Alltoallv<T_NI>/v: <%d,%d> is undefined",
                         XLPGAS_MYNODE, header->tag, header->kind);
  Alltoallv<T_NI,CntType> * s = (Alltoallv<T_NI,CntType> * ) ((char *)base0 + header->offset);
  TRACE((stderr, "%d: ALLTOALL: <%d,%d> INCOMING base=%p ptr=%p len=%d\n",
         XLPGAS_MYNODE, header->tag, header->kind, base0, s, s->_len));

  char * rbuf =  s->_rbuf + s->_rdispls[header->senderID] * s->_rtype->GetDataSize();
  if (pipe_addr)
    memcpy(rbuf, pipe_addr, data_size);
  else if (recv)
  {
    recv->cookie        = s;
    recv->local_fn      = Alltoallv<T_NI,CntType>::cb_recvcomplete;
    recv->addr          = rbuf;
    recv->type          = PAMI_TYPE_BYTE;
    recv->offset        = 0;
    recv->data_fn       = PAMI_DATA_COPY;
    recv->data_cookie   = (void*)NULL;
    TRACE((stderr, "ALLTOALLV: <%d,%d> INCOMING RETURING base=%p ptr=%p\n",
           header->tag, header->id, base0, s));
    return;
  }
  Alltoallv::cb_recvcomplete(context, s, PAMI_SUCCESS);
  return;
}
