/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/schedule/RingSchedule.h
 * \brief ???
 */

#ifndef __algorithms_schedule_OldRingSchedule_h__
#define __algorithms_schedule_OldRingSchedule_h__

#include "algorithms/schedule/Schedule.h"


/////////////////////////////////////////////////////////////////////
///
///  In this schedule we place the nodes on a topological Ring in the
///  order in which the nodes are listed. The root node is the head
///  node and modulo root-1 is the tail. Data moves from the tail to
///  the head along the Ring. The arithmatic is loadbalanced while
///  network performance is only optimal when the nodes are on meshes
///  and tori in the correct order.
///
////////////////////////////////////////////////////////////////////


namespace CCMI
{
  namespace Schedule
  {
    template <class T_Sysdep>
    class OldRingSchedule : public Schedule
    {

    public:
      /**
       * \brief Constructor
       */
      OldRingSchedule () : Schedule ()
      {
      }

      OldRingSchedule (T_Sysdep *map, unsigned nranks, unsigned *ranks);
      OldRingSchedule (unsigned x, unsigned x0, unsigned xN);
      //OldRingSchedule (Rectangle  *rect, T_Sysdep *map);

      //Ring broadcast: Send to next and recv from prev
      void getBroadcastSources (unsigned  phase, unsigned *srcpes,
                                unsigned  &nsrc, unsigned *tasks)
      {
        nsrc = 0;
        if(!_isHead && phase == _bcastStart)
        {
          nsrc    = 1;
          *srcpes = (!_dir) ? _prev : _next;
          *tasks  = XMI_PT_TO_PT_SUBTASK;
        }
      }


      //Ring broadcast: Send to next and recv from prev
      void getBroadcastDestinations (unsigned phase, unsigned *dstpes,
                                     unsigned &ndest, unsigned *subtasks)
      {

        unsigned sendphase = _bcastStart + ((_isHead) ? 0 : 1);

        ndest = 0;
        if(!_isTail && phase == sendphase)
        {
          ndest     = 1;
          *dstpes   = (!_dir) ? _next : _prev;
          *subtasks = XMI_PT_TO_PT_SUBTASK;

          TRACE_SCHEDULE((stderr,"<%#.8X>Schedule::OldRingSchedule::getBroadcastDestinations() %d\n",(int)this, *dstpes));
        }
      }


      ///\brief Reduce methods
      ///Recv from next and send to prev
      void getReduceSources (unsigned  phase, unsigned *srcpes,
                             unsigned  &nsrc, unsigned *tasks)
      {
        nsrc = 0;
        if(!_isTail && phase == _startPhase)
        {
          nsrc    = 1;
          *srcpes = (!_dir) ? _next : _prev;
          *tasks  = XMI_COMBINE_SUBTASK;
        }
      }


      ///Recv from next and send to prev
      void getReduceDestinations (unsigned phase, unsigned *dstpes,
                                  unsigned &ndest, unsigned *subtasks)
      {

        unsigned sendphase = _startPhase + ((_isTail) ? 0 : 1);

        ndest = 0;
        if(!_isHead && phase == sendphase)
        {
          ndest     = 1;
          *dstpes   = (!_dir) ? _prev : _next;
          *subtasks = XMI_PT_TO_PT_SUBTASK;

          TRACE_SCHEDULE((stderr,"<%#.8X>Schedule::OldRingSchedule::getReduceDestinations() %d\n",(int)this, *dstpes));
        }
      }

      unsigned idxToRank (unsigned idx)
      {
        return(_ranks != NULL) ? (_ranks[idx]) : (_x0 + idx);
      }

      unsigned myIdx ()
      {
        unsigned idx = 0;
        if(_ranks != NULL)
        {
          for(idx = 0; idx < _nranks; idx++)
            if(__global.mapping.task() == _ranks[idx])
              return idx;

          return(unsigned)-1;
        }

        return _my_x-_x0;
      }

