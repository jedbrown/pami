/**
 * \file algorithms/interfaces/ReduceSchedule.h
 * \brief ???
 */

#ifndef __algorithms_interfaces_ReduceSchedule_h__
#define __algorithms_interfaces_ReduceSchedule_h__

#include "util/ccmi_util.h"
#include "util/ccmi_debug.h"
#include "algorithms/ccmi.h"
#include "algorithms/interfaces/Schedule.h"


/*-------------------------------------------------*/
/*     Basic utility classes collectives           */
/*-------------------------------------------------*/

#define UNDEFINED_PHASE  (unsigned)-1

namespace CCMI
{
  namespace Interfaces
  {
    /**
     * \brief Abstract reference class to define the schedule of Reduce and Allreduce Collectives
     */
    template <class Topology>
      class ReduceSchedule : public Schedule<T>
    {
    public:

      /**
       * \brief Constructor
       */
      RingSchedule : Schedule ()
      {
      }

      /**
       * \brief Destructor
       */
      virtual ~RingSchedule ()
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
      virtual void init (int root, int op, int &startphase, int &nphases) = 0;

      /**
       * \brief Get the upstream processors. Source processors
       * that send messages to me in this collective operation
       * \param[IN] phase : phase of the collective
       * \retval topology : the topolgy that sends messages to me in this phase
       */
      virtual Toplogy *getSrcTopology (unsigned  phase) = 0;

      /**
       * \brief Get the downstream processors to send data to.
       * \param phase : phase of the collective
       * \retval Topology : The topology to send messages to in this phase
       */
      virtual Topology *getDstTopology (unsigned  phase, Topology *topology) = 0;

      /**
       * \brief Get the union of all sources across all phases
       * \retval Topology : the union of all sources
       */
      virtual Topology *getSrcUnionTopology () = 0;

      /**
       * \brief Get the union of all destinations across all phases
       * \retval Topology : the union of all sources
       */
      /**
       * \brief Get the union of all destinations across all phases
       * \retval Topology : the union of all sources
       */
      virtual Topology *getDstUnionTopology () = 0;

      /**
       * \brief Get the phase which the local computation must be performed
       * \param [inout] phase
       * \param [inout] the source index if there are multiple sources contributing
       *                data in this phase
       */
      virtual getLocalCombinePhase(unsigned &phase, unsigned &index) = 0;

      /**
       * \brief Get the phase in the reduce+bcast allreduce where the
       *        last arithmatic sum operation is performed. The broadcast
       *        phases after this phase have no dependencies on the combine
       *        phases.
       */
      virtual unsigned getLastCombinePhase () = 0;


      /**
       * \brief Get the phase in the reduce+bcast allreduce where
       *        result is broadcast.
       */
      virtual unsigned getBcastSendPhase () = 0;


    };  //-- Schedule
  };  //-- Schedule
};  //-- CCMI

#endif
