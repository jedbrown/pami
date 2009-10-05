/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/schedule/OneColorTorusRect.h
 * \brief ???
 */

#ifndef  __new_rectangle_broadcast_collective_h__
#define  __new_rectangle_broadcast_collective_h__

/*-------------------------------------------------*/
/*     Basic utility classes collectives           */
/*-------------------------------------------------*/

#include "Rectangle.h"

//#define TRACE_ERR(x)  fprintf x
//#define RECTBCAST_DEBUG   1
#define TRACE_ERR(x)

///
/// \brief This schedule implements the following broadcast algorithm
/// on 1-3 dimensional meshes and tori. The following is the schematic
/// of an X color broadcast on a 2D mesh in SMP mode.
///
///    1Y  2G  1Y  1Y
///    1Y  2G  1Y  1Y
///    0X   R  0X  0X
///    1Y  2G  1Y  1Y
///
///  R : Root
///  0X : Processor receives data in phase 0 from dimension X
///  1Y : Processor receives data in phase 1 from dimension Y
///  2G : As the root needs to use the Y links for another color, we have ghost nodes that
///       dont get data in the first two phases of a 2D mesh broadcast. Their neigbors
///       have to send them data in phase 2
///
///     In modes where there are more than one core per node the peer of the root
///     locally broadcasts data to all the cores.
///

namespace CCMI
{
  namespace Schedule
  {


    ///Utility Macros

#define MY_X    _mapping->GetCoord(CCMI_X_DIM)
#define MY_Y    _mapping->GetCoord(CCMI_Y_DIM)
#define MY_Z    _mapping->GetCoord(CCMI_Z_DIM)
#define MY_T    _mapping->GetCoord(CCMI_T_DIM)
#define MY_RANK _mapping->rank()

#define ROOT_X  _root_coords[CCMI_X_DIM]
#define ROOT_Y  _root_coords[CCMI_Y_DIM]
#define ROOT_Z  _root_coords[CCMI_Z_DIM]
#define ROOT_T  _root_coords[CCMI_T_DIM]

    static const unsigned _hints[6] = {CCMI_LINE_BCAST_XP,
				       CCMI_LINE_BCAST_YP,
				       CCMI_LINE_BCAST_ZP,
				       CCMI_LINE_BCAST_XM,
				       CCMI_LINE_BCAST_YM,
				       CCMI_LINE_BCAST_ZM};

    //Allocate a 10-way table to convert colors to dimensions
    static const unsigned _color_to_axis[16] = { (unsigned) -1,
						 0, 1, 2,
						 0, 1, 2,
						 0, 1, 2,
                                                 0, 1, 2,
                                                 0, 1, 2};

    class OneColorTorusRect : public Schedule
    {
    protected:

      ///
      /// \brief Find the startphase based coordinates of the root
      /// \param [in] root The root of the collective operation
      ///
      void  initStartPhase       (unsigned root);

      /**
       * \brief Get Destinations for the non-ghost non-local phases
       *
       * \param[in] phase	Phase for which to extract information
       * \param[out] dstranks	Array to hold destination node(s)
       * \param[out] subtask	Array to hold subtasks (operation, e.g. LINE_BCAST_XM)
       * \param[out] ndst	Number of destination nodes (and subtasks)
       * \return	nothing (else).
       */
      void  setupBroadcast (unsigned phase,  unsigned *dstranks,
                            unsigned *subtasks, unsigned &ndst);

      /**
       * \brief Get Destinations for the phases to process ghost nodes
       *
       * \param[out] dstranks	Array to hold destination node(s)
       * \param[out] subtask	Array to hold subtasks (operation, e.g. LINE_BCAST_XM)
       * \param[out] ndst	Number of destination nodes (and subtasks)
       * \return	nothing (else).
       */
      void setupGhost (unsigned *dstranks, unsigned *subtasks, unsigned &ndst);


      /**
       * \brief Get Destinations for the local peers
       *
       * \param[out] dstranks	Array to hold destination node(s)
       * \param[out] subtask	Array to hold subtasks (operation, e.g. LINE_BCAST_XM)
       * \param[out] ndst	Number of destination nodes (and subtasks)
       * \return	nothing (else).
       */
      void setupT (unsigned *dstranks, unsigned *subtasks, unsigned &ndst);

