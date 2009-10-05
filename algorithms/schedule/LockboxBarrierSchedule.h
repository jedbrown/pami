/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/schedule/LockboxBarrierSchedule.h
 * \brief Schedule for Barrier between local cores using lockbox
 */

#ifndef  __LOCKBOX_BARRIER_SCHEDULE__
#define  __LOCKBOX_BARRIER_SCHEDULE__

#include "Schedule.h"
#include "util/ccmi_util.h"
#include "interface/TorusCollectiveMapping.h"

namespace CCMI
{
  namespace Schedule
  {
// reset indentation (3x)

// Simple barrier schedule for local cores.
// Just use lockbox to barrier all the cores.
    class LockboxBarrierSchedule : public Schedule
    {
    public:
      /**
       * \brief default constructor for local core barrier schedule
       */
      LockboxBarrierSchedule() : Schedule()
      {
      }

      /**
       * \brief Constructor for local core barrier schedule
       *
       * \param[in] map	CollectiveMapping object for geometry
       * \param[in] nranks	Number of ranks in geometry
       * \param[in] ranks	List of ranks in geometry
       */
      LockboxBarrierSchedule(TorusCollectiveMapping *map,
                             unsigned nranks, unsigned *ranks)
      {
        // anything? we assume all ranks are on same chip.
      }

      /**
       * \brief Get the upstream processors.
       *
       * In local core barrier, nobody receives (or sends, really)
       *
       * \param[in] phase	phase of the collective
       * \param[out] srcpes	List of source processors
       * \param[out] nsrc	number of source processors
       * \param[out] subtasks what operations to perform? combine?
       */
      virtual void getSrcPeList(unsigned  phase, unsigned *srcpes,
                                unsigned  &nsrc, unsigned *subtasks=NULL)
      {
        nsrc = 0;
      }

      /**
       * \brief Get the downstream processors to send data to.
       *
       * In this barrier, nobody really sends they just do a lockbox barrier
       *
       * \param[in] phase	phase of the collective
       * \param[out] dstpes	List of source processors
       * \param[out] ndst	number of source processors
       * \param[out] subtasks	what operations to perform? pt-to-pt, line bcast
       */
      virtual void getDstPeList(unsigned  phase, unsigned *dstpes,
                                unsigned  &ndst, unsigned *subtasks)
      {
        *dstpes = CCMI_UNDEFINED_RANK;
        *subtasks = CCMI_LOCKBOX_BARRIER;
        ndst = 1;
      }

      /**
       * \brief Return maximum number of phases ever used by this schedule
       *
       * \param[in] map	CollectiveMapping for the partition/geometry
       * \param[in] nranks	Number of ranks in geometry
       */
      static unsigned getMaxPhases(TorusCollectiveMapping *map, unsigned nranks)
      {
        return 1;
      }

      /**
       * \brief Initialize local core barrier schedule
       *
       * \param[in] root	the root node rank of the collective
       * \param[in] op	Collectiove Operation
       * \param[out] startphase	The phase where I become active
       * \param[out] nphases		number of phases
       * \param[out] maxranks		total number of processors to communicate
       *				with. Mainly needed in the executor to
       *				allocate queues and other resources.
       */
      virtual void init(int root, int op, int &startphase,
                        int &nphases, int &maxranks)
      {
        CCMI_assert(op == BARRIER_OP);
        startphase = 0;
        nphases = 1;
        maxranks = 0;
      }
    };

// end reset indentation (3x)
  };
};

#endif /* !__LOCKBOX_BARRIER_SCHEDULE__ */
