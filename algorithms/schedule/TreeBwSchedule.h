/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/schedule/TreeBwSchedule.h
 * \brief ???
 */

#ifndef __algorithms_schedule_TreeBwSchedule_h__
#define __algorithms_schedule_TreeBwSchedule_h__

#include "Schedule.h"
#include "util/ccmi_util.h"
#include "interface/TorusCollectiveMapping.h"

namespace CCMI
{
  namespace Schedule
  {

    class TreeBwSchedule : public Schedule
    {

    public:
      /**
       * \brief Constructor
       */
      TreeBwSchedule () : Schedule ()
      {
      }

      TreeBwSchedule (TorusCollectiveMapping *map, unsigned nranks, unsigned *ranks);

      void getBroadcastSources (unsigned  phase, unsigned *srcpes,
                                unsigned  &nsrc, unsigned *tasks)
      {
        //fprintf (stdout,
        //   "%d: Broadcast::getSrcPeList %d, startbcastphase %d\n",
        //   _mapping->rank(), phase, _startBcastPhase);

        nsrc = 0;
        if(_mapping->rank() != _root && phase == _startBcastPhase)
        {
          nsrc      =  1;
          CCMI_Subtask task;

          if(_op == BROADCAST_OP)
            task = CCMI_BCAST_RECV_STORE;
          else if(_op == ALLREDUCE_OP)
            task = CCMI_REDUCE_RECV_STORE;
          else task = CCMI_UNDEFINED_SUBTASK;

          *tasks   = (_isHead) ? task : CCMI_PT_TO_PT_SUBTASK;
          *srcpes  = (_isHead) ? CCMI_UNDEFINED_RANK : _head;
        }
      }

      /// Spray broadcast
      void getBroadcastDestinations (unsigned phase, unsigned *dstpes,
                                     unsigned &ndest, unsigned *subtasks)
      {
        ndest = 0;

        /// first phase root broadcasts on tree
        if(_mapping->rank() == _root && phase == _startBcastPhase)
        {
          dstpes [ndest]   = _root;
          subtasks [ndest] = CCMI_TREE_BCAST;
          ndest = 1;
        }

        /// second phase all head nodes (including root) broadcast
        /// internally
        if(_isHead && phase == (unsigned)_startBcastPhase+1)
        {
          ndest = _mapping->GetDimLength(CCMI_T_DIM)-1;
          for(unsigned count = 0; count < ndest; count ++)
          {
            dstpes [count]   = _peers[count];
            subtasks [count] = CCMI_PT_TO_PT_SUBTASK;
          }
        }
      }


      /// Chain to the local tail peer and then perform tree collective
      /// (0->1->2->3->Tree send->recv->0)
      void getReduceSources (unsigned  phase, unsigned *srcpes,
                             unsigned  &nsrc, unsigned *tasks)
      {
        nsrc = 0;
        if(!_isHead && phase == _startReducePhase)
        {
          nsrc = 1;
          *srcpes = _prev;
          *tasks  = CCMI_COMBINE_SUBTASK;
        }
        else if(_isHead && (phase == _mapping->GetDimLength(CCMI_T_DIM) - 1))
        {
          nsrc = 1;
          *srcpes = CCMI_UNDEFINED_RANK;

          ///Allreduce operation
          if(_op == ALLREDUCE_OP)
            *tasks  = CCMI_REDUCE_RECV_STORE;
          ///Reduce operation
          else if(_mapping->rank() == _root)
            *tasks = CCMI_REDUCE_RECV_STORE;
          else
            *tasks = CCMI_REDUCE_RECV_NOSTORE;
        }
      }


      //Ring reduce here root will send pt-to-pt message and receive
      //on the collective network. The tail core will inject on the
      //collective network.
      void getReduceDestinations (unsigned phase, unsigned *dstpes,
                                  unsigned &ndest, unsigned *subtasks)
      {
        unsigned sendphase = _startReducePhase + ((_isHead) ? 0 : 1);

        ndest = 0;
        if(phase == sendphase)
        {
          ndest = 1;
          if(!_isTail)
          {
            *dstpes   = _next;
            *subtasks = CCMI_PT_TO_PT_SUBTASK;
          }
          else //tail and phase == sendphase
          {
            *dstpes   = CCMI_UNDEFINED_RANK;
            *subtasks = CCMI_TREE_ALLREDUCE;
          }
        }
      }

