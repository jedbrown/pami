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

#include "algorithms/schedule/BinomialTree.h"
#include "algorithms/schedule/RingSchedule.h"
#include "algorithms/protocols/broadcast/AsyncCompositeT.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Broadcast
    {
      void am_bcast_md(xmi_metadata_t *m)
      {
        // \todo:  fill in other metadata
        strcpy(&m->name[0],"CCMIAMBinomialBcast");
      }

      typedef
      AsyncCompositeT <CCMI::Schedule::BinomialTreeSchedule<XMI_SYSDEP_CLASS>,
                       XMI_SYSDEP_CLASS,
                       XMI_COLL_MCAST_CLASS,
                       CCMI::ConnectionManager::RankBasedConnMgr<XMI_SYSDEP_CLASS> > AsyncBinomialComposite;
      template<>
      void AsyncBinomialComposite::create_schedule(void                      * buf,
                                                   unsigned                    size,
                                                   unsigned                    root,
                                                   XMI_SYSDEP_CLASS     * map,
                                                   XMI_GEOMETRY_CLASS        * g)
      {
        new (buf) CCMI::Schedule::BinomialTreeSchedule<XMI_SYSDEP_CLASS>(map, g->nranks(), g->ranks());
      }

      typedef AsyncCompositeFactoryT <AsyncBinomialComposite,am_bcast_md,XMI_SYSDEP_CLASS,XMI_COLL_MCAST_CLASS>
      AsyncBinomialFactory;
    };
  };
};

#endif
