/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/schedule/MCRect.h
 * \brief ???
 */

#ifndef __algorithms_schedule_MCRect_h__
#define __algorithms_schedule_MCRect_h__

/*-------------------------------------------------*/
/*     Basic utility classes collectives           */
/*-------------------------------------------------*/

#include "algorithms/interfaces/Schedule.h"
#include "common/TorusMappingInterface.h"
#include "Global.h"

namespace CCMI
{
  namespace Schedule
  {
#define POSITIVE XMI::Interface::Mapping::TorusPositive
#define NEGATIVE XMI::Interface::Mapping::TorusNegative

    class MCRect: public CCMI::Interfaces::Schedule
    {

    protected:
      int               _ndims;
      unsigned          _color;
      xmi_coord_t       _root;
      xmi_coord_t       _self;
      xmi_coord_t       _ll;
      xmi_coord_t       _ur;
      unsigned          _start_phase;
      int               _op;
      size_t            _peers;
      unsigned char     _torus_link[XMI_MAX_DIMS];
      size_t            _phases_per_dim[XMI_MAX_DIMS];
      size_t            _dim_sizes[XMI_MAX_DIMS];
      XMI_MAPPING_CLASS *_map;

        
      void setupLocal(XMI::Topology *topo);
      
      int getReduceSrcTopology(unsigned phase, XMI::Topology * topo);
      int getBroadcastSrcTopology(unsigned phase, XMI::Topology * topo);
      int getReduceDstTopology(unsigned phase, XMI::Topology * topo);
      int getBroadcastDstTopology(unsigned phase, XMI::Topology * topo);
      
    public:
      MCRect(): _map(NULL) {}
      MCRect(XMI_MAPPING_CLASS *map,
             XMI::Topology *rect,
             unsigned color): _color(color), _map(map)
      {
        unsigned int i;
        _map->nodePeers(_peers);
        _map->task2network(map->task(), &_self, XMI_N_TORUS_NETWORK);
        _start_phase = (unsigned) -1;
        rect->rectSeg(&_ll, &_ur, &_torus_link[0]);
        
        _ndims = 0;
        for (i = 0; i < _map->torusDims(); i++)
          if (_ur.net_coord(i))
            _dim_sizes[_ndims++] = _ur.net_coord(i) - _ll.net_coord(i) + 1;
      }

      void init(int root, int op, int &start, int &nphases);
      virtual xmi_result_t getSrcUnionTopology(XMI::Topology *topo);
      virtual xmi_result_t getDstUnionTopology(XMI::Topology *topo);
      virtual void getSrcTopology(unsigned phase, XMI::Topology *topo);
      virtual void getDstTopology(unsigned phase, XMI::Topology *topo);

      
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

    };  //-- MCRect
  };  //-- Schedule
}; //-- CCMI


//-------------------------------------------------------------------
//------  MCRect Schedule Functions ----------------------
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
CCMI::Schedule::MCRect::init(int root,
                             int op,
                             int &start,
                             int &nphases)
{
  int i, axis;
  nphases = 0;
  _op = op;
  _map->task2network(root, &_root, XMI_N_TORUS_NETWORK);

  if (root < 0)
    _root = _ll;

  if (op == CCMI::Interfaces::BROADCAST_OP)
  {
    size_t axes[XMI_MAX_DIMS] = {0};
    unsigned int my_phase, color = _color;
    
    for (axis = 0; axis < _ndims; axis++)
      axes[axis] = color++ % _ndims;
  
    for (axis = 0; axis < _ndims; axis++)
    {
      // this checks if I currently share the same examined coordinates of
      // the root (first we check an axis, a plane, a 3D cube, ..etc)
      for (my_phase = 1, i = axis + 1; i < _ndims && my_phase; i++)
        if (_self.net_coord(axes[i]) != _root.net_coord(axes[i]))
          my_phase = 0;
    
      if (my_phase)
        break;
    }
  
    // only root starts at phase 0, shift other's phase by 1
    _start_phase = axis;
    if (_map->task() != (xmi_task_t) root)
      _start_phase += 1;
  

    start = _start_phase;
    nphases = _ndims - start;
    
    // add 1 for local comm if any
    if (_peers > 1)
      nphases++;
  }

  else if (op == CCMI::Interfaces::REDUCE_OP)
  {
    start = 0;

    if (_peers > 1)
      nphases = 1;

    for (axis = 0; axis < _ndims; axis++)
    {
      _phases_per_dim[axis] = _dim_sizes[axis];
      
      // if root is in the middle of this dimension, we add an extra
      // phase since data is coming from another direction to root
      if (_root.net_coord(axis) != 0 &&
          _root.net_coord(axis) != _dim_sizes[axis] - 1 &&
          !_torus_link[axis])
        _phases_per_dim[axis]++;
      
      nphases += _phases_per_dim[axis];
    }
  }
  
}


