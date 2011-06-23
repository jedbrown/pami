#ifndef __xlpgas_ShmCauAllReduce_h__
#define __xlpgas_ShmCauAllReduce_h__

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
  class ShmCauAllReduce : public Collective<T_NI>
  {
  public:
    void * operator new (size_t, void * addr) { return addr; }

    ShmCauAllReduce (int ctxt, Team * comm, CollectiveKind kind, int tag, int offset) :
    Collective<T_NI> (ctxt, comm, kind, tag, NULL, NULL) {
      //empty
    }
    virtual void reset (const void         * sbuf, 
			void               * dbuf, 
			xlpgas_ops_t       op,
			xlpgas_dtypes_t    dt,
			unsigned           nelems,
			user_func_t* uf
			);
  }; /* ShmCauAllReduce */
} /* Xlpgas */

#include "algorithms/protocols/tspcoll/ShmCauAllReduce.cc"

#endif /* __xlpgas_ShmCauAllReduce_h__ */

