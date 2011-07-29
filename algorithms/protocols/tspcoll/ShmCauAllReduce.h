/**
 * \file algorithms/protocols/tspcoll/ShmCauAllReduce.h
 * \brief ???
 */
#ifndef __algorithms_protocols_tspcoll_ShmCauAllReduce_h__
#define __algorithms_protocols_tspcoll_ShmCauAllReduce_h__

#include "algorithms/protocols/tspcoll/Team.h"
#include "algorithms/protocols/tspcoll/CollExchange.h"
#include "algorithms/protocols/tspcoll/cau_collectives.h"

#undef TRACE
#ifdef DEBUG_COLL
#define TRACE(x)  fprintf x;
#else
#define TRACE(x)
#endif

namespace xlpgas
{
  template <class T_NI, class T_Device>
  class ShmCauAllReduce : public Collective<T_NI>
  {
  public:

    void * operator new (size_t, void * addr) { return addr; }

    ShmCauAllReduce (int ctxt, Team * comm, CollectiveKind kind, int tag, int offset, void* device_info, T_NI* ni) :
      Collective<T_NI> (ctxt, comm, kind, tag, NULL, NULL, ni) {
      this->_device_info = device_info;
    }
    virtual void reset (const void         * sbuf, 
			void               * dbuf, 
			pami_data_function   op,
			TypeCode           * sdt,
			unsigned             nelems,
                        TypeCode           * rdt,
			user_func_t        * uf
			);

    virtual void kick();

    virtual bool isdone           () const;
    virtual void setComplete (xlpgas_LCompHandler_t cb,
			      void *arg);
    virtual void setContext (pami_context_t ctxt);
  public:
    xlpgas::Collective<T_NI> *shm_reduce, *shm_bcast, *cau_reduce, *cau_bcast;
    int64_t s;
    int64_t tmp;
    int64_t tmp_cau;

  }; /* ShmCauAllReduce */
} /* Xlpgas */

#include "algorithms/protocols/tspcoll/ShmCauAllReduce.cc"

#endif /* __xlpgas_ShmCauAllReduce_h__ */
