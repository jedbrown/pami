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
/**
 * \file algorithms/protocols/tspcoll/BlockingBarrier.h
 * \brief ???
 */

#ifndef __tspcoll_blockingbarrier_h__
#define __tspcoll_blockingbarrier_h__

#include "interface/Communicator.h"
#include <assert.h>
#include <stdio.h>

/* *********************************************************************** */
/*                 Pairwise exchange barrier                               */
/* *********************************************************************** */

namespace TSPColl
{
  class BlockingBarrier
  {
  private:
    static const int MAX_PHASES=32;

  public:
    void * operator new (size_t, void * addr)    { return addr; }
    BlockingBarrier (Communicator * comm, int tag, int instanceID);
    void execute();

  private:
    static __pgasrt_local_addr_t
      cb_incoming (const struct __pgasrt_AMHeader_t * hdr,
		   void (** cH)(void *, void *),
		   void ** arg);


  private:
    int _counter;
    int _numphases;
    struct AMHeader
    {
      __pgasrt_AMHeader_t hdr;
      int                 tag;           /* tag in tag list */
      int                 id;            /* instance id in tag list */
      int                 counter;
      int                 phase;
    }
    _header [MAX_PHASES] __attribute__((__aligned__(16)));
    int _dest[MAX_PHASES];
    int _recvcomplete[MAX_PHASES];
  };
};

/* *********************************************************************** */
/*                 constructor                                             */
/* *********************************************************************** */

inline TSPColl::BlockingBarrier::
BlockingBarrier (Communicator * comm, int tag, int id)
{
  assert (comm != NULL);
  _numphases = -1; for (int n=2*comm->size()-1; n>0; n>>=1) _numphases++;
  for (int i=0; i<_numphases; i++)
    {
      _dest[i] = comm->absrankof((comm->rank() + (1<<i)) % comm->size());
      _recvcomplete[i] = 0;
      _header[i].hdr.handler   = cb_incoming;
      _header[i].hdr.headerlen = sizeof (struct AMHeader);
      _header[i].tag           = tag;
      _header[i].id            = id;
      _header[i].counter       = 0;
      _header[i].phase         = i;
    }
  _counter = 0;
}

/* *********************************************************************** */
/*                     send an active message                              */
/* *********************************************************************** */

inline void TSPColl::BlockingBarrier::execute (CCMI::MultiSend::OldMulticastInterface *mcast_iface)
{
  _counter ++;
  for (int i=0; i<_numphases; i++)
    {
      _header[i].counter = _counter;
#if 0
      void * r = __pgasrt_tsp_amsend (_dest[i],
				      & _header[i].hdr,
				      NULL, 0,
				      NULL, NULL);
#endif
      unsigned        hints   = CCMI_PT_TO_PT_SUBTASK;
      unsigned        ranks   = _dest[i];
      XMI_Callback_t cb_done;
      cb_done.function        = NULL;
      cb_done.clientdata     =  NULL;

      void * r = mcast_iface->send (&_req,
				    cb_done,
				    CCMI_MATCH_CONSISTENCY,
				    & _header[phase].hdr,
				    _counter,
				    NULL,
				    0,
				    &hints,
				    &ranks,
				    1);


      __pgasrt_tsp_wait (r);
      while (_recvcomplete[i] < _counter) __pgasrt_tsp_wait (NULL);
    }
}

/* *********************************************************************** */
/*                   incoming active message                               */
/* *********************************************************************** */

inline __pgasrt_local_addr_t TSPColl::BlockingBarrier::
cb_incoming (const struct __pgasrt_AMHeader_t * hdr,
             void (** completionHandler)(void *, void *),
             void ** arg)
{
  struct AMHeader * h = (struct AMHeader *) hdr;
  // BlockingBarrier * b = (BlockingBarrier *)TagList::get (h->tag, h->id);
  BlockingBarrier * b = NULL;
  assert (b != NULL);
  assert (b->_header[0].id == h->id);
  assert (h->phase < b->_numphases);
  b->_recvcomplete[h->phase]++;
  *completionHandler = NULL;
  *arg = NULL;
  return NULL;
}


#endif
