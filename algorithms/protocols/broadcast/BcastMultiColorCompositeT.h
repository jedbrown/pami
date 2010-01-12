
#ifndef __algorithms_protocols_broadcast_BcastMultiColorCompositeT_h__
#define __algorithms_protocols_broadcast_BcastMultiColorCompositeT_h__

#include "algorithms/executor/Broadcast.h"
#include "algorithms/executor/Barrier.h"
#include "algorithms/composite/Composite.h"
#include "algorithms/composite/MultiColorCompositeT.h"
#include "algorithms/connmgr/SimpleConnMgr.h"
#include "algorithms/connmgr/RankBasedConnMgr.h"
#include "algorithms/protocols/broadcast/BroadcastFactory.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Broadcast
    {
      ///
      ///  \brief Base class for synchronous broadcasts
      ///
      template <int NUMCOLORS, class T_Sched, class T_Conn, Executor::GetColorsFn pwcfn>
	class BcastMultiColorCompositeT : public Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::BarrierExec, CCMI::Executor::BroadcastExec<T_Conn>, T_Sched, T_Conn, pwcfn>
      {
      public:
	
      BcastMultiColorCompositeT(void                                     * cmd, 
				T_Conn                                   * cmgr,
				Interfaces::NativeInterface              * mf): 
	Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::BarrierExec, CCMI::Executor::BroadcastExec<T_Conn>, T_Sched, T_Conn, pwcfn>
	  ( ((XMI_GEOMETRY_CLASS *)((xmi_broadcast_t *)cmd)->geometry)->comm(), 
	    (XMI::Topology*)((XMI_GEOMETRY_CLASS *)((xmi_broadcast_t *)cmd)->geometry)->getTopology(0), 
	    cmgr, 
	    ((xmi_broadcast_t *)cmd)->cb_done, 
	    ((xmi_broadcast_t *)cmd)->cookie, 
	    mf,		       	   
	    ((xmi_broadcast_t *)cmd)->root,
	    ((xmi_broadcast_t *)cmd)->buf,
	    ((xmi_broadcast_t *)cmd)->typecount )
	  {
	    SyncBcastPost();
	  }

	void SyncBcastPost () {
	  int ncolors = Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::BarrierExec, CCMI::Executor::BroadcastExec<T_Conn>, T_Sched, T_Conn, pwcfn>::_numColors;
	  for(unsigned c = 0; c < ncolors; c++) {
	    Executor::BroadcastExec<T_Conn> *exec = (Executor::BroadcastExec<T_Conn> *) Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::BarrierExec, CCMI::Executor::BroadcastExec<T_Conn>, T_Sched, T_Conn, pwcfn>::getExecutor(c);
	    exec->postReceives();
	  }
	}

      };
    };
  };
};

#endif
