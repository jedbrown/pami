/**
 * \file algorithms/protocols/tspcoll/AMExchange.cc
 * \brief ???
 */
#include "algorithms/protocols/tspcoll/AMExchange.h"
#include "algorithms/protocols/tspcoll/AMBcast.h"
#include "algorithms/protocols/tspcoll/AMGather.h"

//registration of the callbacks

void  xlpgas::am_collectives_reg       (xlpgas_AMHeaderReg_t) {
  //for am broadcast
  xlpgas_tsp_amsend_reg (XLPGAS_TSP_AMBCAST_PREQ, xlpgas::AMExchange<AMHeader_bcast, xlpgas::AMBcast>::cb_p_incoming);

  //for am reduce
  xlpgas_tsp_amsend_reg (XLPGAS_TSP_AMREDUCE_PREQ, xlpgas::AMExchange<AMHeader_reduce, xlpgas::AMReduce>::cb_p_incoming);
  xlpgas_tsp_amsend_reg (XLPGAS_TSP_AMREDUCE_CREQ, xlpgas::AMExchange<AMHeader_reduce, xlpgas::AMReduce>::cb_c_incoming);

  //for am gather
  xlpgas_tsp_amsend_reg (XLPGAS_TSP_AMGATHER_PREQ, xlpgas::AMExchange<AMHeader_gather, xlpgas::AMGather>::cb_p_incoming);
  xlpgas_tsp_amsend_reg (XLPGAS_TSP_AMGATHER_CREQ, xlpgas::AMExchange<AMHeader_gather, xlpgas::AMGather>::cb_c_incoming);

}//
