/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/barrier/msync_impl.h
 * \brief Implement the simple multisync barrier factory
 */

#ifndef __algorithms_protocols_barrier_msync_impl_h__
#define __algorithms_protocols_barrier_msync_impl_h__

#include "algorithms/connmgr/SimpleConnMgr.h"
#include "algorithms/protocols/barrier/MultiSyncComposite.h"
#include "algorithms/protocols/AllSidedCollectiveProtocolFactoryT.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Barrier
    {

      void msync_barrier_md(pami_metadata_t *m)
      {
        // \todo:  fill in other metadata
        strcpy(&m->name[0],"CCMIMsyncBarrier");
      }

      typedef AllSidedCollectiveProtocolFactoryT<MultiSyncComposite,
                                                 msync_barrier_md,
                                                 ConnectionManager::SimpleConnMgr> MultiSyncFactory;
    };
  };
};

#endif
