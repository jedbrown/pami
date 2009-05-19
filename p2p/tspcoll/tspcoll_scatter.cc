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


// #define DEBUG_SCATTER 1
#undef TRACE
#ifdef DEBUG_SCATTER
#define TRACE(x)  fprintf x;
#else
#define TRACE(x)
#endif

/* **************************************************************** */
/* **************************************************************** */

void TSPColl::Scatter::amsend_reg (void)
{
  __pgasrt_tsp_amsend_reg (PGASRT_TSP_AMSEND_COLLSCATTER,Scatter::cb_incoming);
}

/* **************************************************************** */
/*                 Scatterv constructor                             */
/* **************************************************************** */

TSPColl::Scatter::Scatter (Communicator * comm, NBTag tag, 
			   int instID, int tagoff):
  NBColl (comm, tag, instID, NULL, NULL)
{
  _counter         = 0;
  _complete        = 0;
  _sendidx         = 0;

  _sbuf            = NULL;
  _rbuf            = NULL;
  _isroot          = false;

  {
    _header.hdr.handler   = (__pgasrt_AMHeaderHandler_t )
      PGASRT_TSP_AMSEND_COLLSCATTER;
    _header.hdr.headerlen = sizeof (struct scatter_header);
    _header.tag           = tag;
    _header.id            = instID;
    _header.tagoff        = tagoff;
    _header.self          = this;
  }
}

/* **************************************************************** */
/*               reset the scatterv routine                         */
/* **************************************************************** */

void TSPColl::Scatter::
reset (int root, const void * sbuf, void * rbuf, size_t length)
{
  _isroot = (root == _comm->rank());
  _rbuf   = rbuf;
  _sbuf   = (const char *)sbuf;
  _length = length;
  _counter++;
  _sendidx = 0;
}

/* **************************************************************** */
/*              kick the scatter routine                            */
/* **************************************************************** */

void TSPColl::Scatter::kick()
{
  if (!_isroot) return;
  for (int i=0; i < _comm->size(); i++)
    if (i == _comm->rank()) 
      { 
	memcpy (_rbuf, _sbuf+i*_length, _length); 
	cb_senddone (&_header);
      }
    else
      __pgasrt_tsp_amsend (_comm->absrankof (i),
			   (__pgasrt_AMHeader_t *)&_header,
			   (__pgasrt_local_addr_t) _sbuf + i * _length, 
			   _length,
			   cb_senddone, &_header);
}

/* **************************************************************** */
/*               send completion in scatter                         */
/* **************************************************************** */

void TSPColl::Scatter::cb_senddone (void * arg)
{
  Scatter * self = ((struct scatter_header *)arg)->self;
  /* LOCK */
  TRACE((stderr, "%d: SCATTER SDONE ctr=%d cplt=%d sidx=%d\n",
	 PGASRT_MYNODE, self->_counter, self->_complete, self->_sendidx));
  if (++(self->_sendidx) < self->_comm->size()) return;
  self->_sendidx = 0;
  self->_complete++;
  /* UNLOCK */
  if (self->_cb_complete) self->_cb_complete (self->_arg);
}

/* **************************************************************** */
/*               reset the scatterv routine                         */
/* **************************************************************** */

void TSPColl::Scatterv::
reset (int root, const void * sbuf, void * rbuf, size_t * lengths)
{
  _isroot = (root == _comm->rank());
  _rbuf   = rbuf;
  _sbuf   = (const char *)sbuf;
  _lengths = lengths;
  _length  = 0;
  _counter++;
  _sendidx = 0;
  TRACE((stderr, "%d: SCATTERV RESET ctr=%d this=%p sbuf=%p rbuf=%p\n",
	 PGASRT_MYNODE, _counter, this, sbuf, rbuf));
}

/* **************************************************************** */
/*             kick the scatterv routine                            */
/* **************************************************************** */

void TSPColl::Scatterv::kick()
{
  if (!_isroot) return;
  assert (_lengths != NULL && _sbuf != NULL);
  TRACE((stderr, "%d: SCATTERV KICK ctr=%d cplt=%d\n",
	 PGASRT_MYNODE, _counter, _complete));

  for (int i=0; i < _comm->size(); i++)
    {
      const char * s = _sbuf; for (int j=0; j<i; j++) s += _lengths[j];
      TRACE((stderr, "%d: SCATTERV SEND ctr=%d cplt=%d (%d/%d) len=%d "
	     "s=%p 0x%02x 0x%02x 0x%02x\n", 
	     PGASRT_MYNODE, _counter, _complete, i, _comm->size(), _lengths[i],
	     s, s[0], s[1], s[2]));
      
      if (i == _comm->rank()) 
	{
	  memcpy (_rbuf, s, _lengths[i]);
	  cb_senddone (&_header);
	}
      else
	__pgasrt_tsp_amsend (_comm->absrankof (i),
			     (__pgasrt_AMHeader_t *) &_header,
			     (__pgasrt_local_addr_t) s,
			     _lengths[i],
			     cb_senddone, &_header);
    }
}

/* **************************************************************** */
/*            incoming active message                               */
/* **************************************************************** */

__pgasrt_local_addr_t TSPColl::Scatter::
cb_incoming (const struct __pgasrt_AMHeader_t * hdr,
	     void (** completionHandler)(void *, void *),
	     void ** arg)
{
  struct scatter_header * header = (struct scatter_header *) hdr;
  void * base0 =  NBCollManager::instance()->find (header->tag, header->id);
  if (base0 == NULL)
    __pgasrt_fatalerror (-1, "%d: Scatter/v: <%d,%d> is undefined",
                         PGASRT_MYNODE, header->tag, header->id);
  Scatter * s = (Scatter * ) ((char *)base0 + header->tagoff);
  TRACE((stderr, "%d: SCATTERV: <%d,%d> INCOMING base=%p ptr=%p\n", 
	 PGASRT_MYNODE, header->tag, header->id, base0, s));
  *completionHandler = &Scatter::cb_recvcomplete;
  *arg = s;
  assert (s->_rbuf != NULL);
  return (__pgasrt_local_addr_t) s->_rbuf;
}

/* **************************************************************** */
/*           active message reception complete                      */
/* **************************************************************** */

void TSPColl::Scatter::cb_recvcomplete (void * unused, void * arg)
{
  Scatter * s = (Scatter *) arg;
  s->_complete++;
  if (s->_cb_complete) s->_cb_complete (s->_arg);
}