    public:

      ///
      /// \brief The class constructor
      ///
      OneColorTorusRect (): _rect(NULL) {}

      ///
      /// \brief The class constructor
      ///
      OneColorTorusRect (TorusCollectiveMapping *map, unsigned color, const Rectangle &rect):
      _rect(&rect)
      {

        TRACE_ERR((stderr, "In One Color Torus Rect Bcast Constructor\n"));
        _color   =  color;
        _mapping =  map;
        _startphase = (unsigned) -1;
        _root       = (unsigned) -1;
        ROOT_X      = (unsigned) -1;
        ROOT_Y      = (unsigned) -1;
        ROOT_Z      = (unsigned) -1;
        ROOT_T      = (unsigned) -1;

        _my_coord = _mapping->Coords();
      }

      /********************************************
       ** Main entry points into the schedule
       ********************************************/

      /**
       * \brief Get Source node phase list
       *
       * \param[in] phase	Phase for which to extract information
       * \param[out] srcpes	Array to hold source node(s)
       * \param[out] nsrc	Number of source nodes
       * \param[out] subtasks	Operation for receive
       * \return	nothing (else)
       */
      virtual void getSrcPeList(unsigned phase, unsigned *srcpes,
                                unsigned &nsrc, unsigned *subtasks=NULL)
      {
        //CCMI_abort();

        ///This function isnt implemented yet as it isnt critical for
        ///broadcast
        nsrc = 0;
        return;
      }

      /**
       * \brief Get Destination node phase list
       *
       * \param[in] phase	Phase for which to extract information
       * \param[out] dstranks	Array to hold destination node(s)
       * \param[out] ndst	Number of destination nodes (and subtasks)
       * \param[out] subtask	Array to hold subtasks (operation, e.g. LINE_BCAST_XM)
       * \return	nothing (else).
       */
      virtual void getDstPeList(unsigned phase, unsigned *dstranks,
                                unsigned &ndst, unsigned *subtasks);


      /**
       * \brief Initialize the schedule for collective operation
       * \param root : the root of the collective
       * \param op : the collective operation
       * \param startphase : the phase where I become active
       * \param nphases : number of phases
       * \param maxranks : total number of processors to communicate
       *  with. Mainly needed in the executor to allocate queues
       *  and other resources
       */
      virtual void init(int root, int op, int &start, int &nphases,
                        int &nmessages);

      ///
      /// \brief Return the color of the schedule
      ///
      unsigned  color ()
      {
        return _color;
      }

      /// \brief Set the color of the schedule
      /// \param [in] c  color
      void      setColor (unsigned c)
      {
        _color  = c;
      }

      /**
       * \brief Get colors that make sense for this rectangle.
       *
       * \param[in] rect	The rectange in question
       * \param[out] ideal	The recommended number of colors
       * \param[out] max	The maximum number of colors (returned in colors)
       * \param[out] colors	(optional) Array of enum Color's usable on rect
       */
      static void getColors(const Rectangle &rect,
                            unsigned &ideal, unsigned &max, Color *colors = NULL)
      {
        int x = 0;
        const unsigned  *sizes     = &rect.xs;
        const unsigned  *is_torus  = &rect.isTorusX;

        ideal = 0;
        for(x = 0; x < CCMI_TORUS_NDIMS -1; x++)
          if(sizes[x] > 1)
            colors[ideal ++] = (Color)_MK_COLOR(x,P_DIR);

        max = ideal;

        for(x = 0; x < CCMI_TORUS_NDIMS-1; x++)
          if((sizes[x] > 1) && is_torus[x])
            colors[max ++] = (Color)_MK_COLOR(x,N_DIR);

        if(max == 2*ideal)
          ideal = max;

        if(ideal == 0)
        {
          ideal = max = 1;
          colors[0] = XP_Y_Z;
        }
      }

