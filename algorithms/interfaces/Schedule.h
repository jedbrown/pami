/**
 * \file algorithms/interfaces/Schedule.h
 * \brief ???
 */

#ifndef __algorithms_interfaces_Schedule_h__
#define __algorithms_interfaces_Schedule_h__

#include "util/ccmi_util.h"
#include "util/ccmi_debug.h"
#include "algorithms/ccmi.h"
#include "Topology.h"

/*-------------------------------------------------*/
/*     Basic utility classes collectives           */
/*-------------------------------------------------*/

namespace CCMI
{
      /**
     * \brief The different collective operations supported
     */
    typedef enum
    {
      BROADCAST_OP  =      1,
      BARRIER_OP    =      2,
      ALLREDUCE_OP  =      4,
      REDUCE_OP     =      8
    } CollectiveOperation;


  namespace Interfaces
  {
    /**
     * \brief Abstract reference class to define the schedule of a collective
     */
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
       * \brief Initialize the schedule for collective operation
       * \param root : the root of the collective
       * \param startphase : The phase where I become active
       * \param nphases : number of phases
       */
      virtual void
      init(int root, int op, int &startphase, int &nphases, int &maxranks)=0;
      virtual void
      init(int root, int op, int &startphase, int &nphases)=0 ;

      /**
       * \brief Get the upstream processors. Source processors
       * that send messages to me in this collective operation
       * \param[IN] phase : phase of the collective
       * \param[INOUT] topology : the topolgy that sends messages to me in this phase
       */
      virtual void
      getSrcTopology (unsigned phase, PAMI::Topology *topology)=0;

      /**
       * \brief Get the downstream processors to send data to.
       * \param phase : phase of the collective
       * \param[INOUT] topology : The topology to send messages to in this phase
       */
      virtual void
      getDstTopology (unsigned phase, PAMI::Topology *topology)=0;

      /**
       * \brief Get the union of all sources across all phases
       * \param[INOUT] topology : the union of all sources
       */
      virtual pami_result_t
      getSrcUnionTopology (PAMI::Topology *topology)=0;

      /**
       * \brief Get the union of all destinations across all phases
       * \param[INOUT] topology : the union of all sources
       */
      virtual pami_result_t
      getDstUnionTopology (PAMI::Topology *topology)=0;

      /**
       * Old ccmi schedule interfaces
       */

      /**
       * \brief Get the upstream processors. Source processors
       * that send messages to me in this collective operation
       * \param phase : phase of the collective
       * \param srcpes : List of source processors
       * \param nsrc :  number of source processors
       * \param subtasks : what operations to perform? combine?
       */

      virtual void getSrcPeList (unsigned  phase, unsigned *srcpes,
                                 unsigned  &nsrc, unsigned *subtasks=NULL)=0;


      /**
       * \brief Get the downstream processors to send data to.
       * \param phase : phase of the collective
       * \param dstpes : List of source processors
       * \param ndst :  number of source processors
       * \param subtasks : what operations to perform? pt-to-pt, line bcast
       */
      virtual void getDstPeList (unsigned  phase, unsigned *dstpes,
                                 unsigned  &ndst, unsigned *subtasks)=0;

      /**
       * \brief Initialize the schedule for collective operation
       * \param root : the root of the collective
       * \param startphase : The phase where I become active
       * \param nphases : number of phases
       * \param maxranks : total number of processors to communicate
       *  with. Mainly needed in the executor to allocate queues
       *  and other resources
       */


    };  //-- Schedule
  };  //-- Schedule
};  //-- CCMI

#endif
