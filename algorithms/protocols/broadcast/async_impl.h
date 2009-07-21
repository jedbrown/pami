/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/broadcast/async_impl.h
 * \brief ???
 */

#ifndef   __ccmi_adaptor_broadcast_async_impl__
#define   __ccmi_adaptor_broadcast_async_impl__

#include "algorithms/schedule/BinomialTree.h"
#include "algorithms/schedule/RingSchedule.h"
#include "./AsyncCompositeT.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Broadcast
    {

      typedef 
      AsyncCompositeT <CCMI::Schedule::BinomialTreeSchedule, CCMI::CollectiveMapping> AsyncBinomialComposite;      
      template<> 
      void AsyncBinomialComposite::create_schedule(void                      * buf,
                                                   unsigned                    size,
                                                   unsigned                    root,
                                                   CCMI::CollectiveMapping             * map,
                                                   Geometry                  * g)
      {
        new (buf) CCMI::Schedule::BinomialTreeSchedule(map, g->nranks(), g->ranks());
      }

      typedef AsyncCompositeFactoryT <AsyncBinomialComposite, true_analyze, CCMI::CollectiveMapping> 
      AsyncBinomialFactory;
    };
  };
};

#endif
