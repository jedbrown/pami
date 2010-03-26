/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/broadcast/mcast_impl.h
 * \brief Implement the simple multicast broadcast factory
 */

#ifndef __algorithms_protocols_broadcast_mcast_impl_h__
#define __algorithms_protocols_broadcast_mcast_impl_h__

#include "algorithms/connmgr/SimpleConnMgr.h"
#include "algorithms/protocols/broadcast/MultiCastComposite.h"
#include "algorithms/protocols/AllSidedCollectiveProtocolFactoryT.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Broadcast
    {

      void mcast_broadcast_md(pami_metadata_t *m)
      {
        // \todo:  fill in other metadata
        strcpy(&m->name[0],"CCMIMcastBroadcast");
      }

      typedef AllSidedCollectiveProtocolFactoryT<MultiCastComposite,
                                                 mcast_broadcast_md,
                                                 ConnectionManager::SimpleConnMgr<PAMI_SYSDEP_CLASS> > MultiCastFactory;
    };
  };
};

#endif
