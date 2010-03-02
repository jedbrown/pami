/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/schedule/TorusRect.h
 * \brief ???
 */

#ifndef __algorithms_schedule_TorusRect_h__
#define __algorithms_schedule_TorusRect_h__

/*-------------------------------------------------*/
/*     Basic utility classes collectives           */
/*-------------------------------------------------*/

#include "algorithms/interfaces/Schedule.h"
#include "common/TorusMappingInterface.h"
#include "Global.h"
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
#define POSITIVE XMI::Interface::Mapping::TorusPositive
#define NEGATIVE XMI::Interface::Mapping::TorusNegative

    class TorusRect: public CCMI::Interfaces::Schedule
    {
      public:
        TorusRect(): _rect(NULL), _map(NULL) {}
        TorusRect(XMI_MAPPING_CLASS *map,
                  XMI::Topology *rect,
                  xmi_coord_t self,
                  unsigned color):
        _ndims(0),
        _color(color),
//      _root(?),
//      _root_coord(?),
        _self_coord(self),
        _start_phase((unsigned)-1),
//      _nphases(?),
        _rect(rect),
        _map(map)
        {
          TRACE_ERR((stderr, "In One Color Torus Rect Bcast Constructor\n"));
          unsigned int i;

          XMI_assert(_rect->type() == XMI_COORD_TOPOLOGY);
          _rect->rectSeg(&_ll, &_ur, &_torus_link[0]);

          for (i = 0; i < _map->torusDims(); i++)
            if (_ur.net_coord(i))
              _dim_sizes[_ndims++] = _ur.net_coord(i) - _ll.net_coord(i) + 1;

        }
        void init(int root, int op, int &start, int &nphases);

        virtual xmi_result_t getSrcUnionTopology(XMI::Topology *topo);
        virtual xmi_result_t getDstUnionTopology(XMI::Topology *topology);
        virtual void getSrcTopology(unsigned phase, XMI::Topology *topology);
        virtual void getDstTopology(unsigned phase, XMI::Topology *topology);

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
        static void getColors(XMI::Topology *rect, unsigned &ideal,
                              unsigned &max, Color *colors = NULL)
        {
          int i = 0;
          ideal = 0;
          xmi_coord_t ll, ur;
          unsigned char torus_link[XMI_MAX_DIMS];
          size_t torus_dims, sizes[XMI_MAX_DIMS];
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

    protected:
        unsigned          _ndims;
        unsigned          _color;
        xmi_task_t        _root;
        xmi_coord_t       _root_coord;
        xmi_coord_t       _self_coord;
        xmi_coord_t       _ll;
        xmi_coord_t       _ur;
        unsigned          _start_phase;
        unsigned int      _nphases;
        unsigned char     _torus_link[XMI_MAX_DIMS];
        size_t            _dim_sizes[XMI_MAX_DIMS];
        XMI::Topology     *_rect;
        XMI_MAPPING_CLASS *_map;

        void setupBroadcast(int phase,  XMI::Topology *topo);
        void setupGhost(XMI::Topology *topo);
        void setupLocal(XMI::Topology *topo);
    };  //-- TorusRect
  };  //-- Schedule
}; //-- CCMI


  //-------------------------------------------------------------------
  //------  TorusRect Schedule Functions ----------------------
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
  CCMI::Schedule::TorusRect::init(int root,
                                  int op,
                                  int &start,
                                  int &nphases)
  {
    CCMI_assert (op == CCMI::Interfaces::BROADCAST_OP);

    _root = root;
    _map->task2network(root, &_root_coord, XMI_N_TORUS_NETWORK);

    size_t peers, axes[XMI_MAX_DIMS] = {0};
    unsigned int i, myphase, axis, color = _color;
      
    for (axis = 0; axis < _ndims; axis++)
      axes[axis] = color++ % _ndims;

    if (MY_TASK == (unsigned) root)
      _start_phase = 0;
    // ghost nodes
    else if (_self_coord.net_coord(axes[0]) == _root_coord.net_coord(axes[0]))
      _start_phase = _ndims;
    else
    {
      for (axis = 0; axis < _ndims; axis++)
      {
        for (myphase = 1, i = axis + 1; i < _ndims && myphase; i++)
          if (_self_coord.net_coord(axes[i]) != _root_coord.net_coord(axes[i]))
            myphase = 0;

        if (myphase)
          break;
      }
      _start_phase = axis;
    }

    start = _start_phase;
    nphases = _ndims + 2 - start; // 2: 1 for local comm if any, 1 for ghost
    _map->nodePeers(peers);
    if (peers == 1)
      _nphases = --nphases;
  }


  inline xmi_result_t
  CCMI::Schedule::TorusRect::getSrcUnionTopology(XMI::Topology *topo)
  {
    printf("no need to implement\n");
    return XMI_SUCCESS;
  }
  inline void
  CCMI::Schedule::TorusRect::getSrcTopology(unsigned phase,
                                            XMI::Topology *topo)
  {
    printf("no need to implement\n");
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
  CCMI::Schedule::TorusRect::getDstTopology(unsigned phase,
                                            XMI::Topology *topo)
  {
    CCMI_assert (phase >= _start_phase);

    size_t core_dim, torus_dims, peers = 0;

    torus_dims = _map->torusDims();
    _map->nodePeers(peers);

    core_dim = torus_dims;
    if (_self_coord.net_coord(core_dim) == _root_coord.net_coord(core_dim))
    {
      //call setup broadcast with phase 0 which implies root
      if (MY_TASK == _root && phase == 0)
        setupBroadcast(phase, topo);
      else if (MY_TASK != _root && phase > _start_phase)
      {
        // setup the destination processors to foreward the data along
        // the next dimension in the torus
        if (phase < _ndims)
          setupBroadcast(phase, topo);

        ///Process ghost nodes
        else if (phase == _ndims)
          setupGhost(topo);
      }

      ///Process local broadcasts
      if (phase == ( _ndims + 1) && peers > 1)
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
    CCMI::Schedule::TorusRect::setupBroadcast(int phase,
                                              XMI::Topology *topo)
    {
      xmi_coord_t low, high;

      //Find the axis to do the line broadcast on
      int axis = (phase + _color) % _ndims;
      if(_torus_link[0])
      {
        _torus_link[axis] = POSITIVE;
        if (_color >= _ndims)
          _torus_link[axis] = NEGATIVE;
      }

      low = _self_coord;
      high = _self_coord;
      low.net_coord(axis) = MIN(_ll.net_coord(axis),
                                _self_coord.net_coord(axis));
      high.net_coord(axis) = MAX(_ur.net_coord(axis),
                                 _self_coord.net_coord(axis));

      new (topo) XMI::Topology(&low, &high, &_self_coord,
                               &_torus_link[0]);
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
    CCMI::Schedule::TorusRect::setupGhost(XMI::Topology *topo)
    {
      xmi_coord_t ref, dst;

      //size_t torus_dims = _map->torusDims();
      size_t axis = _color % _ndims;

      CCMI_assert(_dim_sizes[axis] > 1);

      ref = _self_coord;

      if (_torus_link[axis]) // if this dim or axis is a torus
      {
        ref.net_coord(axis) = (_root_coord.net_coord(axis) + 1) % _dim_sizes[axis];
      }
      else
      {
        ref.net_coord(axis) = _root_coord.net_coord(axis) + 1;
        if (ref.net_coord(axis) >= _dim_sizes[axis] + _ll.net_coord(axis))
          {
            ref.net_coord(axis) = _root_coord.net_coord(axis) - 1;
          }
      }

      //The nodes that are different from the root in one dimension (not
      //the leading dimension of the color) are the ghosts. The nodes
      //just before them have to send data to them
      if(_self_coord.net_coord(axis) == ref.net_coord(axis))
      {
        xmi_network *type=NULL;
        xmi_task_t dst_task=0;
        xmi_coord_t low, high;

        dst = _self_coord;

        dst.net_coord(axis) = _root_coord.net_coord(axis);
        _map->network2task(&dst, &dst_task, type);

        if(dst_task != _root)
        {
          low = _self_coord;
          high = _self_coord;
          low.net_coord(axis) = MIN(dst.net_coord(axis),
                                    _self_coord.net_coord(axis));
          high.net_coord(axis) = MAX(dst.net_coord(axis),
                                     _self_coord.net_coord(axis));
          /// \todo why build an axial topoology for one ghost?   You can't
          /// multicast/deposit to it?  Why not leave it as a single rank topology.
          new (topo) XMI::Topology(&low, &high, &_self_coord, 
                                   &_torus_link[0]);
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
      CCMI::Schedule::TorusRect::setupLocal(XMI::Topology *topo)
      {
        *topo = __global.topology_local;
        /// \todo why build an axial topoology on the local cores?   You can't
        /// multicast/deposit to them?  Why not leave it as the local topo type.
        topo->convertTopology(XMI_AXIAL_TOPOLOGY);
      }

      inline xmi_result_t
      CCMI::Schedule::TorusRect::getDstUnionTopology(XMI::Topology *topology)
      {
        int i, j;
        unsigned char torus_link[XMI_MAX_DIMS] = {0};
        unsigned char tmp_torus_link[XMI_MAX_DIMS] = {0};

        XMI::Topology tmp;
        xmi_coord_t tmp_low, tmp_high, tmp_ref;
        xmi_coord_t low, high;

        ///? we're creating topology? CCMI_assert(topology->size() != 0);
        low = _self_coord;
        high = _self_coord;

        for (i = _start_phase; i < (int) (_start_phase + _nphases); i++)
        {
          getDstTopology(i, &tmp);

          if (tmp.size())
          {
            // Get the axial members
            xmi_result_t result = tmp.axial(&tmp_low, &tmp_high,
                                             &tmp_ref,
                                             tmp_torus_link);

            XMI_assert(result == XMI_SUCCESS);

            // now add this topology to the union
            for (j = 0; j < (int) _ndims; j++)
            {
              torus_link[j] |= tmp_torus_link[j];
              low.net_coord(j) = MIN(low.net_coord(j), tmp_low.net_coord(j));
              high.net_coord(j) = MAX(high.net_coord(j), tmp_high.net_coord(j));
            }

            // make an axial topology
            new (topology) XMI::Topology(&low, &high, &_self_coord,
                                         &torus_link[0]);

            return XMI_SUCCESS;
          }
        }
        return XMI_ERROR;
      }
#endif