    protected:
      unsigned          _color;
      unsigned          _root;
      unsigned          _root_coords[CCMI_TORUS_NDIMS];
      const Rectangle * _rect;
      unsigned          _startphase;
      TorusCollectiveMapping    * _mapping;

      unsigned        * _my_coord;
    };  //-- OneColorTorusRect
  };  //-- Schedule
}; //-- CCMI


//-------------------------------------------------------------------
//------  OneColorTorusRect Schedule Functions ----------------------
//-------------------------------------------------------------------

///
/// \brief Find the startphase based coordinates of the root
/// \param [in] root The root of the collective operation
///
inline void CCMI::Schedule::OneColorTorusRect::initStartPhase (unsigned root)
{

  _mapping->Rank2Torus (&(_root_coords[0]), root);

  if(MY_RANK == root)
  {
    _startphase = 0;
    return;
  }

  int axis0, axis1, axis2;

  axis0 = _color_to_axis [ _color   ]; //(_color -  1) % (CCMI_TORUS_NDIMS - 1);
  axis1 = _color_to_axis [ _color+1 ]; //(_color     ) % (CCMI_TORUS_NDIMS - 1);
  axis2 = _color_to_axis [ _color+2 ]; //(_color +  1) % (CCMI_TORUS_NDIMS - 1);

  const unsigned *r_coord  = _root_coords;

  if(_my_coord[axis1] == r_coord[axis1] &&
     _my_coord[axis2] == r_coord[axis2]) //line
    _startphase = 0;
  else if(_my_coord[axis2] == r_coord[axis2] &&
          _my_coord[axis0] != r_coord[axis0]) //plane - ghost
    _startphase = 1;
  else //neighbor of ghost
    _startphase = 2;
}


/**
 * \brief Initialize the schedule for collective operation
 * \param root : the root of the collective
 * \param op : the collective operation
 * \param startphase : the phase where I become active
 * \param nphases : number of phases
 * \param maxranks : total number of processors to communicate
 *  with. Mainly needed in the executor to allocate queues
 *  and other resources
 */
inline void CCMI::Schedule::OneColorTorusRect::init(int    root,
                                                    int    op,
                                                    int   &start,
                                                    int   &nphases,
                                                    int   &nmessages)
{
  CCMI_assert (op == BROADCAST_OP);

  _root = root;

  initStartPhase(root);
  start = _startphase;
  nphases = (CCMI_TORUS_NDIMS+1) - start;

  //every phase has one message on a torus and 2 on a line
  nmessages = _rect->ts + 1;

  TRACE_ERR ((stderr, "%d: Calling Init with color %d, root %d startphase %d\n",
              MY_RANK, _color, root, _startphase));

}

/**
 * \brief Get Destination node phase list
 *
 * \param[in] phase	Phase for which to extract information
 * \param[out] dstranks	Array to hold destination node(s)
 * \param[out] ndst	Number of destination nodes (and subtasks)
 * \param[out] subtask	Array to hold subtasks (operation, e.g. LINE_BCAST_XM)
 * \return	nothing (else).
 */
inline void CCMI::Schedule::OneColorTorusRect::getDstPeList(unsigned    phase,
                                                            unsigned  * dstranks,
                                                            unsigned  & ndst,
                                                            unsigned  * subtasks)
{
  CCMI_assert (phase >= _startphase);
  ndst = 0;

  TRACE_ERR ((stderr,"%d: In getDstPeList, root_t %d my_t %d\n", MY_RANK, ROOT_T, MY_T));

  if(MY_T == ROOT_T)
  {
    if(MY_RANK == _root && phase == 0)
      ///call setup broadcast with phase 0 which imples root
      setupBroadcast (phase, dstranks, subtasks, ndst);
    else if(MY_RANK != _root && phase > _startphase)
    {
      /// setup the destination processors to foreward the data along
      /// the next dimension in the torus
      if(phase < CCMI_TORUS_NDIMS-1)
        setupBroadcast (phase, dstranks, subtasks, ndst);
      ///Process ghost nodes
      else if(phase == CCMI_TORUS_NDIMS-1)
        setupGhost (dstranks, subtasks, ndst);
    }

    ///Process local broadcasts
    if(phase == CCMI_TORUS_NDIMS && _rect->ts > 1)
      setupT (dstranks, subtasks, ndst);
  }
}

