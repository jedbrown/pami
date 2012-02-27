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

#include "algorithms/executor/Barrier.h"
#include "math/math_coremath.h"
#include "algorithms/executor/AllreduceBaseExec.h"
#include "algorithms/composite/MultiColorCompositeT.h"

#include "util/trace.h"

#ifdef CCMI_TRACE_ALL
  #define DO_TRACE_ENTEREXIT 1
  #define DO_TRACE_DEBUG     1
#else
  #define DO_TRACE_ENTEREXIT 0
  #define DO_TRACE_DEBUG     0
#endif


namespace CCMI
{
  namespace Adaptor
  {
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
      template <int NUMCOLORS, class T_Exec, class T_Sched, class T_Conn, Executor::GetColorsFn pwcfn,int T_PostReceives = 0>
      class MultiColorCompositeT : public Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, T_Exec, T_Sched, T_Conn, pwcfn>
	{
	protected:
	pami_work_t              _work;
	PAMI_GEOMETRY_CLASS    * _geometry;
	T_Conn                 * _bcmgr;
        size_t                   _contextid;
        public:
	  MultiColorCompositeT ()
          {
            CCMI_abort();
          }

	  MultiColorCompositeT (pami_context_t                             context,
                                size_t                                     ctxt_id,
                                Interfaces::NativeInterface              * mf,
				T_Conn                                   * rcmgr,
				pami_geometry_t                             g,
				void                                     * cmd,
				pami_event_function                         fn,
				void                                     * cookie)
	  {
	    CCMI_abort();
	  }
	
	  MultiColorCompositeT (pami_context_t                             context,
                                size_t                                     ctxt_id,
                                Interfaces::NativeInterface              * mf,
				T_Conn                                   * rcmgr,
				T_Conn                                   * bcmgr,
                                void                                     * algorithmFactory,
                                pami_geometry_t                             g,
                                void                                     * cmd,
                                pami_event_function                         fn,
                                void                                     * cookie):
              Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, T_Exec, T_Sched, T_Conn, pwcfn>
              (rcmgr,
               fn,
               cookie,
               mf,
               NUMCOLORS),
              _contextid(ctxt_id)
          {
            TRACE_FN_ENTER();
            uintptr_t op, dt;
	    _geometry = (PAMI_GEOMETRY_CLASS*) g;
            PAMI::Type::TypeFunc::GetEnums(((pami_xfer_t *)cmd)->cmd.xfer_allreduce.stype,
                                           ((pami_xfer_t *)cmd)->cmd.xfer_allreduce.op,
                                           dt,op);
            TRACE_FORMAT( "<%p>Allreduce::MultiColorCompositeT::ctor() count %zu, dt %#X, op %#X\n", this, ((pami_xfer_t *)cmd)->cmd.xfer_allreduce.stypecount,
                           (pami_dt)dt, (pami_op)op);

	    this->_context = context;
            this->setAlgorithmFactory(algorithmFactory);
	    _bcmgr         = bcmgr;

            TypeCode * stype_obj = (TypeCode *)((pami_xfer_t *)cmd)->cmd.xfer_allreduce.stype;
            TypeCode * rtype_obj = (TypeCode *)((pami_xfer_t *)cmd)->cmd.xfer_allreduce.rtype;

            /// \todo Support non-contiguous
            assert(stype_obj->IsContiguous() &&  stype_obj->IsPrimitive());

            Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, T_Exec, T_Sched, T_Conn, pwcfn>::
            initialize (((PAMI_GEOMETRY_CLASS *)g)->comm(),
                        (PAMI::Topology*)((PAMI_GEOMETRY_CLASS *)g)->getTopology(PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX),
                        (unsigned) - 1,/*((pami_allreduce_t *)cmd)->root,*/
                        ((pami_xfer_t *)cmd)->cmd.xfer_allreduce.stypecount,
                        stype_obj,
                        ((pami_xfer_t *)cmd)->cmd.xfer_allreduce.rtypecount,
                        rtype_obj,
                        ((pami_xfer_t *)cmd)->cmd.xfer_allreduce.sndbuf,
                        ((pami_xfer_t *)cmd)->cmd.xfer_allreduce.rcvbuf);

	    int iteration = 0; //((PAMI_GEOMETRY_CLASS *)g)->getAllreduceIteration();

            for (unsigned c = 0; c < Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, T_Exec, T_Sched, T_Conn, pwcfn>::_numColors; c++)
              {
                T_Exec *allreduce = Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, T_Exec, T_Sched, T_Conn, pwcfn>::getExecutor(c);
                initialize(allreduce,
                           ((pami_xfer_t *)cmd)->cmd.xfer_allreduce.stypecount,
                           stype_obj, rtype_obj,
                           (pami_dt)dt,(pami_op)op);
                allreduce->reset();
                allreduce->setIteration(iteration);

		if (bcmgr)
		  allreduce->setBroadcastConnectionManager(bcmgr);		
              }


	    if (T_PostReceives) {
	      postReceives();
	      mf->postWork (this->_context, 0, &_work, static_advance, this);
	    }	    
	    
            PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS *)g;
            CCMI::Executor::Composite  *barrier =  (CCMI::Executor::Composite *)
                                                   geometry->getKey((size_t)0, /// \todo does NOT support multicontext
                                                                    PAMI::Geometry::CKEY_OPTIMIZEDBARRIERCOMPOSITE);

            Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, T_Exec, T_Sched, T_Conn, pwcfn>::addBarrier(barrier);
            barrier->setDoneCallback(Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, T_Exec, T_Sched, T_Conn, pwcfn>::cb_barrier_done, this);
            barrier->start();
            TRACE_FN_EXIT();
          }


