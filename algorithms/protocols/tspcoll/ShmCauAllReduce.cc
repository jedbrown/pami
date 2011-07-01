/**
 * \file algorithms/protocols/tspcoll/ShmCauAllReduce.cc
 * \brief ???
 */
#include "algorithms/protocols/tspcoll/Allreduce.h"
#include "algorithms/protocols/tspcoll/ShmCauAllReduce.h"
#include "algorithms/protocols/tspcoll/Team.h"
#include "algorithms/protocols/tspcoll/cau_collectives.h"

template <class T_NI>
void xlpgas::ShmCauAllReduce<T_NI>::reset (const void         * sbuf, 
				     void               * rbuf, 
				     xlpgas_ops_t       op,
				     xlpgas_dtypes_t    dtype,
				     unsigned           nelems,
				     user_func_t* uf
				     ) {  
#if 0
  //if(XLPGAS_MYTHREAD==0) printf("Composed allreduce\n");
  xlpgas::Team * team        = this->_comm;
  xlpgas::Team * local_team  = xlpgas::Team::get (this->_ctxt, team->local_team_id());
  xlpgas::Team * leader_team = xlpgas::Team::get (this->_ctxt, team->leader_team_id());
  bool finish_early=false;//if shared memory only the collective ends without bcast;
  if(local_team->size() == team->size()) finish_early = true;
  
  int64_t s;
  memcpy(&s, sbuf, xlpgas::Allreduce::datawidthof(dtype) );
  int64_t tmp = 0;
  int64_t tmp_cau = 0;
  
  xlpgas::Collective * a;
  int lid = local_team->ordinal();
  int gid=-1;
  int leader = 0; //ordinal zero in the current group
  
  if(leader_team->is_leader()) gid = leader_team->ordinal();
  //printf("L[%d,%d] local_team_id = %d [%x] leader_id=%d\n", XLPGAS_MYNODE, XLPGAS_MYSMPTHREAD, lid, local_team, gid);
  //for(int i=0;i<nelems;++i){
  //  printf("L[%d,%d] SS[%ld]=%d\n",XLPGAS_MYNODE, XLPGAS_MYSMPTHREAD,i,ss[i]); 
  //}
  
  //phase 1; reduce on local teams;
  if(local_team->size()>1){
    //if(cntcnt<3) fprintf(stdout, "L%d local reduce start \n", XLPGAS_MYTHREAD);
    a = local_team->coll (xlpgas::SHMReduceKind);
    assert (a != NULL);
    if(!finish_early)
	a->reset (leader, &s, &tmp, op, dtype, nelems, uf);
    else
      a->reset (leader, &s, &tmp_cau, op, dtype, nelems, uf);
    a->kick();
    while (!a->isdone()) {
      xlpgas_tsp_wait (_ctxt);
    }
    //if(cntcnt<3) fprintf(stdout, "L%d local reduce finished \n", XLPGAS_MYTHREAD);
  }
  else{
    if(!finish_early){
	//tmp = s;
      memcpy(&tmp, &s, xlpgas::Allreduce::datawidthof(dtype) );
    }
    else {
      //tmp_cau = s;
      memcpy(&tmp_cau, &s, xlpgas::Allreduce::datawidthof(dtype) );
    }
  }
    
  if(!finish_early){
    // phase 2; allreduce across leaders
    if(leader_team->is_leader()){ //if not leader this pointer is null;
      cau_reduce_op_t cau_op = xlpgas::cau_op_dtype(op,dtype);
      xlpgas::cau_fast_allreduce(&tmp_cau,&tmp, cau_op, _ctxt);
    }
  }
#ifdef REDUCE_PHASES 
  t0 = timer();
#endif
  //phase 3: bcastr on local teams
  //if(cntcnt<3) fprintf(stdout, "%d L[%d,%d] PREPARE LBCAST=%d\n",cntcnt, XLPGAS_MYNODE, XLPGAS_MYSMPTHREAD,leader);
  if(local_team->size()>1){
    a = local_team->coll (xlpgas::SHMBcastKind);
    assert (a != NULL);
    a->reset (leader, &tmp_cau, rbuf, nelems * xlpgas::Allreduce::datawidthof(dtype) );
    a->kick();
    while (!a->isdone()) {
      xlpgas_tsp_wait (_ctxt);
    }
  }
  else{
    memcpy( rbuf, &tmp_cau, xlpgas::Allreduce::datawidthof(dtype) );
  }
#endif

}