      void getBarrierSources (unsigned  phase, unsigned *srcpes,
                              unsigned  &nsrc, unsigned *tasks)
      {
        nsrc      =  0;
        *srcpes   =  CCMI_UNDEFINED_RANK;
        *tasks    =  CCMI_TREE_BARRIER;
      }

      void getBarrierDestinations (unsigned  phase, unsigned *dstpes,
                                   unsigned  &ndst, unsigned *subtasks)
      {
        ndst = 0;
        switch(phase)
        {
        case 0 :
        case 2:
          * dstpes   =  CCMI_UNDEFINED_RANK;
          * subtasks =  CCMI_LOCKBOX_BARRIER;
          ndst    =  1;
          break;

        case 1:
          if(_isHead)
          {
            *dstpes   =  CCMI_UNDEFINED_RANK;
            *subtasks =  CCMI_TREE_BARRIER;
            ndst    =  1;
          }
          break;

        default:
          CCMI_abort();
          break;
        };
      }


      void local_init (int root, int op, int &startphase,
                       int &nphases, int &maxranks)
      {
        if(root >= 0)
          _root = root;
        else
          _root = 0;

        unsigned coords[CCMI_TORUS_NDIMS], root_t;
        unsigned* my_coords = _mapping->Coords();

        _mapping->Rank2Torus (&(coords[0]), _root);
        root_t = coords[CCMI_T_DIM];

        if(my_coords[CCMI_T_DIM] == root_t) {
          _isHead = true;
	  _head   = _mapping->rank();
	}
	else {
          CCMI_COPY_COORDS(coords, my_coords);
          coords[CCMI_T_DIM] = root_t;
	  _mapping->Torus2Rank (coords, &_head);
	}

        maxranks += 4;

        if(op == BARRIER_OP)
        {
          _startPhase = 0;
          nphases     = 3;
        }
        else if(op == BROADCAST_OP)
        {
          nphases     = (_isHead) ? 2 : 1;
          _startPhase = _startBcastPhase = (_isHead) ? 0 : 1;
        }
        else if(op == REDUCE_OP)
        {
	  unsigned tail_t = (root_t + _mapping->GetDimLength(CCMI_T_DIM) - 1) %
	    (_mapping->GetDimLength(CCMI_T_DIM));

	  if(my_coords[CCMI_T_DIM] == tail_t)
	    _isTail = true;

	  int next_t = my_coords[CCMI_T_DIM] +
            ((root_t <= my_coords[CCMI_T_DIM]) ? 1 : (-1));
	  if(next_t < 0)
	    next_t += _mapping->GetDimLength(CCMI_T_DIM);

          CCMI_COPY_COORDS(coords, my_coords);
          coords[CCMI_T_DIM] = next_t;
	  _mapping->Torus2Rank (coords, &_next);

	  int prev_t = my_coords[CCMI_T_DIM] +
            ((root_t <= my_coords[CCMI_T_DIM]) ? (-1) : 1);
	  if(prev_t < 0)
	    prev_t += _mapping->GetDimLength(CCMI_T_DIM);

          coords[CCMI_T_DIM] = prev_t;
	  _mapping->Torus2Rank (coords, &_prev);

          int tsize = _mapping->GetDimLength(CCMI_T_DIM);
          /// Non tail nodes have an extra receive phase
          if(_isHead)
            _startReducePhase = 0;
          else
            _startReducePhase = (my_coords[CCMI_T_DIM] - root_t + tsize - 1) % tsize;

          /// The head node does two sends
          nphases = _nReducePhases = (_isHead) ?
            (_mapping->GetDimLength(CCMI_T_DIM)) : 2;

          _startPhase = _startReducePhase;
        }
        else
          CCMI_abort();

        startphase = _startPhase;

        // printf ("Initializing schedule %d, %d, %d\n", _prev,
        // _next, _startphase);
      }

      /**
       * \brief Get the upstream processors. Source processors
       * that send messages to me in this collective operation
       * \param [in] phase  : phase of the collective
       * \param [out] srcpes : List of source processors
       * \param [out] nsrc  :  number of source processors
       * \param [out] subtasks : Hits for the receives
       */

