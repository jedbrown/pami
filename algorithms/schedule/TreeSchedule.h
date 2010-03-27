/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/schedule/TreeSchedule.h
 * \brief Schedule for Barrier, Broadcast, and [All]Reduce
 * on the BlueGene tree network
 */

#ifndef __algorithms_schedule_TreeSchedule_h__
#define __algorithms_schedule_TreeSchedule_h__

#include "algorithms/interfaces/Schedule.h"
#include "util/ccmi_util.h"
#include "interface/TorusCollectiveMapping.h"
#include "algorithms/schedule/BinomialTree.h"	// or SpraySchedule.h
#include "LockboxBarrierSchedule.h"

#ifndef TREE_LOCAL_SCHED_TYPE
  #define TREE_LOCAL_SCHED_TYPE	BinomialTreeSchedule	// or SpraySchedule
#endif /* !TREE_LOCAL_SCHED_TYPE */


namespace CCMI
{
  namespace Schedule
  {
// reset indentation (3x)

#define MAX_TREE_PHASES	1
#define NUM_TREE_PHASES	MAX_TREE_PHASES

// Hybrid schedule for using external schedule(s) for local reduce/bcast
    class TreeSchedule : public Schedule
    {
    public:
      /**
       * \brief Constructor
       */
      TreeSchedule() :  Schedule()
      {
      }

      TreeSchedule(TorusCollectiveMapping *map, unsigned nranks, unsigned *ranks);

      /**
       * \brief Return maximum number of phases ever used by this schedule
       *
       * \param[in] map	CollectiveMapping for the partition/geometry
       * \param[in] nranks	Number of ranks in geometry
       */
      static unsigned getMaxPhases(TorusCollectiveMapping *map, unsigned nranks)
      {
        unsigned m, max = 0;

        /*
         * One or other, LockboxBarrierSchedule or TREE_LOCAL_SCHED_TYPE.
         * If this decision gets any more complex, it will complicate
         * this computation. Unknown if this ever will be used, anyway.
         */
        max = LockboxBarrierSchedule::getMaxPhases(map, MAX_NUM_CORES);
        m = TREE_LOCAL_SCHED_TYPE::getMaxPhases(map, MAX_NUM_CORES);
        if(m > max)
        {
          max = m;
        }
        /* two are used, potentially */
        max *= 2;
        /* plus any phases used internally (Tree) */
        max += MAX_TREE_PHASES;

        return max;
      }

    protected:

      /// \note _peer_ranks[0] is always the designated local root.
      /// May also be the collective root.
      ///
      TorusCollectiveMapping *_mapping;   /**< mapping for geometry */
      CollectiveOperation _op;  /**< collective operation being done */
      unsigned _root;     /**< root node in current op */
      unsigned _npeers;   /**< number of peer nodes */
      unsigned _peer_ranks[MAX_NUM_CORES];  /**< peer nodes ranks */

      // One for (pre-)reduce and one for (post-)broadcast
#define NUM_EXT_SCHED	2

      // These must be large enough to hold the larget schedule
      // used, but otherwise can be declared of any type.
      // We use TREE_LOCAL_SCHED_TYPE as a convenience, for now.
      //
      char _scheds[NUM_EXT_SCHED][sizeof(TREE_LOCAL_SCHED_TYPE)]; /**< external schedule objects */
      unsigned _nxsched;        /**< allocator for _scheds[] */

#define NUM_SCHED	3
      Schedule *_loc_sched[NUM_SCHED];  /**< sub-schedule pointers */
      unsigned _startph[NUM_SCHED];   /**< start phase used by sub-sched */
      unsigned _numph[NUM_SCHED];   /**< num phases used by sub-sched */
      unsigned _maxph[NUM_SCHED];   /**< max phases used by sub-sched */
      unsigned _nsched;     /**< allocator for above arrays */

      int _nmsg;  /**< number of receives needed, max */

