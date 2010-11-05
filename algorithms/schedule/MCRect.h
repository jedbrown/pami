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

#include "util/ccmi_debug.h"
/// \todo put this trace facility somewhere common
#include "components/devices/bgq/mu2/trace.h"

#ifdef CCMI_TRACE_ALL
  #define DO_TRACE_ENTEREXIT 1
  #define DO_TRACE_DEBUG     1
#else
  #define DO_TRACE_ENTEREXIT 0
  #define DO_TRACE_DEBUG     0
#endif

namespace CCMI
{
  namespace Schedule
  {
#define POSITIVE PAMI::Interface::Mapping::TorusPositive
#define NEGATIVE PAMI::Interface::Mapping::TorusNegative

    class MCRect: public CCMI::Interfaces::Schedule
    {

    protected:
      int               _nReduceDims;
      unsigned          _color;
      pami_coord_t       _root;
      pami_coord_t       _self;
      pami_coord_t       _ll;
      pami_coord_t       _ur;
      unsigned          _start_phase;
      unsigned          _nphases;
      int               _op;
      size_t            _peers;
      unsigned char     _torus_link[PAMI_MAX_DIMS];
      size_t            _phases_per_dim[PAMI_MAX_DIMS];
      size_t            _dim_sizes[PAMI_MAX_DIMS];
      PAMI_MAPPING_CLASS *_map;


      void setupLocal(PAMI::Topology *topo);

      int getReduceSrcTopology(unsigned phase, PAMI::Topology * topo);
      int getBroadcastSrcTopology(unsigned phase, PAMI::Topology * topo);
      int getReduceDstTopology(unsigned phase, PAMI::Topology * topo);
      int getBroadcastDstTopology(unsigned phase, PAMI::Topology * topo);

    public:
      MCRect(): _map(NULL)
      {
      }
      MCRect(unsigned myrank, 
             PAMI::Topology *rect,
             unsigned color): _color(color), _map(&__global.mapping)
      {
        TRACE_FN_ENTER();
        PAMI_assert(_map->globalDims() <= PAMI_MAX_DIMS);
        unsigned int i;
        _map->nodePeers(_peers);
        _map->task2network(_map->task(), &_self, PAMI_N_TORUS_NETWORK);
        DO_DEBUG(for (unsigned j = 0; j < _map->torusDims(); ++j) TRACE_FORMAT("<%u:%p>MCRect:: Rank %zu, coord[%u]=%zu",_color,this, _map->task(), j, _self.u.n_torus.coords[j]));
        _start_phase = (unsigned) -1;
        PAMI::Topology tmp = *rect;
        if (rect->type() != PAMI_COORD_TOPOLOGY) tmp.convertTopology(PAMI_COORD_TOPOLOGY);
        PAMI_assert(tmp.type() == PAMI_COORD_TOPOLOGY);
        tmp.rectSeg(&_ll, &_ur, &_torus_link[0]);
        DO_DEBUG(for (unsigned j = 0; j < _map->torusDims(); ++j) TRACE_FORMAT("<%u:%p>MCRect:: Rank %zu, ll coord[%u]=%zu",_color,this, _map->task(), j, _ll.u.n_torus.coords[j]));
        DO_DEBUG(for (unsigned j = 0; j < _map->torusDims(); ++j) TRACE_FORMAT("<%u:%p>MCRect:: Rank %zu, ur coord[%u]=%zu",_color,this, _map->task(), j, _ur.u.n_torus.coords[j]));
        _nReduceDims = 0;
        for (i = 0; i < _map->torusDims(); i++)
          if (_ur.net_coord(i))
            _dim_sizes[_nReduceDims++] = _ur.net_coord(i) - _ll.net_coord(i) + 1;

        TRACE_FORMAT("<%u:%p>_nReduceDims %d, _color  %u",_color,this,_nReduceDims, _color);
        TRACE_FN_EXIT();
      }

