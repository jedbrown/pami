
#ifndef __algorithms_composite_MultiColorCompositeT_h__
#define __algorithms_composite_MultiColorCompositeT_h__


namespace CCMI
{
  namespace Executor
  {    
    ///
    /// \brief Get optimal colors based on bytes and schedule
    ///
    typedef void      (*GetColorsFn) (XMI::Topology             * t,
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
        xmi_event_function                            _cb_done;
	void                                        * _clientdata;
	
        T_Exec                                        _executors  [NUMCOLORS] __attribute__((__aligned__(16)));
        T_Sched                                       _schedules  [NUMCOLORS];
        char *                                        _srcbufs    [NUMCOLORS];
        unsigned                                      _bytecounts [NUMCOLORS];

	Interfaces::NativeInterface                 * _native;  //native interface

      public:
        MultiColorCompositeT () : CompositeT<NUMCOLORS, T_Bar, T_Exec>(), _doneCount(0), _nComplete(0)
        {
        }

      ///
      /// \brief Receive the broadcast message and notify the executor
      ///
      static void staticRecvFn(xmi_context_t context, void *executor, xmi_result_t err)
      {
	xmi_quad_t *info = NULL;
	
	T_Exec *exe = (T_Exec *) executor;
	
	TRACE_ADAPTOR ((stderr, "<%#.8X>Broadcast::MultiColorCompositeT::staticRecvFn() \n",(int)exe));
	
	exe->notifyRecv ((unsigned)-1, *info, NULL, exe->getPwidth());
      }
      
      ///
      /// \brief The Broadcast Constructor
      ///
      MultiColorCompositeT (unsigned                                   comm,
			    XMI::Topology                            * topology,
			    T_Conn                                   * cmgr,
			    xmi_event_function                         cb_done,
			    void                                     * clientdata,
			    Interfaces::NativeInterface              * mf,
			    unsigned                                   root,
			    char                                     * src,
			    unsigned                                   bytes,
			    unsigned                                   ncolors=1) :
      CCMI::Executor::CompositeT<NUMCOLORS, T_Bar, T_Exec>(), _doneCount(0), _numColors(ncolors), _cb_done(cb_done), _clientdata(clientdata), _native(mf)
      {
	//fprintf(stderr, "%d:MultiColorCompositeT constructor\n", mf->myrank());
	pwcfn (topology, bytes, _colors, _numColors);

	_srcbufs [0]   = src;
	_bytecounts[0] = bytes;
	_nComplete     = _numColors + 1;

	if(_numColors > 1) {
	  unsigned aligned_bytes = (bytes/_numColors) & (0xFFFFFFF0);
	  _bytecounts[0] =  aligned_bytes;
	  for(unsigned c = 1; c < _numColors; ++c) {
	    _bytecounts[c] = aligned_bytes;
	    _srcbufs[c]    = (char *)((unsigned long)_srcbufs[c-1] + _bytecounts[c-1]);
	  }
	  _bytecounts[_numColors-1]  = bytes -  (aligned_bytes * ( _numColors - 1));
	}
	
	for(unsigned c = 0; c < _numColors; c++) {
	  CCMI_assert (c < NUMCOLORS);
	  T_Exec *exec  =
	    new (& _executors[c]) T_Exec (mf,
					  comm, 
					  cmgr,
					  _colors[c],
					  true);
	  
	  exec->setInfo (root, _srcbufs[c], _bytecounts[c]);
	  exec->setDoneCallback (cb_composite_done, this);
	  
	  addExecutor (exec);
	  COMPILE_TIME_ASSERT(sizeof(_schedules[0]) >= sizeof(T_Sched));
	  new (&_schedules[c]) T_Sched(mf->myrank(), topology, _colors[c]);
	  exec->setSchedule (&_schedules[c]);
	}
      }
      
      void setDoneCallback(XMI_Callback_t  cb_done, void * clientdata) { _cb_done = cb_done; _clientdata = clientdata;}
      
      ///
      /// \brief For sync broadcasts, the done call back to be called
      ///        when barrier finishes
      ///
      static void cb_barrier_done(xmi_context_t context, void *me, xmi_result_t err)
      {
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
	  composite->_cb_done(NULL, composite->_clientdata,XMI_SUCCESS);
	}
      }
      
      static void cb_composite_done(xmi_context_t context, void *me, xmi_result_t err)
      {
	MultiColorCompositeT * composite = (MultiColorCompositeT *) me;
	CCMI_assert (composite != NULL);
	
	CCMI_assert (composite->_doneCount <  composite->_nComplete);
	++composite->_doneCount;
	
	if(composite->_doneCount == composite->_nComplete) // call users done function
        {
	  composite->_cb_done(context, composite->_clientdata, XMI_SUCCESS);
	  //printf ("%d: Composite Done\n", composite->_native->myrank());
	}
      }
    };  //-- MultiColorCompositeT
    
  };  //- end namespace Adaptor
};  //- end CCMI

#endif
