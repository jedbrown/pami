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

      typedef OldBarrierT <CCMI::Schedule::BinomialTreeSchedule<XMI_SYSDEP_CLASS>,
                           binomial_analyze,
                           XMI_SYSDEP_CLASS,
                           XMI_COLL_MCAST_CLASS> BinomialBarrier;
      typedef OldBarrierFactoryT <BinomialBarrier,
                                  XMI_SYSDEP_CLASS,
                                  XMI_COLL_MCAST_CLASS> BinomialBarrierFactory;
    };
  };
};
