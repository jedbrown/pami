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

#include "./Gather.h"
#include "./TagList.h"

//#define TRACE(x) fprintf x
#define TRACE(x)


/* **************************************************************** */
/*                 Gatherv constructor                             */
/* **************************************************************** */

TSPColl::Gather::Gather (Communicator * comm, NBTag tag, int id, int tagoff):
  NBColl (comm, tag, id), _barrier (comm, tag, id, ...)
{
  _counter              = 0;
  _complete             = 0;
  _sbuf                 = NULL;
  _rbuf                 = NULL;
  _header.hdr.handler   = Gather::cb_incoming;
  _header.hdr.headerlen = sizeof (struct gather_header);
  _header.tag           = tag;
  _header.id            = id;
  _header.tagoff        = tagoff;
  _header.offset        = 0;
}

/* **************************************************************** */
/*               reset the scatterv routine                         */
/* **************************************************************** */

void TSPColl::Gather::
reset (int root, const void * sbuf, void * rbuf, size_t length)
{
  assert (rbuf != NULL && sbuf != NULL);
  Communicator * comm = TSPColl::_commlist[_commID];
  _root          = root;
  _rbuf          = rbuf;
  _sbuf          = sbuf;
  _length        = length;
  _incoming      = comm->size();
  _header.offset = comm->rank() * length;
  _counter++;
  TRACE((stderr, "%d: GATHER: root=%d\n", comm->rank(), _root));
}

/* **************************************************************** */
/*               reset the scatterv routine                         */
/* **************************************************************** */

void TSPColl::Gatherv::
reset (int root, const void * sbuf, void * rbuf, size_t * lengths)
{
  Communicator * comm = TSPColl::_commlist[_commID];
  _root          = root;
  _rbuf          = rbuf;
  _sbuf          = sbuf;
  _length        = lengths[comm->rank()];
  _incoming      = comm->size();
  _header.offset = 0;
  for (int i=0; i<comm->rank(); i++) _header.offset += lengths[i];
  _counter++;
}

/* **************************************************************** */
/*              kick the scatter routine                            */
/* **************************************************************** */

void TSPColl::Gather::kick(CCMI::MultiSend::MulticastInterface *mcast_iface)
{
  Communicator * comm = TSPColl::_commlist[_commID];

  TRACE((stderr, "%d: Sending %d bytes to %d/%d\n", 
	 comm->rank(), _length, _root, comm->absrankof(_root)));
  
  if (comm->rank() == _root) 
    {
      memcpy ((__pgasrt_local_addr_t)_rbuf + _header.offset, _sbuf, _length);
      if (--_incoming <= 0) _complete ++;
    }
  else
    {
#if 0
      __pgasrt_tsp_amsend (comm->absrankof (_root),
			   (__pgasrt_AMHeader_t *)&_header,
			   (__pgasrt_local_addr_t) _sbuf, _length,
			   cb_senddone, (void *) this);
#endif
      unsigned        hints   = CCMI_PT_TO_PT_SUBTASK;
      unsigned        ranks   = comm->absrankof (_root);
      CCMI_Callback_t cb_done;
      cb_done.function        = cb_senddone;
      cb_done.clientdata      = this;
      void * r = mcast_iface->send (&_req,
				    cb_done,
				    CCMI_MATCH_CONSISTENCY,
				    & _header,
				    _counter,
				    _sbuf,
				    _length,
				    &hints,
				    &ranks,
				    1);
      
    }
}

/* **************************************************************** */
/*                 send callback                                    */
/* **************************************************************** */

void TSPColl::Gather::cb_senddone (void * arg)
{
  Gather * g = (Gather *) arg;
  g->_complete++;
}

/* **************************************************************** */
/*            incoming active message                               */
/* **************************************************************** */

__pgasrt_local_addr_t TSPColl::Gather::
cb_incoming (const struct __pgasrt_AMHeader_t * hdr,
	     void (** completionHandler)(void *, void *),
	     void ** arg)
{
  struct gather_header * header = (struct gather_header *) hdr;
  void * base0 = TagList::find (header->tag, header->id);
  if (base0 == NULL)
    CCMI_FATALERROR (-1, "%d: Gather/v: <%d,%d> is undefined",
                         PGASRT_MYNODE, header->tag, header->id);
  Gather * g = (Gather * ) ((char *)base0 + header->tagoff);
  *completionHandler = &Gather::cb_recvcomplete;
  *arg = g;
  return (__pgasrt_local_addr_t) g->_rbuf + header->offset;
}

/* **************************************************************** */
/*           active message reception complete                      */
/* **************************************************************** */

void TSPColl::Gather::cb_recvcomplete (void * unused, void * arg)
{
  Gather * g = (Gather *) arg;
  if (--g->_incoming <= 0) g->_complete++;
}