      MCRect(PAMI_MAPPING_CLASS *map,
             PAMI::Topology *rect,
             unsigned color): _color(color), _map(map)
      {
        TRACE_FN_ENTER();
        unsigned int i;
        _map->nodePeers(_peers);
        _map->task2network(map->task(), &_self, PAMI_N_TORUS_NETWORK);
        DO_DEBUG(for (unsigned j = 0; j < _map->torusDims(); ++j) TRACE_FORMAT("<%u:%p>MCRect:: Rank %zu, coord[%u]=%zu",_color,this, _map->task(), j, _self.u.n_torus.coords[j]));
        _start_phase = (unsigned) -1;
        PAMI::Topology tmp = *rect;
        if (rect->type() != PAMI_COORD_TOPOLOGY) tmp.convertTopology(PAMI_COORD_TOPOLOGY);
        PAMI_assert(tmp.type() == PAMI_COORD_TOPOLOGY);
        tmp.rectSeg(&_ll, &_ur, &_torus_link[0]);
        DO_DEBUG(for (unsigned j = 0; j < _map->torusDims(); ++j) TRACE_FORMAT("<%u:%p>MCRect:: Rank %zu, ll coord[%u]=%zu",_color,this, _map->task(), j, _ll.u.n_torus.coords[j]));
        DO_DEBUG(for (unsigned j = 0; j < _map->torusDims(); ++j) TRACE_FORMAT("<%u:%p>MCRect:: Rank %zu, ur coord[%u]=%zu",_color,this, _map->task(), j, _ur.u.n_torus.coords[j]));

        _nReduceDims = 0;
        for (i = 0; i < _map->torusDims(); i++)
          if (_ur.net_coord(i))
            _dim_sizes[_nReduceDims++] = _ur.net_coord(i) - _ll.net_coord(i) + 1;
        TRACE_FORMAT("<%u:%p>_nReduceDims %d, _color  %u",_color,this,_nReduceDims, _color);
        TRACE_FN_EXIT();
      }

      void init(int root, int op, int &start, int &nphases);
      virtual void
      init(int root, int op, int &startphase, int &nphases, int &maxranks)
      {
        PAMI_abort();
      }
      virtual void getSrcPeList (unsigned  phase, unsigned *srcpes,
                                 unsigned  &nsrc, unsigned *subtasks=NULL)
      {
        PAMI_abort();
      }
      virtual void getDstPeList (unsigned  phase, unsigned *dstpes,
                                 unsigned  &ndst, unsigned *subtasks)
      {
        PAMI_abort();
      }
      virtual pami_result_t getSrcUnionTopology(PAMI::Topology *topo);
      virtual pami_result_t getDstUnionTopology(PAMI::Topology *topo);
      virtual void getSrcTopology(unsigned phase, PAMI::Topology *topo);
      virtual void getDstTopology(unsigned phase, PAMI::Topology *topo);


      unsigned color()
      {
        TRACE_FN_ENTER();
        TRACE_FORMAT("<%u:%p> color  %u",_color,this,  _color);
        TRACE_FN_EXIT();
        return _color;
      }

      void setColor(unsigned c)
      {
        TRACE_FN_ENTER();
        _color  = c;
        TRACE_FORMAT("<%u:%p> color  %u",_color,this,  _color);
        TRACE_FN_EXIT();
      }

