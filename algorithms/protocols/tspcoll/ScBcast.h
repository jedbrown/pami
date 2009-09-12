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

#ifndef __tspcoll_scatter_bcast_h__
#define __tspcoll_scatter_bcast_h__

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "algorithms/protocols/tspcoll/Scatter.h"
#include "algorithms/protocols/tspcoll/Allgatherv.h"
#include "algorithms/protocols/tspcoll/Barrier.h"
// #define DEBUG_SCBCAST 1
#undef TRACE
#ifdef DEBUG_SCBCAST
#define TRACE(x)  fprintf x;
#else
#define TRACE(x)
#endif

/* *********************************************************************** */
/*                 scatter-allgather broadcast                             */
/* *********************************************************************** */

namespace TSPColl
{

  template <class T_Mcast>
  class ScBcast: public NBColl<T_Mcast>
  {
  public:
    void * operator new (size_t, void * addr) { return addr; }
    ScBcast (XMI_GEOMETRY_CLASS * comm, NBTag tag, int instID, int tagoff);
    void reset (int root, const void * sbuf, void *buf, size_t);
    virtual void kick (T_Mcast *mcast_iface);
    virtual bool isdone (void) const;
    static void amsend_reg  (T_Mcast *mcast_iface);
  protected:
    T_Mcast *_mcast_iface;
  private:
    size_t             *_lengths;
    Scatterv<T_Mcast>   _scatterv;
    Barrier<T_Mcast>    _barrier;
    Barrier<T_Mcast>    _barrier2;
    Barrier<T_Mcast>    _barrier3;
    Allgatherv<T_Mcast> _allgatherv;

  private:

    static void scattercomplete (void *arg);
    static void barriercomplete (void * arg);
    static void barrier2complete (void * arg);
    static void barrier3complete (void * arg);
    static void allgathervcomplete (void * arg);
  };
}

/* *********************************************************************** */
/*                    broadcast constructor                                */
/* *********************************************************************** */
template <class T_Mcast>
inline TSPColl::ScBcast<T_Mcast>::
ScBcast(XMI_GEOMETRY_CLASS * comm, NBTag tag, int instID, int tagoff) :
               NBColl<T_Mcast> (comm, tag, instID, NULL, NULL),
	       _scatterv (comm, tag, instID, 
			  (size_t)&_scatterv - (size_t) this + tagoff),
	       _barrier (comm, tag, instID,
			 (size_t)&_barrier - (size_t) this + tagoff),
	       _barrier2 (comm, tag, instID,
			  (size_t)&_barrier2 - (size_t) this + tagoff),
	       _barrier3 (comm, tag, instID,
			  (size_t)&_barrier3 - (size_t) this + tagoff),
	       _allgatherv (comm, tag, instID,
			    (size_t)&_allgatherv - (size_t) this + tagoff)
{
  TRACE((stderr, "%d: SCBCAST<%d,%d> constr. this=%p sc=%p ag=%p\n",
	 PGASRT_MYNODE, tag, instID, this, &_scatterv, &_allgatherv));


  _lengths = (size_t *) malloc (sizeof(size_t) * comm->size());
  _scatterv.setComplete (scattercomplete, this);
  _barrier.setComplete  (barriercomplete, this);
  _barrier2.setComplete (barrier2complete, this);
  _barrier3.setComplete (barrier3complete, this);
  _allgatherv.setComplete (allgathervcomplete, this);
  assert (_lengths != NULL);
}

/* *********************************************************************** */
/*               reset the broadcast                                       */
/* *********************************************************************** */

#define CEIL(x,y) (((x)+(y)-1)/(y))
#define MIN(x,y) (((x)<(y))?(x):(y))