	void setContext (pami_context_t context) {
	  this->_context = context;
	}
       
	void postReceives () {
	  for (unsigned c = 0; c < this->_numColors; c++)
	  {
	    T_Exec *allreduce = (T_Exec *)this->getExecutor(c);
	    allreduce->postReceives();
	  }       
	}

          /// Default Destructor
          virtual ~MultiColorCompositeT()
          {
            TRACE_FN_ENTER();
            TRACE_FORMAT( "<%p>", this);
            TRACE_FN_EXIT();
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
                            TypeCode                        * stype,
                            TypeCode                        * rtype,
                            pami_dt                           dtype,
                            pami_op                           op,
                            unsigned                          pipelineWidth = 0)// none specified, calculate it
          {
            TRACE_FN_ENTER();
            TRACE_FORMAT( "<%p>Allreduce::MultiColorCompositeT::initialize() count %u, dt %#X, op %#X\n", this, count, dtype, op);

            if ((op != allreduce->getOp()) || (dtype != allreduce->getDt()) ||
                (count != allreduce->getCount()) || (stype != allreduce->getStype()))
              {
                coremath func;
                unsigned sizeOfType;
                CCMI::Adaptor::Allreduce::getReduceFunction(dtype, op, sizeOfType, func);

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
                // SSS: Set sizeOfType based on stype not primitive type for non-contig support
                sizeOfType = stype->GetDataSize();
                pwidth = (pwidth == (unsigned) - 1) ? count * sizeOfType : pwidth;
                /*
                   Use specified (non-zero) value or calculate (if zero is specified)
                */
                pwidth = pwidth ? pwidth : computePipelineWidth (count, sizeOfType, min_pwidth);

                allreduce->setReduceInfo ( count, pwidth, sizeOfType, func, stype, rtype, op, dtype );
              }
            TRACE_FN_EXIT();
          }

          unsigned computePipelineWidth (unsigned count, unsigned sizeOfType, unsigned min_pwidth)
          {
            TRACE_FN_ENTER();
            TRACE_FORMAT( "<%p>Allreduce::MultiColorCompositeT::computePipelineWidth() count %#X, size %#X, min %#X\n", this,
                           count, sizeOfType, min_pwidth);
            unsigned pwidth = min_pwidth;

            if (count * sizeOfType > 1024 * pwidth)
	      pwidth *= 16;
            else if (count * sizeOfType > 256  * pwidth)
	      pwidth *= 8;
            else if (count * sizeOfType > 64  * pwidth)
	      pwidth *= 4;            	
	    else if (count * sizeOfType > 16  * pwidth)
	      pwidth *= 2;

            TRACE_FORMAT( "<%p>Allreduce::MultiColorCompositeT::computePipelineWidth() pwidth %#X\n", this,
                           pwidth);
            TRACE_FN_EXIT();
            return pwidth;
          }

