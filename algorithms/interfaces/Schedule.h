#ifndef  __ccmi_interfaces_schedule_h__
#define  __ccmi_interfaces_schedule_h__

#include "util/ccmi_util.h"
#include "util/ccmi_debug.h"
#include "algorithms/ccmi.h"


/*-------------------------------------------------*/
/*     Basic utility classes collectives           */
/*-------------------------------------------------*/

namespace CCMI
{
  namespace Interfaces
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

    /**
     * \brief Abstract reference class to define the schedule of a collective
     */
    template <class Topology> class Schedule
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
      virtual Topology *getDstUnionTopology () = 0;

    };  //-- Schedule
  };  //-- Schedule
};  //-- CCMI

#endif
