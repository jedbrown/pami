/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/schedule/OneColorTorusRect.h
 * \brief ???
 */

#ifndef __algorithms_schedule_OneColorTorusRect_h__
#define __algorithms_schedule_OneColorTorusRect_h__

/*-------------------------------------------------*/
/*     Basic utility classes collectives           */
/*-------------------------------------------------*/

//#include "Rectangle.h"

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
///  2G : As the root needs to use the Y links for another color, we have
///       ghost nodes that dont get data in the first two phases of a 2D mesh
///       broadcast. Their neigbors have to send them data in last phase (2).
///
///     In modes where there are more than one core per node the peer of the
///     root locally broadcasts data to all the cores.
///

namespace CCMI
{
  namespace Schedule
  {

#define MY_TASK _map->task()
#define POSITIVE 0
#define NEGATIVE 1

    class OneColorTorusRect : public Schedule
    {
      protected:
        unsigned          _color;
        pami_task_t        _root;
        pami_coord_t       _root_coord;
        pami_coord_t       _self_coord;
        pami_coord_t       _ll;
        pami_coord_t       _ur;
        unsigned          _startphase;
        unsinged char     _torus_link[PAMI_MAX_DIMS];
        size_t            _dim_sizes[PAMI_MAX_DIMS];
        PAMI_MAPPING_CLASS *_map;
        PAMI::Topology     *_rect;

        void setupBroadcast(unsigned phase,  PAMI::Topology *topo);
        void setupGhost(PAMI::Topology *topo);
        void setupLocal(PAMI::Topology *topo);

      public:
        OneColorTorusRect(): _rect(NULL), _map(NULL) {}
        OneColorTorusRect(PAMI_MAPPING_CLASS *_map,
                          PAMI::Topology *rect,
                          pami_coord_t self,
                          unsigned color)
        {
          TRACE_ERR((stderr, "In One Color Torus Rect Bcast Constructor\n"));
          int i;
          _map = map;
          _rect = rect;
          _color = color;
          _self_coord = self;
          _startphase = (unsigned) -1;
          _rect->rectSeg(&_ll, &_ur, &_torus_link);
          for (i = 0; i < _map->torusDims(); i++)
            _dim_sizes[i] = _ur.net_coord(i) - _ll.net_coord(i) + 1;

          
          virtual void
            init(int root, int op, int &startphase, int &nphases, int &maxranks)
          {PAMI_abort();}
          virtual void getSrcPeList (unsigned  phase, unsigned *srcpes,
                                     unsigned  &nsrc, unsigned *subtasks=NULL)
          {PAMI_abort();}
          virtual void getDstPeList (unsigned  phase, unsigned *dstpes,
                                     unsigned  &ndst, unsigned *subtasks)
          {PAMI_abort();}

          virtual void init(pami_task_t root, int op, int &start, int &nphases);
          virtual void getSrcTopology(PAMI::Topology *topo);
          {
            return;
          }

          virtual void getDstTopology(unsigned phase, PAMI::Topology *topology);

          unsigned color()
          {
            return _color;
          }

          void setColor(unsigned c)
          {
            _color  = c;
          }

#if 0
          /**
           * \brief Get colors that make sense for this rectangle.
           *
           * \param[in] rect	The rectange in question
           * \param[out] ideal	The recommended number of colors
           * \param[out] max	The maximum number of colors (returned in colors)
           * \param[out] colors	(optional) Array of enum Color's usable on rect
           */
          static void getColors(PAMI::Topology *rect, unsigned &ideal,
                                unsigned &max, Color *colors = NULL)
          {
            int i = 0;
            ideal = 0;
            pami_coord_t ll, ur;
            unsigned char torus_link[PAMI_MAX_DIMS];
            size_t torus_dims, sizes[PAMI_MAX_DIMS];
            torus_dims = _map->torusDims();

            rect->rectSeg(&ll, &ur, &torus_link);
            for(i = 0; i < torus_dims; i++)
              if (sizes[i] > 1)
                colors[ideal++] = (Color)_MK_COLOR(i, P_DIR);

            max = ideal;

            for(i = 0; i < torus_dims; i++)
              if (sizes[i] > 1 && torus_link[i])
                colors[max++] = (Color)_MK_COLOR(i, N_DIR);

            if (max == 2 * ideal)
              ideal = max;

            if (ideal == 0)
            {
              ideal = max = 1;
              colors[0] = XP_Y_Z;
            }
          }
#endif

        };  //-- OneColorTorusRect
    };  //-- Schedule
  }; //-- CCMI


