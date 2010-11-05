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
#ifndef __pami_target_socklinux__

/*-------------------------------------------------*/
/*     Basic utility classes collectives           */
/*-------------------------------------------------*/

  #include "util/ccmi_debug.h"
  #include "util/ccmi_util.h"

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


//#define RECTBCAST_DEBUG   1

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
#define POSITIVE PAMI::Interface::Mapping::TorusPositive
#define NEGATIVE PAMI::Interface::Mapping::TorusNegative

    class TorusRect: public CCMI::Interfaces::Schedule
    {
    public:
      TorusRect(): _rect(), _map(NULL)
      {
      }

      TorusRect(unsigned myrank, 
                PAMI::Topology *rect,
                unsigned color):
      _ndims(0),
      _color(color),
//      _root(?),
//      _root_coord(?),
      _start_phase((unsigned)-1),
//      _nphases(?),
      _rect(*rect),
      _map(&__global.mapping)
      {
        TRACE_FN_ENTER();
        unsigned int i;
        _ndims = _map->torusDims();
        _map->task2network(_map->task(), &_self_coord, PAMI_N_TORUS_NETWORK);
        DO_DEBUG(for (unsigned j = 0; j < _map->torusDims(); ++j) TRACE_FORMAT("<%u:%p>TorusRect:: Rank %zu, coord[%u]=%zu",_color,this, _map->task(), j, _self_coord.u.n_torus.coords[j]));
        _rect.convertTopology(PAMI_COORD_TOPOLOGY);
        PAMI_assert(_rect.type() == PAMI_COORD_TOPOLOGY);
        _rect.rectSeg(&_ll, &_ur, &_torus_link[0]);
        DO_DEBUG(for (unsigned j = 0; j < _map->torusDims(); ++j) TRACE_FORMAT("<%u:%p>TorusRect:: Rank %zu, _ll coord[%u]=%zu",_color,this, _map->task(), j, _ll.u.n_torus.coords[j]));
        DO_DEBUG(for (unsigned j = 0; j < _map->torusDims(); ++j) TRACE_FORMAT("<%u:%p>TorusRect:: Rank %zu, _ur coord[%u]=%zu",_color,this, _map->task(), j, _ur.u.n_torus.coords[j]));
	
        for (i = 0; i < _ndims+1; i++)
	  _dim_sizes[i] = _ur.net_coord(i) - _ll.net_coord(i) + 1;

        TRACE_FORMAT("<%u:%p>_ndims %d, _color  %u",_color,this,_ndims, _color);
        TRACE_FN_EXIT();
      }
      TorusRect(PAMI_MAPPING_CLASS *map,
                PAMI::Topology *rect,
                pami_coord_t self,
                unsigned color):
      _ndims(map->torusDims()),
      _color(color),
//      _root(?),
//      _root_coord(?),
      _self_coord(self),
      _start_phase((unsigned)-1),
//      _nphases(?),
      _rect(*rect),
      _map(map)
      {
        TRACE_FN_ENTER();
        unsigned int i;
        DO_DEBUG(for (unsigned j = 0; j < _map->torusDims(); ++j) TRACE_FORMAT("<%u:%p>TorusRect:: Rank %zu, coord[%u]=%zu",_color,this, _map->task(), j, _self_coord.u.n_torus.coords[j]));

        _rect.convertTopology(PAMI_COORD_TOPOLOGY);
        PAMI_assert(_rect.type() == PAMI_COORD_TOPOLOGY);
        _rect.rectSeg(&_ll, &_ur, &_torus_link[0]);
        DO_DEBUG(for (unsigned j = 0; j < _map->torusDims(); ++j) TRACE_FORMAT("<%u:%p>TorusRect:: Rank %zu, _ll coord[%u]=%zu",_color,this, _map->task(), j, _ll.u.n_torus.coords[j]));
        DO_DEBUG(for (unsigned j = 0; j < _map->torusDims(); ++j) TRACE_FORMAT("<%u:%p>TorusRect:: Rank %zu, _ur coord[%u]=%zu",_color,this, _map->task(), j, _ur.u.n_torus.coords[j]));

	//get the size of the t dimension as well
        for (i = 0; i < _ndims+1; i++)
	  _dim_sizes[i] = _ur.net_coord(i) - _ll.net_coord(i) + 1;

        TRACE_FORMAT("<%u:%p>_ndims %d, _color  %u",_color,this,_ndims, _color);
        TRACE_FN_EXIT();
      }
      void init(int root, int op, int &start, int &nphases);

      virtual pami_result_t getSrcUnionTopology(PAMI::Topology *topo);
      virtual pami_result_t getDstUnionTopology(PAMI::Topology *topology);
      virtual void getSrcTopology(unsigned phase, PAMI::Topology *topology);
      virtual void getDstTopology(unsigned phase, PAMI::Topology *topology);
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


      unsigned color()
      {
        TRACE_FN_ENTER();
        TRACE_FORMAT("<%u:%p>_color=%u ",_color,this,_color);
        TRACE_FN_EXIT();
        return _color;
      }

      void setColor(unsigned c)
      {
        TRACE_FN_ENTER();
        _color  = c;
        TRACE_FORMAT("<%u:%p>_color=%u ",_color,this,_color);
        TRACE_FN_EXIT();
      }


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
            TRACE_FORMAT("color[%u]=%u",ideal, i);
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
      }
    protected:
      unsigned          _ndims;
      unsigned          _color;
      pami_task_t        _root;
      pami_coord_t       _root_coord;
      pami_coord_t       _self_coord;
      pami_coord_t       _ll;
      pami_coord_t       _ur;
      unsigned          _start_phase;
      unsigned int      _nphases;
      unsigned char     _torus_link[PAMI_MAX_DIMS];
      size_t            _dim_sizes[PAMI_MAX_DIMS];
      PAMI::Topology     _rect;
      PAMI_MAPPING_CLASS *_map;

      void setupBroadcast(int phase,  PAMI::Topology *topo);
      void setupGhost(PAMI::Topology *topo);
      void setupLocal(PAMI::Topology *topo);
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
  TRACE_FN_ENTER();
  CCMI_assert (op == CCMI::BROADCAST_OP);

  _root = root;
  _map->task2network(root, &_root_coord, PAMI_N_TORUS_NETWORK);

  DO_DEBUG(for (unsigned j = 0; j < _map->torusDims(); ++j) TRACE_FORMAT("<%u:%p>TorusRect:: Root %d, coord[%u]=%zu",_color,this, root, j, _root_coord.u.n_torus.coords[j]));

  size_t peers, axes[PAMI_MAX_DIMS] = {0};
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
      {
        TRACE_FORMAT("<%u:%p>_self_coordnet_coord(axes[%u]=%zu, _root_coord.net_coord(axes[%u]=%zu",_color,this,i,_self_coord.net_coord(axes[i]),i,_root_coord.net_coord(axes[i]));
        if (_self_coord.net_coord(axes[i]) != _root_coord.net_coord(axes[i]))
          myphase = 0;
      }
      TRACE_FORMAT("<%u:%p>axis %d, myphase %d, ndims %u, axes[%u]=%zu",_color,this,axis,myphase,_ndims,i,axes[i]);
      if (myphase)
        break;
    }
    _start_phase = axis;
  }

  start = _start_phase;
  nphases = _ndims + 2 - start; // 2: 1 for local comm if any, 1 for ghost
  //_map->nodePeers(peers);
  peers = _dim_sizes[_ndims];
  _nphases = nphases;
  if (peers == 1)
    _nphases = --nphases;
  TRACE_FORMAT("<%u:%p>_peers %zu, _start_phase %u, nphases %d",_color,this,peers, _start_phase, _nphases);
  TRACE_FN_EXIT();
}


