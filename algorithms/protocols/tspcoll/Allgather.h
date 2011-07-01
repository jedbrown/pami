/**
 * \file algorithms/protocols/tspcoll/Allgather.h
 * \brief ???
 */
#ifndef __algorithms_protocols_tspcoll_Allgather_h__
#define __algorithms_protocols_tspcoll_Allgather_h__

#include "algorithms/protocols/tspcoll/CollExchange.h"

namespace xlpgas
{
  template <class T_NI>
  class Allgather: public CollExchange<T_NI>
  {
	public:
	  void * operator new (size_t, void * addr) { return addr; }
	  Allgather (int ctxt, Team * comm, CollectiveKind kind, int tag, int offset);
	  void reset (const void * sbuf,
		      void * dbuf,
		      unsigned nbytes);

	protected:
	  int           _logMaxBF;
	  unsigned      _nbytes;
	  void        * _dbuf;
	  char          _dummy;
	  void        * _tmpbuf;
	  size_t        _tmpbuflen;
  }; /* Allgather */
} /* Xlpgas */

#include "algorithms/protocols/tspcoll/Allgather.cc"

#endif /* __xlpgas_Allgather_h__ */
