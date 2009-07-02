
#ifndef   __ccmi_adaptor_async_broadcast_impl__
#define   __ccmi_adaptor_async_broadcast_impl__

#include "collectives/algorithms/schedule/BinomialTree.h"
#include "collectives/algorithms/schedule/RingSchedule.h"
#include "collectives/algorithms/protocols/broadcast/AsyncCompositeT.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Broadcast
    {
      template <class T> void create_async_schedule (void                      * buf,
                                                     unsigned                    size,
                                                     unsigned                    root,
                                                     CCMI::Mapping             * map,
                                                     Geometry                  * g)
      {
	//        new (buf, size) T (map, g->nranks(), g->ranks());
        new (buf) T (map, g->nranks(), g->ranks());
      }

      typedef AsyncCompositeT <CCMI::Schedule::BinomialTreeSchedule, 
      create_async_schedule<CCMI::Schedule::BinomialTreeSchedule> > AsyncBinomialComposite;      

      typedef AsyncCompositeFactoryT <AsyncBinomialComposite, true_analyze> AsyncBinomialFactory;
    };
  };
};

#endif