      /**
       * \brief Setup a schedule to process local cores.
       *
       * \param[in] op	Collective Op to create schedule for
       * \param[in,out] maxph	Cummulative number of phases used, maximum
       */
      inline void initLocSched(CollectiveOperation op, unsigned &maxph)
      {
        int start = 0, nph = 0, nr = 0, max = 0;
        unsigned idx = _nsched++;
        unsigned ix = _nxsched++;
        CCMI_assert((ix < NUM_EXT_SCHED) && (idx < NUM_SCHED));

        if(_op == BARRIER_OP)
        {
          max = LockboxBarrierSchedule::getMaxPhases(_mapping, MAX_NUM_CORES);
          if(_npeers > 1)
          {
           COMPILE_TIME_ASSERT(sizeof(_scheds[0]) >= sizeof(LockboxBarrierSchedule));
            _loc_sched[idx] = new (&_scheds[ix])
            LockboxBarrierSchedule(_mapping,
                                   _npeers, _peer_ranks);
            _loc_sched[idx]->init(_peer_ranks[0],
                                  BARRIER_OP, start, nph, nr);
          }
          else
          {
            _loc_sched[idx] = NULL;
          }
        }
        else // op == BROADCAST_OP...
        {
          max = TREE_LOCAL_SCHED_TYPE::getMaxPhases(_mapping, MAX_NUM_CORES);
          if(_npeers > 1)
          {
           COMPILE_TIME_ASSERT(sizeof(_scheds[0]) >= sizeof(TREE_LOCAL_SCHED_TYPE));
            _loc_sched[idx] = new (&_scheds[ix])
            TREE_LOCAL_SCHED_TYPE(_mapping,
                                  _npeers, _peer_ranks);
            _loc_sched[idx]->init(_peer_ranks[0],
                                  op, start, nph, nr);
          }
          else
          {
            _loc_sched[idx] = NULL;
          }
        }
        _maxph[idx] = max;
        maxph += max;
        _startph[idx] = start;
        _numph[idx] = nph;
        _nmsg += nr;
      }

      /**
       * \brief Do final initialization of the Tree schedule
       *
       * This schedule always starts at phase 0 and contains 1 phase,
       * but only for nodes which are the "local root" - i.e. the designated
       * networking core on this chip. That is defined as _peer_ranks[0].
       *
       * \param[in,out] maxph	Cummulative number of phases used, maximum
       */
      inline void initTreeSched(unsigned &maxph)
      {
        unsigned idx = _nsched++;
        _loc_sched[idx] = NULL;
        _maxph[idx] = MAX_TREE_PHASES;
        maxph += MAX_TREE_PHASES;
        if(_mapping->rank() == _peer_ranks[0])
        {
          _startph[idx] = 0;
          _numph[idx] = NUM_TREE_PHASES;
          _nmsg += NUM_TREE_PHASES;
        }
        else
        {
          _startph[idx] = 0;
          _numph[idx] = 0;
        }
      }

      /**
       * \brief Translate global phase number into schedule-specific phase
       *
       * Also idenifies the schedule object to be used for that phase.
       * Note, schedule pointer may be NULL in the case of the (builtin)
       * Tree schedule.
       *
       * \param[in] phase	Global phase number to convert
       * \param[out] which	Schedule used in phase, if not UNDEFINED_PHASE
       * \return	Phase number and schedule in 'which', or UNDEFINED_PHASE
       */
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

      /**
       * \brief Initialize Tree schedule including sub-schedules for local cores
       *
       * \param[out] max	maximum number of phases used by all/any nodes
       */
      inline void initAllSched(unsigned &max)
      {
        unsigned m = 0;
        _nsched = 0;
        _nxsched = 0;
        if(_op != BROADCAST_OP)
        {
          initLocSched(REDUCE_OP, m);
        }
        initTreeSched(m);
        if(_op != REDUCE_OP)
        {
          initLocSched(BROADCAST_OP, m);
        }
        max = m;
      }

      /**
       * \brief Get the upstream processors.
       * Source processors that send messages to me in this collective operation
       * \param[in] phase	phase of the collective
       * \param[out] srcpes	List of source processors
       * \param[out] nsrc	number of source processors
       * \param[out] subtasks what operations to perform? combine?
       */
      virtual void getSrcPeList(unsigned phase, unsigned *srcpes,
                                unsigned &nsrc, unsigned *subtasks=NULL)
      {
        unsigned p;
        Schedule *s;
        unsigned n = 0;

        p = xlatPhase(phase, &s);
        if(p == CCMI_UNDEFINED_PHASE)
        {
          // nsrc = 0;
        }
        else if(s)
        {
          s->getSrcPeList(p, srcpes, n, subtasks);
        }
        else
        {
          switch(_op)
          {
          case BARRIER_OP:
            // nothing to do (recv) in Tree phase
            break;
          case BROADCAST_OP:
            if(_mapping->rank() != _root)
            {
              srcpes[n] = _mapping->rank();
              subtasks[n] = CCMI_BCAST_RECV_STORE;
              ++n;
            }
            break;
          case ALLREDUCE_OP:
            srcpes[n] = _mapping->rank();
            subtasks[n] = CCMI_REDUCE_RECV_STORE;
            ++n;
            break;
          case REDUCE_OP:
            srcpes[n] = _mapping->rank();
            subtasks[n] = _mapping->rank() == _root ?
                          CCMI_REDUCE_RECV_STORE :
                          CCMI_REDUCE_RECV_NOSTORE;
            ++n;
            break;
          default:
            CCMI_abort();
          }
        }
        nsrc = n;
      }