      /**
       * \brief Get colors that make sense for this rectangle.
       *
       * \param[in] rect	The rectange in question
       * \param[out] ideal	The recommended number of colors
       * \param[out] max	The maximum number of colors (returned in colors)
       * \param[out] colors	(optional) Array of enum Color's usable on rect
       */
      static void getColors(PAMI::Topology *rect, unsigned &ideal,
                            unsigned &max, unsigned *colors = NULL)
      {
        TRACE_FN_ENTER();
        uint i = 0;
        ideal = 0;
        pami_coord_t *ll=NULL, *ur=NULL;
        unsigned char *torus_link=NULL;
        size_t torus_dims, sizes[PAMI_MAX_DIMS];

        torus_dims = __global.mapping.torusDims();
        PAMI::Topology tmp = *rect;
        if (rect->type() != PAMI_COORD_TOPOLOGY) tmp.convertTopology(PAMI_COORD_TOPOLOGY);
        tmp.rectSeg(&ll, &ur, &torus_link);

        for (i = 0; i < torus_dims; i++)
        {  
          sizes[i] = ur->u.n_torus.coords[i] - ll->u.n_torus.coords[i] + 1;
          if (sizes[i] > 1)
          {  
            TRACE_FORMAT("color[%u]=%u", ideal, i);
            colors[ideal++] = i;
          }
        }

        max = ideal;

#if 0 //disable -ve colors
        for (i = 0; i < torus_dims; i++)
          if (sizes[i] > 1 && torus_link[i])
          {  
            TRACE_FORMAT("color[%u]=%zu",  max, i + torus_dims);
            colors[max++] = i + torus_dims;
          }

        if (max == 2 * ideal)
          ideal = max;
#endif

        if (ideal == 0)
        {
          ideal = max = 1;
          colors[0] = 0;
        }
        TRACE_FORMAT("ideal %u, max %u, color[%u]=%u",ideal,max,0,colors[0]);
        TRACE_FN_EXIT();
      }
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
  TRACE_FN_ENTER();
  int i, axis;
  nphases = 0;
  _op = op;
  _map->task2network(root, &_root, PAMI_N_TORUS_NETWORK);
  DO_DEBUG(for (unsigned j = 0; j < _map->torusDims(); ++j) TRACE_FORMAT("<%u:%p>MCRect:: Root %d, coord[%u]=%zu",_color,this, root, j, _root.u.n_torus.coords[j]));

  if (root < 0)
    _root = _ll;

  if (op == CCMI::BROADCAST_OP)
  {
    size_t axes[PAMI_MAX_DIMS] = {0};
    unsigned int my_phase, color = _color;

    size_t ndims = _map->torusDims();
    for (axis = 0; (size_t) axis < ndims; axis++)
      axes[axis] = color++ % ndims;

    for (axis = 0; (size_t) axis < ndims; axis++)
    {
      // this checks if I currently share the same examined coordinates of
      // the root (first we check an axis, a plane, a 3D cube, ..etc)
      for (my_phase = 1, i = axis + 1; (size_t)i < ndims && my_phase; i++)
      {
        TRACE_FORMAT("<%u:%p>_self.net_coord(axes[%u]=%zu, _root.net_coord(axes[%u]=%zu",_color,this,i,_self.net_coord(axes[i]),i,_root.net_coord(axes[i]));
        if (_self.net_coord(axes[i]) != _root.net_coord(axes[i]))
          my_phase = 0;
      }
      TRACE_FORMAT("<%u:%p>axis %d, my_phase %d, ndims %zu, axes[%u]=%zu",_color,this,axis,my_phase,ndims,i,axes[i]);

      if (my_phase)
        break;
    }

    // only root starts at phase 0, shift other's phase by 1
    _start_phase = axis;
    if (_map->task() != (pami_task_t) root)
      _start_phase += 1;


    start = _start_phase;
    nphases = ndims - start;

    // add 1 for local comm if any
    if (_peers > 1)
      nphases++;
  }

