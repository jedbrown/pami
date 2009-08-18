/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/**
 * \file xmi_topology.h
 * \brief Topology interface.
 */
#ifndef __xmi_topology_h__
#define __xmi_topology_h__

#include "xmi_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /// \brief The various types a Topology can be
  typedef enum {
    XMI_EMPTY_TOPOLOGY = 0,  ///< topology represents no (zero) ranks
    XMI_SINGLE_TOPOLOGY,     ///< topology is for one rank
    XMI_RANGE_TOPOLOGY,      ///< topology is a simple range of ranks
    XMI_LIST_TOPOLOGY,       ///< topology is an unordered list of ranks
    XMI_COORD_TOPOLOGY,      ///< topology is a rectangular segment 
                              ///  represented by coordinates
    XMI_TOPOLOGY_COUNT
  } XMI_TopologyType_t;

  typedef XMIQuad XMI_Topology_t[2];

  /// \brief default constructor (XMI_EMPTY_TOPOLOGY)
  ///
  /// \param[out] topo	Opaque memory for topology
  ///
  void XMI_Topology_create(XMI_Topology_t *topo);

  /// \brief rectangular segment with torus (XMI_COORD_TOPOLOGY)
  ///
  /// Assumes no torus links if no 'tl' param.
  ///
  /// \param[out] topo	Opaque memory for topology
  /// \param[in] ll	lower-left coordinate
  /// \param[in] ur	upper-right coordinate
  /// \param[in] tl	optional, torus links flags
  ///
  void XMI_Topology_create_rect(XMI_Topology_t *topo,
  		XMI_Coord_t *ll, XMI_Coord_t *ur, unsigned char *tl);

  /// \brief single rank constructor (XMI_SINGLE_TOPOLOGY)
  ///
  /// \param[out] topo	Opaque memory for topology
  /// \param[in] rank	The rank
  ///
  void XMI_Topology_create_rank(XMI_Topology_t *topo, size_t rank);

  /// \brief rank range constructor (XMI_RANGE_TOPOLOGY)
  ///
  /// \param[out] topo	Opaque memory for topology
  /// \param[in] rank0	first rank in range
  /// \param[in] rankn	last rank in range
  ///
  void XMI_Topology_create_range(XMI_Topology_t *topo, size_t rank0, size_t rankn);

  /// \brief rank list constructor (XMI_LIST_TOPOLOGY)
  ///
  /// caller must not free ranks[]!
  ///
  /// \param[out] topo	Opaque memory for topology
  /// \param[in] ranks	array of ranks
  /// \param[in] nranks	size of array
  ///
  /// \todo create destructor to free list, or establish rules
  ///
  void XMI_Topology_create_list(XMI_Topology_t *topo, size_t *ranks, size_t nranks);

  /// \brief destructor
  ///
  /// For XMI_LIST_TOPOLOGY, would free the ranks list array... ?
  ///
  /// \param[out] topo	Opaque memory for topology
  ///
  void XMI_Topology_destroy(XMI_Topology_t *topo);

  /// \brief accessor for size of a Topology object
  ///
  /// \param[in] topo	Opaque memory for topology
  /// \return	size of XMI::Topology
  ///
  unsigned XMI_Topology_size_of(XMI_Topology_t *topo);

  /// \brief number of ranks in topology
  /// \param[in] topo	Opaque memory for topology
  /// \return	number of ranks
  size_t XMI_Topology_size(XMI_Topology_t *topo);

  /// \brief type of topology
  /// \param[out] topo	Opaque memory for topology
  /// \return	topology type
  XMI_TopologyType_t XMI_Topology_type(XMI_Topology_t *topo);

  /// \brief Nth rank in topology
  ///
  /// \param[in] topo	Opaque memory for topology
  /// \param[in] ix	Which rank to select
  /// \return	Nth rank or (size_t)-1 if does not exist
  ///
  size_t XMI_Topology_index2Rank(XMI_Topology_t *topo, size_t ix);

  /// \brief determine index of rank in topology
  ///
  /// This is the inverse function to rank(ix) above.
  ///
  /// \param[in] topo	Opaque memory for topology
  /// \param[in] rank	Which rank to get index for
  /// \return	index of rank (rank(ix) == rank) or (size_t)-1
  ///
  size_t XMI_Topology_rank2Index(XMI_Topology_t *topo, size_t rank);

  /// \brief return range
  ///
  /// \param[in] topo	Opaque memory for topology
  /// \param[out] first	Where to put first rank in range
  /// \param[out] last	Where to put last rank in range
  /// \return	XMI_SUCCESS, or XMI_UNIMPL if not a range topology
  ///
  XMI_Result XMI_Topology_rankRange(XMI_Topology_t *topo, size_t *first, size_t *last);

  /// \brief return rank list
  ///
  /// \param[in] topo	Opaque memory for topology
  /// \param[out] list	pointer to list stored here
  /// \return	XMI_SUCCESS, or XMI_UNIMPL if not a list topology
  ///
  XMI_Result XMI_Topology_rankList(XMI_Topology_t *topo, size_t **list);

  /// \brief return rectangular segment coordinates
  ///
  /// This method copies data to callers buffers. It is safer
  /// as the caller cannot directly modify the topology.
  ///
  /// \param[in] topo	Opaque memory for topology
  /// \param[out] ll	lower-left coord pointer storage
  /// \param[out] ur	upper-right coord pointer storage
  /// \param[out] tl	optional, torus links flags
  /// \return	XMI_SUCCESS, or XMI_UNIMPL if not a coord topology
  ///
  XMI_Result XMI_Topology_rectSeg(XMI_Topology_t *topo,
			XMI_Coord_t *ll, XMI_Coord_t *ur,
  			unsigned char *tl);

  /// \brief does topology consist entirely of ranks local to self
  ///
  /// \param[in] topo	Opaque memory for topology
  /// \return boolean indicating locality of ranks
  ///
  bool XMI_Topology_isLocalToMe(XMI_Topology_t *topo);

  /// \brief is topology a rectangular segment
  /// \param[in] topo	Opaque memory for topology
  /// \return	boolean indicating rect seg topo
  bool XMI_Topology_isRectSeg(XMI_Topology_t *topo);

  /// \brief extract Nth dimensions from coord topology
  ///
  /// \param[in] topo	Opaque memory for topology
  /// \param[in] n	Which dim to extract
  /// \param[out] c0	lower value for dim range
  /// \param[out] cn	upper value for dim range
  /// \param[out] tl	optional, torus link flag
  ///
  void XMI_Topology_getNthDims(XMI_Topology_t *topo, unsigned n,
			unsigned *c0, unsigned *cn, unsigned char *tl);

  /// \brief is rank in topology
  ///
  /// \param[in] topo	Opaque memory for topology
  /// \param[in] rank	Rank to test
  /// \return	boolean indicating rank is in topology
  ///
  bool XMI_Topology_isRankMember(XMI_Topology_t *topo, size_t rank);

  /// \brief is coordinate in topology
  ///
  /// \param[in] topo	Opaque memory for topology
  /// \param[in] c0	Coord to test
  /// \return	boolean indicating coord is a member of topology
  ///
  bool XMI_Topology_isCoordMember(XMI_Topology_t *topo, XMI_Coord_t *c0);

  /// \brief create topology of ranks local to self
  ///
  /// \param[out] _new	Where to build topology
  /// \param[in] topo	Opaque memory for topology
  ///
  void XMI_Topology_sub_LocalToMe(XMI_Topology_t *_new, XMI_Topology_t *topo);

  /// \brief create topology from all Nth ranks globally
  ///
  /// \param[out] _new	Where to build topology
  /// \param[in] topo	Opaque memory for topology
  /// \param[in] n	Which local rank to select on each node
  ///
  void XMI_Topology_sub_NthGlobal(XMI_Topology_t *_new, XMI_Topology_t *topo, int n);

  /// \brief reduce dimensions of topology (cube -> plane, etc)
  ///
  /// The 'fmt' param is a pattern indicating which dimensions
  /// to squash, and what coord to squash into. A dim in 'fmt'
  /// having "-1" will be preserved, while all others will be squashed
  /// into a dimension of size 1 having the value specified.
  ///
  /// \param[out] _new	where to build new topology
  /// \param[in] topo	Opaque memory for topology
  /// \param[in] fmt	how to reduce dimensions
  ///
  void XMI_Topology_sub_ReduceDims(XMI_Topology_t *_new, XMI_Topology_t *topo, XMI_Coord_t *fmt);

  /// \brief Return list of ranks representing contents of topology
  ///
  /// This always returns a list regardless of topology type.
  /// Caller must allocate space for list, and determine an
  /// appropriate size for that space. Note, there might be a
  /// number larger than 'max' returned in 'nranks', but there
  /// are never more than 'max' ranks put into the array.
  /// If the caller sees that 'nranks' exceeds 'max' then it
  /// should assume it did not get the whole list, and could
  /// allocate a larger array and try again.
  ///
  /// \param[in] topo	Opaque memory for topology
  /// \param[in] max	size of caller-allocated array
  /// \param[out] ranks	array where rank list is placed
  /// \param[out] nranks	actual number of ranks put into array
  ///
  void XMI_Topology_getRankList(XMI_Topology_t *topo, size_t max, size_t *ranks, size_t *nranks);

  /// \brief check if rank range or list can be converted to rectangle
  ///
  /// Since a rectangular segment is consider the optimal state, no
  /// other analysis is done. A XMI_SINGLE_TOPOLOGY cannot be optimized,
  /// either. Optimization levels:
  ///
  ///	XMI_SINGLE_TOPOLOGY (most)
  ///	XMI_COORD_TOPOLOGY
  ///	XMI_RANGE_TOPOLOGY
  ///	XMI_LIST_TOPOLOGY (least)
  ///
  /// \param[in,out] topo	Opaque memory for topology
  /// \return	'true' if topology was changed
  ///
  bool XMI_Topology_analyze(XMI_Topology_t *topo);

  /// \brief check if topology can be converted to type
  ///
  /// Does not differentiate between invalid conversions and
  /// 'null' conversions (same type).
  ///
  /// \param[in,out] topo	Opaque memory for topology
  /// \param[in] new_type	Topology type to try and convert into
  /// \return	'true' if topology was changed
  ///
  bool XMI_Topology_convert(XMI_Topology_t *topo, XMI_TopologyType_t new_type);

  /// \brief produce the intersection of two topologies
  ///
  /// produces: _new = this ./\. other
  ///
  /// \param[out] _new	New topology created there
  /// \param[in] topo	Opaque memory for topology
  /// \param[in] other	The other topology
  ///
  void XMI_Topology_intersect(XMI_Topology_t *_new, XMI_Topology_t *topo, XMI_Topology_t *other);

  /// \brief produce the difference of two topologies
  ///
  /// produces: _new = this .-. other
  ///
  /// \param[out] _new	New topology created there
  /// \param[in] topo	Opaque memory for topology
  /// \param[in] other	The other topology
  ///
  void XMI_Topology_subtract(XMI_Topology_t *_new, XMI_Topology_t *topo, XMI_Topology_t *other);

#ifdef __cplusplus
};
#endif

#endif /* __xmi_topology_h__ */
