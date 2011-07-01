/**
 * \file algorithms/protocols/tspcoll/ShmHybridBcast.cc
 * \brief ???
 */
#include "algorithms/protocols/tspcoll/ShmHybridBcast.h"
#include "algorithms/protocols/tspcoll/local/FixedLeader.h"
#include "algorithms/protocols/tspcoll/Team.h"

template <class T_NI>
void xlpgas::ShmHybridBcast<T_NI>::reset (int root, 
				    const void         * sbuf, 
				    void               * rbuf, 
				    unsigned           nbytes) {
#if 0
  xlpgas::Team * team        = this->_comm;//xlpgas::Team::get (_ctxt, teamID);
  xlpgas::Team * local_team  = xlpgas::Team::get (_ctxt, team->local_team_id());
  xlpgas::Team * leader_team = xlpgas::Team::get (_ctxt, team->leader_team_id());
  bool local_done=false;
  bool root_is_leader=false;

  xlpgas_endpoint_t root_ep = team->endpoint(root);
  if(leader_team->contains_endp(root_ep)) root_is_leader = true;//else will be false  
  xlpgas::Collective * a;

  //printf("L%d BCAST from  %d  | is root leader %d  sizes[%d %d %d] \n", XLPGAS_MYTHREAD, root, root_is_leader, team->size(), local_team->size(), leader_team->size() );

  if(! root_is_leader){
    //local bcast first to reach the leader
    if(local_team->contains_endp(root_ep)){
      a = local_team->coll (xlpgas::SHMLargeBcastKind);
      assert (a != NULL);
      void* lsbuf = const_cast<void*>(sbuf);
      void* lrbuf = rbuf;
      int l_root = local_team->ordinal(root_ep);

      //printf("L%d does local bcast SSS from lsbuf to lrbuf l_root=%d\n", XLPGAS_MYTHREAD, l_root);
      int NB=nbytes;
      while (NB > 0){
	int sz = (NB > SHM_BUF_SIZE)?SHM_BUF_SIZE:NB;
	a->reset (l_root, lsbuf, lrbuf, sz);
	a->kick();
	if(NB > SHM_BUF_SIZE) NB -= SHM_BUF_SIZE;
	else NB = 0;
	lsbuf = (void*)((char*)lsbuf+SHM_BUF_SIZE);
	lrbuf = (void*)((char*)lrbuf+SHM_BUF_SIZE);
      } 
    }
  }

  if(leader_team->is_leader() && leader_team->size() > 1){
    xlpgas_endpoint_t r_ep;
    r_ep.node = team->leader(root);
    r_ep.ctxt = 0;
    int g_root = leader_team->ordinal(r_ep);
    a = leader_team->coll (xlpgas::BcastPPKind);
    assert (a != NULL);
    if(root_is_leader) {
      a->reset (g_root, sbuf, rbuf, nbytes);
    }
    else {
      a->reset (g_root, rbuf, rbuf, nbytes);
    }
    a->kick();
    while (!a->isdone()) {
      xlpgas_tsp_wait (_ctxt);
    }
  }

  if(root_is_leader || (!root_is_leader && !local_team->contains_endp(root_ep))){  
    //here locally broadcast on all local teams except the one containing original root;
    a = local_team->coll (xlpgas::SHMLargeBcastKind);
    assert (a != NULL);
    void* lsbuf = const_cast<void*>(sbuf);
    void* lrbuf = rbuf;
    int l_root = 0;//the leader of the local team has ordinal 0
    while (nbytes > 0){
      int sz = (nbytes > SHM_BUF_SIZE)?SHM_BUF_SIZE:nbytes;
      if(leader_team->size() > 1) {
	//printf("L%d does local bcast from rbuf to rbuf after sz > 1 l_root=%d\n", XLPGAS_MYTHREAD, l_root);
	a->reset (l_root, lrbuf, lrbuf, sz);
      }
      else {
	//printf("L%d does local bcast from sbuf to rbuf after sz=1 l_root=%d\n", XLPGAS_MYTHREAD, l_root);
	a->reset (l_root, lsbuf, lrbuf, sz);
      }
      a->kick();
      if(nbytes > SHM_BUF_SIZE) nbytes -= SHM_BUF_SIZE;
      else nbytes = 0;
      lsbuf = (void*)((char*)lsbuf+SHM_BUF_SIZE);
      lrbuf = (void*)((char*)lrbuf+SHM_BUF_SIZE);
    }
  }
#endif
}