inline pami_result_t
CCMI::Schedule::TorusRect::getSrcUnionTopology(PAMI::Topology *topo)
{
  printf("no need to implement\n");
  return PAMI_SUCCESS;
}
inline void
CCMI::Schedule::TorusRect::getSrcTopology(unsigned phase,
                                          PAMI::Topology *topo)
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
                                          PAMI::Topology *topo)
{
  TRACE_FN_ENTER();
  CCMI_assert (phase >= _start_phase);

  size_t core_dim, torus_dims, peers = 0;

  torus_dims = _map->torusDims();
  core_dim = torus_dims;
  peers = _dim_sizes[torus_dims];
  //_map->nodePeers(peers);

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
    if ((phase == ( _ndims + 1)) && (peers > 1))
      setupLocal(topo);
  }
  if ((topo->size() == 1) && (topo->index2Rank(0) == _map->task()))
    new (topo) PAMI::Topology();

#if DO_DEBUG(1)+0
  for (unsigned j = 0;  j < topo->size(); ++j) 
  {  
    TRACE_FORMAT("<%u:%p>topology size %zu, topology[%d]=%u",_color,this,topo->size(),j,topo->index2Rank(j))
    pami_coord_t tmpcoord; 
    _map->task2network(topo->index2Rank(j), &tmpcoord, PAMI_N_TORUS_NETWORK);
    for (unsigned k = 0; k < _map->torusDims(); ++k) TRACE_FORMAT("<%u:%p>coord[%u]=%zu",_color,this, k, tmpcoord.u.n_torus.coords[k]);
  }
