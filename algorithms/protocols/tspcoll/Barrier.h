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
 * \file algorithms/protocols/tspcoll/Barrier.h
 * \brief ???
 */

#ifndef __algorithms_protocols_tspcoll_Barrier_h__
#define __algorithms_protocols_tspcoll_Barrier_h__

#include "algorithms/protocols/tspcoll/CollExchange.h"

#undef TRACE
//#define DEBUG_TSPCOLL_BARRIER 1
#ifdef DEBUG_TSPCOLL_BARRIER
#define TRACE(x)  fprintf x;
#else
#define TRACE(x)
#endif

/* *********************************************************************** */
/*                 Pairwise exchange barrier                               */
/* *********************************************************************** */

namespace TSPColl
{
  template <class T_Mcast>
  class Barrier: public CollExchange<T_Mcast>
  {
  public:
    void * operator new (size_t, void * addr)    { return addr; }
    Barrier (XMI_GEOMETRY_CLASS * comm, NBTag tag, int instID, int offset);
    void reset () { CollExchange<T_Mcast>::reset(); }
  private:
    char _dummy;
  };
};

/* *********************************************************************** */
/*                 constructor                                             */
/* *********************************************************************** */

template <class T_Mcast>
inline TSPColl::Barrier<T_Mcast>::
Barrier (XMI_GEOMETRY_CLASS * comm, NBTag tag, int instID, int offset) :
  CollExchange<T_Mcast> (comm, tag, instID, offset, false)
{
  this->_numphases = -1; for (int n=2*this->_comm->size()-1; n>0; n>>=1) this->_numphases++;
  this->_sendcomplete = this->_numphases;
  this->_phase        = this->_numphases;

  /* --------------------------------------------------- */
  /* initialize destinations, offsets and buffer lengths */
  /* --------------------------------------------------- */

  for (int i=0; i<this->_numphases; i++)
    {
      this->_dest[i]      = comm->absrankof((this->_comm->virtrank() + (1<<i))%this->_comm->size());
      this->_sbuf[i]      = &_dummy;
      this->_sbufln[i]    = 1;
      this->_rbuf[i]      = &_dummy;
    }

}

#endif
