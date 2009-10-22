/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/schedule/ShortRectangle.h
 * \brief ???
 */

#ifndef __algorithms_schedule_ShortRectangle_h__
#define __algorithms_schedule_ShortRectangle_h__

#include "Schedule.h"
#include "util/ccmi_util.h"
#include "interface/TorusCollectiveMapping.h"
#include "OneColorTorusRect.h"

static unsigned tmp_coords[CCMI_TORUS_NDIMS];

namespace CCMI
{
  namespace Schedule
  {

#define SR_MAX_NUM_DIMENSIONS 3

    typedef enum
    {
      LOCAL_REDUCE = 0,
      PHASE_X      = 1,
      PHASE_Y      = 2,
      PHASE_Z      = 3,
      LOCAL_BCAST  = 4,
      SR_NUMPHASES = 5
    } ShortRectanglePhases;


// Reset indentation (3x)
// Rectangle schedule optimized for short allreduce calls and barriers
    class ShortRectangleSchedule : public Schedule
    {
    protected:

      Rectangle       * _rect;        ///The base rectangle structure
      TorusCollectiveMapping    * _mapping; ///CollectiveMapping to translate between torus coordinates and ranks
      bool              _useLocalBarrier;  ///Barriers can use an
      ///optimized local
      ///barrier instead of
      ///message passing
      unsigned          _op;               ///The operation for this schedule
    public:
      /**
       * \brief default constructor for local core barrier schedule
       */
      ShortRectangleSchedule() : Schedule()
      {
      }