template <class T_Mcast>
inline void TSPColl::ScBcast<T_Mcast>::
reset (int root, const void * sbuf, void *rbuf, size_t len)
{
  int myoffset = -1;
  size_t pernodelen = CEIL (len, this->_comm->size());
  for (int i=0, current = 0; i<this->_comm->size(); i++) 
    {
      if (this->_comm->rank() == i) myoffset = current;
      current += (_lengths[i] = MIN (pernodelen, len - current));
    }

  assert (myoffset != -1);
  TRACE((stderr, "%d: SCBCAST reset (root=%d sbuf=%p rbuf=%p len=%d)\n",
	 PGASRT_MYNODE, root, sbuf, rbuf, len));
  _scatterv.reset (root, sbuf, (char *)rbuf + myoffset, this->_lengths);
  _allgatherv.reset ((char *)rbuf + myoffset, rbuf, this->_lengths);
  _barrier.reset ();
  _barrier2.reset();
  _barrier3.reset();
}

/* *********************************************************************** */
/*              start the broadcast rolling                                */
/* *********************************************************************** */
template <class T_Mcast>
inline void TSPColl::ScBcast<T_Mcast>::kick (T_Mcast *mcast_iface)
{
  TRACE((stderr, "%d: SCBCAST kick\n", PGASRT_MYNODE));
  _mcast_iface = mcast_iface;
  _barrier.kick(mcast_iface);
}

/* *********************************************************************** */
/*               first phase is complete: start allgather                  */
/* *********************************************************************** */
template <class T_Mcast>
inline void TSPColl::ScBcast<T_Mcast>::scattercomplete(void *arg)
{
  ScBcast * self = (ScBcast *) arg;
  TRACE((stderr, "%d: SCBCAST scattercomplete\n", PGASRT_MYNODE));
  assert (self != NULL);
  // self->_barrier2.kick();
  self->_allgatherv.kick(self->_mcast_iface);
}

/* *********************************************************************** */
/* *********************************************************************** */
template <class T_Mcast>
inline void TSPColl::ScBcast<T_Mcast>::barriercomplete(void *arg)
{
  ScBcast * self = (ScBcast *) arg;
  TRACE((stderr, "%d: SCBCAST barriercomplete\n", PGASRT_MYNODE));
  assert (self != NULL);
  self->_scatterv.kick(self->_mcast_iface);
}

/* *********************************************************************** */
/* *********************************************************************** */
template <class T_Mcast>
inline void TSPColl::ScBcast<T_Mcast>::barrier2complete(void *arg)
{
  ScBcast * self = (ScBcast *) arg;
  TRACE((stderr, "%d: SCBCAST barrier2complete\n", PGASRT_MYNODE));
  assert (self != NULL);
  self->_allgatherv.kick(self->_mcast_iface);
}

/* *********************************************************************** */
/* *********************************************************************** */
template <class T_Mcast>
inline void TSPColl::ScBcast<T_Mcast>::barrier3complete(void *arg)
{
  TRACE((stderr, "%d: SCBCAST barrier3complete\n", PGASRT_MYNODE));
  // ScBcast * self = (ScBcast *) arg;
}

/* *********************************************************************** */
/* *********************************************************************** */
template <class T_Mcast>
inline void TSPColl::ScBcast<T_Mcast>::allgathervcomplete(void *arg)
{
  ScBcast * self = (ScBcast *) arg;
  TRACE((stderr, "%d: SCBCAST agvcomplete\n", PGASRT_MYNODE));
  assert (self != NULL);
  // self->_barrier3.kick(_mcast_iface);
}

/* *********************************************************************** */
/* *********************************************************************** */
template <class T_Mcast>
inline bool TSPColl::ScBcast<T_Mcast>::isdone (void) const
{
  return _allgatherv.isdone();
  // return _barrier3.isdone();
}
template <class T_Mcast>
inline void TSPColl::ScBcast<T_Mcast>::amsend_reg  (T_Mcast *mcast_iface)
{
  assert(0);
  //  mcast_iface->setCallback(cb_incoming, NULL);
  // __pgasrt_tsp_amsend_reg (PGASRT_TSP_AMSEND_COLLEXCHANGE, cb_incoming);
}

#endif
