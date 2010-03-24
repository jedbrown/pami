/**
 * \file algorithms/protocols/barrier/MultiSyncComposite.h
 * \brief ???
 */
#ifndef __algorithms_protocols_barrier_MultiSyncComposite_h__
#define __algorithms_protocols_barrier_MultiSyncComposite_h__

#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include "algorithms/executor/Barrier.h"
#include "algorithms/composite/Composite.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Barrier
    {

      class MultiSyncComposite : public CCMI::Executor::Composite {
      protected:
	Interfaces::NativeInterface        * _native;
	PAMI_GEOMETRY_CLASS                 * _geometry;

      public:
	MultiSyncComposite (Interfaces::NativeInterface          * mInterface,
			    ConnectionManager::SimpleConnMgr<PAMI_SYSDEP_CLASS>     * cmgr,
			    pami_geometry_t                         g,
			    void                                 * cmd,
                            pami_event_function                     fn,
                            void                                 * cookie) :
	Composite(), _native(mInterface), _geometry((PAMI_GEOMETRY_CLASS*)g)
	{
	}

	virtual void start() {
	  pami_multisync_t  minfo;

	  minfo.cb_done.function   = _cb_done;
	  minfo.cb_done.clientdata = _clientdata;
	  minfo.connection_id      = 0;
	  minfo.roles              = -1U;
	  minfo.participants       = _geometry->getTopology(0);

	  _native->multisync(&minfo);
	}
      };
    };
  };
};

#endif
