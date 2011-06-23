#ifndef __xlpgas_QuadnomBcast_h__
#define __xlpgas_QuadnomBcast_h__

#if 0
#include "algorithms/protocols/tspcoll/CollExchangeX.h"

namespace xlpgas
{
  template <class T_NI>
  class QuadnomBcast: public CollExchangeX<T_NI, 3>
  {
  public:
    void * operator new (size_t, void * addr) { return addr; }
    QuadnomBcast (int ctxt, 
		  Team * comm, 
		  CollectiveKind kind, int tag, int offset);
    virtual void reset (int root,
			const void * sbuf,
			void * dbuf,
			unsigned nbytes);
    
  protected:
    int _logN, _qN;
    char          _dummy;
  }; /* QuadnomBcast */
} /* xlpgas */

#include "algorithms/protocols/tspcoll/QuadnomBcast.cc"
#endif

#endif /* __xlpgas_QuadnomBcast_h__ */
