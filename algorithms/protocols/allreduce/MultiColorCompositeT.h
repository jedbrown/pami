/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/MultiColorCompositeT.h
 * \brief CCMI composite adaptor for allreduce with barrier support
 */

#ifndef __algorithms_protocols_allreduce_MultiColorCompositeT_h__
#define __algorithms_protocols_allreduce_MultiColorCompositeT_h__

#include "algorithms/protocols/allreduce/BaseComposite.h"
#include "algorithms/executor/Barrier.h"
#include "math/math_coremath.h"
#include "algorithms/executor/AllreduceBaseExec.h"
#include "algorithms/composite/MultiColorCompositeT.h"

namespace CCMI
{
  namespace Adaptor
  {
    /// \brief Default for reuse_storage_limit is currently MAXINT/2G-1
#define CCMI_DEFAULT_REUSE_STORAGE_LIMIT ((unsigned)2*1024*1024*1024 - 1)
    /// \brief configuration flags/options for creating the factory
    namespace Allreduce
    {
      //-- MultiColorCompositeT
      /// \brief The Composite for the Allreduce (and reduce)
      /// kernel executor.
      ///
      /// It does common initialization for all subclasses (protocols)
      /// such as mapping the operator and datatype to a function and
      /// calling various setXXX() functions in the kernel executor.
      ///
      /// It also adds support for an optional barrier to synchronize
      /// the kernel executor.  It coordinates the barrier done
      /// callback and the [all]reduce done callback to call the
      /// client done callback.
      ///
      template <int NUMCOLORS, class T_Exec, class T_Sched, class T_Conn, Executor::GetColorsFn pwcfn>
      class MultiColorCompositeT : public Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, T_Exec, T_Sched, T_Conn, pwcfn>
      {
        public:
          MultiColorCompositeT ()
          {
            CCMI_abort();
          }

          MultiColorCompositeT (Interfaces::NativeInterface              * mf,
                                T_Conn                                   * cmgr,
                                pami_geometry_t                             g,
                                void                                     * cmd,
                                pami_event_function                         fn,
                                void                                     * cookie):
              Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, T_Exec, T_Sched, T_Conn, pwcfn>
              (cmgr,
               fn,
               cookie,
               mf,
               NUMCOLORS)
          {
            TRACE_ADAPTOR((stderr, "<%p>Allreduce::MultiColorCompositeT::ctor() count %zu, dt %#X, op %#X\n", this,((pami_xfer_t *)cmd)->cmd.xfer_allreduce.stypecount,
                           ((pami_xfer_t *)cmd)->cmd.xfer_allreduce.dt,((pami_xfer_t *)cmd)->cmd.xfer_allreduce.op));
            /// \todo only supporting PAMI_BYTE right now
            PAMI_assertf((((pami_xfer_t *)cmd)->cmd.xfer_allreduce.stype == PAMI_BYTE)&&(((pami_xfer_t *)cmd)->cmd.xfer_allreduce.rtype == PAMI_BYTE),"Not PAMI_BYTE? %#zX %#zX\n",(size_t)((pami_xfer_t *)cmd)->cmd.xfer_allreduce.stype,(size_t)((pami_xfer_t *)cmd)->cmd.xfer_allreduce.rtype);

//            PAMI_Type_sizeof(((pami_xfer_t *)cmd)->cmd.xfer_allreduce.stype); /// \todo PAMI_Type_sizeof() is PAMI_UNIMPL

            coremath func;
            unsigned sizeOfType;
            CCMI::Adaptor::Allreduce::getReduceFunction(((pami_xfer_t *)cmd)->cmd.xfer_allreduce.dt,
                                                        ((pami_xfer_t *)cmd)->cmd.xfer_allreduce.op,
                                                        ((pami_xfer_t *)cmd)->cmd.xfer_allreduce.stypecount,
                                                        sizeOfType,
                                                        func);
            //For now assume stypecount == rtypecount
            unsigned bytes = ((pami_xfer_t *)cmd)->cmd.xfer_allreduce.stypecount * 1; /// \todo presumed size of PAMI_BYTE is 1?

            /// \todo only supporting PAMI_BYTE right now, so better be a valid count of dt's
            PAMI_assertf(!(bytes%sizeOfType),"Not a valid PAMI_BYTE count of dt[%#X] bytes %u, sizeOfType %u\n",((pami_xfer_t *)cmd)->cmd.xfer_allreduce.dt,bytes,sizeOfType);

            Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, T_Exec, T_Sched, T_Conn, pwcfn>::
            initialize (((PAMI_GEOMETRY_CLASS *)g)->comm(),
                        (PAMI::Topology*)((PAMI_GEOMETRY_CLASS *)g)->getTopology(0),
                        (unsigned) - 1,/*((pami_allreduce_t *)cmd)->root,*/
                        bytes,
                        ((pami_xfer_t *)cmd)->cmd.xfer_allreduce.sndbuf,
                        ((pami_xfer_t *)cmd)->cmd.xfer_allreduce.rcvbuf);

            int iteration = ((PAMI_GEOMETRY_CLASS *)g)->getAllreduceIteration();

            for (unsigned c = 0; c < Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, T_Exec, T_Sched, T_Conn, pwcfn>::_numColors; c++)
              {
                T_Exec *allreduce = Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, T_Exec, T_Sched, T_Conn, pwcfn>::getExecutor(c);
                initialize(allreduce,
                           ((pami_xfer_t *)cmd)->cmd.xfer_allreduce.stypecount/sizeOfType, /// \todo presumed PAMI_BYTE count, convert to dt count
                           ((pami_xfer_t *)cmd)->cmd.xfer_allreduce.dt,
                           ((pami_xfer_t *)cmd)->cmd.xfer_allreduce.op);
                allreduce->reset();
                allreduce->setIteration(iteration);
              }

            PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS *)g;
            CCMI::Executor::Composite  *barrier =  (CCMI::Executor::Composite *)
                                                   geometry->getKey((size_t)0, /// \todo does NOT support multicontext
                                                                    PAMI::Geometry::PAMI_CKEY_BARRIERCOMPOSITE1);

            Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, T_Exec, T_Sched, T_Conn, pwcfn>::addBarrier(barrier);
            barrier->setDoneCallback(Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, T_Exec, T_Sched, T_Conn, pwcfn>::cb_barrier_done, this);
            barrier->start();
          }


