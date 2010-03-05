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
	XMI_GEOMETRY_CLASS                 * _geometry;

      public:
	MultiSyncComposite (Interfaces::NativeInterface          * mInterface,
			    ConnectionManager::SimpleConnMgr<XMI_SYSDEP_CLASS>     * cmgr,
			    xmi_geometry_t                         g,
			    void                                 * cmd,
                            xmi_event_function                     fn,
                            void                                 * cookie) :
	Composite(), _native(mInterface), _geometry((XMI_GEOMETRY_CLASS*)g)
	{
	}

	virtual void start() {
	  xmi_multisync_t  minfo;

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
