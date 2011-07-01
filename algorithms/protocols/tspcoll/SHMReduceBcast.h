/**
 * \file algorithms/protocols/tspcoll/SHMReduceBcast.h
 * \brief ???
 */
#ifndef __algorithms_protocols_tspcoll_SHMReduceBcast_h__
#define __algorithms_protocols_tspcoll_SHMReduceBcast_h__

#include "algorithms/protocols/tspcoll/Collective.h"
#include "algorithms/protocols/tspcoll/shm_collectives.h"

#ifdef XLPGAS_PAMI_CAU

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
  class SHMBcast: public Collective<T_NI>
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

class SHMLargeBcast: public Collective
   {
     typedef xlpgas::local::FixedLeader2LB<Wait> FL;
     FL  fl;
     const void* _sbuf;
     void*       _dbuf;
     unsigned    _nbytes;
   public:
     void * operator new (size_t, void * addr) { return addr; }
     SHMLargeBcast (int ctxt, Team * comm, CollectiveKind kind, int tag, int offset);
     
     virtual void reset (int rootindex,
                       const void         * sbuf, 
                       void               * dbuf,
                       unsigned           nbytes);
     
     virtual void kick();
   }; /* ShmLargebcast */


} /* Xlpgas */

#include "algorithms/protocols/tspcoll/SHMReduceBcast.cc"

#endif // XLPGAS_PAMI_CAU

#endif /* __xlpgas_Shmreducebcast_h__ */
