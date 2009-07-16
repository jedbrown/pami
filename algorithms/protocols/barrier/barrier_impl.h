/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/barrier/barrier_impl.h
 * \brief ???
 */

#include "algorithms/schedule/BinomialTree.h"
#include "interface/Geometry.h"
#include "BarrierT.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Barrier
    {
      extern inline bool binomial_analyze (Geometry *geometry)
      {
        return true;
      }

      typedef BarrierT <CCMI::Schedule::BinomialTreeSchedule, binomial_analyze> BinomialBarrier;      
      typedef BarrierFactoryT <BinomialBarrier>            BinomialBarrierFactory;
    };
  };
};
