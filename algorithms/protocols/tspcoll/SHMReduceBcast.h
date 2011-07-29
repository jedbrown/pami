/**
 * \file algorithms/protocols/tspcoll/SHMReduceBcast.h
 * \brief ???
 */

#ifndef __algorithms_protocols_tspcoll_Shmreduce_h__
#define __algorithms_protocols_tspcoll_Shmreduce_h__

#include "algorithms/protocols/tspcoll/CollExchange.h"
#include "algorithms/protocols/tspcoll/shm_collectives.h"

namespace xlpgas
{
  template<class T_NI>
  class SHMReduce: public CollExchange<T_NI>
  {
    typedef cau_device_info<T_NI> device_info_type;
    typedef xlpgas::local::FixedLeader<Wait<T_NI> > FL;
    FL  fl;
    cau_reduce_op_t cau_op;
    bool            _done;
    pami_work_t     _work_pami;
  public:
    typedef typename FL::State State;
    void * operator new (size_t, void * addr) { return addr; }
    SHMReduce (int ctxt, Team * comm, CollectiveKind kind, int tag, int offset, void* device_info, T_NI* ni);
    virtual void setComplete (xlpgas_LCompHandler_t cb,
		      void *arg) {
      fl.setComplete(cb,arg,this->_ctxt);
    }
    
    virtual bool isdone           () const;
    virtual void  kick            (void);
    virtual void  kick_internal   (void);
    virtual void setContext (pami_context_t ctxt);
    virtual void reset (int rootindex,
			const void         * sbuf, 
			void               * dbuf,
			xlpgas_ops_t       op,
			xlpgas_dtypes_t    dt,
			unsigned           nelems,
			user_func_t*       uf);

    void repost(void);
  }; /* Shmreduce */

  template<class T_NI>
  class SHMBcast: public CollExchange<T_NI>
  {
    typedef cau_device_info<T_NI> device_info_type;
    typedef xlpgas::local::FixedLeader<Wait<T_NI> > FL;
    FL              fl;
    size_t          _nbytes;
    bool            _done;
    pami_work_t     _work_pami;
  public:
    void * operator new (size_t, void * addr) { return addr; }
    SHMBcast (int ctxt, Team * comm, CollectiveKind kind, int tag, int offset, void* device_info, T_NI* ni);

    virtual void setComplete (xlpgas_LCompHandler_t cb,
		      void *arg) {
      fl.setComplete(cb,arg,this->_ctxt);
    }
    virtual bool isdone           () const;
    virtual void  kick            (void);
    virtual void  kick_internal   (void);
    virtual void setContext (pami_context_t ctxt);
    virtual void reset (int rootindex,
			const void         * sbuf, 
			void               * dbuf,
			unsigned           nbytes);

	  
  }; /* Shmbcast */

template<class T_NI>
class SHMLargeBcast: public Collective<T_NI>
   {
     typedef cau_device_info<T_NI> device_info_type;
     typedef xlpgas::local::FixedLeader2LB<Wait<T_NI> > FL;
     FL  fl;
     const void* _sbuf;
     void*       _dbuf;
     unsigned    _nbytes;
   public:
     void * operator new (size_t, void * addr) { return addr; }
     SHMLargeBcast (int ctxt, Team * comm, CollectiveKind kind, int tag, int offset,void* device_info, T_NI* ni);
     virtual void setComplete (xlpgas_LCompHandler_t cb,
			       void *arg) {
       fl.setComplete(cb,arg,this->_ctxt);
     }
     virtual void setContext (pami_context_t ctxt);
     virtual void reset (int rootindex,
                       const void         * sbuf, 
                       void               * dbuf,
                       unsigned           nbytes);
     
     virtual void kick();
   }; /* ShmLargebcast */


} /* Xlpgas */

#include "algorithms/protocols/tspcoll/SHMReduceBcast.cc"

#endif /* __xlpgas_Shmreducebcast_h__ */
