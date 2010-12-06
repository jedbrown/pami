/**
 * \file algorithms/composite/MultiColorCompositeT.h
 * \brief ???
 */
#ifndef __algorithms_composite_MultiColorCompositeT_h__
#define __algorithms_composite_MultiColorCompositeT_h__

#include "algorithms/composite/Composite.h"

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

      static const uint32_t alignment = 0x40; //64 byte aligment
      static const uint32_t alignment_mask = ~(alignment-1); //the mask for sizes

    public:
      MultiColorCompositeT () : CompositeT<NUMCOLORS, T_Bar, T_Exec>(), 
        _doneCount(0), _nComplete(0)
      {
        TRACE_FN_ENTER();
        TRACE_FORMAT("<%p>", this);
        TRACE_FN_EXIT();
      }

      virtual ~MultiColorCompositeT ()
      {
      }

      ///
      /// \brief Receive the broadcast message and notify the executor
      ///
      static void staticRecvFn(pami_context_t context, void *executor, pami_result_t err)
      {
        TRACE_FN_ENTER();
        pami_quad_t *info = NULL;
        T_Exec *exe = (T_Exec *) executor;
        TRACE_FORMAT( "<%p>", (int)exe);
        exe->notifyRecv ((unsigned) - 1, *info, NULL, exe->getPwidth());
        TRACE_FN_EXIT();
      }

      ///
      /// \brief The Composite Constructor
      ///
      MultiColorCompositeT (T_Conn                                * cmgr,
                            pami_event_function                     cb_done,
                            void                                  * clientdata,
                            Interfaces::NativeInterface           * mf,
                            unsigned                                ncolors = NUMCOLORS):
      CCMI::Executor::CompositeT<NUMCOLORS, T_Bar, T_Exec>(), 
        _doneCount(0), _numColors(ncolors), _cb_done(cb_done), _clientdata(clientdata), _native(mf), _cmgr(cmgr)
      {
        TRACE_FN_ENTER();
        _nComplete     = _numColors + 1;
        TRACE_FORMAT("<%p> numcolors %u, complete %u", this,_numColors,_nComplete);
        TRACE_FN_EXIT();
      }

      uint32_t getNumColors ()
      {
        return _numColors;
      }

      void initialize (unsigned                                comm,
                       PAMI::Topology                        * topology,
                       unsigned                                root,
                       unsigned                                bytes,
                       char                                  * src,
                       char                                  * dst)
      {
        TRACE_FN_ENTER();
        TRACE_FORMAT("<%p> root %u, bytes %u", this,root, bytes);
        pwcfn (topology, bytes, _colors, _numColors);

        //printf ("Using %d colors, %d\n", _numColors, _colors[0]);
        if (_numColors > NUMCOLORS)
          _numColors = NUMCOLORS;

        _nComplete     = _numColors + 1;
        TRACE_FORMAT("<%p> numcolors %u, complete %u", this,_numColors,_nComplete);

        unsigned bytecounts[NUMCOLORS];
        bytecounts[0] = bytes;

        unsigned aligned_bytes = 0;

        if (_numColors > 1)
        {
          aligned_bytes = (bytes / _numColors) & alignment_mask;
          bytecounts[0] =  aligned_bytes;

          for (unsigned c = 1; c < _numColors; ++c)
          {
            bytecounts[c] = aligned_bytes;
            TRACE_FORMAT("<%p> bytecounts[%u] %u", this,c,bytecounts[c]);
          }

          bytecounts[_numColors-1]  = bytes - (aligned_bytes * (_numColors - 1));
          TRACE_FORMAT("<%p> bytecounts[%u] %u", this,_numColors-1,bytecounts[_numColors-1]);
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
        TRACE_FN_EXIT();
      }

      //The Command will be interpretted in the child class
      virtual unsigned restart(void *cmd)
      {
        TRACE_FN_ENTER();
        TRACE_FORMAT("<%p>", this);
        if (CompositeT<NUMCOLORS, T_Bar, T_Exec>::_barrier != NULL)
        {
          // reset barrier since it may be been used between calls
          TRACE_FORMAT("<%p> reset barrier(%p)", this, (CompositeT<NUMCOLORS, T_Bar, T_Exec>::_barrier));

          T_Bar  *barrier =  CompositeT<NUMCOLORS, T_Bar, T_Exec>::_barrier;
          barrier->setDoneCallback(cb_barrier_done, this);
          barrier->start();
        }

        TRACE_FN_EXIT();
        return PAMI_SUCCESS;
      }

      void setDoneCallback(PAMI_Callback_t  cb_done, void * clientdata)
      {
        _cb_done = cb_done;
        _clientdata = clientdata;
      }

      void addBarrier (T_Bar *exec)
      {
        TRACE_FN_ENTER();
        CompositeT<NUMCOLORS, T_Bar, T_Exec>::addBarrier(exec);
        TRACE_FORMAT("<%p> numcolors %u, donecount %u, complete count %u", this,_numColors,_doneCount,_nComplete);
        TRACE_FN_EXIT();
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
        TRACE_FN_ENTER();
        MultiColorCompositeT * composite = (MultiColorCompositeT *) me;
        TRACE_FORMAT("<%p> numcolors %u, donecount %u, complete count %u", me,composite->_numColors,composite->_doneCount,composite->_nComplete);
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
        TRACE_FN_EXIT();
      }

      static void cb_composite_done(pami_context_t context, void *me, pami_result_t err)
      {
        TRACE_FN_ENTER();
        MultiColorCompositeT * composite = (MultiColorCompositeT *) me;
        TRACE_FORMAT("<%p> numcolors %u, donecount %u, complete count %u", me,composite->_numColors,composite->_doneCount,composite->_nComplete);
        CCMI_assert (composite != NULL);

        CCMI_assert (composite->_doneCount <  composite->_nComplete);
        ++composite->_doneCount;

        if (composite->_doneCount == composite->_nComplete) // call users done function
        {
          composite->_cb_done(context, composite->_clientdata, PAMI_SUCCESS);
          //printf ("%d: Composite Done\n", composite->_native->myrank());
        }
        TRACE_FN_EXIT();
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
