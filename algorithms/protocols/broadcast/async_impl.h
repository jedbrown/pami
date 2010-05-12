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
      void am_bcast_rb(pami_metadata_t *m)
      {
        // \todo:  fill in other metadata
        strcpy(&m->name[0],"CCMI_AMRB_BinomialBcast");
      }

      void am_bcast_cs(pami_metadata_t *m)
      {
        // \todo:  fill in other metadata
        strcpy(&m->name[0],"CCMI_AMCS_BinomialBcast");
      }

      typedef AsyncBroadcastT <CCMI::Schedule::ListMultinomial,
	CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS> > AsyncRBBinomBcastComposite;
      template<>
	void AsyncRBBinomBcastComposite::create_schedule(void                        * buf,
						       unsigned                      size,
						       unsigned                      root,
						       Interfaces::NativeInterface * native,
						       PAMI_GEOMETRY_CLASS          * g)
	{
	  new (buf) CCMI::Schedule::ListMultinomial(native->myrank(), (PAMI::Topology *)g->getTopology(0), 0);
	}

      typedef AsyncBroadcastFactoryT<AsyncRBBinomBcastComposite,
	am_bcast_rb,
	CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS> > AsyncRBBinomBcastFactory;

      template<>
	unsigned AsyncRBBinomBcastFactory::getKey(unsigned                                                root,
						  unsigned                                                connid,
						  PAMI_GEOMETRY_CLASS                                    *geometry,
						  ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS> **connmgr)
	{
	  return root;
	}

      typedef AsyncBroadcastT <CCMI::Schedule::ListMultinomial,
	CCMI::ConnectionManager::CommSeqConnMgr> AsyncCSBinomBcastComposite;
      template<>
	void AsyncCSBinomBcastComposite::create_schedule(void                        * buf,
							 unsigned                      size,
							 unsigned                      root,
							 Interfaces::NativeInterface * native,
							 PAMI_GEOMETRY_CLASS          * g)
	{
	  new (buf) CCMI::Schedule::ListMultinomial(native->myrank(), (PAMI::Topology *)g->getTopology(0), 0);
	}

      typedef AsyncBroadcastFactoryT<AsyncCSBinomBcastComposite,
	am_bcast_cs,
	CCMI::ConnectionManager::CommSeqConnMgr > AsyncCSBinomBcastFactory;

      template<>
	unsigned AsyncCSBinomBcastFactory::getKey(unsigned                                   root,
						  unsigned                                   connid,
						  PAMI_GEOMETRY_CLASS                      * geometry,
						  ConnectionManager::CommSeqConnMgr        **connmgr)
	{
	  if (connid != (unsigned)-1) {
	    *connmgr = NULL; //use this key as connection id
	    return connid;
	  }
	  return (*connmgr)->updateConnectionId( geometry->comm() );
	}

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