          ///
          /// \brief At this level we only support single color
          /// collectives
          ///
	  virtual unsigned restart   ( void *cmd )
          {
            TRACE_FN_ENTER();
            TRACE_FORMAT( "<%p>", this);	    	    

	    uintptr_t op, dt;
	    PAMI::Type::TypeFunc::GetEnums(((pami_xfer_t *)cmd)->cmd.xfer_allreduce.stype,
					   ((pami_xfer_t *)cmd)->cmd.xfer_allreduce.op,
					   dt,op);
	    
	    TypeCode * stype_obj = (TypeCode *)((pami_xfer_t *)cmd)->cmd.xfer_allreduce.stype;
            TypeCode * rtype_obj = (TypeCode *)((pami_xfer_t *)cmd)->cmd.xfer_allreduce.rtype;
	    
	    Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, T_Exec, T_Sched, T_Conn, pwcfn>::
	    reset (_geometry->comm(),
		   (PAMI::Topology*)_geometry->getTopology(PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX),
		   (unsigned) - 1,/*((pami_allreduce_t *)cmd)->root,*/
		   ((pami_xfer_t *)cmd)->cmd.xfer_allreduce.stypecount,
		   stype_obj,
		   ((pami_xfer_t *)cmd)->cmd.xfer_allreduce.rtypecount,
		   rtype_obj,
		   ((pami_xfer_t *)cmd)->cmd.xfer_allreduce.sndbuf,
		   ((pami_xfer_t *)cmd)->cmd.xfer_allreduce.rcvbuf);

	    int iteration = 0; //_geometry->getAllreduceIteration();
            for (unsigned c = 0; c <  Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, T_Exec, T_Sched, T_Conn, pwcfn>::_numColors; c++)
              {
                T_Exec *allreduce = Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, T_Exec, T_Sched, T_Conn, pwcfn>::getExecutor(c);
                initialize(allreduce,
                           ((pami_xfer_t *)cmd)->cmd.xfer_allreduce.stypecount,
			   stype_obj,
			   rtype_obj,   
                           (pami_dt)dt,
                           (pami_op)op);

                allreduce->reset();
		allreduce->setIteration(iteration);

		if (_bcmgr)
		  allreduce->setBroadcastConnectionManager(_bcmgr);     
              }

	    if (T_PostReceives) {
	      postReceives();	      
	      this->_native->postWork(this->_context, 0, &_work, static_advance, this);
	    }	    

            Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, T_Exec, T_Sched, T_Conn, pwcfn>::restart(cmd);
            TRACE_FN_EXIT();
            return PAMI_SUCCESS;
          }

          virtual void start()
          {
            TRACE_FN_ENTER();
            TRACE_FORMAT( "<%p>", this);
            TRACE_FN_EXIT();
            //Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, T_Exec, T_Sched, T_Conn, pwcfn>::getExecutor(0)->start();
          }

	  static pami_result_t static_advance (pami_context_t     context,
					       void             * clientdata)
	  {
	    pami_result_t erc = PAMI_SUCCESS, rc = PAMI_SUCCESS;
	    MultiColorCompositeT *composite = (MultiColorCompositeT*)clientdata;
	    for (unsigned i = 0; i < composite->_numColors; ++i) {
	      T_Exec *executor = composite->getExecutor(i);
	      erc = executor->advance();
	      if (erc != PAMI_SUCCESS)
		rc = erc;
	    }
	    return rc;
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
            TRACE_FN_ENTER();
            TRACE_FORMAT( "<%p>Allreduce::MultiColorCompositeT::cb_receiveHead peer %zd, conn_id %d\n",
                           arg, peer, conn_id);
            CCMI_assert (info && arg);
            ExtCollHeaderData  *cdata = (ExtCollHeaderData *) info;
            CollectiveProtocolFactory *factory = (CollectiveProtocolFactory *) arg;

            Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, T_Exec, T_Sched, T_Conn, pwcfn> *composite = (Executor::MultiColorCompositeT<NUMCOLORS, CCMI::Executor::Composite, T_Exec, T_Sched, T_Conn, pwcfn> *)
	    ((PAMI_GEOMETRY_CLASS *)factory->getGeometry(ctxt, cdata->_comm))->getAllreduceComposite(composite->_context_id,
                                                                                                     0 /*cdata->_iteration*/);
	    
            CCMI_assert (composite != NULL);
            //Use color 0 for now
            T_Exec *executor = composite->getExecutor(0);
            CCMI_assert (executor != NULL);

            executor->notifyRecvHead (info,      count,
                                      conn_id,   peer,
                                      sndlen,    arg,
                                      rcvlen,    rcvpwq,
                                      cb_done);
            TRACE_FN_EXIT();
            return;
          };

      };  //-- MultiColorCompositeT
    };
  };
}; // namespace CCMI::Adaptor::Allreduce

#undef  DO_TRACE_ENTEREXIT
#undef  DO_TRACE_DEBUG

#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
