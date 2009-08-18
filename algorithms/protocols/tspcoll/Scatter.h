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

#ifndef __tspcoll_scatter_h__
#define __tspcoll_scatter_h__

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "./NBColl.h"
#include "interface/Communicator.h"
/* **************************************************************** */
/*                      Scatter                                     */
/* **************************************************************** */

namespace TSPColl
{
  class Scatter: public NBColl
  {
  protected:
    static const int MAX_CONCURRENCY=10;

  public:
    void * operator new (size_t, void * addr)    { return addr; }
    Scatter      (Communicator * comm, NBTag tag, int instID, int tagoff);
    void reset   (int root, const void * sbuf, void * rbuf, size_t length);
    virtual void kick    (CCMI::MultiSend::OldMulticastInterface *mcast_iface);
    virtual bool isdone  (void) const { return _complete >= _counter; }
    static void amsend_reg  (CCMI::MultiSend::OldMulticastInterface *mcast_iface);
  protected:
    XMI_Request_t                       *_req;
    XMI_Request_t                       _rreq;
    CCMI::MultiSend::OldMulticastInterface *_mcast_iface;
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
    static  XMI_Request_t * cb_incoming(const CMQuad  * hdr,
					 unsigned          count,
					 unsigned          peer,
					 unsigned          sndlen,
					 unsigned          conn_id,
					 void            * arg,
					 unsigned        * rcvlen,
					 char           ** rcvbuf,
					 unsigned        * pipewidth,
					 XMI_Callback_t * cb_done);
    static void cb_recvcomplete (void *arg, XMI_Error_t*err);
    static void cb_senddone (void *);
  };
};

inline void TSPColl::Scatter::amsend_reg  (CCMI::MultiSend::OldMulticastInterface *mcast_iface)
    {
      mcast_iface->setCallback(TSPColl::Scatter::cb_incoming, NULL);
    }
/* **************************************************************** */
/*                    Scatterv                                      */
/* **************************************************************** */

namespace TSPColl
{
  class Scatterv: public Scatter
  {
  public:
    void * operator new (size_t, void * addr)    { return addr; }
    Scatterv (Communicator * comm, NBTag tag, int instID, int tagoff):
              Scatter (comm, tag, instID, tagoff), _lengths(0) { }
    void reset (int root, const void * sbuf, void * rbuf, size_t * lengths);
    virtual void kick (CCMI::MultiSend::OldMulticastInterface *mcast_iface);
  protected:
    size_t * _lengths;  
  };
};

#endif