      /**
       * \brief Get the downstream processors to send data to.
       * \param[in] phase	phase of the collective
       * \param[out] dstpes	List of source processors
       * \param[out] ndst	number of source processors
       * \param[out] subtasks	what operations to perform? pt-to-pt, line bcast
       */
      virtual void getDstPeList(unsigned phase, unsigned *dstpes,
                                unsigned &ndst, unsigned *subtasks)
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
          switch(_op)
          {
          case BARRIER_OP:
            dstpes[n] = CCMI_UNDEFINED_RANK;
            subtasks[n] = CCMI_TREE_BARRIER;
            ++n;
            break;
          case BROADCAST_OP:
            if(_mapping->rank() == _root)
            {
              dstpes[n] = _root;
              subtasks[n] = CCMI_TREE_BCAST;
              ++n;
            }
            break;
          case ALLREDUCE_OP:
            // everyone "does" the ALLREDUCE - all except root add data
            dstpes[n] = _root;
            subtasks[n] = CCMI_TREE_ALLREDUCE;
            n++;
            break;
          case REDUCE_OP:
            // everyone "does" the ALLREDUCE - all except root add data
            dstpes[n] = _root;
            subtasks[n] = CCMI_TREE_ALLREDUCE;
            ++n;
            break;
          default:
            CCMI_abort();
          }
        }
        ndst = n;
      }

      /**
       * \brief Initialize the schedule for collective operation
       *
       * This routine is only concerned with the Tree Network
       * portion of the schedule. The local core parts are done
       * by other, external, schedules (e.g. BinomialTreeSchedule).
       * However, those external schedules are initialized through
       * this routine.
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
        unsigned max = 0;
        int start = 0;

        _op = (CollectiveOperation)op;
        _root = root;

        /// \note _root might be invalid (-1),
        /// i.e. not applicable. So we must ensure getXXXPeList()
        /// does the right thing.

        // move root to _peer_ranks[0].
        // NOTE! Only one collective operation in progress at a time!
        if(_op != BARRIER_OP && _op != ALLREDUCE_OP)
        {
          for(unsigned i = 0; i < _npeers; ++i)
          {
            if(_peer_ranks[i] == _root)
            {
              unsigned tmp = _peer_ranks[i];
              _peer_ranks[i] = _peer_ranks[0];
              _peer_ranks[0] = tmp;
              break;
            }
          }
        }
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


      virtual void
      init(int root, int op, int &startphase, int &nphases) { PAMI_abort();}
      virtual void
      getSrcTopology (unsigned phase, PAMI::Topology *topology){ PAMI_abort();}
      virtual void
      getDstTopology (unsigned phase, PAMI::Topology *topology){ PAMI_abort();}
      virtual pami_result_t
      getSrcUnionTopology (PAMI::Topology *topology){ PAMI_abort();}
      virtual pami_result_t
      getDstUnionTopology (PAMI::Topology *topology){ PAMI_abort();}




      static inline void _compile_time_assert_ ()
      {
        // Compile time assert
        COMPILE_TIME_ASSERT(sizeof(LockboxBarrierSchedule) <= sizeof(TREE_LOCAL_SCHED_TYPE));
      }

    };

// end reset indentation (3x)
  };
};



/**
 * \brief	Constructor for a Tree-network Schedule
 *
 * Builds a list of peer nodes - nodes which share the same chip as us.
 * i.e. "local cores".  The rest of schedule cannot be built until we
 * know the root node and collective operation.
 *
 * \param[in] map	CollectiveMapping object for geometry
 * \param[in] nranks	Number of ranks in geometry
 * \param[in] ranks	List of ranks in geometry
 */
inline CCMI::Schedule::TreeSchedule::
TreeSchedule(TorusCollectiveMapping *map, unsigned nranks, unsigned *ranks)
{
  unsigned i, t_size;
  unsigned tmp_coords[CCMI_TORUS_NDIMS];

  CCMI_assert_debug(map != NULL);
  _mapping = map;
  // CCMI_assert_debug(nranks == _mapping->size());
  /**
   * \note _peer_ranks[] does include ourself.
   */
  if((t_size = _mapping->GetDimLength(CCMI_T_DIM)) > 1)
  {
    _npeers = 0;
    unsigned trank;
    CCMI_COPY_COORDS(tmp_coords, _mapping->Coords());
    for(i = 0; i < t_size; ++i)
    {
      tmp_coords[CCMI_T_DIM] = i;
      if(PAMI_SUCCESS  ==
         _mapping->Torus2Rank(tmp_coords, &trank))
        _peer_ranks[_npeers++] = trank;
    }
  }
  else
  {
    _npeers = 1;
    _peer_ranks[0] = _mapping->rank();
  }
}

#endif
