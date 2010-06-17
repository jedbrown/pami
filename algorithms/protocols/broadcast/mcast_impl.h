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
        TRACE_ADAPTOR((stderr,"%s\n", __PRETTY_FUNCTION__));
        // \todo:  fill in other metadata
        strcpy(&m->name[0],"CCMIMcastBroadcast");
      }

      typedef AllSidedCollectiveProtocolFactoryT<MultiCastComposite,
                                                 mcast_broadcast_md,
                                                 ConnectionManager::SimpleConnMgr<PAMI_SYSDEP_CLASS> > MultiCastFactory;

// Needs to be updated for Trac #180 if it's to be used.
//    void mcast2_broadcast_md(pami_metadata_t *m)
//    {
//      TRACE_ADAPTOR((stderr,"%s\n", __PRETTY_FUNCTION__));
//      // \todo:  fill in other metadata
//      strcpy(&m->name[0],"CCMIMcast2Broadcast");
//    }
//
//    typedef AllSidedCollectiveProtocolFactoryT<MultiCastComposite2,
//                                               mcast2_broadcast_md,
//                                               ConnectionManager::SimpleConnMgr<PAMI_SYSDEP_CLASS> > MultiCast2Factory;

      void mcast3_broadcast_md(pami_metadata_t *m)
      {
        TRACE_ADAPTOR((stderr,"%s\n", __PRETTY_FUNCTION__));
        // \todo:  fill in other metadata
        strcpy(&m->name[0],"CCMIMcast3Broadcast");
      }

      typedef AllSidedCollectiveProtocolFactoryT<MultiCastComposite3,
                                                 mcast3_broadcast_md,
                                                 ConnectionManager::SimpleConnMgr<PAMI_SYSDEP_CLASS> > MultiCast3Factory;
    };
  };
};

#endif
