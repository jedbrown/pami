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
#include "algorithms/ccmi.h" // for XMI_Request_t
#include "./NBColl.h"

/* **************************************************************** */
/*                      Scatter                                     */
/* **************************************************************** */

namespace TSPColl
{
  template <class T_Mcast>
  class Scatter: public NBColl<T_Mcast>
  {
  protected:
    static const int MAX_CONCURRENCY=10;

  public:
    void * operator new (size_t, void * addr)    { return addr; }
    Scatter      (XMI::Geometry::Geometry<XMI_GEOMETRY_CLASS> * comm, NBTag tag, int instID, int tagoff);
    void reset   (int root, const void * sbuf, void * rbuf, size_t length);
    virtual void kick    (T_Mcast *mcast_iface);
    virtual bool isdone  (void) const { return _complete >= _counter; }
    static void amsend_reg  (T_Mcast *mcast_iface);
  protected:
    XMI_Request_t                       *_req;
    XMI_Request_t                       _rreq;
    T_Mcast       *_mcast_iface;
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
    static  XMI_Request_t * cb_incoming(const xmi_quad_t  * hdr,
					 unsigned          count,
					 unsigned          peer,
					 unsigned          sndlen,
					 unsigned          conn_id,
					 void            * arg,
					 unsigned        * rcvlen,
					 char           ** rcvbuf,
					 unsigned        * pipewidth,
					 XMI_Callback_t * cb_done);
    static void cb_recvcomplete (void *arg, xmi_result_t*err);
    static void cb_senddone (void *);
  };
};
template <class T_Mcast>
inline void TSPColl::Scatter<T_Mcast>::amsend_reg  (T_Mcast *mcast_iface)
    {
      mcast_iface->setCallback(TSPColl::Scatter<T_Mcast>::cb_incoming, NULL);
    }
/* **************************************************************** */
/*                    Scatterv                                      */
/* **************************************************************** */

namespace TSPColl
{
  template <class T_Mcast>
  class Scatterv: public Scatter<T_Mcast>
  {
  public:
    void * operator new (size_t, void * addr)    { return addr; }
    Scatterv (XMI::Geometry::Geometry<XMI_GEOMETRY_CLASS> * comm, NBTag tag, int instID, int tagoff):
      Scatter<T_Mcast> (comm, tag, instID, tagoff), _lengths(0) { }
    void reset (int root, const void * sbuf, void * rbuf, size_t * lengths);
    virtual void kick (T_Mcast *mcast_iface);
  protected:
    size_t * _lengths;  
  };
};

#endif
