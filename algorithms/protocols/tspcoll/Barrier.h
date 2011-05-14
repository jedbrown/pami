#ifndef __xlpgas_barrier_h__
#define __xlpgas_barrier_h__

#include "algorithms/protocols/tspcoll/CollExchange.h"
#include "algorithms/protocols/tspcoll/Team.h"

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

namespace xlpgas
{
  template <class T_NI>
  class Barrier: public CollExchange<T_NI>
  {
  public:
    void * operator new (size_t, void * addr)    { return addr; }
    Barrier (int                 ctxt,
	     Team              * comm,
	     CollectiveKind      kind,
	     int                 tag,
	     int                 offset);
    void reset () { CollExchange<T_NI>::reset(); }
  private:
    char _dummy;
  };
}

/* *********************************************************************** */
/*                 constructor                                             */
/* *********************************************************************** */
template <class T_NI>
inline xlpgas::Barrier<T_NI>::Barrier (int               ctxt,
				Team            * comm,
				CollectiveKind    kind,
				int               tag,
				int               offset) :
  CollExchange<T_NI> (ctxt, comm, kind, tag, offset)
{
  TRACE((stderr, "%d: Barrier constructor: rank=%d of %d\n",
	 XLPGAS_MYNODE, this->_comm->rank(), this->_comm->size()));

  this->_numphases = -1; for (int n=2*this->_comm->size()-1; n>0; n>>=1) this->_numphases++;
  this->_sendcomplete = this->_numphases;
  this->_phase        = this->_numphases;

  /* --------------------------------------------------- */
  /* initialize destinations, offsets and buffer lengths */
  /* --------------------------------------------------- */

  for (int i=0; i<this->_numphases; i++)
    {
      this->_dest[i]      = comm->endpoint((comm->ordinal()+(1<<i))%comm->size());

      this->_sbuf[i]      = &this->_dummy;
      this->_sbufln[i]    = 1;
      this->_rbuf[i]      = &this->_dummy;
    }
}

#endif