inline int
CCMI::Schedule::MCRect::getReduceSrcTopology(unsigned phase,
                                             XMI::Topology * topo)
{
  char my_phase;
  size_t axes[XMI_MAX_DIMS] = {0};
  int i, axis, j;
  unsigned int total = 0, color = _color;
  xmi_network type;
  xmi_task_t src_id = 0;
  xmi_coord_t src = _self;

  char dir = 1; // positive
  if (_color >= (unsigned) _ndims) dir = 0; // negative
  
  if (_peers > 1)
  {
    total++;
    if (phase == 0)
    {
      setupLocal(topo);
      return XMI_SUCCESS;
    }
  }
  
  for (i = 0; i < _ndims; i++)
    axes[i] = color++ % _ndims;
    
  for (axis = _ndims - 1; axis >= 0; axis--)
  {
    i = axes[axis];
    my_phase = 1;
    
    for (j = _ndims - 1; j > axis && my_phase; j--)
      if (_self.net_coord(axes[j]) != _root.net_coord(axes[j]))
        my_phase = 0;

    if (my_phase && phase >= total && phase < total + _phases_per_dim[i])
    {
      size_t head = 0;
      size_t tail = _dim_sizes[i] - 1;
      size_t my_coord = _self.net_coord(i);
      size_t root_coord = _root.net_coord(i);
        
      if (root_coord == head) // if root is at head of this dim
      {
        //if (my_coord == (_dim_sizes[i] - 1 - phase % _dim_sizes[i]))
        if (my_coord == _dim_sizes[i] - 1 + total - phase)
        {
          // if this line is a torus and data is moving in a positive direction,
          // and I am not the node after the root
          if (_torus_link[i] && dir == 1 && my_coord != head + 1)
          {
            src.net_coord(i) = (my_coord - 1 + _dim_sizes[i]) % _dim_sizes[i];
            break;
          }
          
          // otherwise, data either moves in a negative direction, or this line
          // isn't a torus line. In either case, make sure tail recvs nothing
          else if (my_coord != tail)
          {
            src.net_coord(i) = my_coord + 1;
            break;
          }
        }
      }
      
      else if (root_coord == tail) // if root is at tail of this dim
      {
        if (my_coord == phase - total)
        {
          // if this line is a torus and data is moving in a negative direction,
          // and I am not the node before the root
          if (_torus_link[i] && dir == 0 && my_coord != tail - 1)
          {
            src.net_coord(i) = (my_coord + 1) % _dim_sizes[i];
            break;
          }

          // otherwise, data either moves in positive direction, or this line
          // isn't a torus line. In either case, make sure head recvs nothing
          else if (my_coord != head)
          {
            src.net_coord(i) = my_coord - 1;
            break;
          }
        }
      }
      
      else // root is in the middle of this dim
      {
        if (_torus_link[i]) // if torus dim, then data moves in one direction
        {
          if (my_coord == ((root_coord + 1 + phase - total) % _dim_sizes[i]))
          {
            // negative direction, the node before the root recv nothing
            if (dir == 0 && my_coord != root_coord - 1)
            {
              src.net_coord(i) = (my_coord + 1) % _dim_sizes[i];
              break;
            }
            // positive direction, the node after the root recv nothing
            else if (my_coord != root_coord + 1)
            {
              src.net_coord(i) = (my_coord - 1 + _dim_sizes[i]) % _dim_sizes[i];
              break;
            }
          }

        }
          
        else // data gets reduced to root of this dim from - and + directions
        {
          if (my_coord <= root_coord &&
              my_coord == phase - total &&
              my_coord != head)
          {
            src.net_coord(i) = my_coord - 1;
            break;
          }
          if (my_coord >= root_coord &&
              my_coord == (_dim_sizes[i] + total - phase + root_coord) &&
              my_coord != tail)
          {
            src.net_coord(i) = my_coord + 1;
            break;
          }
        }
      }
    }

    total += _phases_per_dim[i];    
  }

  if (src_id >= 0)
  {
    _map->network2task(&src, &src_id, &type);
    if (src_id != _map->task())
      new (topo) XMI::Topology(src_id);
  }
  return XMI_SUCCESS;
}