  else if (op == CCMI::REDUCE_OP)
  {
    start = 0;

    if (_peers > 1)
      nphases = 1;

    for (axis = 0; axis < _nReduceDims; axis++)
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
  _nphases = nphases;
  TRACE_FORMAT("<%u:%p>op %d, _peers %zu, _start_phase %u, nphases %d",_color,this,op, _peers, _start_phase, nphases);
  TRACE_FN_EXIT();
}


inline int
CCMI::Schedule::MCRect::getReduceSrcTopology(unsigned phase,
                                             PAMI::Topology * topo)
{
  TRACE_FN_ENTER();
  char my_phase;
  size_t axes[PAMI_MAX_DIMS] = {0};
  int i, axis, j;
  unsigned int total = 0, color = _color;
  pami_network type;
  pami_task_t src_id = 0;
  pami_coord_t src = _self;

  char dir = 1; // positive
  if (_color >= (unsigned) _nReduceDims) dir = 0; // negative

  if (_peers > 1)
  {
    total++;
    if (phase == 0)
    {
      setupLocal(topo);
      TRACE_FN_EXIT();
      return PAMI_SUCCESS;
    }
  }

  for (i = 0; i < _nReduceDims; i++)
    axes[i] = color++ % _nReduceDims;

  for (axis = _nReduceDims - 1; axis >= 0; axis--)
  {
    i = axes[axis];
    my_phase = 1;
    TRACE_FORMAT("<%u:%p>i %d, axis %d, my_phase %u, color %u",_color,this,i,axis,my_phase,color);

    for (j = _nReduceDims - 1; j > axis && my_phase; j--)
      if (_self.net_coord(axes[j]) != _root.net_coord(axes[j]))
        my_phase = 0;

    if (my_phase && phase >= total && phase < total + _phases_per_dim[i])
    {
      TRACE_STRING(" ");
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
      new (topo) PAMI::Topology(src_id);
  }
  DO_DEBUG(for (unsigned j = 0; j < _map->torusDims(); ++j) TRACE_FORMAT("<%u:%p>MCRect:: src %u, coord[%u]=%zu",_color,this, src_id, j, src.u.n_torus.coords[j]));
  TRACE_FN_EXIT();
  return PAMI_SUCCESS;
}


inline int 
CCMI::Schedule::MCRect::getBroadcastSrcTopology(unsigned phase,
                                                PAMI::Topology * topo)
{
  TRACE_FN_ENTER();
  TRACE_FN_EXIT();
  return PAMI_SUCCESS;
}


inline int
CCMI::Schedule::MCRect::getReduceDstTopology(unsigned phase,
                                             PAMI::Topology * topo)
{
  TRACE_FN_ENTER();
  char my_phase;
  size_t axes[PAMI_MAX_DIMS] = {0};
  int i, axis, j;
  unsigned int total = 0, color = _color;
  pami_network type;
  pami_task_t dst_id = 0;
  pami_coord_t dst = _self;

  char dir = 1; // positive
  if (_color >= (unsigned) _nReduceDims) dir = 0; // negative

  if (_peers > 1) total++;

  for (i = 0; i < _nReduceDims; i++)
    axes[i] = color++ % _nReduceDims;

  for (axis = _nReduceDims - 1; axis >= 0; axis--)
  {
    i = axes[axis];
    my_phase = 1;

    for (j = _nReduceDims - 1; j > axis && my_phase; j--)
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
    new (topo) PAMI::Topology(dst_id);
  DO_DEBUG(for (unsigned j = 0; j < _map->torusDims(); ++j) TRACE_FORMAT("<%u:%p>MCRect:: dst %u, coord[%u]=%zu",_color,this, dst_id, j, dst.u.n_torus.coords[j]));

  TRACE_FN_EXIT();
  return PAMI_SUCCESS;
}


inline int
CCMI::Schedule::MCRect::getBroadcastDstTopology(unsigned phase,
                                                PAMI::Topology * topo)
{
  TRACE_FN_ENTER();
  size_t core_dim = _map->torusDims();
  size_t ndims = _map->torusDims();
  if (_self.net_coord(core_dim) == _root.net_coord(core_dim))
  {
    if (phase >= _start_phase)
    {
      pami_coord_t low, high;
      unsigned char tl[PAMI_MAX_DIMS];

      //Find the axis to do the line broadcast on
      int axis = (phase + _color) % ndims;

      if (_torus_link[axis])
      {
        tl[axis] = POSITIVE;
        if (_color >= (unsigned) ndims)
          tl[axis] = NEGATIVE;
      }
      low = _self;
      high = _self;

      low.net_coord(axis) = MIN(_ll.net_coord(axis),
                                _self.net_coord(axis));
      high.net_coord(axis) = MAX(_ur.net_coord(axis),
                                 _self.net_coord(axis));

      TRACE_FORMAT("<%u:%p>phase %u, axis %d, _self %zu, ll %zu, ur %zu ",_color,this,phase, axis,_self.net_coord(axis),_ll.net_coord(axis),_ur.net_coord(axis));
      new (topo) PAMI::Topology(&low, &high, &_self, &tl[0]);
    }

    //Process local broadcasts
    if (phase == (unsigned) ndims && _peers > 1)
      setupLocal(topo);
  }
  if ((topo->size() == 1) && (topo->index2Rank(0) == _map->task()))
    new (topo) PAMI::Topology();
  TRACE_FN_EXIT();
  return PAMI_SUCCESS;
}

inline void
CCMI::Schedule::MCRect::getSrcTopology(unsigned phase,
                                       PAMI::Topology *topology)
{
  TRACE_FN_ENTER();
  switch (_op)
  {
  case CCMI::BROADCAST_OP:
    getBroadcastSrcTopology(phase, topology);
    break;
  case CCMI::REDUCE_OP:
    getReduceSrcTopology(phase, topology);
    break;
  default:
    CCMI_abort();
  }
  TRACE_FN_EXIT();
}

inline void
CCMI::Schedule::MCRect::getDstTopology(unsigned phase,
                                       PAMI::Topology *topology)
{
  TRACE_FN_ENTER();
  switch (_op)
  {
  case CCMI::BROADCAST_OP:
    getBroadcastDstTopology(phase, topology);
    break;
  case CCMI::REDUCE_OP:
    getReduceDstTopology(phase, topology);
    break;
  default:
    CCMI_abort();
  }
  TRACE_FN_EXIT();
}

inline pami_result_t
CCMI::Schedule::MCRect::getSrcUnionTopology(PAMI::Topology *topo)
{
  TRACE_FN_ENTER();
  TRACE_FN_EXIT();
  return PAMI_SUCCESS;
}

inline void
CCMI::Schedule::MCRect::setupLocal(PAMI::Topology *topo)
{
  TRACE_FN_ENTER();
  unsigned char torus_link[PAMI_MAX_DIMS] = {0};
  
  // the cores dim is the first one after the physical torus dims
  size_t core_dim = _map->torusDims();
  bool match = true; // matches the root local dims?
  
  for (size_t i = core_dim; i < _map->globalDims() && match; i++)
    if (_self.net_coord(i) != _root.net_coord(i))
      match = false;
  
  if (match)
  {
    pami_coord_t low, high;
    low = _self;
    high = _self;
    low.net_coord(core_dim) = MIN(low.net_coord(core_dim),
                                  _ll.net_coord(core_dim));
    high.net_coord(core_dim) = MAX(high.net_coord(core_dim),
                                   _ur.net_coord(core_dim));
    
    // make an axial topology
    new (topo) PAMI::Topology(&low, &high, &_self,
                              &torus_link[0]);    
  }
  else new (topo) PAMI::Topology();   // Empty topology
  
  TRACE_FORMAT("<%u:%p>match %u, core_dim %zu, global dims %zu",_color,this,match, core_dim, _map->globalDims());
  TRACE_FN_EXIT();
}

inline pami_result_t
CCMI::Schedule::MCRect::getDstUnionTopology(PAMI::Topology *topology)
{
  TRACE_FN_ENTER();
  unsigned char torus_link[PAMI_MAX_DIMS] = {0};
  unsigned char tmp_torus_link[PAMI_MAX_DIMS] = {0};

  unsigned char risTorus[PAMI_MAX_DIMS];
  PAMI::Topology tmp;
  pami_coord_t tmp_low, tmp_high, tmp_ref;
  pami_coord_t low, high;
  /// \todo Should this be all dims or just the torus dims?  For now, all dims and let multicast separate out the local.
  size_t ndims = _map->globalDims(); /// \todo ? _map->torusDims();
  // Init to empty topology
  new (topology) PAMI::Topology();

  low = _self;
  high = _self;
  pami_result_t result = PAMI_SUCCESS;

  for (int i = _start_phase; i < (int) (_start_phase + _nphases); i++)
  {
    getDstTopology(i, &tmp);
    #if DO_DEBUG(1)+0
    for (unsigned j = 0;  j < tmp.size(); ++j) 
    {  
      TRACE_FORMAT("<%u:%p>phase %d:: topology size %zu, topology[%d]=%u",_color,this,i,tmp.size(),j,tmp.index2Rank(j))
      pami_coord_t tmpcoord; 
      _map->task2network(tmp.index2Rank(j), &tmpcoord, PAMI_N_TORUS_NETWORK);
      for (unsigned k = 0; k < _map->torusDims(); ++k) TRACE_FORMAT("<%u:%p>coord[%u]=%zu",_color,this, k, tmpcoord.u.n_torus.coords[k]);
    }
    #endif
    TRACE_FORMAT("<%u:%p>i %d, tmp.size=%zu, tmp.type %s%u",_color,this,i, tmp.size(), tmp.type() == PAMI_AXIAL_TOPOLOGY? "PAMI_AXIAL_TOPOLOGY:": "", tmp.type());

    DO_DEBUG(for (unsigned j = 0; j < _map->torusDims(); ++j) TRACE_FORMAT("<%u:%p>MCRect:: Rank %zu, coord[%u]=%zu",_color,this, _map->task(), j, _self.u.n_torus.coords[j]));
    if (tmp.size())
    {
      // convert a local coord to axial?
      if (tmp.type() != PAMI_AXIAL_TOPOLOGY)
      {
        PAMI::Topology ctmp = tmp;
        /// \todo doesn't work? tmp.convertTopology(PAMI_AXIAL_TOPOLOGY);
        ctmp.convertTopology(PAMI_AXIAL_TOPOLOGY);
        TRACE_FORMAT("<%u:%p>tmp.convertTopology(PAMI_AXIAL_TOPOLOGY), input topology.type = %s%u",_color,this, 
                     ctmp.type() == PAMI_COORD_TOPOLOGY? "PAMI_COORD_TOPOLOGY:":(ctmp.type() == PAMI_EMPTY_TOPOLOGY? "PAMI_EMPTY_TOPOLOGY:": ""), ctmp.type())
        tmp.rectSeg(&tmp_low, &tmp_high, risTorus);
        _map->task2network(_map->task(), &tmp_ref, PAMI_N_TORUS_NETWORK);
        new (&tmp) PAMI::Topology(&tmp_low, &tmp_high, &tmp_ref, risTorus);
        TRACE_FORMAT("<%u:%p>tmp.convertTopology(PAMI_AXIAL_TOPOLOGY), output topology.type = %s%u",_color,this, 
                     ctmp.type() == PAMI_AXIAL_TOPOLOGY? "PAMI_AXIAL_TOPOLOGY:":(ctmp.type() == PAMI_EMPTY_TOPOLOGY? "PAMI_EMPTY_TOPOLOGY:": ""), ctmp.type())
        PAMI_assert(tmp.type() == PAMI_AXIAL_TOPOLOGY);
      #if DO_DEBUG(1)+0
        for (unsigned j = 0;  j < tmp.size(); ++j) 
        {  
          TRACE_FORMAT("<%u:%p>phase %d:: topology size %zu, topology[%d]=%u",_color,this,i,tmp.size(),j,tmp.index2Rank(j))
          pami_coord_t tmpcoord; 
          _map->task2network(tmp.index2Rank(j), &tmpcoord, PAMI_N_TORUS_NETWORK);
          for (unsigned k = 0; k < _map->torusDims(); ++k) TRACE_FORMAT("<%u:%p>coord[%u]=%zu",_color,this, k, tmpcoord.u.n_torus.coords[k]);
        }
      #endif
      }

      // Get the axial members
      result = tmp.axial(&tmp_low, &tmp_high,
                         &tmp_ref,
                         tmp_torus_link);

      PAMI_assert(result == PAMI_SUCCESS);

      // now add this topology to the union
      for (int j = 0; j < (int) ndims; j++)
      {
        torus_link[j] |= tmp_torus_link[j];
        low.net_coord(j) = MIN(low.net_coord(j), tmp_low.net_coord(j));
        high.net_coord(j) = MAX(high.net_coord(j), tmp_high.net_coord(j));
      }

      // make an axial topology
      new (topology) PAMI::Topology(&low, &high, &_self, torus_link);
    }
  }
  if((topology->size() == 1) && (topology->index2Rank(0) == _map->task()))
    new (topology) PAMI::Topology();

  TRACE_FN_EXIT();
  return result;
}
#endif
