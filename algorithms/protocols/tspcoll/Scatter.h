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
 * \file algorithms/protocols/tspcoll/Scatter.h
 * \brief ???
 */

#ifndef __algorithms_protocols_tspcoll_Scatter_h__
#define __algorithms_protocols_tspcoll_Scatter_h__

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "algorithms/protocols/tspcoll/NBColl.h"
#include "util/common.h"

/* **************************************************************** */
/*                      Scatter                                     */
/* **************************************************************** */

namespace TSPColl
{
  template <class T_NI>
  class Scatter: public NBColl<T_NI>
  {
  protected:
    static const int MAX_CONCURRENCY=10;

  public:
    Scatter      (PAMI_GEOMETRY_CLASS * comm, NBTag tag, int instID, int tagoff);
    void reset   (int root, const void * sbuf, void * rbuf, size_t length);
    virtual void kick    (T_NI *p2p_iface);
    virtual bool isdone  (void) const { return _complete >= _counter; }
    static void amsend_reg  (T_NI *p2p_iface, void *cd);
  protected:
    T_NI       *_p2p_iface;
    const char    * _sbuf;         /* send buffer    */
    void          * _rbuf;         /* receive buffer */
    size_t          _length;       /* msg length     */
    bool            _isroot;       /* am i the sender? */

    unsigned        _counter;      /* instance counter */
    unsigned        _complete;     /* instance completion counter */
    int             _sendidx;      /* message counter in each execution */

    struct scatter_header
    {
      NBTag               tag;
      int                 id;
      int                 tagoff;
      unsigned            counter;
      Scatter           * self;
    }
    _header;

  protected:
    static inline void cb_incoming(pami_context_t    context,
                                   void            * cookie,
                                   const void      * header_addr,
                                   size_t            header_size,
                                   const void      * pipe_addr,
                                   size_t            data_size,
                                   pami_endpoint_t   origin,
                                   pami_recv_t     * recv);
    static void cb_recvcomplete (pami_context_t context, void *arg, pami_result_t err);
    static void cb_senddone(pami_context_t context, void *arg, pami_result_t err);
  };
};
template <class T_NI>
inline void TSPColl::Scatter<T_NI>::amsend_reg  (T_NI *p2p_iface, void *cd)
    {
      pami_dispatch_callback_fn fn;
      fn.p2p = TSPColl::Scatter<T_NI>::cb_incoming;
      p2p_iface->setSendDispatch(fn, cd);
    }
/* **************************************************************** */
/*                    Scatterv                                      */
/* **************************************************************** */

namespace TSPColl
{
  template <class T_NI>
  class Scatterv: public Scatter<T_NI>
  {
  public:
    Scatterv (PAMI_GEOMETRY_CLASS * comm, NBTag tag, int instID, int tagoff):
      Scatter<T_NI> (comm, tag, instID, tagoff), _lengths(0) { }
    void reset (int root, const void * sbuf, void * rbuf, size_t * lengths);
    virtual void kick (T_NI *p2p_iface);
  protected:
    size_t * _lengths;
  };
};



//#define DEBUG_SCATTER 1
#undef TRACE
#ifdef DEBUG_SCATTER
#define TRACE(x)  fprintf x;
#else
#define TRACE(x)
#endif


/* **************************************************************** */
/*                 Scatterv constructor                             */
/* **************************************************************** */
template<class T_NI>
TSPColl::Scatter<T_NI>::Scatter (PAMI_GEOMETRY_CLASS * comm, NBTag tag,
                           int instID, int tagoff):
  NBColl<T_NI> (comm, tag, instID, NULL, NULL)
{
  _counter         = 0;
  _complete        = 0;
  _sendidx         = 0;

  _sbuf            = NULL;
  _rbuf            = NULL;
  _isroot          = false;

  {
    _header.tag           = tag;
    _header.id            = instID;
    _header.tagoff        = tagoff;
    _header.self          = this;
  }
}

/* **************************************************************** */
/*               reset the scatterv routine                         */
/* **************************************************************** */
template<class T_NI>
void TSPColl::Scatter<T_NI>::
reset (int root, const void * sbuf, void * rbuf, size_t length)
{
  _isroot = ((size_t)root == this->_comm->virtrank());
  _rbuf   = rbuf;
  _sbuf   = (const char *)sbuf;
  _length = length;
  _counter++;
  _sendidx = 0;
}

