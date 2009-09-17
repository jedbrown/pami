/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/schedule/Schedule.h
 * \brief ???
 */

#ifndef  __schedule_h__
#define  __schedule_h__

#include "util/ccmi_util.h"
#include "util/ccmi_debug.h"
#include "algorithms/ccmi.h"


/*-------------------------------------------------*/
/*     Basic utility classes collectives           */
/*-------------------------------------------------*/

namespace CCMI
{


  //-------------------------------------------------
  //-- Supported communication operations -----------
  //\todo:  this typedef is common to the scheduler
  //        and executor.
  //        It seems like overkill to give it its own
  //        common header file
  //-------------------------------------------------
  typedef enum
  {
    BROADCAST_OP  =      1,
    BARRIER_OP    =      2,
    ALLREDUCE_OP  =      4,
    REDUCE_OP     =      8
  } CollectiveOperation;



  namespace Schedule
  {
    //Abstract reference class
    class Schedule
    {
    public:

      /**
       * \brief Constructor
       */
      Schedule ()
      {
      }

      /**
       * \brief Destructor
       */
      virtual ~Schedule ()
      {
      }

      /// NOTE: This is required to make "C" programs link successfully with virtual destructors
      void operator delete(void * p)
      {
        CCMI_abort();
      }

      /**
       * \brief Get the upstream processors. Source processors
       * that send messages to me in this collective operation
       * \param phase : phase of the collective
       * \param srcpes : List of source processors
       * \param nsrc :  number of source processors
       * \param subtasks : what operations to perform? combine?
       */

      virtual void getSrcPeList (unsigned  phase, unsigned *srcpes,
                                 unsigned  &nsrc, unsigned *subtasks=NULL) = 0;


      /**
       * \brief Get the downstream processors to send data to.
       * \param phase : phase of the collective
       * \param dstpes : List of source processors
       * \param ndst :  number of source processors
       * \param subtasks : what operations to perform? pt-to-pt, line bcast
       */
      virtual void getDstPeList (unsigned  phase, unsigned *dstpes,
                                 unsigned  &ndst, unsigned *subtasks) = 0;

      /**
       * \brief Initialize the schedule for collective operation
       * \param root : the root of the collective
       * \param startphase : The phase where I become active
       * \param nphases : number of phases
       * \param maxranks : total number of processors to communicate
       *  with. Mainly needed in the executor to allocate queues
       *  and other resources
       */
      virtual void init (int root, int op, int &startphase, int &nphases,
                         int &maxranks) = 0;

    };  //-- Schedule
  };  //-- Schedule
};  //-- CCMI

#endif