int CCMI::Schedule::MCRect::getBroadcastSrcTopology(unsigned phase,
                                                     XMI::Topology * topo)
{
  return XMI_SUCCESS;
}


inline int
CCMI::Schedule::MCRect::getReduceDstTopology(unsigned phase,
                                             XMI::Topology * topo)
    
{
  char my_phase;
  size_t axes[XMI_MAX_DIMS] = {0};
  int i, axis, j;
  unsigned int total = 0, color = _color;
  xmi_network type;
  xmi_task_t dst_id = 0;
  xmi_coord_t dst = _self;

  char dir = 1; // positive
  if (_color >= (unsigned) _ndims) dir = 0; // negative
  
  if (_peers > 1) total++;

  for (i = 0; i < _ndims; i++)
    axes[i] = color++ % _ndims;
    
  for (axis = _ndims - 1; axis >= 0; axis--)
  {
    i = axes[axis];
    my_phase = 1;
    
    for (j = _ndims - 1; j > axis && my_phase; j--)
      if (_self.net_coord(axes[j]) != _root.net_coord(axes[j]))
        my_phase = 0;

    if (my_phase && phase >= total && phase < total + _phases_per_dim[i])
    {
      size_t head = 0;
      size_t tail = _dim_sizes[i] - 1;
      size_t my_coord = _self.net_coord(i);
      size_t root_coord = _root.net_coord(i);

      if (my_coord != root_coord)
      {
        if (root_coord == head)
        {
          if (my_coord == _dim_sizes[i] - 1 + total - phase)
          {
            // if this line is a torus and data is moving in a positive dir
            if (_torus_link[i] && dir == 1)
            {
              dst.net_coord(i) = (my_coord + 1) % _dim_sizes[i];
              break;
            }
            
            // either data moves in a "-" direction, or this isnt a torus line
            else
            {
              dst.net_coord(i) = my_coord - 1;
              break;
            }
          }
        }
        
        else if (root_coord == tail) // if root is at tail of this dim
        {
          if (my_coord == phase - total)
          {
            // if this line is a torus and data is moving in a "+" direction
            if (_torus_link[i] && dir == 1)
            {
              dst.net_coord(i) = my_coord + 1;
              break;
            }
            
            // Either data moves in "-" direction, or this isnt a torus line
            else 
            {
              dst.net_coord(i) = (my_coord - 1 + _dim_sizes[i]) % _dim_sizes[i];
              break;
            }
          }
        }
        
        else // root is in the middle of this dim
        {
          if (_torus_link[i]) // if torus dim, then data moves in one direction
          {
            if (my_coord == ((root_coord + phase - total) % _dim_sizes[i]))
            {
              if (dir == 0) // negative direction
              {
                dst.net_coord(i) = (my_coord - 1 + _dim_sizes[i])%_dim_sizes[i];
                break;
              }
              else // positive direction
              {
                dst.net_coord(i) = (my_coord + 1) % _dim_sizes[i];
                break;
              }
            }
          }
          
          else // data gets reduced to root of this dim from - and + directions
          {
            if (my_coord < root_coord &&
                my_coord == phase - total)
            {
              dst.net_coord(i) = my_coord + 1;
              break;
            }
            if (my_coord > root_coord &&
                my_coord == (_dim_sizes[i] + total - phase + root_coord))
            {
              dst.net_coord(i) = my_coord - 1;
              break;
            }
          }
        }
      }
    }

    total += _phases_per_dim[i];    
  }

  _map->network2task(&dst, &dst_id, &type);

  if (dst_id != _map->task())
    new (topo) XMI::Topology(dst_id);

  return XMI_SUCCESS;
}


