/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/broadcast/async_impl.h
 * \brief ???
 */

#ifndef __algorithms_protocols_broadcast_async_impl_h__
#define __algorithms_protocols_broadcast_async_impl_h__

#include "algorithms/schedule/MultinomialTree.h"
#include "algorithms/schedule/BinomialTree.h"
#include "algorithms/schedule/RingSchedule.h"
#include "algorithms/protocols/broadcast/AsyncCompositeT.h"
#include "algorithms/protocols/broadcast/AsyncBroadcastT.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Broadcast
    {
      void am_bcast_md(pami_metadata_t *m)
      {
        // \todo:  fill in other metadata
        strcpy(&m->name[0],"CCMIAMBinomialBcast");
      }

      typedef AsyncBroadcastT <CCMI::Schedule::ListMultinomial,
	CCMI::ConnectionManager::CommSeqConnMgr> AsyncBinomBcastComposite;
      template<>
	void AsyncBinomBcastComposite::create_schedule(void                        * buf,
						       unsigned                      size,
						       unsigned                      root,
						       Interfaces::NativeInterface * native,
						       PAMI_GEOMETRY_CLASS          * g)
	{
	  new (buf) CCMI::Schedule::ListMultinomial(native->myrank(), (PAMI::Topology *)g->getTopology(0), 0);
	}

      typedef AsyncBroadcastFactoryT<AsyncBinomBcastComposite,
	am_bcast_md,
	CCMI::ConnectionManager::CommSeqConnMgr > AsyncBinomBcastFactory;

      void old_am_bcast_md(pami_metadata_t *m)
      {
        // \todo:  fill in other metadata
        strcpy(&m->name[0],"CCMIOldAMBinomialBcast");
      }

      typedef
      AsyncCompositeT <CCMI::Schedule::BinomialTreeSchedule<PAMI_SYSDEP_CLASS>,
                       PAMI_SYSDEP_CLASS,
                       PAMI_COLL_MCAST_CLASS,
                       CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS> > AsyncBinomialComposite;
      template<>
      void AsyncBinomialComposite::create_schedule(void                      * buf,
                                                   unsigned                    size,
                                                   unsigned                    root,
                                                   PAMI_SYSDEP_CLASS     * map,
                                                   PAMI_GEOMETRY_CLASS        * g)
      {
        new (buf) CCMI::Schedule::BinomialTreeSchedule<PAMI_SYSDEP_CLASS>(map, g->nranks(), g->ranks());
      }

      typedef AsyncCompositeFactoryT <AsyncBinomialComposite,old_am_bcast_md,PAMI_SYSDEP_CLASS,PAMI_COLL_MCAST_CLASS>
      AsyncBinomialFactory;
    };
  };
};

#endif