  //-------------------------------------------------------------------
  //------  OneColorTorusRect Schedule Functions ----------------------
  //-------------------------------------------------------------------

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
  inline void
  CCMI::Schedule::OneColorTorusRect::init(pami_task_t root,
                                          int op,
                                          int &start,
                                          int &nphases)
  {
    CCMI_assert (op == BROADCAST_OP);

    _root = root;
    _map->task2network(root, &_root_coord, PAMI_N_TORUS_NETWORK);

    size_t torus_dims = _map->torusDims();

    if (MY_TASK == root)
      _startphase = 0;
    else
    {
      int i, axis;
      unsigned color = _color;

      size_t axes[PAMI_MAX_DIMS];

      for (axis = 0; axis < torus_dims; axis++)
        axes[i] = color++ % torus_dims;

      for (axis = 0; axis < torus_dims; axis++)
      {
        // other nodes that send to ghost
        if (axis + 1 == torus_dims &&
          _self_coord.net_coord(axes[0]) != root.net_coord(axes[0])
        {
          _startphase = axis;
          break;
        }

        for (success = 1, i = axis + 1; i < torus_dims; i++)
          if (_self_coord.net_coord(axes[i]) != _root.net_coord(axes[i]))
          {
            success = 0;
            break;
          }

        if (success &&
            // excludes ghost nodes
            _self_coord.net_coord(axes[0]) != root.net_coord(axes[0]))
        {
          _startphase = axis;
          break;
        }
      }
      // this means I am a ghost node
      if (_startphase == -1) _startphase = torus_dims;
    }

    start = _startphase;
    nphases = torus_dims + 2 - start; // 2: 1 for local comm if any, 1 for ghost

    TRACE_ERR((stderr,
               "%d: Calling Init with color %d, root %d startphase %d\n",
                MY_TASK, _color, root, _startphase));
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

  inline void
  CCMI::Schedule::OneColorTorusRect::getDstTopology(unsigned phase,
                                                    PAMI::Topology *topo)
  {
    CCMI_assert (phase >= _startphase);

    size_t core_dim, torus_dims, peers = 0;

    torus_dims = _map->torusDims();
    _map->nodePeers(&peers);

    core_dim = torus_dims;
    if (_self_coord.net_coord(core_dim) == _root.net_coord(core_dim))
    {
      //call setup broadcast with phase 0 which implies root
      if (MY_TASK == _root && phase == 0)
        setupBroadcast(phase, topo);
      else if (MY_TASK != _root && phase > _startphase)
      {
        // setup the destination processors to foreward the data along
        // the next dimension in the torus
        if (phase < torus_dims)
          setupBroadcast(phase, topo);

        ///Process ghost nodes
        else if (phase == torus_dims)
          setupGhost(topo);
      }

      ///Process local broadcasts
      if (phase == (torus_dims + 1) && peers > 1)
        setupLocal(topo);
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
    inline void
    CCMI::Schedule::OneColorTorusRect::setupBroadcast(unsigned phase,
                                                      PAMI::Topology *topo)
    {
      pami_coord_t low, high;
      unsigned char dir[PAMI_MAX_DIMS] = {0};
      size_t torus_dims = _map->torusDims();

      //Find the axis to do the line broadcast on
      int axis = (phase + _color) % torus_dims;
      dir[axis] = NEGATIVE;
      if (_color >= torus_dims)
        dir[axis] = POSITIVE;

      low = _self_coord;
      high = _self_coord;
      low.net_coord(axis) = MIN(_ll.net_coord(axis),
                                _self_coord.net_coord(axis));
      high.net_coord(axis) = MAX(_ur.net_coord(axis),
                                 _self_coord.net_coord(axis));

      new (topo) PAMI::Topology(&low, &high, &_self_coord, &dir, &_torus_link);
    }

    /**
     * \brief Get Destinations for the phases to process ghost nodes
     *
     * \param[out] dstranks	Array to hold destination node(s)
     * \param[out] subtask	Array to hold subtasks (operation, e.g. LINE_BCAST_XM)
     * \param[out] ndst	Number of destination nodes (and subtasks)
     * \return	nothing (else).
     */
    inline void
    CCMI::Schedule::OneColorTorusRect::setupGhost(PAMI::Topology *topo)
    {
      int i;
      pami_coord_t ref, dst;
      unsigned char dir[PAMI_MAX_DIMS] = {0};

      size_t torus_dims = _map->torusDims();
      size_t axis = _color % torus_dims;

      CCMI_assert(_dim_sizes[axis] > 1);

      dir[axis] = NEGATIVE;
      if (_color >= torus_dims)
        dir[axis] = POSITIVE;

      ref = _self_coord;

      if (_torus_link[axis]) // if this dim or axis is a torus
      {
        ref.net_coord(axis) = (_root_coord.net_coord(axis) + 1) % torus_dims;
      }
      else
      {
        dir[axis] = POSITIVE;
        ref.net_coord(axis) = _root_coord.net_coord(axis) + 1;
        if (ref.net_coord(axis) >= _dim_sizes[axis] + _ll.net_coord(axis))
          {
            ref.net_coord(axis) = _root_coord.net_coord(axis) - 1;
            dir[axis] = NEGATIVE;
          }
      }

      //The nodes that are different from the root in one dimension (not
      //the leading dimension of the color) are the ghosts. The nodes
      //just before them have to send data to them
      if(_self_coord.net_coord(axis) == ref.net_coord(axis))
      {
        pami_network *type;
        pami_task_t dst_task;
        dst.network = PAMI_N_TORUS_NETWORK;

        dst.net_coord(axis) = _root_coord.net_coord(axis);
        _map->network2Task(dst, &dst_task, &type);

        if(dst_task != _root)
        {
          low = _self_coord;
          high = _self_coord;
          low.net_coord(axis) = MIN(dst.net_coord(axis),
                                    _self_coord.net_coord(axis));
          high.net_coord(axis) = MAX(dst.net_coord(axis),
                                     _self_coord.net_coord(axis));

          new (topo) PAMI::Topology(&low, &high, &_self_coord, &dir,
                                   &_torus_link);
        }
      }
    }


      /**
       * \brief Get Destinations for the local peers
       *
       * \param[out] dstranks	Array to hold destination node(s)
       * \param[out] subtask	Array to hold subtasks (operation, e.g. LINE_BCAST_XM)
       * \param[out] ndst	Number of destination nodes (and subtasks)
       * \return	nothing (else).
       */
      inline void
      CCMI::Schedule::OneColorTorusRect::setupLocal(PAMI::Topology *topo)
      {
        unsigned char dir[PAMI_MAX_DIMS] = {0};
        size_t peers, core_dim, core_dim;
        _map->nodePeers(&peers);

        // the cores dim is the first one after the physical torus dims
        core_dim = _map->torusDims();


        if (_self_coord.net_coord(core_dim) == _root.net_coord(core_dim))
        {
          pami_coord_t low, high;
          low = _self_coord;
          high = _self_coord;
          low.net_coord(core_dim) = 0;
          high.net_coord(core_dim) = peers - 1;
        }
        new (topo) PAMI::Topology(&low, &high, &_self_coord, &dir, &_torus_link);
      }

#endif
