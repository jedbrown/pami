/**
 * \file algorithms/protocols/tspcoll/Collective.cc
 * \brief ???
 */
#include "algorithms/protocols/tspcoll/Collective.h"
#include "algorithms/protocols/tspcoll/Team.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>


/* ************************************************************************ */
/*                 Collective constructor                                   */
/* ************************************************************************ */
template<class T_NI>
xlpgas::Collective<T_NI>::Collective (int                     ctxt,
                                      Team                  * comm,
                                      CollectiveKind          kind,
                                      int                     tag,
                                      xlpgas_LCompHandler_t    cb_complete,
                                      void                  * arg,
                                      T_NI                  * ni):
  _ctxt        (ctxt),
  _comm        (comm),
  _kind        (kind),
  _tag         (tag),
  _cb_complete (cb_complete),
  _arg         (arg),
  _p2p_iface   (ni)
{
  _my_rank  = _p2p_iface->endpoint();
  _my_index = _comm->endpoint2Index(_my_rank);
  _dev = NULL;
}

/* ************************************************************************ */
/*                     Collective factory                                   */
/* ************************************************************************ */


/* ************************************************************************ */
/*                    initialize the factory                                */
/* ************************************************************************ */
template<class T_NI>
void xlpgas::Collective<T_NI>::Initialize ()
{
#if 0
  xlpgas::CollExchange::amsend_reg ();
  xlpgas::Scatter::amsend_reg ();
  xlpgas::Alltoall::amsend_reg ();
  xlpgas::Alltoallv::amsend_reg ();
  // xlpgas::Gather::amsend_reg ();
#endif
}
