/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/schedule/JaynomialTree.h
 * \brief The Jaynomial Schedule for Barrier, Broadcast, and [All]Reduce.
 *
 * Also used by the Rectangle schedule to implement Reduce.
 */

#ifndef __jaynomial_tree_schedule__
#define __jaynomial_tree_schedule__
#define HAVE_BINO_LINE_SCHED	// until branch is merged into main

#include "Schedule.h"


#define Right(rank, phase, N) ( ((rank) + (1<<(phase)) + (N)) % (N) )
#define  Left(rank, phase, N) ( ((rank) - (1<<(phase)) + (N)) % (N) )

#define   Rotate(rank, root, n) ( ((rank) - (root) + (n)) % (n) )
#define Unrotate(rank, root, n) ( ((rank) + (root) + (n)) % (n) )

#define Indextorank(index) (_ranks ? _ranks[index] : (index))


namespace CCMI
{
  namespace Schedule
  {

    //-------------------------------------------------------------
    //------ Supports a generic barrier and allreduce -------------
    //------ The number of phases is logrithmic with a max of 20 --
    //-------------------------------------------------------------

    class JaynomialTreeSchedule : public Schedule
    {
      short    _op;   /* communication operation */
      unsigned _power2;
      unsigned _break_phase;
      unsigned _rootindex;
      unsigned _myindex;
      unsigned _myrotindex;

      unsigned _num_phases;
      unsigned _num_ranks;

      unsigned *_ranks;

      /**
       * \brief Create Jaynomial Schedule for a simple range of indices (or coords on a line)
       *
       * Also called from Rectangle.h when doing a reduce along an axis of the rectangle.
       *
       * \param[in] x		Our coord on axis
       * \param[in] x0	Lowest coord on axis
       * \param[in] xN	Highest coord on axis
       * \param[in] ranks	(optional) Pointer to list of ranks (saved until later)
       * \return	nothing
       */
      inline void initBinoSched(unsigned x, unsigned x0, unsigned xN,
                                unsigned *ranks = NULL)
      {
        _num_ranks = xN - x0 + 1;
        _ranks = ranks;
        _op = -1;

        /* find my index - my place in rank list */
        _myindex = x - x0;

        /* figure out the number of phases */
        _num_phases = 0;
        if(_num_ranks)
        {
          unsigned i;
          for(i = _num_ranks + _num_ranks - 1;
             i;
             i >>= 1)
          {
            _num_phases++;
          }
          --_num_phases;
          _power2 = 1<<_num_phases;
        }
      }

    public:

      JaynomialTreeSchedule () :
      Schedule (),
      _num_phases(0)
      {
      }

      JaynomialTreeSchedule (CollectiveMapping *mapping, unsigned num_ranks,
                             unsigned *ranks);

      JaynomialTreeSchedule (unsigned me, unsigned min, unsigned max);

      virtual void init(int root,
                        int comm_op,
                        int &start_phase,
                        int &num_phases,
                        int &num_ranks);

      /// Send
      virtual void getDstPeList(unsigned phase,
                                unsigned *dstpes,
                                unsigned &ndst,
                                unsigned *subtasks)
      {
        unsigned to = 0;
        ndst = 0;

        if(_op == BROADCAST_OP)
        {


          to = Right(_myrotindex, _num_phases-phase-1, _power2);
          if(to >= _num_ranks)
            return;
          to = Unrotate(to, _rootindex, _num_ranks);

          if(phase + 1 <  _break_phase)
          {
          }
          else if(phase + 1 == _break_phase)
          {
          }
          else if(phase + 1 >  _break_phase)
          {
            ndst = 1;
          }


        }
        else if(_op == REDUCE_OP)
        {


          to = Left(_myrotindex, phase, _power2);
          if(to >= _num_ranks)
            return;
          to = Unrotate(to, _rootindex, _num_ranks);

          if(phase <  _break_phase)
          {
          }
          else if(phase == _break_phase)
          {
            ndst = 1;
          }
          else if(phase >  _break_phase)
          {
          }


        }
        else
          CCMI_abort();

        *dstpes = Indextorank(to);
        *subtasks = CCMI_PT_TO_PT_SUBTASK;
      }

