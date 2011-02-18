/**
 * \file algorithms/interfaces/Schedule.h
 * \brief ???
 */

#ifndef __algorithms_interfaces_Schedule_h__
#define __algorithms_interfaces_Schedule_h__

#include "util/ccmi_util.h"
#include "util/ccmi_debug.h"
#include "algorithms/ccmi.h"
#include "algorithms/interfaces/NativeInterface.h"
#include "Topology.h"

/*-------------------------------------------------*/
/*     Basic utility classes collectives           */
/*-------------------------------------------------*/

namespace CCMI
{
#if 0
  typedef void      (*ScheduleFn)   (void                        * buf,
                                     unsigned                      size,
                                     unsigned                      root,
                                     Interfaces::NativeInterface * native,
                                     PAMI_GEOMETRY_CLASS         * g);
#endif

  template<class T_Geometry>
    struct SFunc
  {
    typedef void      (*ScheduleFn)   (void                        * buf,
				       unsigned                      size,
				       unsigned                      root,
				       Interfaces::NativeInterface * native,
				       T_Geometry         * g);
  };


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


    };  //-- Schedule
  };  //-- Schedule
};  //-- CCMI

#endif