/**
 * \brief Get Destinations for the non-ghost non-local phases
 *
 * \param[in] phase	Phase for which to extract information
 * \param[out] dstranks	Array to hold destination node(s)
 * \param[out] subtask	Array to hold subtasks (operation, e.g. LINE_BCAST_XM)
 * \param[out] ndst	Number of destination nodes (and subtasks)
 * \return	nothing (else).
 */
inline void CCMI::Schedule::OneColorTorusRect::setupBroadcast (unsigned     phase,
                                                               unsigned   * dstranks,
                                                               unsigned   * subtasks,
                                                               unsigned   & ndst)
{
  ///Find the axis to do the line broadcast on
  int axis = _color_to_axis [ phase + _color ]; //(phase + _color -  1) % (CCMI_TORUS_NDIMS - 1);
  int delta = -1;
  if(_color >= XN_Y_Z)
  {
    axis = (phase + _color - XN_Y_Z) % (CCMI_TORUS_NDIMS - 1);
    delta   = 1;
  }

  ///Discover if we need to use torus links, or send two messages
  ///along +ve and -ve directions
  const unsigned *is_torus  = &_rect->isTorusX;
  const unsigned *s_coord   = &_rect->x0;
  const unsigned *sizes     = &_rect->xs;

  if(sizes [axis] == 1)
    return;

  int dest[CCMI_TORUS_NDIMS];
  dest[CCMI_X_DIM] =  MY_X;
  dest[CCMI_Y_DIM] =  MY_Y;
  dest[CCMI_Z_DIM] =  MY_Z;
  dest[CCMI_T_DIM] =  MY_T;
  dest[axis] = -1;  //invalidate

  if(is_torus[axis])
  {
    dest[axis] = _my_coord[axis] + delta;
    if(dest[axis] >= (int)(s_coord[axis] + sizes[axis]))
      dest[axis] -= (int)sizes[axis];
    if(dest[axis] < (int)s_coord[axis])
      dest[axis] += (int)sizes[axis];

    (void)_mapping->Torus2Rank((unsigned*)dest, dstranks);
    if(delta == -1)
      subtasks[0] = _hints[axis];
    else
      subtasks[0] = _hints[axis + CCMI_TORUS_NDIMS-1];

    ndst      = 1;
  }
  else
  {
    if(_my_coord[axis] < s_coord[axis] + sizes[axis] - 1)
    {
      dest[axis]  = s_coord[axis] + sizes[axis] - 1;
      (void)_mapping->Torus2Rank((unsigned*)dest, dstranks);
      subtasks[ndst] = _hints[axis];
      ndst ++;
    }
    if(_my_coord[axis] >  s_coord[axis])
    {
      dest[axis] = s_coord[axis];
      (void)_mapping->Torus2Rank((unsigned*)dest, dstranks+ndst);
      subtasks[ndst] = _hints[axis + CCMI_TORUS_NDIMS-1];
      ndst ++;
    }
  }

#ifdef RECTBCAST_DEBUG
  if(ndst > 0)
  {
    fprintf (stderr, "%d: setupBroadcast phase = %d, color = %d, dstranks[0] = %d, subtasks[0] = %d\n",
             MY_RANK, phase, _color, dstranks[0], subtasks[0]);
  }
#endif
}

/**
 * \brief Get Destinations for the phases to process ghost nodes
 *
 * \param[out] dstranks	Array to hold destination node(s)
 * \param[out] subtask	Array to hold subtasks (operation, e.g. LINE_BCAST_XM)
 * \param[out] ndst	Number of destination nodes (and subtasks)
 * \return	nothing (else).
 */
