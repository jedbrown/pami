/**
 * \file algorithms/protocols/tspcoll/ShmHybridBcast.cc
 * \brief ???
 */
#include "algorithms/protocols/tspcoll/ShmHybridBcast.h"
#include "algorithms/protocols/tspcoll/local/FixedLeader.h"
#include "algorithms/protocols/tspcoll/Team.h"


#include "algorithms/geometry/LapiGeometry.h"

template <class T_NI>
void xlpgas::ShmHybridBcast<T_NI>::kick () {
  //operation is finished in reset; here only invoke completion handler
  if (this->_cb_complete)
    _cb_complete (this->_pami_ctxt, this->_arg, PAMI_SUCCESS);
}

template <class T_NI>
void xlpgas::ShmHybridBcast<T_NI>::reset (int root, 
					  const void         * sbuf,
					  void               * rbuf,
					  unsigned           nbytes) {

#if 0
  PAMI::Topology* team        = (PAMI::Topology*)(this->_comm->geometry()->getTopology(PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX));
  if(team->size()==1){
    memcpy(rbuf, sbuf, nbytes);
    return ;
  }
  PAMI::Topology* local_team  = (PAMI::Topology*)(this->_comm->geometry()->getTopology(PAMI::Geometry::LOCAL_TOPOLOGY_INDEX));
  PAMI::Topology* leader_team = (PAMI::Topology*)(this->_comm->geometry()->getTopology(PAMI::Geometry::MASTER_TOPOLOGY_INDEX));

  bool local_done=false;
  bool root_is_leader=false;
  int my_rank = this->_comm->my_rank();

  //xlpgas_endpoint_t root_ep = this->_comm->endpoint(root);
  if(leader_team->isRankMember(root)) root_is_leader = true;//else will be false
  xlpgas::Collective<T_NI> * a;
  /*
  if(! root_is_leader){
    //local bcast first to reach the leader
    if(local_team->isRankMember(root)){
      a = xlpgas::CollectiveManager<T_NI>::instance(0)->collective(xlpgas::SHMLargeBcastKind);
      assert (a != NULL);
      void* lsbuf = const_cast<void*>(sbuf);
      void* lrbuf = rbuf;
      //int l_root = local_team->ordinal(root_ep);
      int l_root = local_team->rank2Index(root);

      printf("L%d does local bcast SSS from lsbuf to lrbuf l_root=%d\n", my_rank, l_root);
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

  if(leader_team->isRankMember(my_rank) && leader_team->size() > 1){
    //xlpgas_endpoint_t r_ep;
    //r_ep.node = team->leader(root);
    //r_ep.ctxt = 0;
    //int g_root = leader_team->ordinal(r_ep);
    int g_root = leader_team->rank2Index(root);
  */

  if(sbuf != rbuf && (size_t)root == this->_comm->my_rank()){
    memcpy (rbuf, sbuf, nbytes);
  }

  pami_result_t     result           = PAMI_SUCCESS;
  size_t               num_algorithm[2];

  result = PAMI_Geometry_algorithms_num(this->_comm->geometry(),
                                        PAMI_XFER_BROADCAST,
                                        num_algorithm);

  if (result != PAMI_SUCCESS || num_algorithm[0]==0)
    {
      fprintf (stderr,
               "Error. query, Unable to query algorithm, or no algorithms available result = %d\n",
               result);
    }

  pami_algorithm_t* always_works_alg = (pami_algorithm_t*)malloc(sizeof(pami_algorithm_t)*num_algorithm[0]);
  pami_metadata_t*  always_works_md  = (pami_metadata_t*)malloc(sizeof(pami_metadata_t)*num_algorithm[0]);
  pami_algorithm_t* must_query_alg   = (pami_algorithm_t*)malloc(sizeof(pami_algorithm_t)*num_algorithm[1]);
  pami_metadata_t*  must_query_md    = (pami_metadata_t*)malloc(sizeof(pami_metadata_t)*num_algorithm[1]);

  result = PAMI_Geometry_algorithms_query(this->_comm->geometry(),
                                          PAMI_XFER_BROADCAST,
                                          always_works_alg,
                                          always_works_md,
                                          num_algorithm[0],
                                          must_query_alg,
                                          must_query_md,
                                          num_algorithm[1]);

  pami_xfer_t          broadcast;
  //broadcast.cb_done                      = cb_done;
  //broadcast.cookie                       = (void*) & bcast_poll_flag;
  broadcast.algorithm                    = always_works_alg[0];
  broadcast.cmd.xfer_broadcast.root      = root;
  broadcast.cmd.xfer_broadcast.buf       = (char*)rbuf;
  broadcast.cmd.xfer_broadcast.type      = PAMI_TYPE_BYTE;
  broadcast.cmd.xfer_broadcast.typecount = 0;
  broadcast.cmd.xfer_broadcast.typecount = nbytes;


  //PAMI_Collective(this->_pami_ctxt, &broadcast);
  //PAMI_Context_advance (this->_pami_ctxt, 1);

  PAMI::Geometry::Algorithm<PAMI::Geometry::Lapi> *algo = (PAMI::Geometry::Algorithm<PAMI::Geometry::Lapi> *)broadcast.algorithm;
  algo->setContext((pami_context_t) this);
  algo->generate(&broadcast);
  //PAMI_Context_advance (this->_pami_ctxt, 1);
  xlpgas::CollectiveManager<T_NI>::instance(0)->device()->advance();

  int g_root=root;
    printf("L%d does Leaders Bcast from g_root=%d for real root%d  nbytes=%d\n", my_rank, g_root, root, nbytes);
    //a = leader_team->coll (xlpgas::BcastPPKind);
    a = xlpgas::CollectiveManager<T_NI>::instance(0)->collective(xlpgas::BcastKind);

    a->setNI(this->_p2p_iface);

    assert (a != NULL);
    fprintf(stderr, "L%d does Leaders Bcast from g_root=%d for real root%d a=%p a.comm->size=%d  nbytes=%d\n", my_rank, g_root, root,a, a->comm()->size(), nbytes );
    if(root_is_leader) {
      a->reset (g_root, sbuf, rbuf, nbytes);
    }
    else {
      a->reset (g_root, rbuf, rbuf, nbytes);
    }
    a->kick();
    fprintf(stderr, "Device Advance=%p\n", xlpgas::CollectiveManager<T_NI>::instance(0)->device());
    while (!a->isdone()) {
      //fprintf(stderr, "Device Advance=%p\n", xlpgas::CollectiveManager<T_NI>::instance(0)->device());
      xlpgas::CollectiveManager<T_NI>::instance(0)->device()->advance();
    }
#endif

/*
  }

  if(root_is_leader || (!root_is_leader && !local_team->isRankMember(root))){
    //here locally broadcast on all local teams except the one containing original root;
    //a = local_team->coll (xlpgas::SHMLargeBcastKind);
    a = xlpgas::CollectiveManager<T_NI>::instance(0)->collective(xlpgas::SHMLargeBcastKind);
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
*/

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