#endif

  TRACE_FN_EXIT();
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
                                          PAMI::Topology *topo)
{
  TRACE_FN_ENTER();
  pami_coord_t low, high;

  //Find the axis to do the line broadcast on
  int axis = (phase + _color) % _ndims;
  if (_torus_link[0])
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

  TRACE_FORMAT("<%u:%p>phase %u, axis %d, _self %zu, ll %zu, ur %zu ",_color,this,phase, axis,_self_coord.net_coord(axis),_ll.net_coord(axis),_ur.net_coord(axis));
  new (topo) PAMI::Topology(&low, &high, &_self_coord,
                            &_torus_link[0]);
  TRACE_FN_EXIT();
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
CCMI::Schedule::TorusRect::setupGhost(PAMI::Topology *topo)
{
  TRACE_FN_ENTER();
  pami_coord_t ref, dst;

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
  TRACE_FORMAT("<%u:%p>axis %zu, _self %zu, ref %zu, ref %zu ",_color,this, axis,_self_coord.net_coord(axis),ref.net_coord(axis),ref.net_coord(axis));
  //The nodes that are different from the root in one dimension (not
  //the leading dimension of the color) are the ghosts. The nodes
  //just before them have to send data to them
  if (_self_coord.net_coord(axis) == ref.net_coord(axis))
  {
    //pami_network type=PAMI_N_TORUS_NETWORK;
    //pami_task_t dst_task=0;
    pami_coord_t low, high;

    dst = _self_coord;

    dst.net_coord(axis) = _root_coord.net_coord(axis);
    //_map->network2task(&dst, &dst_task, &type);

    //if(dst_task != _root)
    if (!( dst.u.n_torus.coords[0] == _root_coord.u.n_torus.coords[0] &&
           dst.u.n_torus.coords[1] == _root_coord.u.n_torus.coords[1] &&
           dst.u.n_torus.coords[2] == _root_coord.u.n_torus.coords[2] &&
           dst.u.n_torus.coords[3] == _root_coord.u.n_torus.coords[3] &&
           dst.u.n_torus.coords[4] == _root_coord.u.n_torus.coords[4] ))
    {
      low  = _self_coord;
      high = _self_coord;
      low.net_coord(axis) = MIN(dst.net_coord(axis),
                                _self_coord.net_coord(axis));
      high.net_coord(axis) = MAX(dst.net_coord(axis),
                                 _self_coord.net_coord(axis));
      TRACE_FORMAT("<%u:%p>axis %zu, _self %zu, dst %zu, dst %zu ",_color,this,axis,_self_coord.net_coord(axis),dst.net_coord(axis),dst.net_coord(axis));
      /// \todo why build an axial topoology for one ghost?   You can't
      /// multicast/deposit to it?  Why not leave it as a single rank topology.
      new (topo) PAMI::Topology(&low, &high, &_self_coord,
                                &_torus_link[0]);
    }
  }
  TRACE_FN_EXIT();
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
CCMI::Schedule::TorusRect::setupLocal(PAMI::Topology *topo)
{
  TRACE_FN_ENTER();
  unsigned char torus_link[PAMI_MAX_DIMS] = {0};
  //size_t peers;
  //_map->nodePeers(peers);

  // the cores dim is the first one after the physical torus dims
  size_t core_dim = _map->torusDims();
  bool match = true; // matches the root local dims?

  for (size_t i = core_dim; i < _map->globalDims() && match; i++)
    if (_self_coord.net_coord(i) != _root_coord.net_coord(i))
      match = false;

  if (match)
  {
    pami_coord_t low, high;
    low = _self_coord;
    high = _self_coord;
    low.net_coord(core_dim) = MIN(low.net_coord(core_dim),
                                  _ll.net_coord(core_dim));
    high.net_coord(core_dim) = MAX(high.net_coord(core_dim),
                                   _ur.net_coord(core_dim));

    // make an axial topology
    new (topo) PAMI::Topology(&low, &high, &_self_coord,
                              &torus_link[0]);    
  }
  else new (topo) PAMI::Topology();   // Empty topology
  TRACE_FORMAT("<%u:%p>match %u, core_dim %zu, global dims %zu",_color,this,match, core_dim, _map->globalDims());
  TRACE_FN_EXIT();
}


inline pami_result_t
CCMI::Schedule::TorusRect::getDstUnionTopology(PAMI::Topology *topology)
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

  low = _self_coord;
  high = _self_coord;
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
    DO_DEBUG(for (unsigned j = 0; j < _map->torusDims(); ++j) TRACE_FORMAT("<%u:%p> Rank %zu, coord[%u]=%zu",_color,this, _map->task(), j, _self_coord.u.n_torus.coords[j]));

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
      new (topology) PAMI::Topology(&low, &high, &_self_coord, torus_link);
    }
  }
  if((topology->size() == 1) && (topology->index2Rank(0) == _map->task()))
    new (topology) PAMI::Topology();

  TRACE_FN_EXIT();
  return result;
}



#endif // __pami_target_socklinux__
#endif