inline void CCMI::Schedule::OneColorTorusRect::setupGhost (unsigned  * dstranks,
                                                           unsigned  * subtasks,
                                                           unsigned  & ndst)
{
  unsigned axis = (_color - 1) % ( CCMI_TORUS_NDIMS - 1 );

  const unsigned *sizes    = &_rect->xs;
  CCMI_assert (sizes [axis] > 1);

  int dir = 1;
  if(_color >= XN_Y_Z) //-ve color
    dir = -1;

  int dst_coord[4];

  dst_coord[CCMI_X_DIM] =  MY_X;
  dst_coord[CCMI_Y_DIM] =  MY_Y;
  dst_coord[CCMI_Z_DIM] =  MY_Z;
  dst_coord[CCMI_T_DIM] =  MY_T;
  dst_coord[axis] = -1; ///invalidate

  const unsigned *is_torus = &_rect->isTorusX;
  const unsigned *r_coord  = _root_coords;
  const unsigned *s_coord  = &_rect->x0;

  if(is_torus[axis])
  {
    dst_coord[axis] = (int)r_coord[axis] + dir;
    if(dst_coord[axis] < (int)s_coord[axis])
      dst_coord[axis] += sizes[axis];
    if(dst_coord[axis] >= (int)(s_coord[axis] + sizes[axis]))
      dst_coord[axis] -= sizes[axis];
  }
  else
  {
    dir = 1;
    dst_coord[axis] = r_coord[axis] + 1;
    if(dst_coord[axis] >= (int)(sizes[axis] + s_coord[axis]))
    {
      dst_coord[axis] = r_coord[axis] - 1;    //sizes > 1, so either dir or -dir should exist
      dir = -1;
    }

    TRACE_ERR ((stderr, "%d: dst_coord for ghost = %d, given rcoord = %d\n", MY_RANK, dst_coord[axis], r_coord[axis]));
  }

  //The nodes that are different from the root in one dimension (not
  //the leading dimension of the color) are the ghosts. The nodes
  //just before them have to send data to them
  if((int)_my_coord[axis] == dst_coord[axis])
  {
    unsigned dstrank = -1;
    dst_coord[axis]  = r_coord[axis];
    (void)_mapping->Torus2Rank((unsigned*)dst_coord, &dstrank);
    if(dstrank != _root)
    {
      ndst = 1;
      dstranks[0] = dstrank;
      subtasks[0] = CCMI_PT_TO_PT_SUBTASK;
    }
  }

#ifdef RECTBCAST_DEBUG
  if(ndst > 0)
  {
    fprintf (stderr, "%d: setupGhost color = %d, dstranks[0] = %d, subtasks[0] = %d\n",
             MY_RANK, _color, dstranks[0], subtasks[0]);
  }
#endif
}


/**
 * \brief Get Destinations for the local peers
 *
 * \param[out] dstranks	Array to hold destination node(s)
 * \param[out] subtask	Array to hold subtasks (operation, e.g. LINE_BCAST_XM)
 * \param[out] ndst	Number of destination nodes (and subtasks)
 * \return	nothing (else).
 */
inline void CCMI::Schedule::OneColorTorusRect::setupT (unsigned   * dstranks,
                                                       unsigned   * subtasks,
                                                       unsigned   & ndst)
{
  unsigned count = 0;
  if(_rect->ts > 1 && MY_T == ROOT_T)
  {
    unsigned* coords_ptr = _mapping->Coords();
    unsigned tmp_coords[CCMI_TORUS_NDIMS];

    for (count = 0; count < CCMI_TORUS_NDIMS; count++)
      tmp_coords[count] = coords_ptr[count];

    for(count = _rect->t0; count < (_rect->t0+_rect->ts); count++)
      if(MY_T != count)
      {
        tmp_coords[CCMI_T_DIM] = count;
        (void)_mapping->Torus2Rank(tmp_coords, &dstranks[ndst]);
        subtasks[ndst] = CCMI_PT_TO_PT_SUBTASK;
        ndst ++;
      }

#ifdef RECTBCAST_DEBUG
    if(ndst > 0)
    {
      fprintf (stderr, "%d: setupT color = %d, dstranks[0] = %d, subtasks[0] = %d\n",
               MY_RANK, _color, dstranks[0], subtasks[0]);
    }
#endif
  }
}


#endif
