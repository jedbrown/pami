/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/barrier/impl.h
 * \brief ???
 */

#ifndef __algorithms_protocols_barrier_impl_h__
#define __algorithms_protocols_barrier_impl_h__

#include "algorithms/schedule/MultinomialTree.h"
#include "algorithms/schedule/BinomialTree.h"
#include "algorithms/connmgr/SimpleConnMgr.h"
#include "BarrierT.h"
#include "MultiSyncComposite.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Barrier
    {

      void binom_barrier_md(pami_metadata_t *m)
      {
        // \todo:  fill in other metadata
        strcpy(&m->name[0],"CCMIBinomBarrier");
      }

      void msync_barrier_md(pami_metadata_t *m)
      {
        // \todo:  fill in other metadata
        strcpy(&m->name[0],"CCMIMsyncBarrier");
      }

      bool binomial_analyze (PAMI_GEOMETRY_CLASS *geometry)
      {
        return true;
      }


      typedef BarrierT <CCMI::Schedule::ListMultinomial,
        binomial_analyze> BinomialBarrier;

      typedef BarrierFactoryT <BinomialBarrier,
                               binom_barrier_md,
                               ConnectionManager::SimpleConnMgr<PAMI_SYSDEP_CLASS> > BinomialBarrierFactory;

      typedef OldBarrierT <CCMI::Schedule::BinomialTreeSchedule<PAMI_SYSDEP_CLASS>,
                           binomial_analyze,
                           PAMI_SYSDEP_CLASS,
                           PAMI_COLL_MCAST_CLASS> OldBinomialBarrier;
      typedef OldBarrierFactoryT <OldBinomialBarrier,
                                  PAMI_SYSDEP_CLASS,
                                  PAMI_COLL_MCAST_CLASS> OldBinomialBarrierFactory;

      typedef CollectiveProtocolFactoryT<MultiSyncComposite,
                                         msync_barrier_md,
                                         ConnectionManager::SimpleConnMgr<PAMI_SYSDEP_CLASS> > MultiSyncFactory;
    };
  };
};

#endif