      /// Recv
      virtual void getSrcPeList (unsigned phase,
                                 unsigned *srcpes,
                                 unsigned &nsrc,
                                 unsigned *subtasks=NULL)
      {
        unsigned from = 0;
        nsrc = 0;

        if(_op == BROADCAST_OP)
        {


          from = Left(_myrotindex, _num_phases-phase-1, _power2);
          if(from >= _num_ranks)
            return;
          from = Unrotate(from, _rootindex, _num_ranks);

          if(phase + 1 <  _break_phase)
          {
          }
          else if(phase + 1 == _break_phase)
          {
            nsrc = 1;
          }
          else if(phase + 1 >  _break_phase)
          {
          }


        }
        else if(_op == REDUCE_OP)
        {


          from = Right(_myrotindex, phase, _power2);
          if(from >= _num_ranks)
            return;
          from = Unrotate(from, _rootindex, _num_ranks);

          if(phase <  _break_phase)
          {
            nsrc = 1;
          }
          else if(phase == _break_phase)
          {
          }
          else if(phase >  _break_phase)
          {
          }


        }
        else
          CCMI_abort();

        *srcpes = Indextorank(from);
        *subtasks = CCMI_COMBINE_SUBTASK;
      }

    };    //Jaynomial Tree Schedule
  };   //Schedule
}; //CCMI


//--------------------------------------------------------------
//------  Jaynomial Schedule Functions -------------------------
//--------------------------------------------------------------

inline CCMI::Schedule::JaynomialTreeSchedule::
JaynomialTreeSchedule(CollectiveMapping *mapping, unsigned num_ranks, unsigned *ranks) : Schedule ()
{

  CCMI_assert(num_ranks > 1);
  CCMI_assert(ranks != NULL);

  /* find my index - my place in rank list */
  unsigned rank = mapping->rank();
  unsigned i;
  for(i = 0; i < num_ranks && ranks[i] != rank; i++);
  CCMI_assert(i < num_ranks);
  CCMI::Schedule::JaynomialTreeSchedule::
  initBinoSched(i, 0, num_ranks - 1, ranks);
}

inline CCMI::Schedule::JaynomialTreeSchedule::
JaynomialTreeSchedule(unsigned x, unsigned x0, unsigned xN) : Schedule ()
{

  CCMI::Schedule::JaynomialTreeSchedule::
  initBinoSched(x, x0, xN);
}

inline void CCMI::Schedule::JaynomialTreeSchedule::
init(int root, int comm_op, int &start_phase, int &num_phases, int &num_ranks)
{

  CCMI_assert(comm_op == REDUCE_OP ||
              comm_op == BROADCAST_OP);

  _op = comm_op;
  if(_ranks)
  {
    for(_rootindex = 0;
       _rootindex < _num_ranks &&
       _ranks[_rootindex] != (unsigned)root;
       _rootindex++);
  }
  else
  {
    _rootindex = root;
  }


  _myrotindex = Rotate(_myindex, _rootindex, _num_ranks);


  // pick the break element
  if(_op == BROADCAST_OP)
  {

    unsigned n = _myrotindex;
    _break_phase = 0;

    if(n == 0)
      _break_phase = 0;
    else
    {
      while(n)
      {
        if(n & 1)
          break;
        n=n>>1;
        ++_break_phase;
      }
      _break_phase = _num_phases - _break_phase;
    }
    num_ranks   = _num_ranks;
    start_phase = _break_phase ? _break_phase-1 : _break_phase;
    num_phases  = _num_phases - start_phase;

  }
  else if(_op == REDUCE_OP)
  {

    unsigned n = _myrotindex;
    _break_phase = 0;

    if(n == 0)
      _break_phase = _num_phases;
    else
      while(n)
      {
        if(n & 1)
          break;
        n=n>>1;
        ++_break_phase;
      }
    num_ranks   = _num_ranks;
    start_phase = 0;
    num_phases  = (_num_phases - _break_phase) ? _break_phase+1 : _break_phase;

  }
  else
    CCMI_abort();



/*   fprintf(stderr, "nranks: %u,  power2: %u,   ", _num_ranks, _power2); */
/*   fprintf(stderr, "me:     %u,  root:   %u,   ", _myindex, _rootindex); */
/*   fprintf(stderr, "phases: %u,  break:  %u, \n", _num_phases, _break_phase); */
}

#endif