inline int
CCMI::Schedule::MCRect::getBroadcastDstTopology(unsigned phase,
                                                XMI::Topology * topo)
{
  size_t core_dim = _map->torusDims();

  if (_self.net_coord(core_dim) == _root.net_coord(core_dim))
  {
    if (phase >= _start_phase)
    {
      xmi_coord_t low, high;
      unsigned char tl[XMI_MAX_DIMS];
      
      //Find the axis to do the line broadcast on
      int axis = (phase + _color) % _ndims;

      if (_torus_link[axis])
      {
        tl[axis] = POSITIVE;
        if (_color >= (unsigned) _ndims)
          tl[axis] = NEGATIVE;
      }
      low = _self;
      high = _self;

      low.net_coord(axis) = MIN(_ll.net_coord(axis),
                                _self.net_coord(axis));
      high.net_coord(axis) = MAX(_ur.net_coord(axis),
                                 _self.net_coord(axis));
   
      new (topo) XMI::Topology(&low, &high, &_self, &tl[0]);
    }
    
    //Process local broadcasts
    if (phase == (unsigned) _ndims && _peers > 1)
      setupLocal(topo);
  }  
  return XMI_SUCCESS;
}

inline void
CCMI::Schedule::MCRect::getSrcTopology(unsigned phase,
                                       XMI::Topology *topology)
{
  switch(_op)
  {
    case CCMI::Interfaces::BROADCAST_OP:
      getBroadcastSrcTopology(phase, topology);
      break;
    case CCMI::Interfaces::REDUCE_OP:
      getReduceSrcTopology(phase, topology);
      break;
    default:
      CCMI_abort();
  }    
}

inline void
CCMI::Schedule::MCRect::getDstTopology(unsigned phase,
                                       XMI::Topology *topology)
{
  switch(_op)
  {
    case CCMI::Interfaces::BROADCAST_OP:
      getBroadcastDstTopology(phase, topology);
      break;
    case CCMI::Interfaces::REDUCE_OP:
      getReduceDstTopology(phase, topology);
      break;
    default:
      CCMI_abort();
  }    
}

inline xmi_result_t
CCMI::Schedule::MCRect::getSrcUnionTopology(XMI::Topology *topo)
{
  return XMI_SUCCESS;
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
CCMI::Schedule::MCRect::setupLocal(XMI::Topology *topo)
{
  // the cores dim is the first one after the physical torus dims
  size_t core_dim = _map->torusDims();
  bool match = true; // matches the root local dims?
  
  for(size_t i = core_dim; i < _map->globalDims() && match; i++)
    if (_self.net_coord(i) != _root.net_coord(i))
      match = false;
  
  if (match)
  {
    *topo = __global.topology_local;
  }
}

inline xmi_result_t
CCMI::Schedule::MCRect::getDstUnionTopology(XMI::Topology *topology)
{
  /*
    int i, j;
    unsigned char dir[XMI_MAX_DIMS] = {0};
    unsigned char torus_link[XMI_MAX_DIMS] = {0};
    unsigned char tmp_dir[XMI_MAX_DIMS] = {0};
    unsigned char tmp_torus_link[XMI_MAX_DIMS] = {0};

    XMI::Topology tmp;
    xmi_coord_t tmp_low, tmp_high;
    xmi_coord_t low, high;

    CCMI_assert(topology->size() != 0);
    low = _self_coord;
    high = _self_coord;

    for (i = _start_phase; i < (int) (_start_phase + _nphases); i++)
    {
    getDstTopology(i, &tmp);

    if (tmp.size())
    {
    tmp.getAxialOrientation(&tmp_torus_link[0]);
    tmp.getAxialDirs(&tmp_dir[0]);

    // now get the low and high coords of this axial, -1 means I dont
    // care about which axis
    tmp.getAxialEndCoords(&tmp_low, &tmp_high, -1);

    // now add this topology to the union
    for (j = 0; j < (int) _ndims; j++)
    {
    dir[j] |= tmp_dir[j];
    torus_link[j] |= tmp_torus_link[j];
    low.net_coord(j) = MIN(low.net_coord(j), tmp_low.net_coord(j));
    high.net_coord(j) = MAX(high.net_coord(j), tmp_high.net_coord(j));
    }

    // make an axial topology
    new (topology) XMI::Topology(&low, &high, &_self_coord, &dir[0],
    &torus_link[0]);
  */
  return XMI_SUCCESS;
}

#endif
