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

#include "algorithms/schedule/MultinomialTree.h"
#include "algorithms/protocols/ambcast/AMBroadcastT.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace AMBroadcast
    {
      void am_bcast_md(pami_metadata_t *m)
      {
        // \todo:  fill in other metadata
        strcpy(&m->name[0],"CCMIAMBinomialBcast");
      }

      typedef AMBroadcastT <CCMI::Schedule::ListMultinomial,
	CCMI::ConnectionManager::CommSeqConnMgr> AMBinomBcastComposite;
      template<>
	void AMBinomBcastComposite::create_schedule(void                        * buf,
						       unsigned                      size,
						       unsigned                      root,
						       Interfaces::NativeInterface * native,
						       PAMI_GEOMETRY_CLASS          * g)
	{
	  new (buf) CCMI::Schedule::ListMultinomial(native->myrank(), (PAMI::Topology *)g->getTopology(0), 0);
	}

      typedef CollectiveProtocolFactoryT<AMBinomBcastComposite,
	am_bcast_md,
	CCMI::ConnectionManager::CommSeqConnMgr > AMBinomBcastFactory;
      
    };
  };
};

#endif
