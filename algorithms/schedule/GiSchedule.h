/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/schedule/GiSchedule.h
 * \brief ???
 */

#ifndef __algorithms_schedule_GiSchedule_h__
#define __algorithms_schedule_GiSchedule_h__

#include "Schedule.h"
#include "util/ccmi_util.h"
#include "interface/TorusCollectiveMapping.h"
#include "LockboxBarrierSchedule.h"

namespace CCMI
{
  namespace Schedule
  {
// reset indentation (3x)

#define MAX_GI_PHASES 1
#define NUM_GI_PHASES MAX_GI_PHASES

    class GiSchedule : public Schedule
    {
    protected:
      TorusCollectiveMapping *_mapping;     /**< mapping for geometry */
      unsigned _npeers;     /**< number of peer nodes */
      unsigned _peer_ranks[MAX_NUM_CORES];  /**< peer nodes ranks */
      unsigned _masterRank;  /**< the master rank for the node */

      // One for (pre-)reduce and one for (post-)broadcast
#define NUM_EXT_SCHED   2

      // These must be large enough to hold the larget schedule
      // used, but otherwise can be declared of any type.
      // We use "LockboxBarrierSchedule" as a convenience, for now.
      //
      char     _scheds[NUM_EXT_SCHED][sizeof(LockboxBarrierSchedule)]; /**< external schedule objects */
      unsigned _nxsched;        /**< allocator for _scheds[] */

#define NUM_SCHED       3
      Schedule *_loc_sched[NUM_SCHED];  /**< sub-schedule pointers */
      unsigned _startph[NUM_SCHED];   /**< start phase used by sub-sched */
      unsigned _numph[NUM_SCHED];   /**< num phases used by sub-sched */
      unsigned _maxph[NUM_SCHED];   /**< max phases used by sub-sched */
      unsigned _nsched;     /**< allocator for above arrays */

      int _nmsg;  /**< number of receives needed, max */

      inline void initLocSched(CollectiveOperation op, unsigned &maxph)
      {
        int start = 0, nph = 0, nr = 0, max = 0;
        unsigned idx = _nsched++;
        unsigned ix = _nxsched++;

        max = LockboxBarrierSchedule::getMaxPhases(_mapping, MAX_NUM_CORES);
        if(_npeers > 1)
        {
          CCMI_assert((ix < NUM_EXT_SCHED) && (idx < NUM_SCHED));
          COMPILE_TIME_ASSERT(sizeof(_scheds[0]) >= sizeof(LockboxBarrierSchedule));
          _loc_sched[idx] = new (&_scheds[ix])
          LockboxBarrierSchedule(_mapping,
                                 _npeers, _peer_ranks);
          _loc_sched[idx]->init(_peer_ranks[0],
                                BARRIER_OP, start, nph, nr);
        }
        _maxph[idx] = max;
        maxph += max;
        _startph[idx] = start;
        _numph[idx] = nph;
        _nmsg += nr;
      }

      inline void initTreeSched(unsigned &maxph)
      {
        unsigned idx = _nsched++;
        _loc_sched[idx] = NULL;
        _maxph[idx] = MAX_GI_PHASES;
        maxph += MAX_GI_PHASES;
        if(_mapping->rank() == _masterRank)
        {
          _startph[idx] = 0;
          _numph[idx] = NUM_GI_PHASES;
          _nmsg += NUM_GI_PHASES;
        }
        else
        {
          _startph[idx] = 0;
          _numph[idx] = 0;
        }
      }

      inline void initAllSched(unsigned &max)
      {
        unsigned m = 0;
        _nsched = 0;
        _nxsched = 0;

        if(_mapping->GetDimLength(CCMI_T_DIM) == 1)
        {
          initTreeSched(m);
        }
        else
        {
          initLocSched(BARRIER_OP, m);
          initTreeSched(m);
          initLocSched(BARRIER_OP, m);
        }
        max = m;
      }

      inline unsigned xlatPhase(unsigned phase, Schedule **which)
      {
        unsigned i, p = phase;

        for(i = 0; i < _nsched; ++i )
        {
          if(p < _maxph[i])
          {
            if(p >= _startph[i] &&
               p < _startph[i] + _numph[i])
            {
              *which = _loc_sched[i];
              return p;
            }
            else
            {
              break;
            }
          }
          p -= _maxph[i];
        }
        return CCMI_UNDEFINED_PHASE;
      }

