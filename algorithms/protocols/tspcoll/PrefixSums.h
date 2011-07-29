/**
 * \file algorithms/protocols/tspcoll/PrefixSums.h
 * \brief ???
 */
#ifndef __algorithms_protocols_tspcoll_PrefixSums_h__
#define __algorithms_protocols_tspcoll_PrefixSums_h__

#include "algorithms/protocols/tspcoll/CollExchange.h"
#include "algorithms/protocols/tspcoll/Allreduce.h"

namespace xlpgas
{
  template <class T_NI>
  class PrefixSums: public CollExchange<T_NI>
  {
	public:
	  void * operator new (size_t, void * addr) { return addr; }
	  PrefixSums (int ctxt, Team * comm, CollectiveKind kind, int tag, int offset, T_NI*);
          ~PrefixSums()
            {
              if(_tmpbuf)
                __global.heap_mm->free(_tmpbuf);
            }

	  void reset (const void         * sbuf,
		      void               * dbuf,
		      pami_data_function   op,
		      TypeCode           * dt,
		      size_t               nelems);
	  static void cb_prefixsums (CollExchange<T_NI> *coll, unsigned phase);

          void setExclusive(int exclusive) { _exclusive = exclusive; }

	protected:
	  int           _logMaxBF;
	  size_t        _nelems;
	  void        * _dbuf;
	  char          _dummy;
	  void        * _tmpbuf;
	  size_t        _tmpbuflen;
	  coremath      _cb_prefixsums;
	  user_func_t    *_uf;
	  TypeCode       *_dt;
	  int           _exclusive; // 0 = Inclusive scan, 1 = Exclusive scan

  }; /* PrefixSums */
} /* Xlpgas */

#include "algorithms/protocols/tspcoll/PrefixSums.cc"

#endif /* __xlpgas_PrefixSums_h__ */
