/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/composite/Composite.h
 * \brief An executor composite class which supports optional barriers
 */

#ifndef __CCMI_KERNEL_EXECUTOR_COMPOSITE__
#define __CCMI_KERNEL_EXECUTOR_COMPOSITE__

#include "algorithms/executor/Executor.h"

#define MAX_EXECUTORS   6

namespace CCMI
{
  namespace Executor
  {
    class Composite
    {
    protected:
      ///
      /// \brief for synchronous protocols which need a barrier
      /// before the collective. Asynchronous protocols can leave
      /// this variable as NULL.
      ///
      Executor     * _barrier;

      ///
      /// \brief Executors for data movement
      ///
      Executor     * _executors [MAX_EXECUTORS];
      unsigned       _numExecutors;

    public:

      Composite ()
      {
        _barrier = NULL;
        for(int count = 0; count < MAX_EXECUTORS; count ++)
          _executors[count] = NULL;
        _numExecutors = 0;
      }

      /// Default Destructor
      virtual ~Composite()
      {
        for(unsigned count = 0; count < _numExecutors; count ++)
        {
          _executors[count]->~Executor();
          _executors[count] = NULL;
        }
        _numExecutors = 0;
      }
      /// NOTE: This is required to make "C" programs link successfully with virtual destructors
      inline void operator delete(void * p)
      {
        CCMI_abort();
      }

      void addExecutor (Executor *exec)
      {
        CCMI_assert (_numExecutors < MAX_EXECUTORS);
        _executors [_numExecutors] = exec;
        _numExecutors ++;
      }

      void addBarrier (Executor *exec)
      {
        _barrier = exec;
      }

      Executor * getExecutor (int idx)
      {
        return _executors [idx];
      }

      unsigned getNumExecutors ()
      {
        return _numExecutors;
      }

      //virtual void start () = 0;

    };  //-- end class Composite
  };  //-- end namespace Executor
};  //-- end namespace CCMI


#endif