      ///
      ///\brief Get the id of the root node. It should be called
      ///after _root has been set.
      ///
      unsigned headIdx ()
      {
        unsigned head_idx = 0;
        // find head
        if(_ranks != NULL)
        {
          for(unsigned idx = 0; idx < _nranks; idx++)
            if(idxToRank(idx) == _root)
            {
              head_idx = idx;
              break;
            }
        }
        else
          head_idx = _root - _x0;

        return head_idx;
      }

      unsigned prevIdx ()
      {
        unsigned my_idx = myIdx();
        return(my_idx > 0) ? (my_idx - 1) : (_nranks - 1);
      }

      unsigned nextIdx ()
      {
        unsigned my_idx = myIdx();
        return(my_idx < (_nranks-1)) ? (my_idx + 1) : 0;
      }

      void local_init (int root, int op, int &startphase,
                       int &nphases, int &maxranks)
      {
        if(root >= 0)
          _root = root;
        else
          _root = idxToRank(0); //allreduce!

        unsigned head_idx=0, tail_idx = 0, my_idx=0;

        my_idx = myIdx ();

        head_idx = headIdx ();

        // compute tail
        if(head_idx != _nranks - 1)
        {
          tail_idx = (head_idx + _nranks - 1) % _nranks;
          _dir = 0;
        }
        else// We flip the direction of the ring, to allow
        {
          // rectangle mesh broadcasts dedicated access links
          tail_idx = 0;
          _dir = 1;
        }

        //set flags
        if(my_idx == head_idx)
          _isHead = true;
        if(my_idx == tail_idx)
          _isTail = true;

        _prev = idxToRank(prevIdx());
        _next = idxToRank(nextIdx());

        if(op == REDUCE_OP || op == ALLREDUCE_OP)
        {
          if(_isTail)
            _startPhase = 0;
          else if(!_dir)
            _startPhase = (tail_idx - my_idx + _nranks - 1) % _nranks;
          else //tail == 0
            _startPhase = my_idx - 1;

          if(op == REDUCE_OP)
          {
            // The tail node does one send and no recv. Others receive
            // on one phase and send in the next phase
            nphases = (_isTail || _isHead) ? 1 : 2;
          }

          if(op == ALLREDUCE_OP)
          {
            if(_isHead)
            {
              _bcastStart = _startPhase + 1; //on the head start
              //phase is the reduce
              //phase
              nphases = 2; //1 reduce phase and 1 bcast phase
            }
            else if(_isTail)
            {
              _bcastStart = _startPhase + 2 * (_nranks - 2 - _startPhase);
              nphases = _bcastStart + 1;
            }
            else //everyone else
            {
              _bcastStart = _startPhase + 2 * (_nranks - 2 - _startPhase);
              nphases = _bcastStart + 2 - _startPhase;
            }
          }
        }
        else if(op == BROADCAST_OP)
        {
          if(_isHead)
            _startPhase = 0;
          else if(!_dir)
            _startPhase = (my_idx - head_idx + _nranks - 1) % _nranks;
          else //tail == 0
            _startPhase = head_idx - my_idx - 1;

          _bcastStart = _startPhase;

          // The tail node does one send and no recv. Others receive
          // on one phase and send in the next phase
          nphases = (_isTail || _isHead) ? 1 : 2;
        }
        else
          XMI_abort();

        startphase = _startPhase;

        TRACE_SCHEDULE((stderr,"<%#.8X>Schedule::OldRingSchedule::local_init schedule %d, %d, %d, "
                     "idxes = (%d, %d, %d) \n", (int)this,
                     _prev, _next, _startPhase,
                     my_idx, head_idx, tail_idx));
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
        case REDUCE_OP:
          getReduceSources (phase, srcpes, nsrc, subtasks);
          break;
        case BROADCAST_OP:
          getBroadcastSources (phase, srcpes, nsrc, subtasks);
          break;
        case ALLREDUCE_OP:
          if(phase < _bcastStart)
            getReduceSources (phase, srcpes, nsrc, subtasks);
          else
            getBroadcastSources (phase, srcpes, nsrc, subtasks);
          break;

        case BARRIER_OP:
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

        switch(_op)
        {
        case REDUCE_OP:
          getReduceDestinations (phase, dstpes, ndst, subtasks);
          break;
        case BROADCAST_OP:
          getBroadcastDestinations (phase, dstpes, ndst, subtasks);
          break;
        case ALLREDUCE_OP:
          if(phase < _bcastStart)
            getReduceDestinations (phase, dstpes, ndst, subtasks);
          else
            getBroadcastDestinations (phase, dstpes, ndst, subtasks);
          break;

        case BARRIER_OP:
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
        maxranks = 2;

        CCMI_assert (op != BARRIER_OP);

        _op  =  op;
        _startPhase = ((unsigned) -1);
        _bcastStart = ((unsigned) -1);
        _root       = ((unsigned)-1);
        _isHead     = false;
        _isTail     = false;
        _prev       = ((unsigned) -1);
        _next       = ((unsigned) -1);
        _dir        = 0;

        local_init (root, op, startphase, nphases, maxranks);

        TRACE_SCHEDULE((stderr,"<%#.8X>Schedule::OldRingSchedule::init() _prev = %d, _next = %d\n", (int)this,_prev, _next));
      }

      static unsigned getMaxPhases (T_Sysdep *map, unsigned nranks)
      {
        return nranks - 1;
      }


    protected:
      T_Sysdep              * _sysdep;
      unsigned   short       _op;
      unsigned               _root;
      unsigned               _startPhase;
      unsigned               _bcastStart;
      bool                   _isHead, _isTail;

      //for the ring reduce
      unsigned           _next;
      unsigned           _prev;

      unsigned         * _ranks;
      unsigned           _nranks;

      unsigned           _x0, _my_x;
      unsigned           _dir;

      //Rectangle        * _rectangle;
    };
  };
};

template <class T_Sysdep>
inline CCMI::Schedule::OldRingSchedule<T_Sysdep>::OldRingSchedule
(T_Sysdep       * map,
 unsigned        nranks,
 unsigned      * ranks) :
_sysdep (map),
_isHead (false), _isTail (false),
_ranks(ranks), _nranks(nranks),
_x0((unsigned) -1), _my_x ((unsigned) -1)
{
  CCMI_assert (map != NULL);

  _startPhase = ((unsigned) -1);
  _root = ((unsigned)-1);
  _dir = 0;
  //_rectangle = NULL;
}

///
/// \brief constructor with in a line of consequitive processors as
/// inputs.
///
/// \param x    my rank
/// \param x0   the first rank
/// \param xN   the last rank
///
template <class T_Sysdep>
inline CCMI::Schedule::OldRingSchedule<T_Sysdep>::OldRingSchedule
(unsigned        x,
 unsigned        x0,
 unsigned        xN) :
_sysdep (NULL),_isHead (false), _isTail (false),
_ranks(NULL), _nranks(xN - x0 + 1),
_x0 (x0), _my_x (x)
{
  _startPhase = ((unsigned) -1);
  _root = ((unsigned)-1);
  _dir = 0;
  //_rectangle = NULL;
}

#if 0
inline CCMI::Schedule::OldRingSchedule::OldRingSchedule
(Rectangle  *rect, T_Sysdep *map) :
_sysdep (map), _isHead (false), _isTail (false), _ranks(NULL),
_nranks((unsigned)-1), _x0((unsigned) -1), _my_x ((unsigned) -1)
{
  _startPhase = ((unsigned) -1);
  _root       = ((unsigned) -1);
  _dir        = 0;
  //  _rectangle  = rect;

  _nranks = rect->xs * rect->ys * rect->zs * rect->ts;
}
#endif

#endif
