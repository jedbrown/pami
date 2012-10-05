/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* This is an automatically generated copyright prolog.             */
/* After initializing,  DO NOT MODIFY OR MOVE                       */
/*  --------------------------------------------------------------- */
/* Licensed Materials - Property of IBM                             */
/* Blue Gene/Q 5765-PER 5765-PRP                                    */
/*                                                                  */
/* (C) Copyright IBM Corp. 2011, 2012 All Rights Reserved           */
/* US Government Users Restricted Rights -                          */
/* Use, duplication, or disclosure restricted                       */
/* by GSA ADP Schedule Contract with IBM Corp.                      */
/*                                                                  */
/*  --------------------------------------------------------------- */
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
  #include "util/trace.h"

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

#define MY_TASK __global.mapping.task()
#define MESH     PAMI::Interface::Mapping::Mesh
#define POSITIVE PAMI::Interface::Mapping::TorusPositive
#define NEGATIVE PAMI::Interface::Mapping::TorusNegative
    
    static const uint16_t torus_rect_mod5_table [16] = { 0, 1, 2, 3, 4, 
							 0, 1, 2, 3, 4, 
							 0, 1, 2, 3, 4, 
							 0 };
    
    class TorusRect: public CCMI::Interfaces::Schedule
    {
    public:
      static const unsigned NO_COLOR = 0; 

      TorusRect()
      {
      }

      TorusRect(unsigned myrank, 
                PAMI::Topology *rect,
                unsigned color):
      _color(color),
//      _root(?),
//      _root_coord(?),
      _start_phase((unsigned)-1)
//      _nphases(?),
      {
        (void)myrank;
        TRACE_FN_ENTER();
        _ndims = __global.mapping.torusDims();
        __global.mapping.task2network(__global.mapping.task(), &_self_coord, PAMI_N_TORUS_NETWORK);
	
        DO_DEBUG(for (unsigned j = 0; j < __global.mapping.torusDims(); ++j) TRACE_FORMAT("<%u:%p>TorusRect:: Rank %zu, coord[%u]=%zu",_color,this, __global.mapping.task(), j, _self_coord.u.n_torus.coords[j]));

	PAMI_assertf(rect->type() == PAMI_COORD_TOPOLOGY, "Type %u",rect->type());
        rect->rectSeg(&_ll, &_ur, &_torus_link);
        DO_DEBUG(for (unsigned j = 0; j < __global.mapping.torusDims(); ++j) TRACE_FORMAT("<%u:%p>TorusRect:: Rank %zu, _ll coord[%u]=%zu",_color,this, __global.mapping.task(), j, _ll->u.n_torus.coords[j]));
        DO_DEBUG(for (unsigned j = 0; j < __global.mapping.torusDims(); ++j) TRACE_FORMAT("<%u:%p>TorusRect:: Rank %zu, _ur coord[%u]=%zu",_color,this, __global.mapping.task(), j, _ur->u.n_torus.coords[j]));
	
        //for (i = 0; i < _ndims+1; i++)
	//_dim_sizes[i] = _ur->net_coord(i) - _ll->net_coord(i) + 1;
	_peers = _ur->net_coord(_ndims) - _ll->net_coord(_ndims) + 1;

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
      _start_phase((unsigned)-1),
//      _nphases(?),
      _self_coord(self)
      {
        TRACE_FN_ENTER();
        DO_DEBUG(for (unsigned j = 0; j < __global.mapping.torusDims(); ++j) TRACE_FORMAT("<%u:%p>TorusRect:: Rank %zu, coord[%u]=%zu",_color,this, __global.mapping.task(), j, _self_coord.u.n_torus.coords[j]));

	PAMI_assertf(rect->type() == PAMI_COORD_TOPOLOGY, "Type %u",rect->type());
        rect->rectSeg(&_ll, &_ur, &_torus_link);
        DO_DEBUG(for (unsigned j = 0; j < __global.mapping.torusDims(); ++j) TRACE_FORMAT("<%u:%p>TorusRect:: Rank %zu, _ll coord[%u]=%zu",_color,this, __global.mapping.task(), j, _ll->u.n_torus.coords[j]));
        DO_DEBUG(for (unsigned j = 0; j < __global.mapping.torusDims(); ++j) TRACE_FORMAT("<%u:%p>TorusRect:: Rank %zu, _ur coord[%u]=%zu",_color,this, __global.mapping.task(), j, _ur->u.n_torus.coords[j]));

	//get the size of the t dimension as well
        //for (i = 0; i < _ndims+1; i++)
	//_dim_sizes[i] = _ur->net_coord(i) - _ll->net_coord(i) + 1;

	_peers = _ur->net_coord(_ndims) - _ll->net_coord(_ndims) + 1;

        TRACE_FORMAT("<%u:%p>_ndims %d, _color  %u",_color,this,_ndims, _color);
        TRACE_FN_EXIT();
      }
 
      virtual void init(int root, int op, int &start, int &nphases);
      virtual pami_result_t getSrcUnionTopology(PAMI::Topology *topo, pami_endpoint_t *src_eps=NULL);
      virtual pami_result_t getDstUnionTopology(PAMI::Topology *topology,pami_endpoint_t *dst_eps=NULL,unsigned num_eps=0);
      virtual void getSrcTopology(unsigned phase, PAMI::Topology *topology, pami_endpoint_t *src_eps=NULL);
      virtual void getDstTopology(unsigned phase, PAMI::Topology *topology, pami_endpoint_t *dst_eps=NULL);

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
        PAMI_assertf(rect->type() == PAMI_COORD_TOPOLOGY, "Type %u",rect->type());
        rect->rectSeg(&ll, &ur, &torus_link);

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

#if 1 //enable +ve colors
        for (i = 0; i < torus_dims; i++) {
          if (sizes[i] > 1 && torus_link[i])
          {
            TRACE_FORMAT("color[%u]=%zu",  max, i + torus_dims);
            colors[max++] = i+torus_dims;
          }
        }

        if (max == 2 * ideal)
          ideal = max;
#endif
        if (ideal == 0)
        {
          ideal = max = 1;
          colors[0] = 0;
        }
        TRACE_FN_EXIT();
      }
    protected:
      unsigned             _ndims;
      unsigned             _color;
      pami_task_t          _root;
      unsigned             _nphases;
      unsigned             _start_phase;
      unsigned             _peers;
      pami_coord_t       * _ll;
      pami_coord_t       * _ur;
      unsigned char      * _torus_link;
      pami_coord_t         _self_coord;
      pami_coord_t         _root_coord;

      void getDstTopology_internal(unsigned phase,  int &axis, size_t &ll, size_t &ur, uint8_t &tlink);
      void setupBroadcast(unsigned phase,  int &axis, size_t &ll, size_t &ur, uint8_t &tlink);
      void setupGhost(int &axis, size_t &ll, size_t &ur, uint8_t &tlink);
      void setupLocal(int &axis, size_t &ll, size_t &ur, uint8_t &tlink);
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
  __global.mapping.task2network(root, &_root_coord, PAMI_N_TORUS_NETWORK);

  DO_DEBUG(for (unsigned j = 0; j < __global.mapping.torusDims(); ++j) TRACE_FORMAT("<%u:%p>TorusRect:: Root %d, coord[%u]=%zu",_color,this, root, j, _root_coord.u.n_torus.coords[j]));

  size_t axes[PAMI_MAX_DIMS] = {0};
  unsigned int i, myphase, axis, color = _color;

  for (axis = 0; axis < _ndims; axis++) 
    //axes[axis] = color++ % _ndims;
    axes[axis] = torus_rect_mod5_table [color++];

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

  // 2: 1 for local comm if any, 1 for ghost
  _nphases = nphases = _ndims + 2 - start;
  
  if (_peers == 1)
    _nphases = --nphases;

  TRACE_FN_EXIT();
}


