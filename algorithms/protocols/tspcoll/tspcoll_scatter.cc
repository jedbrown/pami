/* ************************************************************************* */
/*                            IBM Confidential                               */
/*                          OCO Source Materials                             */
/*                      IBM XL UPC Alpha Edition, V0.9                       */
/*                                                                           */
/*                      Copyright IBM Corp. 2005, 2007.                      */
/*                                                                           */
/* The source code for this program is not published or otherwise divested   */
/* of its trade secrets, irrespective of what has been deposited with the    */
/* U.S. Copyright Office.                                                    */
/* ************************************************************************* */

#include "./Scatter.h"
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
template<class T_Mcast>
TSPColl::Scatter<T_Mcast>::Scatter (XMI::Geometry::Geometry<XMI_GEOMETRY_CLASS> * comm, NBTag tag, 
			   int instID, int tagoff):
  NBColl<T_Mcast> (comm, tag, instID, NULL, NULL)
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
template<class T_Mcast>
void TSPColl::Scatter<T_Mcast>::
reset (int root, const void * sbuf, void * rbuf, size_t length)
{
  _isroot = (root == this->_comm->rank());
  _rbuf   = rbuf;
  _sbuf   = (const char *)sbuf;
  _length = length;
  _counter++;
  _sendidx = 0;
}

/* **************************************************************** */
/*              kick the scatter routine                            */
/* **************************************************************** */
template<class T_Mcast>
void TSPColl::Scatter<T_Mcast>::kick(T_Mcast *mcast_iface)
{
  _mcast_iface = mcast_iface;
  TRACE((stderr, "SCATTER KICK START\n"));
  if (!_isroot) return;  
  _req = (XMI_Request_t*) malloc(this->_comm->size()*sizeof(XMI_Request_t));
  for (int i=0; i < this->_comm->size(); i++)
    if (i == this->_comm->rank()) 
      { 
	memcpy (_rbuf, _sbuf+i*_length, _length); 
	cb_senddone (&_header);
      }
    else
      {	  
      unsigned        hints   = CCMI_PT_TO_PT_SUBTASK;
      unsigned        ranks   = this->_comm->absrankof (i);
      XMI_Callback_t cb_done;
      cb_done.function        = (void (*)(void*, xmi_result_t*))cb_senddone;
      cb_done.clientdata      = &this->_header;
      void * r = NULL;
      TRACE((stderr, "SCATTER KICK sbuf=%p hdr=%p, tag=%d id=%d\n",
	     this->_sbuf, &this->_header,this->_header.tag, this->_header.id));
      mcast_iface->send (&_req[i],
			 &cb_done,
			 CCMI_MATCH_CONSISTENCY,
			 (xmi_quad_t*)&this->_header,
			 CCMIQuad_sizeof(this->_header),
			 0,
			 (char*)(this->_sbuf + i * this->_length),
			 (unsigned)this->_length,
			 &hints,
			 &ranks,
			 1);
      }
}

/* **************************************************************** */
/*               send completion in scatter                         */
/* **************************************************************** */
template<class T_Mcast>
void TSPColl::Scatter<T_Mcast>::cb_senddone (void * arg)
{
  Scatter * self = ((struct scatter_header *)arg)->self;
  /* LOCK */
  TRACE((stderr, "SCATTER SDONE ctr=%d cplt=%d sidx=%d\n",
	 self->_counter, self->_complete, self->_sendidx));
  if (++(self->_sendidx) < self->_comm->size()) return;
  self->_sendidx = 0;
  self->_complete++;
  /* UNLOCK */
  if (self->_cb_complete) 
      {
	  free(self->_req);
	  self->_cb_complete (self->_arg);
      }
}

