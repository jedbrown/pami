/**
 * \file algorithms/protocols/tspcoll/ShmCauAllReduce.cc
 * \brief ???
 */
#include "algorithms/protocols/tspcoll/Allreduce.h"
#include "algorithms/protocols/tspcoll/ShmCauAllReduce.h"
#include "algorithms/protocols/tspcoll/Team.h"

#include "algorithms/protocols/tspcoll/cau_collectives.h"

template <class T_NI>
void next_phase (void* ctxt, void * arg, pami_result_t){
  xlpgas::Collective<T_NI>* b = (xlpgas::Collective<T_NI>*)arg;
  b->kick();
}

template <class T_NI, class T_Device>
void xlpgas::ShmCauAllReduce<T_NI,T_Device>::kick (){
  shm_reduce->kick();
}

template <class T_NI, class T_Device>
bool xlpgas::ShmCauAllReduce<T_NI,T_Device>::isdone () const {
  if(shm_bcast==NULL) return true;
  else return shm_bcast->isdone();
}

template <class T_NI, class T_Device>
void xlpgas::ShmCauAllReduce<T_NI,T_Device>::setContext (pami_context_t ctxt) {
  this->_pami_ctxt=ctxt;
  shm_reduce->setContext(ctxt);
  shm_bcast->setContext(ctxt);
  if(cau_reduce!=NULL) cau_reduce->setContext(ctxt);
  if(cau_bcast!=NULL)  cau_bcast->setContext(ctxt);
}

template <class T_NI, class T_Device>
void xlpgas::ShmCauAllReduce<T_NI,T_Device>::setComplete (xlpgas_LCompHandler_t cb,
							  void *arg) {
  shm_bcast->setComplete(cb,arg);
}

template <class T_NI, class T_Device>
void xlpgas::ShmCauAllReduce<T_NI,T_Device>::reset (const void         * sbuf,
                                                    void               * rbuf,
                                                    pami_data_function   op,
                                                    TypeCode           * sdt,
                                                    unsigned           nelems,
                                                    TypeCode           * rdt,
                                                    user_func_t        * uf
				     ) {
  assert(nelems == 1);
  uintptr_t i_op, i_dt;
  PAMI::Type::TypeFunc::GetEnums((void*)sdt,
                                 ( void (*)(void*, void*, size_t, void*) )op,
                                 i_dt,i_op);

  bool finish_early=false;//if shared memory only the collective ends without bcast;
  if(local_team->size() == team->size()) finish_early = true;
  
  memcpy(&s, sbuf, sdt->GetDataSize() );
  tmp = tmp_cau = 0;

  int leader = 0; //ordinal zero in the current group
  
  //allocate shm bcast
  assert (shm_bcast != NULL);
  shm_bcast->reset (leader, &tmp_cau, rbuf, nelems * sdt->GetDataSize() );

  //allocate shm reduce
  assert (shm_reduce != NULL);
  if(!finish_early)
    shm_reduce->reset (leader, &s, &tmp, (pami_op)i_op, (pami_dt)i_dt, nelems, uf);
  else
    shm_reduce->reset (leader, &s, &tmp_cau, (pami_op)i_op, (pami_dt)i_dt, nelems, uf);

  if(!finish_early){
    if(this->_is_leader){
      //allocate caureduce
      assert (cau_reduce != NULL);
      cau_reduce->reset (leader, &tmp, &tmp_cau, (pami_op)i_op, (pami_dt)i_dt, nelems, uf);
      shm_reduce->setComplete(&next_phase<T_NI>,cau_reduce);

      //allocate cau bcast
      assert (cau_bcast != NULL);
      cau_bcast->reset (leader, &tmp_cau, &tmp_cau, nelems * sdt->GetDataSize() );
      cau_reduce->setComplete(&next_phase<T_NI>,cau_bcast);

      cau_bcast->setComplete(&next_phase<T_NI>,shm_bcast);
    }
    else {
      shm_reduce->setComplete(&next_phase<T_NI>,shm_bcast);
    }
  } else {
    shm_reduce->setComplete(&next_phase<T_NI>,shm_bcast);
  }
}
