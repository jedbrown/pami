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
 * \file algorithms/protocols/tspcoll/ScBcast.h
 * \brief ???
 */

#ifndef __algorithms_protocols_tspcoll_ScBcast_h__
#define __algorithms_protocols_tspcoll_ScBcast_h__

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "algorithms/protocols/tspcoll/Scatter.h"
#include "algorithms/protocols/tspcoll/Allgatherv.h"
#include "algorithms/protocols/tspcoll/Barrier.h"
#include "util/common.h"
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

  template <class T_NI>
  class ScBcast: public NBColl<T_NI>
  {
    public:
      void * operator new (size_t, void * addr) { return addr; }
      ScBcast (PAMI_GEOMETRY_CLASS * comm, NBTag tag, int instID, int tagoff);
      void reset (int root, const void * sbuf, void *buf, size_t);
      virtual void kick (T_NI *p2p_iface);
      virtual bool isdone (void) const;
      static void amsend_reg  (T_NI *p2p_iface, void *cd);
    protected:
      T_NI *_p2p_iface;
    private:
      size_t             *_lengths;
      Scatterv<T_NI>   _scatterv;
      Barrier<T_NI>    _barrier;
      Barrier<T_NI>    _barrier2;
      Barrier<T_NI>    _barrier3;
      Allgatherv<T_NI> _allgatherv;

    private:
      static void scattercomplete(pami_context_t context, void *arg, pami_result_t res);
      static void barriercomplete(pami_context_t context, void *arg, pami_result_t res);
      static void barrier2complete(pami_context_t context, void *arg, pami_result_t res);
      static void barrier3complete(pami_context_t context, void *arg, pami_result_t res);
      static void allgathervcomplete(pami_context_t context, void *arg, pami_result_t res);
  };
}

/* *********************************************************************** */
/*                    broadcast constructor                                */
/* *********************************************************************** */
template <class T_NI>
inline TSPColl::ScBcast<T_NI>::
ScBcast(PAMI_GEOMETRY_CLASS * comm, NBTag tag, int instID, int tagoff) :
    NBColl<T_NI> (comm, tag, instID, NULL, NULL),
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


  pami_result_t prc;
  prc = __global.heap_mm->memalign((void **)&_lengths, 0,
					sizeof(*_lengths) * comm->size());
  PAMI_assertf(prc == PAMI_SUCCESS, "alloc of _lengths[%d] failed", comm->size());
  _scatterv.setComplete (scattercomplete, this);
  _barrier.setComplete  (barriercomplete, this);
  _barrier2.setComplete (barrier2complete, this);
  _barrier3.setComplete (barrier3complete, this);
  _allgatherv.setComplete (allgathervcomplete, this);
  PAMI_assert(_lengths != NULL);
}

/* *********************************************************************** */
/*               reset the broadcast                                       */
/* *********************************************************************** */

template <class T_NI>
inline void TSPColl::ScBcast<T_NI>::
reset (int root, const void * sbuf, void *rbuf, size_t len)
{
  int myoffset = -1;
  size_t pernodelen = CEIL (len, this->_comm->size());
  int rank = this->_comm->virtrank();

  for (int i = 0, current = 0; i < this->_comm->size(); i++)
    {
      if (rank == i) myoffset = current;

      current += (_lengths[i] = MIN (pernodelen, len - current));
    }

  PAMI_assert(myoffset != -1);
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
template <class T_NI>
inline void TSPColl::ScBcast<T_NI>::kick (T_NI *p2p_iface)
{
  TRACE((stderr, "%d: SCBCAST kick\n", PGASRT_MYNODE));
  _p2p_iface = p2p_iface;
  _barrier.kick(p2p_iface);
}

/* *********************************************************************** */
/*               first phase is complete: start allgather                  */
/* *********************************************************************** */
template <class T_NI>
inline void TSPColl::ScBcast<T_NI>::scattercomplete(pami_context_t context, void *arg, pami_result_t res)
{
  ScBcast * self = (ScBcast *) arg;
  TRACE((stderr, "%d: SCBCAST scattercomplete\n", PGASRT_MYNODE));
  PAMI_assert(self != NULL);
  // self->_barrier2.kick();
  self->_allgatherv.kick(self->_p2p_iface);
}

/* *********************************************************************** */
/* *********************************************************************** */
template <class T_NI>
inline void TSPColl::ScBcast<T_NI>::barriercomplete(pami_context_t context, void *arg, pami_result_t res)
{
  ScBcast * self = (ScBcast *) arg;
  TRACE((stderr, "%d: SCBCAST barriercomplete\n", PGASRT_MYNODE));
  PAMI_assert(self != NULL);
  self->_scatterv.kick(self->_p2p_iface);
}

/* *********************************************************************** */
/* *********************************************************************** */
template <class T_NI>
inline void TSPColl::ScBcast<T_NI>::barrier2complete(pami_context_t context, void *arg, pami_result_t res)
{
  ScBcast * self = (ScBcast *) arg;
  TRACE((stderr, "%d: SCBCAST barrier2complete\n", PGASRT_MYNODE));
  PAMI_assert(self != NULL);
  self->_allgatherv.kick(self->_p2p_iface);
}

/* *********************************************************************** */
/* *********************************************************************** */
template <class T_NI>
inline void TSPColl::ScBcast<T_NI>::barrier3complete(pami_context_t context, void *arg, pami_result_t res)
{
  TRACE((stderr, "%d: SCBCAST barrier3complete\n", PGASRT_MYNODE));
  // ScBcast * self = (ScBcast *) arg;
}

/* *********************************************************************** */
/* *********************************************************************** */
template <class T_NI>
inline void TSPColl::ScBcast<T_NI>::allgathervcomplete(pami_context_t context, void *arg, pami_result_t res)
{
  PAMI_assert(arg != NULL);
  TRACE((stderr, "%d: SCBCAST agvcomplete\n", PGASRT_MYNODE));
  // ScBcast * self = (ScBcast *) arg;
  // self->_barrier3.kick(_p2p_iface);
}

/* *********************************************************************** */
/* *********************************************************************** */
template <class T_NI>
inline bool TSPColl::ScBcast<T_NI>::isdone (void) const
{
  return _allgatherv.isdone();
  // return _barrier3.isdone();
}
template <class T_NI>
inline void TSPColl::ScBcast<T_NI>::amsend_reg  (T_NI *p2p_iface, void *cd)
{
  PAMI_abort();
  //   p2p_iface->setCallback(cb_incoming, cd);
  // __pgasrt_tsp_amsend_reg (PGASRT_TSP_AMSEND_COLLEXCHANGE, cb_incoming);
}

#endif