/* **************************************************************** */
/*              kick the scatter routine                            */
/* **************************************************************** */
template<class T_NI>
void TSPColl::Scatter<T_NI>::kick(T_NI *p2p_iface)
{
  _p2p_iface = p2p_iface;
  TRACE((stderr, "SCATTER KICK START\n"));
  if (!_isroot) return;
  for (size_t i=0; i < this->_comm->size(); i++)
    if (i == this->_comm->virtrank())
      {
        memcpy (_rbuf, _sbuf+i*_length, _length);
        cb_senddone (NULL, &_header, PAMI_SUCCESS);
      }
    else
      {
      TRACE((stderr, "SCATTER KICK sbuf=%p hdr=%p, tag=%d id=%d\n",
             this->_sbuf, &this->_header,this->_header.tag, this->_header.id));

      pami_send_t p_send;
      p_send.send.header.iov_base  = &this->_header;
      p_send.send.header.iov_len   = sizeof(this->_header);
      p_send.send.data.iov_base    = (char*)(this->_sbuf + i * this->_length);
      p_send.send.data.iov_len     = this->_length;
      p_send.send.dispatch         = -1;
      memset(&p_send.send.hints, 0, sizeof(p_send.send.hints));
      p_send.send.dest             = this->_comm->absrankof (i);
      p_send.events.cookie         = &this->_header;
      p_send.events.local_fn       = this->cb_senddone;
      p_send.events.remote_fn      = NULL;
      p2p_iface->send(&p_send);
      }
}

/* **************************************************************** */
/*               send completion in scatter                         */
/* **************************************************************** */
template<class T_NI>
void TSPColl::Scatter<T_NI>::cb_senddone (pami_context_t context, void *arg, pami_result_t err)
{
  Scatter * self = ((struct scatter_header *)arg)->self;
  /* LOCK */
  TRACE((stderr, "SCATTER SDONE ctr=%d cplt=%d sidx=%d\n",
         self->_counter, self->_complete, self->_sendidx));
  if ((size_t)++(self->_sendidx) < self->_comm->size()) return;
  self->_sendidx = 0;
  self->_complete++;
  /* UNLOCK */
  if (self->_cb_complete)
      {
          self->_cb_complete (NULL, self->_arg, PAMI_SUCCESS);
      }
}

/* **************************************************************** */
/*               reset the scatterv routine                         */
/* **************************************************************** */
template<class T_NI>
void TSPColl::Scatterv<T_NI>::
reset (int root, const void * sbuf, void * rbuf, size_t * lengths)
{
  this->_isroot = ((size_t)root == this->_comm->virtrank());
  this->_rbuf   = rbuf;
  this->_sbuf   = (const char *)sbuf;
  this->_lengths = lengths;
  this->_length  = 0;
  this->_counter++;
  this->_sendidx = 0;
  TRACE((stderr, "SCATTERV RESET ctr=%d this=%p sbuf=%p rbuf=%p\n",
         this->_counter, this, sbuf, rbuf));
}

/* **************************************************************** */
/*             kick the scatterv routine                            */
/* **************************************************************** */
template<class T_NI>
void TSPColl::Scatterv<T_NI>::kick(T_NI *p2p_iface)
{
  this->_p2p_iface = p2p_iface;
  if (!this->_isroot) return;
  PAMI_assert(this->_lengths != NULL && this->_sbuf != NULL);
  TRACE((stderr, "SCATTERV KICK ctr=%d cplt=%d\n",
         this->_counter, this->_complete));

  for (size_t i=0; i < this->_comm->size(); i++)
      {
        const char * s = this->_sbuf; for (unsigned j=0; j<i; j++) s += this->_lengths[j];
        TRACE((stderr, "SCATTERV SEND ctr=%d cplt=%d (%d/%d) len=%d "
               "s=%p 0x%02x 0x%02x 0x%02x\n",
               this->_counter, this->_complete, i, this->_comm->size(), this->_lengths[i],
               s, s[0], s[1], s[2]));

        if (i == this->_comm->virtrank())
            {
              memcpy (this->_rbuf, s, this->_lengths[i]);
              cb_senddone (NULL, &this->_header, PAMI_SUCCESS);
            }
        else
            {
              pami_send_t p_send;
              p_send.send.header.iov_base  = &this->_header;
              p_send.send.header.iov_len   =  sizeof(this->_header);
              p_send.send.data.iov_base    = (void*)s;
              p_send.send.data.iov_len     = this->_lengths[i];
              p_send.send.dispatch         = -1;
              memset(&p_send.send.hints, 0, sizeof(p_send.send.hints));
              p_send.send.dest             = this->_comm->absrankof(i);
              p_send.events.cookie         = &this->_header;
              p_send.events.local_fn       = this->cb_senddone;
              p_send.events.remote_fn      = NULL;
              p2p_iface->send(&p_send);
            }
      }
}


/* **************************************************************** */
/*           active message reception complete                      */
/* **************************************************************** */
template<class T_NI>
void TSPColl::Scatter<T_NI>::cb_recvcomplete (pami_context_t context, void *arg, pami_result_t err)
{
  Scatter * s = (Scatter *) arg;
  s->_complete++;
  if (s->_cb_complete) s->_cb_complete (context, s->_arg, err);
}








#endif
