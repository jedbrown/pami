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
 * \file algorithms/protocols/tspcoll/Gather.h
 * \brief ???
 */

#ifndef __algorithms_protocols_tspcoll_Gather_h__
#define __algorithms_protocols_tspcoll_Gather_h__

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "interface/Communicator.h"
#include "algorithms/protocols/tspcoll/Barrier.h"

/* **************************************************************** */
/*                      Gather                                     */
/* **************************************************************** */

namespace TSPColl
{

  class Gather: public NBColl
  {
    public:
      void * operator new (size_t, void * addr)    { return addr; }
      Gather       (Communicator *comm, NBTag tag, int id, int tagoff);
      void reset   (int root, const void * sbuf, void * rbuf, size_t length);
      virtual void kick    (void);
      virtual bool isdone  (void) const { return _complete >= _counter; }
    protected:
      const void    * _sbuf;         /* send buffer    */
      void          * _rbuf;         /* receive buffer */
      size_t          _length;       /* msg length     */
      int             _root;         /* root thread ID */
      int             _incoming;     /* how many messages I got */

      int             _commID;       /* communicator   */
      unsigned        _counter;      /* instance counter */
      unsigned        _complete;     /* instance completion counter */

      struct gather_header
      {
        __pgasrt_AMHeader_t hdr;
        int                 tag;
        int                 id;
        int                 tagoff;
        unsigned            counter;
        size_t              offset;   /* message offset in recv buffer */
      }
      _header __attribute__((__aligned__(16)));

    protected:
      static void cb_senddone (void *arg);
      static __pgasrt_local_addr_t
      cb_incoming (const struct __pgasrt_AMHeader_t *,
                   void (**)(void *, void *), void **);
      static void cb_recvcomplete (void * unused, void * arg);
      Barrier _barrier;
  };
};


/* **************************************************************** */
/*                    Gatherv                                      */
/* **************************************************************** */

namespace TSPColl
{
  class Gatherv: public Gather
  {
    public:
      void * operator new (size_t, void * addr)    { return addr; }
      Gatherv (int comm, int tag, int id, int tagoff):
          Gather(comm, tag, id, tagoff) { }
      void reset (int root, const void * sbuf, void * rbuf, size_t * lengths);
  };
};

#endif
