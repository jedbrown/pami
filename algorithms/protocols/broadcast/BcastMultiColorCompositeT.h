/**
 * \file algorithms/protocols/broadcast/BcastMultiColorCompositeT.h
 * \brief ???
 */
#ifndef __algorithms_protocols_broadcast_BcastMultiColorCompositeT_h__
#define __algorithms_protocols_broadcast_BcastMultiColorCompositeT_h__

#include "algorithms/executor/Broadcast.h"
#include "algorithms/executor/Barrier.h"
#include "algorithms/composite/Composite.h"
#include "algorithms/composite/MultiColorCompositeT.h"
#include "algorithms/connmgr/SimpleConnMgr.h"
#include "algorithms/connmgr/RankBasedConnMgr.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Broadcast
    {
      ///
      ///  \brief Base class for synchronous broadcasts
      ///
      template < int NUMCOLORS, class T_Sched, class T_Conn, Executor::GetColorsFn pwcfn, PAMI::Geometry::topologyIndex_t T_Geometry_Index = PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX >
      class BcastMultiColorCompositeT : public Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, CCMI::Executor::BroadcastExec<T_Conn>, T_Sched, T_Conn, pwcfn>
      {
      public:

	BcastMultiColorCompositeT() {}

        BcastMultiColorCompositeT(Interfaces::NativeInterface              * mf,
                                  T_Conn                                   * cmgr,
                                  pami_geometry_t                             g,
                                  void                                     * cmd,
                                  pami_event_function                         fn,
                                  void                                     * cookie):
        Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, CCMI::Executor::BroadcastExec<T_Conn>, T_Sched, T_Conn, pwcfn>
        ( cmgr,
          fn,
          cookie,
          mf,
          NUMCOLORS),
	  _geometry ((PAMI_GEOMETRY_CLASS *)g), _status(INTERNAL_BARRIER)
        {
          TRACE_ADAPTOR((stderr, "<%p>BcastMultiColorCompositeT()\n",this));
          Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, CCMI::Executor::BroadcastExec<T_Conn>, T_Sched, T_Conn, pwcfn>::
	    initialize (_geometry->comm(),
			(PAMI::Topology*)_geometry->getTopology(T_Geometry_Index),
			((pami_xfer_t *)cmd)->cmd.xfer_broadcast.root,
			((pami_xfer_t *)cmd)->cmd.xfer_broadcast.typecount,
			((pami_xfer_t *)cmd)->cmd.xfer_broadcast.buf,
			((pami_xfer_t *)cmd)->cmd.xfer_broadcast.buf);
	  
          SyncBcastPost();
	  
          PAMI_GEOMETRY_CLASS *geometry = ((PAMI_GEOMETRY_CLASS *)g);
          CCMI::Executor::Composite  *barrier =  (CCMI::Executor::Composite *)
	                                          geometry->getKey((size_t)0, /// \todo does NOT support multicontext
                                                                  PAMI::Geometry::CKEY_BARRIERCOMPOSITE1);
          this->addBarrier(barrier);
          barrier->setDoneCallback(Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, CCMI::Executor::BroadcastExec<T_Conn>, T_Sched, T_Conn, pwcfn>::cb_barrier_done, this);

          barrier->start();
        }

	/// \brief constructor for allgather/allgatherv
	BcastMultiColorCompositeT(Interfaces::NativeInterface              *  mf,
				  T_Conn                                   *  cmgr,
				  pami_geometry_t                             g,
				  pami_event_function                         fn,
				  void                                     *  cookie):
	  Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, CCMI::Executor::BroadcastExec<T_Conn>, T_Sched, T_Conn, pwcfn>
	  ( cmgr,
	    fn,
	    cookie,
	    mf,
	    NUMCOLORS),
	    _geometry ((PAMI_GEOMETRY_CLASS *)g),_status(EXTERNAL_BARRIER)
        {
	}

	/// \brief initialize routing for allgatherv
	unsigned initialize (size_t                                  root,
			     size_t                                  bytes,
			     char                                  * src,
			     char                                  * dst) {
	  TRACE_ADAPTOR((stderr, "<%p>BcastMultiColorCompositeT()\n",this));
	  
	  Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, CCMI::Executor::BroadcastExec<T_Conn>, T_Sched, T_Conn, pwcfn>::initialize 
	    (_geometry->comm(),						
	     (PAMI::Topology*)_geometry->getTopology(T_Geometry_Index),
	     root,
	     bytes,
	     src,
	     dst );
	  
          SyncBcastPost();
	  
	  return 0;
        }
	
	void start () {
	  //If composite calls barrier then do nothing. The barrier
	  //completion will trigger data movement
	  if (_status == EXTERNAL_BARRIER)
	    //Start all executors
	    Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, CCMI::Executor::BroadcastExec<T_Conn>, T_Sched, T_Conn, pwcfn>::cb_barrier_done(NULL, this, PAMI_SUCCESS);
	  
	}
	   
	void SyncBcastPost ()
        {
          //fprintf(stderr, "<%p>BcastMultiColorCompositeT::SyncBcastPost()\n",this);
	  unsigned ncolors = Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, CCMI::Executor::BroadcastExec<T_Conn>, T_Sched, T_Conn, pwcfn>::_numColors;
	  //fprintf(stderr,"SyncBcastPost ncolors %d\n",ncolors);
	  for (unsigned c = 0; c < ncolors; c++)
          {
	    Executor::BroadcastExec<T_Conn> *exec = (Executor::BroadcastExec<T_Conn> *) Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, CCMI::Executor::BroadcastExec<T_Conn>, T_Sched, T_Conn, pwcfn>::getExecutor(c);
	    unsigned root = exec->getRoot();
	    
	    if (Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, CCMI::Executor::BroadcastExec<T_Conn>, T_Sched, T_Conn, pwcfn>::_native->myrank() != root) {
	      //fprintf(stderr, "<%p>BcastMultiColorCompositeT::SyncBcastPost() post receives for color %u, root %u\n", this,c,root);
	      exec->setPostReceives(true);
	      exec->postReceives();
	    }
	  }
        }

      protected:
	PAMI_GEOMETRY_CLASS     * _geometry;
	unsigned                  _status;

	static const unsigned INTERNAL_BARRIER = 0;
	static const unsigned EXTERNAL_BARRIER = 1;
      };
    };
  };
};

#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
