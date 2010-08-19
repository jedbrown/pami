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
#include "algorithms/protocols/barrier/BarrierT.h"
#include "algorithms/protocols/barrier/MultiSyncComposite.h"
#include "algorithms/protocols/barrier/msync_impl.h"

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

      bool binomial_analyze (PAMI_GEOMETRY_CLASS *geometry)
      {
        return true;
      }


      typedef BarrierT <CCMI::Schedule::ListMultinomial,
        binomial_analyze> BinomialBarrier;

      typedef BarrierFactoryT <BinomialBarrier,
                               binom_barrier_md,
                               ConnectionManager::SimpleConnMgr> BinomialBarrierFactory;

      typedef OldBarrierT <CCMI::Schedule::BinomialTreeSchedule,
                           binomial_analyze,
                           PAMI_COLL_MCAST_CLASS> OldBinomialBarrier;
      typedef OldBarrierFactoryT <OldBinomialBarrier,
                                  PAMI_COLL_MCAST_CLASS> OldBinomialBarrierFactory;

    };
  };
};

#endif
