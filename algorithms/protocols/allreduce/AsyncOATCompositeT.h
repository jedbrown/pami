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

#ifndef __algorithms_protocols_allreduce_AsyncOATCompositeT_h__
#define __algorithms_protocols_allreduce_AsyncOATCompositeT_h__

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
      template <class T_Exec, class T_Sched, class T_Conn>
      class AsyncOATCompositeT : public CCMI::Executor::Composite
      {
      protected:
	T_Exec                   _executor __attribute__((__aligned__(32)));
	T_Sched                  _schedule;

	PAMI_GEOMETRY_CLASS    * _geometry;
	T_Conn                 * _bcmgr;
	
      public:
	AsyncOATCompositeT ()
        {
	  CCMI_abort();
	}
	
	AsyncOATCompositeT (Interfaces::NativeInterface       * mf,
			    T_Conn                            * rcmgr,
			    pami_geometry_t                     g,
			    void                              * cmd,
			    pami_event_function                 fn,
			    void                              * cookie) 
	  {
	    CCMI_abort();
	  }

	
	AsyncOATCompositeT (pami_context_t                      context,
			    Interfaces::NativeInterface       * mf,
			    T_Conn                            * rcmgr,
			    T_Conn                            * bcmgr,
			    pami_geometry_t                     g,
			    void                              * cmd,
			    pami_event_function                 fn,
			    void                              * cookie):
	  Executor::Composite(),
	  _executor (mf, rcmgr, ((PAMI_GEOMETRY_CLASS*)g)->comm()),
	  _schedule (mf->myrank(), (PAMI::Topology*)((PAMI_GEOMETRY_CLASS *)g)->getTopology(PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX), 0)
        {
	  TRACE_FN_ENTER();
	  uintptr_t op, dt;
	  _geometry = (PAMI_GEOMETRY_CLASS*) g;	  
	  this->_context = context;
	  _executor.setContext(context);
	  _bcmgr = bcmgr;

	  PAMI::Type::TypeFunc::GetEnums
	    (((pami_xfer_t *)cmd)->cmd.xfer_allreduce.stype,
	     ((pami_xfer_t *)cmd)->cmd.xfer_allreduce.op,
	     dt, op);
	  
	  TypeCode * stype_obj = 
	    (TypeCode *)((pami_xfer_t *)cmd)->cmd.xfer_allreduce.stype;
	  TypeCode * rtype_obj = 
	    (TypeCode *)((pami_xfer_t *)cmd)->cmd.xfer_allreduce.rtype;

	  /// \todo Support non-contiguous
	  CCMI_assert(stype_obj->IsContiguous() &&  stype_obj->IsPrimitive());

	  initialize(((pami_xfer_t *)cmd)->cmd.xfer_allreduce.sndbuf,
		     ((pami_xfer_t *)cmd)->cmd.xfer_allreduce.rcvbuf,
		     ((pami_xfer_t *)cmd)->cmd.xfer_allreduce.stypecount,
		     stype_obj, 
		     rtype_obj,
		     (pami_dt)dt,
		     (pami_op)op);
	  
	  if (bcmgr)
	    _executor.setBroadcastConnectionManager (bcmgr);

	  _executor.setDoneCallback(fn, cookie);

	  _executor.reset();
	  int iteration = _geometry->getAllreduceIteration();
	  _executor.setIteration(iteration);   	  
	  _executor.start();

	  TRACE_FN_EXIT();
	}

	AsyncOATCompositeT (pami_context_t                      context,
			    Interfaces::NativeInterface       * mf,
			    T_Conn                            * rcmgr,
			    T_Conn                            * bcmgr,
			    pami_geometry_t                     g,
			    unsigned                            root,
			    unsigned                            iteration):
	  Executor::Composite(),
	  _executor (mf, rcmgr, ((PAMI_GEOMETRY_CLASS*)g)->comm()),
	  _schedule (mf->myrank(), (PAMI::Topology*)((PAMI_GEOMETRY_CLASS *)g)->getTopology(PAMI::Geometry::DEFAULT_TOPOLOGY_INDEX), 0)
	{
	  _geometry = (PAMI_GEOMETRY_CLASS*) g;	  
	  this->_context = context;
	  _bcmgr = bcmgr;
	  _executor.setContext(context);

	  if (bcmgr)
	    _executor.setBroadcastConnectionManager (bcmgr);
	  
	  _executor.setDoneCallback(NULL, NULL);
	  _executor.setIteration(iteration);   	  
	}	

	void setContext (pami_context_t context) {
	  this->_context = context;
	}

	T_Exec *getExecutor () {
	  return &_executor;
	}
       
	/// Default Destructor
        virtual ~AsyncOATCompositeT()
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
	void initialize ( char                            * sndbuf,
			  char                            * rcvbuf,
			  unsigned                          count,
			  TypeCode                        * stype,
			  TypeCode                        * rtype,
			  pami_dt                           dtype,
			  pami_op                           op )
	{
	  TRACE_FN_ENTER();

	  unsigned old_count = _executor.getCount();
	  _executor.setRoot ((unsigned)-1);
	  _executor.setBuffers
	    ( sndbuf,
	      rcvbuf,
	      count,
	      stype,
	      rtype );	  
	  _executor.setSchedule (&_schedule);
  
	  //Stype not available in early arrival packet
	  if ( (op != _executor.getOp()) || (dtype != _executor.getDt()) 
	       || (count != old_count) )
	  {
	    coremath func;
	    unsigned sizeOfType;
	    CCMI::Adaptor::Allreduce::getReduceFunction(dtype, op, 
							sizeOfType, func);
	    unsigned pwidth = count * sizeOfType;
	    
	    _executor.setReduceInfo ( count, pwidth, sizeOfType, 
				      func, stype, rtype, op, dtype );
	  }

	  TRACE_FN_EXIT();
	}

	///
        /// \brief At this level we only support single color
        /// collectives
        ///
	unsigned restart   ( void *cmd )
	{
	  TRACE_FN_ENTER();
	  TRACE_FORMAT( "<%p>", this);	    	    
	  
	  uintptr_t op, dt;
	  PAMI::Type::TypeFunc::GetEnums
	    (((pami_xfer_t *)cmd)->cmd.xfer_allreduce.stype,
	     ((pami_xfer_t *)cmd)->cmd.xfer_allreduce.op,
	     dt,op);
	  
	  TypeCode * stype_obj = 
	    (TypeCode *)((pami_xfer_t *)cmd)->cmd.xfer_allreduce.stype;
	  TypeCode * rtype_obj = 
	    (TypeCode *)((pami_xfer_t *)cmd)->cmd.xfer_allreduce.rtype;
	  	  
	  initialize(((pami_xfer_t *)cmd)->cmd.xfer_allreduce.sndbuf,
		     ((pami_xfer_t *)cmd)->cmd.xfer_allreduce.rcvbuf,
		     ((pami_xfer_t *)cmd)->cmd.xfer_allreduce.stypecount,
		     stype_obj,
		     rtype_obj,   
		     (pami_dt) dt,
		     (pami_op) op);
	  
	  if (_bcmgr)
	    _executor.setBroadcastConnectionManager (_bcmgr);
	  
	  _executor.setDoneCallback(((pami_xfer_t *)cmd)->cb_done,
				    ((pami_xfer_t *)cmd)->cookie);
	  _executor.reset();
	  _executor.setIteration(_geometry->getAllreduceIteration());
	  _executor.start();	

	  return PAMI_SUCCESS;
	}

	virtual void start()
	{
	  TRACE_FN_ENTER();
	  TRACE_FORMAT( "<%p>", this);
	  CCMI_abort();
	  TRACE_FN_EXIT();
	}
	
      };  //-- AsyncOATCompositeT
      
      ///
      /// \brief Generate a non-blocking allreduce message.
      ///
      template <class T_Composite, class T_Factory>
      void  cb_async_OAT_receiveHead
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
	CollHeaderData  *cdata = (CollHeaderData *) info;
	
	//fprintf (stderr, "In cb_async from %ld on iteration %d\n", peer, cdata->_iteration);
	
	T_Factory *factory = (T_Factory *) arg;
	
	PAMI_GEOMETRY_CLASS *geometry = (PAMI_GEOMETRY_CLASS *)
	  factory->getGeometry(ctxt, cdata->_comm);
	T_Composite *composite = (T_Composite *) geometry->getAllreduceComposite(cdata->_iteration);
	
	if ( likely(composite != NULL  &&  
		    composite->getAlgorithmFactory() == factory &&
		    !composite->getExecutor()->earlyArrival()) ) {
	  composite->getExecutor()->notifyRecvHead	    
	    (info,      count,
	     conn_id,   peer,
	     sndlen,    arg,
	     rcvlen,    rcvpwq,
	     cb_done);	  
	  TRACE_FN_EXIT();
	  return;
	}
	
	//Composite from different old algorithm
	if (composite != NULL && 
	    composite->getAlgorithmFactory() != factory) {
	  geometry->setAllreduceComposite(NULL, cdata->_iteration);
	  composite->~T_Composite(); //Call destructor
	  factory->_alloc.returnObject(composite);
	  composite = NULL;
	}
	
	if (composite == NULL) {
	  //Need to create a new composite
	  void *obj = factory->allocateObject();
	  geometry->setAllreduceComposite(obj, cdata->_iteration);
	  composite = new (obj) T_Composite
	    ( ctxt,
	      factory->native(), 
	      factory->connmgr(),    // Connection Manager
	      factory->getBcastConnMgr(),
	      geometry,          // Geometry Object
	      cdata->_root, 
	      cdata->_iteration );
	}
	
	composite->initialize(NULL, 
			      NULL, 
			      cdata->_count, 
			      (TypeCode *) PAMI_TYPE_BYTE,
			      (TypeCode *) PAMI_TYPE_BYTE,	   
			      (pami_dt)cdata->_dt,
			      (pami_op)cdata->_op );
	composite->getExecutor()->reset();
        composite->setContext(ctxt);
	composite->setAlgorithmFactory(factory);		
	
	composite->getExecutor()->notifyRecvHead	    
	  (info,      count,
	   conn_id,   peer,
	   sndlen,    arg,
	   rcvlen,    rcvpwq,
	   cb_done);	  
	
	TRACE_FN_EXIT();
      };

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