          /// Default Destructor
          virtual ~MultiColorCompositeT()
          {
            TRACE_ALERT((stderr, "<%p>Allreduce::MultiColorCompositeT::dtor() ALERT:\n", this));
            TRACE_ADAPTOR((stderr, "<%p>Allreduce::MultiColorCompositeT::dtor()\n", this));
          }

          void operator delete (void *p)
          {
            CCMI_abort();
          }

          ///
          /// \brief initialize should be called after the executors
          /// have been added to the composite
          ///
          void initialize ( T_Exec                          * allreduce,
                            unsigned                          count,
                            pami_dt                           dtype,
                            pami_op                           op,
                            unsigned                          pipelineWidth = 0)// none specified, calculate it
          {

            TRACE_ADAPTOR((stderr, "<%p>Allreduce::MultiColorCompositeT::initialize() count %u, dt %#X, op %#X\n", this, count, dtype, op));

            if ((op != allreduce->getOp()) || (dtype != allreduce->getDt()) ||
                (count != allreduce->getCount()))
              {
                coremath func;
                unsigned sizeOfType;
                CCMI::Adaptor::Allreduce::getReduceFunction(dtype, op, count, sizeOfType, func);

                unsigned min_pwidth = MIN_PIPELINE_WIDTH;

                if (dtype == PAMI_DOUBLE && op == PAMI_SUM)
                  min_pwidth = MIN_PIPELINE_WIDTH_SUM2P;

                /* Select pipeline width.
                    Zero which means calculate it.
                    -1 means no pipelining so use count*sizeOfType.
                */

                /*
                   First, the function parameter overrides the config value.
                */
                unsigned pwidth = pipelineWidth /*? pipelineWidth : _flags.pipeline_override*/;
                /*
                   If -1, disable pipelining or use specified value
                */
                pwidth = (pwidth == (unsigned) - 1) ? count * sizeOfType : pwidth;
                /*
                   Use specified (non-zero) value or calculate (if zero is specified)
                */
                pwidth = pwidth ? pwidth : computePipelineWidth (count, sizeOfType, min_pwidth);

                allreduce->setReduceInfo ( count, pwidth, sizeOfType, func, op, dtype );
              }
          }

