/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
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
#include "BarrierT.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Barrier
    {
      extern inline bool binomial_analyze(XMI_GEOMETRY_CLASS *geometry)
      {
        return true;
      }

      typedef BarrierT <CCMI::Schedule::ListMultinomial,
	binomial_analyze> BinomialBarrier;
      typedef BarrierFactoryT <BinomialBarrier> BinomialBarrierFactory;

      typedef OldBarrierT <CCMI::Schedule::BinomialTreeSchedule<XMI_SYSDEP_CLASS>,
                           binomial_analyze,
                           XMI_SYSDEP_CLASS,
                           XMI_COLL_MCAST_CLASS> OldBinomialBarrier;
      typedef OldBarrierFactoryT <BinomialBarrier,
                                  XMI_SYSDEP_CLASS,
                                  XMI_COLL_MCAST_CLASS> OldBinomialBarrierFactory;
    };
  };
};

#endif
