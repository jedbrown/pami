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
          TRACE_ADAPTOR((stderr, "<%p>Executor::MultiColorCompositeT()\n", this));
        }

        virtual ~MultiColorCompositeT ()
        {
        }

        ///
        /// \brief Receive the broadcast message and notify the executor
        ///
        static void staticRecvFn(pami_context_t context, void *executor, pami_result_t err)
        {
          pami_quad_t *info = NULL;
          T_Exec *exe = (T_Exec *) executor;
          TRACE_ADAPTOR ((stderr, "<%p>Executor::MultiColorCompositeT::staticRecvFn() \n", (int)exe));
          exe->notifyRecv ((unsigned) - 1, *info, NULL, exe->getPwidth());
        }

        ///
        /// \brief The Composite Constructor
        ///
        MultiColorCompositeT (T_Conn                                * cmgr,
                              pami_event_function                     cb_done,
                              void                                  * clientdata,
                              Interfaces::NativeInterface           * mf,
                              unsigned                                ncolors = NUMCOLORS):
            CCMI::Executor::CompositeT<NUMCOLORS, T_Bar, T_Exec>(), _doneCount(0), _numColors(ncolors), _cb_done(cb_done), _clientdata(clientdata), _native(mf), _cmgr(cmgr)
        {
          _nComplete     = _numColors + 1;
          TRACE_ADAPTOR((stderr, "<%p>Executor::MultiColorCompositeT(..) numcolors %u, complete %u\n", this,_numColors,_nComplete));
        }

        void initialize (unsigned                                comm,
                         PAMI::Topology                        * topology,
                         unsigned                                root,
                         unsigned                                bytes,
                         char                                  * src,
                         char                                  * dst)
        {
          TRACE_ADAPTOR((stderr, "<%p>Executor::MultiColorCompositeT::initialize() root %u, bytes %u\n", this,root, bytes));
          pwcfn (topology, bytes, _colors, _numColors);

          //printf ("Using %d colors, %d\n", _numColors, _colors[0]);

          if (_numColors > NUMCOLORS)
            _numColors = NUMCOLORS;
          
          _nComplete     = _numColors + 1;
          TRACE_ADAPTOR((stderr, "<%p>Executor::MultiColorCompositeT::initialize() numcolors %u, complete %u\n", this,_numColors,_nComplete));

          unsigned bytecounts[NUMCOLORS];
          bytecounts[0] = bytes;

          unsigned aligned_bytes = 0;

          if (_numColors > 1)
            {
              aligned_bytes = (bytes / _numColors) & (0xFFFFFFF0);
              bytecounts[0] =  aligned_bytes;

              for (unsigned c = 1; c < _numColors; ++c)
                {
                  bytecounts[c] = aligned_bytes;
                  TRACE_ADAPTOR((stderr, "<%p>Executor::MultiColorCompositeT::initialize() bytecounts[%u] %u\n", this,c,bytecounts[c]));
                }

              bytecounts[_numColors-1]  = bytes - (aligned_bytes * (_numColors - 1));
              TRACE_ADAPTOR((stderr, "<%p>Executor::MultiColorCompositeT::initialize() bytecounts[%u] %u\n", this,_numColors-1,bytecounts[_numColors-1]));
            }

          for (unsigned c = 0; c < _numColors; c++)
            {
              CCMI_assert (c < NUMCOLORS);

              new (&_schedules[c]) T_Sched(_native->myrank(), topology, _colors[c]);

              T_Exec *exec  =
                new (& _executors[c]) T_Exec (_native,
                                              _cmgr,
                                              comm);

              exec->setRoot (root);
              exec->setBuffers (src + aligned_bytes*c,
                                dst + aligned_bytes*c,
                                bytecounts[c]);
              exec->setDoneCallback (cb_composite_done, this);
              exec->setSchedule (&_schedules[c], _colors[c]);

              addExecutor (exec);
              COMPILE_TIME_ASSERT(sizeof(_schedules[0]) >= sizeof(T_Sched));
            }
        }

        //The Command will be interpretted in the child class
        virtual unsigned restart(void *cmd)
        {
          TRACE_ADAPTOR((stderr, "<%p>Executor::MultiColorCompositeT::restart()\n", this));
          if (CompositeT<NUMCOLORS, T_Bar, T_Exec>::_barrier != NULL)
            {
              // reset barrier since it may be been used between calls
            TRACE_ADAPTOR((stderr, "<%p>Executor::MultiColorCompositeT::restart() reset barrier(%p)\n", this, (CompositeT<NUMCOLORS, T_Bar, T_Exec>::_barrier)));

              T_Bar  *barrier =  CompositeT<NUMCOLORS, T_Bar, T_Exec>::_barrier;
              barrier->setDoneCallback(cb_barrier_done, this);
              barrier->start();
            }

          return PAMI_SUCCESS;
        }

        void setDoneCallback(PAMI_Callback_t  cb_done, void * clientdata)
        {
          _cb_done = cb_done;
          _clientdata = clientdata;
        }

        void addBarrier (T_Bar *exec)
        {
          CompositeT<NUMCOLORS, T_Bar, T_Exec>::addBarrier(exec);
          TRACE_ADAPTOR((stderr, "<%p>Executor::MultiColorCompositeT::addBarrier() numcolors %u, donecount %u, complete count %u\n", this,_numColors,_doneCount,_nComplete));
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
          MultiColorCompositeT * composite = (MultiColorCompositeT *) me;
          TRACE_ADAPTOR((stderr, "<%p>Executor::MultiColorCompositeT::cb_barrier_done() numcolors %u, donecount %u, complete count %u\n", me,composite->_numColors,composite->_doneCount,composite->_nComplete));
          CCMI_assert (composite != NULL);

          //printf ("In cb_barrier_done donec=%d\n",
          for (unsigned i = 0; i < composite->_numColors; ++i)
            {
              composite->getExecutor(i)->start();
            }

          CCMI_assert (composite->_doneCount <  composite->_nComplete);
          ++composite->_doneCount;

          if (composite->_doneCount == composite->_nComplete) // call users done function
            {
              //printf ("%d: Composite Done\n", composite->_native->myrank());
              composite->_cb_done(NULL, composite->_clientdata, PAMI_SUCCESS);
            }
        }

        static void cb_composite_done(pami_context_t context, void *me, pami_result_t err)
        {
          MultiColorCompositeT * composite = (MultiColorCompositeT *) me;
          TRACE_ADAPTOR((stderr, "<%p>Executor::MultiColorCompositeT::cb_composite_done() numcolors %u, donecount %u, complete count %u\n", me,composite->_numColors,composite->_doneCount,composite->_nComplete));
          CCMI_assert (composite != NULL);

          CCMI_assert (composite->_doneCount <  composite->_nComplete);
          ++composite->_doneCount;

          if (composite->_doneCount == composite->_nComplete) // call users done function
            {
              composite->_cb_done(context, composite->_clientdata, PAMI_SUCCESS);
              //printf ("%d: Composite Done\n", composite->_native->myrank());
            }
        }
    };  //-- MultiColorCompositeT

  };  //- end namespace Executor
};  //- end CCMI

#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
