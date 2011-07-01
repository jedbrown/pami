/**
 * \file algorithms/protocols/tspcoll/Allgatherv.h
 * \brief ???
 */
#ifndef __algorithms_protocols_tspcoll_Allgatherv_h__
#define __algorithms_protocols_tspcoll_Allgatherv_h__

#include "algorithms/protocols/tspcoll/CollExchange.h"

namespace xlpgas
{
  template <class T_NI>
  class Allgatherv: public CollExchange<T_NI>
  {
	public:
	  void * operator new (size_t, void * addr) { return addr; }
	  Allgatherv (int ctxt, Team * comm, CollectiveKind kind, int tag, int offset);
	  void reset (const void * sbuf,
		      void   * dbuf,
		      size_t * lengths);

	protected:
	  int           _logMaxBF;
	  void        * _dbuf;
	  char          _dummy;
	  void        * _tmpbuf;
	  size_t        _tmpbuflen;
	  size_t      * _lengths;
  }; /* Allgatherv */
} /* Xlpgas */

#include "algorithms/protocols/tspcoll/Allgatherv.cc"

#endif /* __xlpgas_Allgatherv_h__ */