      virtual void getSrcPeList (unsigned  phase, unsigned *srcpes,
                                 unsigned  &nsrc, unsigned *subtasks)
      {
        nsrc = 0;
        switch(_op)
        {
        case BARRIER_OP:
          getBarrierSources (phase, srcpes, nsrc, subtasks);
          break;
        case REDUCE_OP:
          getReduceSources (phase, srcpes, nsrc, subtasks);
          break;
        case BROADCAST_OP:
          getBroadcastSources (phase, srcpes, nsrc, subtasks);
          break;
        case ALLREDUCE_OP:
          if(phase < _startReducePhase + _nReducePhases)
            getReduceSources (phase, srcpes, nsrc, subtasks);
          else
            getBroadcastSources (phase, srcpes, nsrc, subtasks);
          break;

        default:
          CCMI_abort();
        }
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
        ndst = 0;

        //fprintf (stdout, "%d: Allreduce::getDstPeList %d, op=%d\n",
        //   _mapping->rank(), phase, _op);

        switch(_op)
        {
        case BARRIER_OP:
          getBarrierDestinations (phase, dstpes, ndst, subtasks);
          break;
        case REDUCE_OP:
          getReduceDestinations (phase, dstpes, ndst, subtasks);
          break;
        case BROADCAST_OP:
          getBroadcastDestinations (phase, dstpes, ndst, subtasks);
          break;

        case ALLREDUCE_OP:
          if(phase < _startReducePhase + _nReducePhases)
            getReduceDestinations (phase, dstpes, ndst, subtasks);
          else
            getBroadcastDestinations (phase, dstpes, ndst, subtasks);
          break;

        default:
          CCMI_abort();
        }

        return;
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

      virtual void init (int root, int op, int &startphase, int &nphases,
                         int &maxranks)
      {
        startphase = 0;
        nphases = 0;
        maxranks = 0;

        _op  =  op;
        _isHead = false;
        _isTail = false;

        switch(op)
        {
        case REDUCE_OP:
        case BARRIER_OP:
        case BROADCAST_OP:
          local_init (root, op, startphase, nphases, maxranks);
          break;

        case ALLREDUCE_OP:
          //SMP mode is just a reduce
          if(_mapping->GetDimLength(CCMI_T_DIM) == 1)
            local_init (root, REDUCE_OP, startphase, nphases, maxranks);
          else
          {
            local_init (root, REDUCE_OP, startphase, nphases, maxranks);
            int b_startphase, b_nphases;
            local_init (root, BROADCAST_OP, b_startphase, b_nphases, maxranks);

            // add the broadcast phases
            nphases = _mapping->GetDimLength(CCMI_T_DIM) - 1 + _startBcastPhase +
                      b_nphases - _startReducePhase;
            startphase = _startPhase = _startReducePhase;
            _startBcastPhase += _mapping->GetDimLength(CCMI_T_DIM) - 1;
          }

          break;

        default:
          CCMI_abort();
        }
      }

    protected:
      TorusCollectiveMapping         * _mapping;
      unsigned   short       _op;
      unsigned               _root;
      unsigned               _head; //the core with the same local rank as root
      unsigned   short       _startPhase;
      unsigned   short       _startBcastPhase;
      unsigned   short       _startReducePhase;
      unsigned   short       _nReducePhases;

      bool                   _isHead, _isTail;

      //for the ring reduce
      unsigned               _next;
      unsigned               _prev;
      unsigned               _peers [4];
    };
  };
};



inline CCMI::Schedule::TreeBwSchedule::TreeBwSchedule
(TorusCollectiveMapping       * map,
 unsigned             nranks,
 unsigned           * ranks) :
_mapping (map),_isHead (false), _isTail (false)
{
  CCMI_assert (map != NULL);
  CCMI_assert (nranks == map->size());

  _startPhase = _startReducePhase = _startBcastPhase = ((unsigned short) -1);
  _root = _head = ((unsigned)-1);
  _nReducePhases = ((unsigned short)-1);

  int idx = 0;
  unsigned* my_coords = _mapping->Coords();
  unsigned coords[CCMI_TORUS_NDIMS];
  CCMI_COPY_COORDS(coords, my_coords);

  for(unsigned count = 0; count < _mapping->GetDimLength(CCMI_T_DIM); count++)
    if(count != _mapping->GetCoord(CCMI_T_DIM))
    {
      coords[CCMI_T_DIM] = count;
      _mapping->Torus2Rank (coords, &_peers [idx]);
      idx ++;
    }
}


#endif