          unsigned computePipelineWidth (unsigned count, unsigned sizeOfType, unsigned min_pwidth)
          {
            TRACE_ADAPTOR((stderr, "<%p>Allreduce::MultiColorCompositeT::computePipelineWidth() count %#X, size %#X, min %#X\n", this,
                           count, sizeOfType, min_pwidth));
            unsigned pwidth = min_pwidth;

            if (count * sizeOfType > 1024 * pwidth)
              pwidth *= 32;
            else if (count * sizeOfType > 256  * pwidth)
              pwidth *= 16;
            else if (count * sizeOfType > 64 * pwidth)
              pwidth *= 8;
            else if (count * sizeOfType > 16 * pwidth)
              pwidth *= 4;

            TRACE_ADAPTOR((stderr, "<%p>Allreduce::MultiColorCompositeT::computePipelineWidth() pwidth %#X\n", this,
                           pwidth));
            return pwidth;
          }

          ///
          /// \brief At this level we only support single color
          /// collectives
          ///
          virtual unsigned restart   ( void *cmd )
          {
            TRACE_ADAPTOR((stderr, "<%p>Allreduce::MultiColorCompositeT::restart()\n", this));

            for (unsigned c = 0; c <  Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, T_Exec, T_Sched, T_Conn, pwcfn>::_numColors; c++)
              {
                T_Exec *allreduce = Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, T_Exec, T_Sched, T_Conn, pwcfn>::getExecutor(c);

                initialize(allreduce,
                           ((pami_xfer_t *)cmd)->cmd.xfer_allreduce.stypecount,
                           ((pami_xfer_t *)cmd)->cmd.xfer_allreduce.dt,
                           ((pami_xfer_t *)cmd)->cmd.xfer_allreduce.op);

                allreduce->reset();
              }

            Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, T_Exec, T_Sched, T_Conn, pwcfn>::_doneCount = 0; // default to just a composite done needed

            Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, T_Exec, T_Sched, T_Conn, pwcfn>::restart(cmd);
            return PAMI_SUCCESS;
          }

          virtual void start()
          {
            CCMI_abort();

            TRACE_ADAPTOR((stderr, "<%p>Allreduce::MultiColorCompositeT::start()\n", this));
            //Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, T_Exec, T_Sched, T_Conn, pwcfn>::getExecutor(0)->start();
          }

          ///
          /// \brief Generate a non-blocking allreduce message.
          ///
          static void  cb_receiveHead
          (pami_context_t         ctxt,
           const pami_quad_t     * info,
           unsigned               count,
           unsigned               conn_id,
           size_t                 peer,
           size_t                 sndlen,
           void                 * arg,
           size_t               * rcvlen,
           pami_pipeworkqueue_t ** rcvpwq,
           PAMI_Callback_t       * cb_done)
          {
            TRACE_ADAPTOR((stderr,"<%p>Allreduce::MultiColorCompositeT::cb_receiveHead peer %zd, conn_id %d\n",
                           arg, peer, conn_id));
            CCMI_assert (info && arg);
            CollHeaderData  *cdata = (CollHeaderData *) info;
            CollectiveProtocolFactory *factory = (CollectiveProtocolFactory *) arg;

            Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, T_Exec, T_Sched, T_Conn, pwcfn> *composite = (Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, T_Exec, T_Sched, T_Conn, pwcfn> *)
                ((PAMI_GEOMETRY_CLASS *)factory->getGeometry(ctxt, cdata->_comm))->getAllreduceComposite(cdata->_iteration);

            CCMI_assert (composite != NULL);
            //Use color 0 for now
            T_Exec *executor = composite->getExecutor(0);
            CCMI_assert (executor != NULL);

            executor->notifyRecvHead (info,      count,
                                                          conn_id,   peer,
                                                          sndlen,    arg,
                                                          rcvlen,    rcvpwq,
                                                          cb_done);
            return;
          };

      };  //-- MultiColorCompositeT
    };
  };
}; // namespace CCMI::Adaptor::Allreduce

#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