      /**
       * \brief Constructor for local core barrier schedule
       *
       * \param[in] map	CollectiveMapping object for geometry
       * \param[in] rectangle The rectangle to do the barrier on
       */
      ShortRectangleSchedule(TorusCollectiveMapping     * map,
                             Rectangle        & rectangle,
                             bool               uselocalbarrier=false)
      {
        _rect      = &rectangle;
        _mapping   = map;
        _useLocalBarrier = uselocalbarrier;
        _op = BARRIER_OP;
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

        nsrc = 0;  //We have nothing to receive

        //Use a fast memory barrier and hence nothing to recv in the
        //local phases
        if( _useLocalBarrier && (phase == LOCAL_BCAST || phase == LOCAL_REDUCE) )
          return;

        unsigned sub_task = ((_op == BARRIER_OP || phase == LOCAL_BCAST) ? CCMI_PT_TO_PT_SUBTASK :
                             CCMI_COMBINE_SUBTASK);

        //We are using message passing for local phases
        //Non t0 cores only participate in the first and last phases
        if( _mapping->GetCoord(CCMI_T_DIM) != _rect->t0 )
        {
          if(phase == LOCAL_BCAST)
          {
            CCMI_COPY_COORDS(tmp_coords, _mapping->Coords());
            tmp_coords[CCMI_T_DIM] = _rect->t0;
            nsrc =  1;
            _mapping->Torus2Rank(tmp_coords, srcpes);
            *subtasks =  sub_task;
          }
          return;
        }

        if(phase >= LOCAL_BCAST)
          return;

        ///
        ///Assume Torus phases (and the LOCAL_REDUCE phase when local_barrier isnt used)
        ///

        unsigned dir         = phase - 1;
        //If my t coord is t0 then I need to recv local messages and
        //then participate in the torus phases
        if(phase == LOCAL_REDUCE)
          dir = 3; // the t phase

        const unsigned  *start     = &_rect->x0;
        const unsigned  *sizes     = &_rect->xs;

        //The rectangle has no nodes in this dimension
        if(sizes[dir] <= 1)
          return;

        unsigned* my_coord = _mapping->Coords();
        unsigned dst_coord [4];
        CCMI_COPY_COORDS(dst_coord, my_coord);

        nsrc = 0;
        for(unsigned count = 0; count < sizes[dir]; count ++)
        {
          if(start[dir] + count != my_coord[dir])
          {
            dst_coord[dir] = start[dir] + count;
            _mapping->Torus2Rank(dst_coord, &srcpes[nsrc]);
            subtasks[nsrc++] =  sub_task;
          }
        }
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

        ndst = 0;  //We have nothing to send

        if( _useLocalBarrier && (phase == LOCAL_REDUCE || phase == LOCAL_BCAST) )
        {
          ndst      =  1;
          *dstpes   =  CCMI_UNDEFINED_RANK;
          *subtasks =  CCMI_LOCKBOX_BARRIER;

          return;  //We are done for local phases
        }

        //We are using message passing for a local barrier
        //Non t0 processors only participate in the LOCAL_REDUCE_PHASE
        if( _mapping->GetCoord(CCMI_T_DIM) != _rect->t0 )
        {
          if(phase == LOCAL_REDUCE)
          {
            CCMI_COPY_COORDS(tmp_coords, _mapping->Coords());
            tmp_coords[CCMI_T_DIM] = _rect->t0;
            ndst = 1;
            _mapping->Torus2Rank(tmp_coords, dstpes);
            *subtasks = CCMI_PT_TO_PT_SUBTASK;
          }

          //If t!=0, we are done
          return;
        }
        // t0 processors only - at this point
        if(phase <= LOCAL_REDUCE)
          return; // t0 processors have no dst in LOCAL_REDUCE


        //t == 0 and phase is the last local bcast phase
        if(phase == LOCAL_BCAST)
        {
          for(unsigned count = 0; count < _rect->ts; count ++)
            if(_rect->t0 + count != _mapping->GetCoord(CCMI_T_DIM))
            {
              CCMI_COPY_COORDS(tmp_coords, _mapping->Coords());
              tmp_coords[CCMI_T_DIM] = _rect->t0 + count;
              _mapping->Torus2Rank(tmp_coords, &dstpes[ndst]);
              subtasks[ndst++] = CCMI_PT_TO_PT_SUBTASK;
            }

            //Non torus phases are done
          return;
        }

        ///
        /// Nonlocal Torus phases
        /// Compute deposit bit messages to be sent
        ///
        const unsigned   dir       =  phase - 1;
        const unsigned  *start     = &_rect->x0;
        const unsigned  *sizes     = &_rect->xs;
        const unsigned  *is_torus  = &_rect->isTorusX;

        //The rectangle has no nodes in this dimension
        if(sizes[dir] <= 1)
          return;

        unsigned dest[SR_NUMPHASES];
        unsigned* my_coord = _mapping->Coords();
        CCMI_COPY_COORDS(dest, my_coord);

        //Am I a corner node in the lower left corner
        if(my_coord[dir] == start[dir])
        {
          ndst = 1;
          dest[dir] = start[dir] + sizes[dir] - 1;
          subtasks[0] = _hints[dir]; //start of the line to the end in +ve dir

          _mapping->Torus2Rank(dest, dstpes);
        }
        //Am I a corner node at the top right corner
        else if(my_coord[dir] == start[dir] + sizes[dir] - 1)
        {
          ndst = 1;
          dest[dir] = start[dir];
          subtasks[0] = _hints[dir + SR_MAX_NUM_DIMENSIONS];  //-ve direction
          _mapping->Torus2Rank(dest, dstpes);
        }
        //All other nodes in the middle of the line
        else
        {
          if(is_torus[dir])
          {
            ndst = 1;
            dest[dir] = my_coord[dir] - 1;
            subtasks[0] = _hints[dir];  //+ve direction to my_coord - 1
            _mapping->Torus2Rank(dest, dstpes);
          }
          else
          {
            ndst = 2;
            dest[dir] = start[dir];
            _mapping->Torus2Rank(dest, dstpes);
            subtasks[0] = _hints[dir + SR_MAX_NUM_DIMENSIONS];  //-ve direction

            dest[dir] = start[dir] + sizes[dir] - 1;
            _mapping->Torus2Rank(dest, &dstpes[1]);
            subtasks[1] = _hints[dir];  //+ve direction
          }
        }
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
        CCMI_assert(op == BARRIER_OP || op == ALLREDUCE_OP);

        _op = (unsigned) op;

        if(op != BARRIER_OP)
          _useLocalBarrier = false;

        startphase = PHASE_X;
        nphases = SR_MAX_NUM_DIMENSIONS;

        if(_rect->ts > 1)
        {
          startphase = LOCAL_REDUCE;
          nphases += 2;  //LOCAL REDUCE + LOCAL BROADCAST
        }
        else //SMP mode or a communicator with only 1 core in the
        {
          //t dimension
          if(_rect->xs <= 1)
          {
            startphase = PHASE_Y;

            if(_rect->ys <= 1)
            {
              startphase = PHASE_Z;

              if(_rect->zs <= 1)
                CCMI_abort();  //This schedule isnt designed for
                                  //1 core where xs==1,ys==1 and zs==1
            }
          }

        }

        maxranks = _rect->xs;
        if(maxranks < (int)_rect->ys)
          maxranks = _rect->ys;

        if(maxranks < (int)_rect->zs)
          maxranks = _rect->zs;
      }
    };

// end reset indentation (3x)
  };
};

#endif /* !__SHORT_RECTANGLE_SCHEDULE__ */
