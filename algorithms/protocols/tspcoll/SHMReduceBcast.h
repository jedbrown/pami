#ifndef __xlpgas_Shmreduce_h__
#define __xlpgas_Shmreduce_h__

#include "algorithms/protocols/tspcoll/Collective.h"
#include "algorithms/protocols/tspcoll/shm_collectives.h"

namespace xlpgas
{
  template<class T_NI>
  class SHMReduce: public Collective<T_NI>
  {
    typedef xlpgas::local::FixedLeader<Wait> FL;
    FL  fl;
  public:
    void * operator new (size_t, void * addr) { return addr; }
    SHMReduce (int ctxt, Team * comm, CollectiveKind kind, int tag, int offset);
    
    virtual void reset (int rootindex,
			const void         * sbuf, 
			void               * dbuf,
			xlpgas_ops_t       op,
			xlpgas_dtypes_t    dt,
			unsigned           nelems,
			user_func_t*       uf);

	  
  }; /* Shmreduce */

  template<class T_NI>
  class SHMBcast: public Collective<class T_NI>
  {
    typedef xlpgas::local::FixedLeader<Wait> FL;
    FL  fl;
  public:
    void * operator new (size_t, void * addr) { return addr; }
    SHMBcast (int ctxt, Team * comm, CollectiveKind kind, int tag, int offset);
    
    virtual void reset (int rootindex,
			const void         * sbuf, 
			void               * dbuf,
			unsigned           nbytes);

	  
  }; /* Shmbcast */
} /* Xlpgas */

#include "algorithms/protocols/tspcoll/SHMReduceBcast.cc"

#endif /* __xlpgas_Shmreducebcast_h__ */
