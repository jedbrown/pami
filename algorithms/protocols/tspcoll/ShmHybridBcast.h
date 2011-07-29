/**
 * \file algorithms/protocols/tspcoll/ShmHybridBcast.h
 * \brief ???
 */
#ifndef __algorithms_protocols_tspcoll_ShmHybridBcast_h__
#define __algorithms_protocols_tspcoll_ShmHybridBcast_h__

#include "algorithms/protocols/tspcoll/Team.h"
#include "algorithms/protocols/tspcoll/CollExchange.h"

#undef TRACE
#ifdef DEBUG_COLL
#define TRACE(x)  fprintf x;
#else
#define TRACE(x)
#endif

namespace xlpgas
{
  template <class T_NI>
  class ShmHybridBcast : public Collective<T_NI>
  {
  public:
    void * operator new (size_t, void * addr) { return addr; }

    ShmHybridBcast (int ctxt, Team * comm, CollectiveKind kind, int tag, int offset, T_NI* ni) :
      Collective<T_NI> (ctxt, comm, kind, tag, NULL, NULL, ni) {
      //empty
    }

    virtual void reset (int root,
			const void         * sbuf, 
			void               * dbuf, 
			unsigned           nbytes
			);
    virtual void kick();

  }; /* ShmHybridBcast */
} /* Xlpgas */

#include "algorithms/protocols/tspcoll/ShmHybridBcast.cc"

#endif /* __xlpgas_ShmHybridBcast_h__ */
