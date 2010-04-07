/**
 * \file algorithms/composite/MultiColorCompositeT.h
 * \brief ???
 */
#ifndef __algorithms_composite_MultiColorCompositeT_h__
#define __algorithms_composite_MultiColorCompositeT_h__

#include "algorithms/composite/Composite.h"


namespace CCMI
{
  namespace Executor
  {
    ///
    /// \brief Get optimal colors based on bytes and schedule
    ///
    typedef void      (*GetColorsFn) (PAMI::Topology             * t,
				      unsigned                    bytes,
				      unsigned                  * colors,
				      unsigned                  & ncolors);

    ///
    ///  \brief Base class for synchronous broadcasts
    ///
    template <int NUMCOLORS, class T_Bar, class T_Exec, class T_Sched, class T_Conn, GetColorsFn pwcfn>
    class MultiColorCompositeT : public CompositeT<NUMCOLORS, T_Bar, T_Exec>
    {
    protected:
        ///
	/// \brief number of done callbacks called
	///
	unsigned                 _doneCount;
	unsigned                 _nComplete;
	unsigned                 _numColors;
	unsigned                 _colors [NUMCOLORS];
	
        ///
        ///  \brief Application callback to call when the broadcast has finished
        ///
        pami_event_function                            _cb_done;
	void                                        * _clientdata;
        T_Exec                                        _executors  [NUMCOLORS] __attribute__((__aligned__(16)));
        T_Sched                                       _schedules  [NUMCOLORS];
	Interfaces::NativeInterface                 * _native;  //native interface
	T_Conn                                      * _cmgr;
      public:
        MultiColorCompositeT () : CompositeT<NUMCOLORS, T_Bar, T_Exec>(), _doneCount(0), _nComplete(0)
        {
            TRACE_ADAPTOR((stderr,"%s\n", __PRETTY_FUNCTION__));
        }

	virtual ~MultiColorCompositeT () {}

      ///
      /// \brief Receive the broadcast message and notify the executor
      ///
      static void staticRecvFn(pami_context_t context, void *executor, pami_result_t err)
      {
	pami_quad_t *info = NULL;
	T_Exec *exe = (T_Exec *) executor;
	TRACE_ADAPTOR ((stderr, "<%p>Broadcast::MultiColorCompositeT::staticRecvFn() \n",(int)exe));
	exe->notifyRecv ((unsigned)-1, *info, NULL, exe->getPwidth());
      }

      ///
      /// \brief The Broadcast Constructor
      ///
      MultiColorCompositeT (T_Conn                                * cmgr,
			    pami_event_function                     cb_done,
			    void                                  * clientdata,
			    Interfaces::NativeInterface           * mf,
			    unsigned                                ncolors=1):
      CCMI::Executor::CompositeT<NUMCOLORS, T_Bar, T_Exec>(), _doneCount(0), _numColors(ncolors), _cb_done(cb_done), _clientdata(clientdata), _native(mf), _cmgr(cmgr)
      {
	_nComplete     = _numColors + 1;
      }

      void initialize (unsigned                                comm,
		       PAMI::Topology                        * topology,
		       unsigned                                root,
		       unsigned                                bytes,
		       char                                  * src,
		       char                                  * dst) 
      {	
	//fprintf(stderr, "%d: MultiColorCompositeT::intialize src %x dst %x\n", _native->myrank(), src, dst);
	pwcfn (topology, bytes, _colors, _numColors);

	unsigned bytecounts[NUMCOLORS];
	bytecounts[0] = bytes;

	unsigned aligned_bytes = 0;
	if(_numColors > 1) {
	  aligned_bytes = (bytes/_numColors) & (0xFFFFFFF0);
	  bytecounts[0] =  aligned_bytes;
	  for(unsigned c = 1; c < _numColors; ++c) {
	    bytecounts[c] = aligned_bytes;
	  }
	  bytecounts[_numColors-1]  = bytes - (aligned_bytes * (_numColors-1));
	}

	for(unsigned c = 0; c < _numColors; c++) {
	  CCMI_assert (c < NUMCOLORS);

	  new (&_schedules[c]) T_Sched(_native->myrank(), topology, _colors[c]);

	  T_Exec *exec  =
	    new (& _executors[c]) T_Exec (_native,
					  _cmgr,
					  comm);

	  exec->setSchedule (&_schedules[c], c);
	  exec->setRoot (root);
	  exec->setBuffers (src + aligned_bytes*c,
			    dst + aligned_bytes*c,
			    bytecounts[c]);
	  exec->setDoneCallback (cb_composite_done, this);

	  addExecutor (exec);
	  COMPILE_TIME_ASSERT(sizeof(_schedules[0]) >= sizeof(T_Sched));
	}
      }

      //The Command will be interpretted in the child class
      virtual unsigned restart(void *cmd) {
	if(CompositeT<NUMCOLORS, T_Bar, T_Exec>::_barrier != NULL)
	{
	  // reset barrier since it may be been used between calls
	  TRACE_ADAPTOR((stderr,"<%p>Allreduce::Composite::reset barrier(%p)\n",this,(CompositeT<NUMCOLORS, T_Bar, T_Exec>::_barrier)));

	  T_Bar  *barrier =  CompositeT<NUMCOLORS, T_Bar, T_Exec>::_barrier;
	  barrier->setDoneCallback(cb_barrier_done, this);
	  barrier->start();
	}
	
	return PAMI_SUCCESS;
      }

      void setDoneCallback(PAMI_Callback_t  cb_done, void * clientdata) { _cb_done = cb_done; _clientdata = clientdata;}

      void addBarrier (T_Bar *exec)
      {
	CompositeT<NUMCOLORS, T_Bar, T_Exec>::addBarrier(exec);
      }

      T_Exec * getExecutor (int idx)
      {
	return 	CompositeT<NUMCOLORS, T_Bar, T_Exec>::getExecutor(idx);
      }

      ///
      /// \brief For sync broadcasts, the done call back to be called
      ///        when barrier finishes
      ///
      static void cb_barrier_done(pami_context_t context, void *me, pami_result_t err)
      {
            TRACE_ADAPTOR((stderr,"%s\n", __PRETTY_FUNCTION__));
	MultiColorCompositeT * composite = (MultiColorCompositeT *) me;
	CCMI_assert (composite != NULL);

	//printf ("In cb_barrier_done donec=%d\n", composite->_doneCount);
	for(unsigned i=0; i < composite->_numColors; ++i)
        {
	  composite->getExecutor(i)->start();
	}

	CCMI_assert (composite->_doneCount <  composite->_nComplete);
	++composite->_doneCount;
	if(composite->_doneCount == composite->_nComplete) // call users done function
        {
	  //printf ("%d: Composite Done\n", composite->_native->myrank());
	  composite->_cb_done(NULL, composite->_clientdata,PAMI_SUCCESS);
	}
      }

      static void cb_composite_done(pami_context_t context, void *me, pami_result_t err)
      {
            TRACE_ADAPTOR((stderr,"%s\n", __PRETTY_FUNCTION__));
	MultiColorCompositeT * composite = (MultiColorCompositeT *) me;
	CCMI_assert (composite != NULL);

	CCMI_assert (composite->_doneCount <  composite->_nComplete);
	++composite->_doneCount;

	if(composite->_doneCount == composite->_nComplete) // call users done function
        {
	  composite->_cb_done(context, composite->_clientdata, PAMI_SUCCESS);
	  //printf ("%d: Composite Done\n", composite->_native->myrank());
	}
      }
    };  //-- MultiColorCompositeT

  };  //- end namespace Adaptor
};  //- end CCMI

#endif