inline pami_result_t
CCMI::Schedule::TorusRect::getSrcUnionTopology(PAMI::Topology *topo, pami_endpoint_t *src_eps)
{
  (void)src_eps;
  //  printf("no need to implement\n");
  new (topo) PAMI::Topology(_root);
  return PAMI_SUCCESS;
}
inline void
CCMI::Schedule::TorusRect::getSrcTopology(unsigned phase,
                                          PAMI::Topology *topo,
                                          pami_endpoint_t *src_eps)
{
  (void)phase;(void)topo;(void)src_eps;
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
CCMI::Schedule::TorusRect::getDstTopology_internal(unsigned  phase,  
						   int     & raxis, 
						   size_t  & ll, 
						   size_t  & ur, 
						   uint8_t & tlink) 
{
  size_t core_dim, torus_dims;
  torus_dims = _ndims;
  core_dim = torus_dims;

  if (_self_coord.net_coord(core_dim) == _root_coord.net_coord(core_dim))
  {
    //call setup broadcast with phase 0 which implies root
    if (MY_TASK == _root && phase == 0) 
      setupBroadcast(phase, raxis, ll, ur, tlink);
    else if (MY_TASK != _root && phase > _start_phase)
    {
      // setup the destination processors to foreward the data along
      // the next dimension in the torus
      if (phase < _ndims)
        setupBroadcast(phase, raxis, ll, ur, tlink);

      ///Process ghost nodes
      else if (phase == _ndims)
        setupGhost(raxis, ll, ur, tlink);
    }

    ///Process local broadcasts
    if ((phase == ( _ndims + 1)) && (_peers > 1))
      setupLocal(raxis, ll, ur, tlink);
  }
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
                                          PAMI::Topology *topo,
                                          pami_endpoint_t *dst_eps)
{
  (void)dst_eps;
  TRACE_FN_ENTER();
  CCMI_assert (phase >= _start_phase);

  int axis = -1;
  size_t ll = 0, ur = 0;
  uint8_t toruslinks[PAMI_MAX_DIMS] = {0};
  uint8_t tlink = 0;

  getDstTopology_internal(phase, axis, ll, ur, tlink);

  //We have something to do
  if (axis >= 0) {
    pami_coord_t low, high;
    low  = _self_coord;
    high = _self_coord;  
    low.net_coord(axis) = ll;
    high.net_coord(axis) = ur; 
    toruslinks[axis] = tlink;
    new (topo) PAMI::Topology(&low, &high, &_self_coord,
			      toruslinks);
  }
  //We have nothing to do, create dummy topo
  else  new (topo) PAMI::Topology();

  if ((topo->size() == 1) && (topo->index2Endpoint(0) == __global.mapping.task()))
    new (topo) PAMI::Topology();

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
CCMI::Schedule::TorusRect::setupBroadcast(unsigned  phase,  
					  int     & raxis, 
					  size_t  & ll, 
					  size_t  & ur, 
					  uint8_t & tlink)
{
  TRACE_FN_ENTER();
  //Find the axis to do the line broadcast on
  //int axis = (phase + _color) % _ndims;
  int axis = torus_rect_mod5_table[(phase + _color)];
  
  if (_torus_link[axis])
  {
    tlink = POSITIVE;
    if (_color >= _ndims)
      tlink = NEGATIVE;
  }

  raxis = (int) axis;
  ll = MIN(_ll->net_coord(axis),
	   _self_coord.net_coord(axis));
  ur = MAX(_ur->net_coord(axis),
	   _self_coord.net_coord(axis));

  TRACE_FORMAT("<%u:%p>phase %u, axis %d, _self %zu, ll %zu, ur %zu \n",_color,this,phase, axis,_self_coord.net_coord(axis),_ll->net_coord(axis),_ur->net_coord(axis));
  
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
CCMI::Schedule::TorusRect::setupGhost(int     & raxis, 
				      size_t  & ll, 
				      size_t  & ur, 
				      uint8_t & tlink)
{
  TRACE_FN_ENTER();
  //pami_coord_t dst;
  size_t dst_coord;
  size_t ref;

  //size_t axis = _color % _ndims;
  size_t axis = torus_rect_mod5_table[_color];
  size_t dim_size = _ur->net_coord(axis) - _ll->net_coord(axis) + 1;

  CCMI_assert(dim_size > 1);

  ref = _self_coord.net_coord(axis);
  if (_torus_link[axis]) // if this dim or axis is a torus
  {
    if (_color < _ndims) { //+ve colors
      ref = (_root_coord.net_coord(axis) + 1) % dim_size;
      tlink = NEGATIVE;
    }
    else {
      ref = (_root_coord.net_coord(axis) + dim_size - 1) % dim_size;
      tlink = POSITIVE;
    }
  }
  else
  {
    ref = _root_coord.net_coord(axis) + 1;
    if (ref >= dim_size + _ll->net_coord(axis))
    {
      ref = _root_coord.net_coord(axis) - 1;
    }
  }
  TRACE_FORMAT("<%u:%p>axis %zu, _self %zu, ref %zu\n",_color,this, axis,_self_coord.net_coord(axis),ref);
  //The nodes that are different from the root in one dimension (not
  //the leading dimension of the color) are the ghosts. The nodes
  //just before them have to send data to them
  if (_self_coord.net_coord(axis) == ref /*&& _start_phase > 0*/)
  {
    pami_coord_t dst = _self_coord;
    dst.net_coord(axis) = _root_coord.net_coord(axis);
     
    if ( dst.u.n_torus.coords[0] == _root_coord.u.n_torus.coords[0] &&
	 dst.u.n_torus.coords[1] == _root_coord.u.n_torus.coords[1] &&
	 dst.u.n_torus.coords[2] == _root_coord.u.n_torus.coords[2] &&
	 dst.u.n_torus.coords[3] == _root_coord.u.n_torus.coords[3] &&
	 dst.u.n_torus.coords[4] == _root_coord.u.n_torus.coords[4] )
      return;
    
    dst_coord = _root_coord.net_coord(axis);
    raxis = (int) axis;
    ll = _self_coord.net_coord(axis);
    ur = _self_coord.net_coord(axis);
    
    if (tlink == MESH) {
      ll  = MIN(dst_coord,
		_self_coord.net_coord(axis));
      ur = MAX(dst_coord,
	       _self_coord.net_coord(axis));
    }
    //Data must go from self to dst
    else if (tlink == POSITIVE) {
      //In the case of torus we may have a wrapped  (low is self)
      ur = dst_coord;
    }
    else if (tlink == NEGATIVE) {
      //In the case of torus we may have a wrapped  (high is self)
      ll = dst_coord;
    }
    
    TRACE_FORMAT("<%u:%p>axis %zu, _self %zu, dst %zu tlink %d\n",_color,this,axis,_self_coord.net_coord(axis),dst_coord, 
		 tlink);      
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
CCMI::Schedule::TorusRect::setupLocal(int     & raxis, 
				      size_t  & ll, 
				      size_t  & ur, 
				      uint8_t & tlink)
{
  TRACE_FN_ENTER();
  tlink = 0;

  // the cores dim is the first one after the physical torus dims
  size_t core_dim = _ndims;
  bool match = true; // matches the root local dims?

  for (size_t i = core_dim; i < __global.mapping.globalDims() && match; i++)
    if (_self_coord.net_coord(i) != _root_coord.net_coord(i))
      match = false;

  if (match)
  {
    raxis = (int) core_dim;
    ll = MIN(_self_coord.net_coord(core_dim),
	     _ll->net_coord(core_dim));
    ur = MAX(_self_coord.net_coord(core_dim),
	     _ur->net_coord(core_dim));
  }

  TRACE_FORMAT("<%u:%p>match %u, core_dim %zu, global dims %zu",_color,this,match, core_dim, __global.mapping.globalDims());
  TRACE_FN_EXIT();
}


inline pami_result_t
CCMI::Schedule::TorusRect::getDstUnionTopology(PAMI::Topology *topology,pami_endpoint_t *dst_eps,unsigned num_eps)
{
  (void)dst_eps;
  (void)num_eps;
  TRACE_FN_ENTER();
  unsigned char torus_link[PAMI_MAX_DIMS] = {0};

  pami_coord_t low, high;
  low  = _self_coord;
  high = _self_coord;
  pami_result_t result = PAMI_SUCCESS;

  for (int i = _start_phase; i < (int) (_start_phase + _nphases); i++)
  {
    int axis = -1;
    size_t ll = 0, ur = 0;
    uint8_t tlink = 0;

    getDstTopology_internal(i, axis, ll, ur, tlink);

    if (axis >= 0)
    {
      torus_link[axis] |= tlink;
      
      ///On a torus network when the wrap links are used lo and hi are relative (SK)       
      if (ll  != _self_coord.net_coord(axis))
	low.net_coord(axis)  =  ll; 
      
      if (ur != _self_coord.net_coord(axis))
	high.net_coord(axis) = ur; 
    }      
  }

  // make an axial topology
  new (topology) PAMI::Topology(&low, &high, &_self_coord, torus_link);  
  if((topology->size() == 1) /*&& (topology->index2Endpoint(0) == __global.mapping.task())*/)
    new (topology) PAMI::Topology();

  TRACE_FN_EXIT();
  return result;
}



#endif // __pami_target_socklinux__
#endif
