/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/composite/Composite.h
 * \brief An executor composite class which supports optional barriers
 */

#ifndef __algorithms_composite_Composite_h__
#define __algorithms_composite_Composite_h__

#include "algorithms/executor/Executor.h"

#define MAX_EXECUTORS   6

namespace CCMI
{
  namespace Executor
  {

    class Composite {
    private:
      /// This is required to make "C" programs link successfully with virtual destructors.
      inline void operator delete(void * p)
      {
        PAMI_abort();
      }
    public:
      //Base Composite class
      Composite() {TRACE_ADAPTOR((stderr,"%s\n", __PRETTY_FUNCTION__));}

      ///
      /// \brief Destructor
      ///
      virtual ~Composite() {}

      void setDoneCallback (pami_event_function cb_done, void *cd)
      {
        _cb_done    =   cb_done;
        _clientdata =   cd;
      }

      ///
      /// \brief start a freshly constructed algorithm composite
      ///
      virtual void start() {
            TRACE_ADAPTOR((stderr,"%s\n", __PRETTY_FUNCTION__));
        //Currently not all composites implement this method
	//	CCMI_abort();
      }

      ///
      /// \breif start a previously constructed algorithm composite
      ///
      virtual unsigned restart (void *cmd) {
        //Currently not all composites implement this method
        CCMI_abort();
        return PAMI_SUCCESS;
      } 
      
      void   setAlgorithmFactory (void *f) { _afactory = f; }
      
      void  * getAlgorithmFactory() { return _afactory; }


    protected:
      ///
      ///  \brief Callback to call when the barrier has finished
      ///
      pami_event_function    _cb_done;
      void                * _clientdata;

      //Store a pointer to the algorithm factory
      void                * _afactory;

    };


    template <int NUM_EXECUTORS, class T_Bar, class T_Exec>
    class CompositeT : public Composite
    {
    protected:
      ///
      /// \brief for synchronous protocols which need a barrier
      /// before the collective. Asynchronous protocols can leave
      /// this variable as NULL.
      ///
      T_Bar        * _barrier;

      ///
      /// \brief Executors for data movement
      ///
      T_Exec       * _executors [NUM_EXECUTORS];
      unsigned       _numExecutors;

    public:

      CompositeT () : Composite()
      {
            TRACE_ADAPTOR((stderr,"%s\n", __PRETTY_FUNCTION__));
        _barrier = NULL;
        for(int count = 0; count < MAX_EXECUTORS; count ++)
          _executors[count] = NULL;
        _numExecutors = 0;
      }

      /// Default Destructor
      virtual ~CompositeT()
      {
        for(unsigned count = 0; count < _numExecutors; count ++)
        {
          _executors[count]->~T_Exec();
          _executors[count] = NULL;
        }
        _numExecutors = 0;
      }
      /// NOTE: This is required to make "C" programs link successfully with virtual destructors
      inline void operator delete(void * p)
      {
        CCMI_abort();
      }

      void addExecutor (T_Exec *exec)
      {
        CCMI_assert (_numExecutors < MAX_EXECUTORS);
        _executors [_numExecutors] = exec;
        _numExecutors ++;
      }

      void addBarrier (T_Bar *exec)
      {
        _barrier = exec;
      }

      T_Exec * getExecutor (int idx)
      {
        return _executors [idx];
      }

      unsigned getNumExecutors ()
      {
        return _numExecutors;
      }

      //virtual void start () = 0;

    };  //-- end class Composite

    //The old composite class that depends on Executor::Executor
    typedef CompositeT<6, Composite, Executor> OldComposite;
  };  //-- end namespace Executor
};  //-- end namespace CCMI


#endif
