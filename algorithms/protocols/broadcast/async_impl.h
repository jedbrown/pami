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

      void create_schedule(void                        * buf,
                           unsigned                      size,
                           unsigned                      root,
                           Interfaces::NativeInterface * native,
                           PAMI_GEOMETRY_CLASS          * g)
	{
	  new (buf) CCMI::Schedule::ListMultinomial(native->myrank(), (PAMI::Topology *)g->getTopology(0), 0);
	}

      unsigned getKey(unsigned                                                root,
                      unsigned                                                connid,
                      PAMI_GEOMETRY_CLASS                                    *geometry,
                      ConnectionManager::BaseConnectionManager              **connmgr)
      {
        return root;
      }

      unsigned getKey_as(unsigned                                   root,
                         unsigned                                   connid,
                         PAMI_GEOMETRY_CLASS                      * geometry,
                         ConnectionManager::BaseConnectionManager **connmgr)
      {
        ConnectionManager::CommSeqConnMgr *cm = (ConnectionManager::CommSeqConnMgr *)*connmgr;
        if (connid != (unsigned)-1) {
          *connmgr = NULL; //use this key as connection id
          return connid;
        }
        return cm->updateConnectionId( geometry->comm() );
      }

      typedef AsyncBroadcastT <CCMI::Schedule::ListMultinomial,
                               CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS>,
                               create_schedule>
      AsyncRBBinomBcastComposite;

      typedef AsyncBroadcastFactoryT<AsyncRBBinomBcastComposite,
                                     am_bcast_rb,
                                     CCMI::ConnectionManager::RankBasedConnMgr<PAMI_SYSDEP_CLASS>,
                                     getKey>
      AsyncRBBinomBcastFactory;



      typedef AsyncBroadcastT <CCMI::Schedule::ListMultinomial,
                               CCMI::ConnectionManager::CommSeqConnMgr,
                               create_schedule >
      AsyncCSBinomBcastComposite;


      typedef AsyncBroadcastFactoryT<AsyncCSBinomBcastComposite,
                                     am_bcast_cs,
                                     CCMI::ConnectionManager::CommSeqConnMgr,
                                     getKey_as>
      AsyncCSBinomBcastFactory;



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
