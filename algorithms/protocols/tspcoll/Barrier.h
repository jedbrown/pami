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

#ifndef __tspcoll_barrier_h__
#define __tspcoll_barrier_h__

#include "./CollExchange.h"
#include "collectives/interface/Communicator.h"

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
  class Barrier: public CollExchange
  {
  public:
    void * operator new (size_t, void * addr)    { return addr; }
    Barrier (Communicator * comm, NBTag tag, int instID, int offset);
    void reset () { CollExchange::reset(); }
  private:
    char _dummy;
  };
};

/* *********************************************************************** */
/*                 constructor                                             */
/* *********************************************************************** */

inline TSPColl::Barrier::
Barrier (Communicator * comm, NBTag tag, int instID, int offset) :
	       CollExchange (comm, tag, instID, offset, false)
{
  TRACE((stderr, "%d: Barrier constructor: rank=%d of %d\n", 
	 PGASRT_MYNODE, _comm->rank(), _comm->size()));
  
  this->_numphases = -1; for (int n=2*this->_comm->size()-1; n>0; n>>=1) this->_numphases++;
  this->_sendcomplete = this->_numphases;
  this->_phase        = this->_numphases;
  
  /* --------------------------------------------------- */
  /* initialize destinations, offsets and buffer lengths */
  /* --------------------------------------------------- */

  for (int i=0; i<this->_numphases; i++)
    {
      this->_dest[i]      = comm->absrankof((this->_comm->rank() + (1<<i))%this->_comm->size());
      this->_sbuf[i]      = &_dummy;
      this->_sbufln[i]    = 1;
      this->_rbuf[i]      = &_dummy;
    }

}

#endif
