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
      template <int NUMCOLORS, class T_Sched, class T_Conn, Executor::GetColorsFn pwcfn, int T_Geometry_Index=PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX>
      class BcastMultiColorCompositeT : public Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, CCMI::Executor::BroadcastExec<T_Conn>, T_Sched, T_Conn, pwcfn>
      {
      public:

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
          NUMCOLORS)
        {
          TRACE_ADAPTOR((stderr, "<%p>BcastMultiColorCompositeT()\n",this));
          Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, CCMI::Executor::BroadcastExec<T_Conn>, T_Sched, T_Conn, pwcfn>::
          initialize (((PAMI_GEOMETRY_CLASS *)g)->comm(),
                      (PAMI::Topology*)((PAMI_GEOMETRY_CLASS *)g)->getTopology(T_Geometry_Index),
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
          //barrier->setConsistency (consistency);
          barrier->start();
        }

        void SyncBcastPost ()
        {
          TRACE_ADAPTOR((stderr, "<%p>BcastMultiColorCompositeT::SyncBcastPost()\n",this));
          Executor::BroadcastExec<T_Conn> *exec = (Executor::BroadcastExec<T_Conn> *) Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, CCMI::Executor::BroadcastExec<T_Conn>, T_Sched, T_Conn, pwcfn>::getExecutor(0);
          unsigned root = exec->getRoot();

          if (Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, CCMI::Executor::BroadcastExec<T_Conn>, T_Sched, T_Conn, pwcfn>::_native->myrank() != root)
          {
            unsigned ncolors = Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, CCMI::Executor::BroadcastExec<T_Conn>, T_Sched, T_Conn, pwcfn>::_numColors;
            //fprintf(stderr,"SyncBcastPost ncolors %d\n",ncolors);
            for (unsigned c = 0; c < ncolors; c++)
            {
              TRACE_ADAPTOR((stderr, "<%p>BcastMultiColorCompositeT::SyncBcastPost() post receives for color %u, root %u\n",this,c,root));
              Executor::BroadcastExec<T_Conn> *exec = (Executor::BroadcastExec<T_Conn> *) Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, CCMI::Executor::BroadcastExec<T_Conn>, T_Sched, T_Conn, pwcfn>::getExecutor(c);
              exec->setPostReceives(true);
              exec->postReceives();
            }
          }
        }

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