    public:
      /**
       * \brief Constructor
       */
      GiSchedule() : Schedule()
      {
      }

      GiSchedule(TorusCollectiveMapping *map, unsigned nranks, unsigned *ranks);

      /**
       * \brief Get the upstream processors. Source processors
       * that send messages to me in this collective operation
       * \param phase : phase of the collective
       * \param srcpes : List of source processors
       * \param nsrc :  number of source processors
       * \param subtasks: what operations to perform? combine?
       */
      virtual void getSrcPeList (unsigned  phase, unsigned *srcpes,
                                 unsigned  &nsrc, unsigned *subtasks=NULL)
      {

        unsigned p;
        Schedule *s;
        unsigned n = 0;

        p = xlatPhase(phase, &s);
        if(p == CCMI_UNDEFINED_PHASE)
        {
          // n = 0;
        }
        else if(s)
        {
          s->getSrcPeList(p, srcpes, n, subtasks);
        }
        else
        {
          // n = 0;
        }
        nsrc = n;
      }


      /**
       * \brief Get the downstream processors to send data to.
       * \param phase : phase of the collective
       * \param dstpes : List of source processors
       * \param ndst :  number of source processors
       * \param subtasks : what operations to perform? pt-to-pt, line bcast
       */
      virtual void getDstPeList (unsigned  phase, unsigned *dstpes,
                                 unsigned  &ndst, unsigned *subtasks)
      {
        unsigned p;
        Schedule *s;
        unsigned n = 0;

        p = xlatPhase(phase, &s);
        if(p == CCMI_UNDEFINED_PHASE)
        {
          // n = 0;
        }
        else if(s)
        {
          s->getDstPeList(p, dstpes, n, subtasks);
        }
        else
        {
          dstpes[n] = CCMI_UNDEFINED_RANK;
          subtasks[n] =  CCMI_GI_BARRIER;
          ++n;
        }
        ndst = n;
      }


      /**
       * \brief Initialize the schedule for collective operation
       * \param root : the root of the collective
       * \param startphase : The phase where I become active
       * \param nphases : number of phases
       * \param maxranks : total number of processors to communicate
       *  with. Mainly needed in the executor to allocate queues
       *  and other resources
       */
      virtual void init(int root, int op, int &startphase,
                        int &nphases, int &maxranks)
      {
        unsigned max = 0;
        int start = 0;
        CCMI_assert(op == BARRIER_OP);

        _nmsg = 0;
        initAllSched(max);
        CCMI_assert(_nsched >= 1);
        for(unsigned i = 0; i < _nsched; ++i)
        {
          if(_numph[i])
          {
            start += _startph[i];
            break;
          }
          start += _maxph[i];
        }
        startphase = start;
        nphases = max - start;
        maxranks = _nmsg;
      }
    };
// restore indentation (3x)
  };
};



inline CCMI::Schedule::GiSchedule::GiSchedule(TorusCollectiveMapping *map,
                                              unsigned nranks,
                                              unsigned *ranks) :
_masterRank(CCMI_UNDEFINED_RANK)
{
  unsigned i, t_min;
  unsigned coords[CCMI_TORUS_NDIMS];
  unsigned* my_coords;

  CCMI_assert (map != NULL);
  _mapping  =  map;

  //CCMI_assert_debug(nranks == map->size());

  if((t_min = _mapping->GetDimLength(CCMI_T_DIM)) > 1)
  {
    _npeers = 0;
    my_coords = _mapping->Coords();
    for(i = 0; i < nranks; ++i)
    {
      _mapping->Rank2Torus(&(coords[0]), ranks[i]);
      //_mapping->rank2torus(ranks[i], x, y, z, t);
      if( coords[CCMI_X_DIM] == my_coords[CCMI_X_DIM] &&
          coords[CCMI_Y_DIM] == my_coords[CCMI_Y_DIM] &&
          coords[CCMI_Z_DIM] == my_coords[CCMI_Z_DIM])
      {
        // Pick the lowest core on the node as master (to match sysdep LockManager)
        if(coords[CCMI_T_DIM] < t_min)
        {
          _masterRank = ranks[i];
          t_min = coords[CCMI_T_DIM];
        }
        _peer_ranks[_npeers++] = ranks[i];
      }
    }
  }
  else
  {
    _npeers = 1;
    _masterRank = _peer_ranks[0] = _mapping->rank();
  }
}


#endif	/* !__GI_SCHEDULE__ */
