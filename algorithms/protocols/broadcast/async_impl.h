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
#include "algorithms/protocols/broadcast/AsyncCompositeT.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Broadcast
    {
      typedef
      AsyncCompositeT <CCMI::Schedule::BinomialTreeSchedule<XMI_COLL_SYSDEP_CLASS>,
                       XMI_COLL_SYSDEP_CLASS,
                       XMI_COLL_MCAST_CLASS,
                       CCMI::ConnectionManager::RankBasedConnMgr<XMI_COLL_SYSDEP_CLASS> > AsyncBinomialComposite;
      template<>
      void AsyncBinomialComposite::create_schedule(void                      * buf,
                                                   unsigned                    size,
                                                   unsigned                    root,
                                                   XMI_COLL_SYSDEP_CLASS     * map,
                                                   XMI_GEOMETRY_CLASS        * g)
      {
        new (buf) CCMI::Schedule::BinomialTreeSchedule<XMI_COLL_SYSDEP_CLASS>(map, g->nranks(), g->ranks());
      }

      typedef AsyncCompositeFactoryT <AsyncBinomialComposite, true_analyze,XMI_COLL_SYSDEP_CLASS,XMI_COLL_MCAST_CLASS>
      AsyncBinomialFactory;
    };
  };
};

#endif