/* **************************************************************** */
/*               reset the scatterv routine                         */
/* **************************************************************** */
template<class T_Mcast>
void TSPColl::Scatterv<T_Mcast>::
reset (int root, const void * sbuf, void * rbuf, size_t * lengths)
{
  this->_isroot = (root == this->_comm->rank());
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
template<class T_Mcast>
void TSPColl::Scatterv<T_Mcast>::kick(T_Mcast *mcast_iface)
{
  this->_mcast_iface = mcast_iface;
  if (!this->_isroot) return;
  assert (this->_lengths != NULL && this->_sbuf != NULL);
  TRACE((stderr, "SCATTERV KICK ctr=%d cplt=%d\n",
	 this->_counter, this->_complete));

  this->_req = (XMI_Request_t*) malloc(this->_comm->size()*sizeof(XMI_Request_t));
  for (int i=0; i < this->_comm->size(); i++)
    {
      const char * s = this->_sbuf; for (int j=0; j<i; j++) s += this->_lengths[j];
      TRACE((stderr, "SCATTERV SEND ctr=%d cplt=%d (%d/%d) len=%d "
	     "s=%p 0x%02x 0x%02x 0x%02x\n", 
	     this->_counter, this->_complete, i, this->_comm->size(), this->_lengths[i],
	     s, s[0], s[1], s[2]));
      
      if (i == this->_comm->rank()) 
	{
	  memcpy (this->_rbuf, s, this->_lengths[i]);
	  cb_senddone (&this->_header);
	}
      else
	{
#if 0
	__pgasrt_tsp_amsend (this->_comm->absrankof (i),
			     (__pgasrt_AMHeader_t *) &this->_header,
			     (__pgasrt_local_addr_t) s,
			     this->_lengths[i],
			     this->cb_senddone, &this->_header);
#endif
	unsigned        hints   = CCMI_PT_TO_PT_SUBTASK;
	unsigned        ranks   = this->_comm->absrankof (i);
	XMI_Callback_t cb_done;
	cb_done.function        = (void (*)(void*, xmi_result_t*))this->cb_senddone;
	cb_done.clientdata      = &this->_header;
	void * r = NULL;
	mcast_iface->send (&this->_req[i],
			   &cb_done,
			   CCMI_MATCH_CONSISTENCY,
			   (xmi_quad_t*)&this->_header,
			   CCMIQuad_sizeof(this->_header),
			   0,
			   (char*)(s),
			   (unsigned)this->_lengths[i],
			   &hints,
			   &ranks,
			   1);
	}
    }
}

/* **************************************************************** */
/*            incoming active message                               */
/* **************************************************************** */
//__pgasrt_local_addr_t TSPColl::Scatter::
//cb_incoming (const struct __pgasrt_AMHeader_t * hdr,
//	     void (** completionHandler)(void *, void *),
//	     void ** arg)
template<class T_Mcast>
XMI_Request_t * TSPColl::Scatter<T_Mcast>::cb_incoming(const xmi_quad_t  * hdr,
					       unsigned          count,
					       unsigned          peer,
					       unsigned          sndlen,
					       unsigned          conn_id,
					       void            * arg,
					       unsigned        * rcvlen,
					       char           ** rcvbuf,
					       unsigned        * pipewidth,
					       XMI_Callback_t * cb_done)
{
  struct scatter_header * header = (struct scatter_header *) hdr;
  void * base0 =  NBCollManager<T_Mcast>::instance()->find (header->tag, header->id);
  if (base0 == NULL)
    CCMI_FATALERROR (-1, "Scatter/v: <%d,%d> is undefined",
		     header->tag, header->id);
  Scatter * s = (Scatter * ) ((char *)base0 + header->tagoff);
  TRACE((stderr, "SCATTER/v: <%d,%d> INCOMING base=%p ptr=%p\n", 
	 header->tag, header->id, base0, s));

    // multisend stuff
  *rcvbuf             = (char*)s->_rbuf;
  *rcvlen             = sndlen;
  *pipewidth          = sndlen;
  cb_done->function   = (void (*)(void*, xmi_result_t*))&Scatter::cb_recvcomplete;
  cb_done->clientdata = s;
  
  TRACE((stderr, "SCATTER/v: <%d,%d> INCOMING RETURING base=%p ptr=%p\n", 
	 header->tag, header->id, base0, s));
  

  return &s->_rreq;

  //  *completionHandler = &Scatter::cb_recvcomplete;
  //  *arg = s;
  //  assert (s->_rbuf != NULL);
  //  return (__pgasrt_local_addr_t) s->_rbuf;
}

/* **************************************************************** */
/*           active message reception complete                      */
/* **************************************************************** */
template<class T_Mcast>
void TSPColl::Scatter<T_Mcast>::cb_recvcomplete (void *arg, xmi_result_t*err)
{
  Scatter * s = (Scatter *) arg;
  s->_complete++;
  if (s->_cb_complete) s->_cb_complete (s->_arg);
}


