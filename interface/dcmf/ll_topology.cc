/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/**
 * \file ll_topology.cc
 * \brief Topology C interface.
 */

#include "ll_topology.h"
#include "interface/Topology.h"

/// \brief default constructor (LL_EMPTY_TOPOLOGY)
///
/// \param[out] topo	Opaque memory for topology
///
extern "C" void LL_Topology_create(LL_Topology_t *topo) {
	new (topo) LL::Topology();
}

/// \brief rectangular segment with torus (LL_COORD_TOPOLOGY)
///
/// Assumes no torus links if no 'tl' param.
///
/// \param[out] topo	Opaque memory for topology
/// \param[in] ll	lower-left coordinate
/// \param[in] ur	upper-right coordinate
/// \param[in] tl	optional, torus links flags
///
extern "C" void LL_Topology_create_rect(LL_Topology_t *topo,
		CM_Coord_t *ll, CM_Coord_t *ur,
					unsigned char *tl) {
	new (topo) LL::Topology(ll, ur, tl);
}

/// \brief single rank constructor (LL_SINGLE_TOPOLOGY)
///
/// \param[out] topo	Opaque memory for topology
/// \param[in] rank	The rank
///
extern "C" void LL_Topology_create_rank(LL_Topology_t *topo, size_t rank) {
	new (topo) LL::Topology(rank);
}

/// \brief rank range constructor (LL_RANGE_TOPOLOGY)
///
/// \param[out] topo	Opaque memory for topology
/// \param[in] rank0	first rank in range
/// \param[in] rankn	last rank in range
///
extern "C" void LL_Topology_create_range(LL_Topology_t *topo, size_t rank0, size_t rankn) {
	new (topo) LL::Topology(rank0, rankn);
}

/// \brief rank list constructor (LL_LIST_TOPOLOGY)
///
/// caller must not free ranks[]!
///
/// \param[out] topo	Opaque memory for topology
/// \param[in] ranks	array of ranks
/// \param[in] nranks	size of array
///
/// \todo create destructor to free list, or establish rules
///
extern "C" void LL_Topology_create_list(LL_Topology_t *topo, size_t *ranks, size_t nranks) {
	new (topo) LL::Topology(ranks, nranks);
}

/// \brief destructor
///
/// For LL_LIST_TOPOLOGY, would free the ranks list array... ?
///
/// \param[out] topo	Opaque memory for topology
///
extern "C" void LL_Topology_destroy(LL_Topology_t *topo) {
	LL::Topology *topology = (LL::Topology *)topo;
	topology->~Topology();
}

/// \brief accessor for size of a Topology object
///
/// \param[in] topo	Opaque memory for topology
/// \return	size of LL::Topology
///
extern "C" unsigned LL_Topology_size_of(LL_Topology_t *topo) {
	LL::Topology *topology = (LL::Topology *)topo;
	return topology->size_of();
}

/// \brief number of ranks in topology
/// \param[in] topo	Opaque memory for topology
/// \return	number of ranks
extern "C" size_t LL_Topology_size(LL_Topology_t *topo) {
	LL::Topology *topology = (LL::Topology *)topo;
	return topology->size();
}

/// \brief type of topology
/// \param[out] topo	Opaque memory for topology
/// \return	topology type
extern "C" LL_TopologyType_t LL_Topology_type(LL_Topology_t *topo) {
	LL::Topology *topology = (LL::Topology *)topo;
	return topology->type();
}

/// \brief Nth rank in topology
///
/// \param[in] topo	Opaque memory for topology
/// \param[in] ix	Which rank to select
/// \return	Nth rank or (size_t)-1 if does not exist
///
extern "C" size_t LL_Topology_index2Rank(LL_Topology_t *topo, size_t ix) {
	LL::Topology *topology = (LL::Topology *)topo;
	return topology->index2Rank(ix);
}

/// \brief determine index of rank in topology
///
/// This is the inverse function to rank(ix) above.
///
/// \param[in] topo	Opaque memory for topology
/// \param[in] rank	Which rank to get index for
/// \return	index of rank (rank(ix) == rank) or (size_t)-1
///
extern "C" size_t LL_Topology_rank2Index(LL_Topology_t *topo, size_t rank) {
	LL::Topology *topology = (LL::Topology *)topo;
	return topology->rank2Index(rank);
}

/// \brief return range
///
/// \param[in] topo	Opaque memory for topology
/// \param[out] first	Where to put first rank in range
/// \param[out] last	Where to put last rank in range
/// \return	CM_SUCCESS, or CM_UNIMPL if not a range topology
///
extern "C" CM_Result LL_Topology_rankRange(LL_Topology_t *topo, size_t *first, size_t *last) {
	LL::Topology *topology = (LL::Topology *)topo;
	return topology->rankRange(first, last);
}

/// \brief return rank list
///
/// \param[in] topo	Opaque memory for topology
/// \param[out] list	pointer to list stored here
/// \return	CM_SUCCESS, or CM_UNIMPL if not a list topology
///
extern "C" CM_Result LL_Topology_rankList(LL_Topology_t *topo, size_t **list) {
	LL::Topology *topology = (LL::Topology *)topo;
	return topology->rankList(list);
}

/// \brief return rectangular segment coordinates
///
/// This method copies data to callers buffers. It is safer
/// as the caller cannot directly modify the topology.
///
/// \param[in] topo	Opaque memory for topology
/// \param[out] ll	lower-left coord pointer storage
/// \param[out] ur	upper-right coord pointer storage
/// \param[out] tl	optional, torus links flags
/// \return	CM_SUCCESS, or CM_UNIMPL if not a coord topology
///
extern "C" CM_Result LL_Topology_rectSeg(LL_Topology_t *topo,
			CM_Coord_t *ll, CM_Coord_t *ur,
			unsigned char *tl) {
	LL::Topology *topology = (LL::Topology *)topo;
	return topology->rectSeg(ll, ur, tl);
}

/// \brief does topology consist entirely of ranks local to self
///
/// \param[in] topo	Opaque memory for topology
/// \return boolean indicating locality of ranks
///
extern "C" bool LL_Topology_isLocalToMe(LL_Topology_t *topo) {
	LL::Topology *topology = (LL::Topology *)topo;
	return topology->isLocalToMe();
}

/// \brief is topology a rectangular segment
/// \param[in] topo	Opaque memory for topology
/// \return	boolean indicating rect seg topo
extern "C" bool LL_Topology_isRectSeg(LL_Topology_t *topo) {
	LL::Topology *topology = (LL::Topology *)topo;
	return topology->isRectSeg();
}

/// \brief extract Nth dimensions from coord topology
///
/// \param[in] topo	Opaque memory for topology
/// \param[in] n	Which dim to extract
/// \param[out] c0	lower value for dim range
/// \param[out] cn	upper value for dim range
/// \param[out] tl	optional, torus link flag
///
extern "C" void LL_Topology_getNthDims(LL_Topology_t *topo, unsigned n,
			unsigned *c0, unsigned *cn, unsigned char *tl) {
	LL::Topology *topology = (LL::Topology *)topo;
	topology->getNthDims(n, c0, cn, tl);
}

/// \brief is rank in topology
///
/// \param[in] topo	Opaque memory for topology
/// \param[in] rank	Rank to test
/// \return	boolean indicating rank is in topology
///
extern "C" bool LL_Topology_isRankMember(LL_Topology_t *topo, size_t rank) {
	LL::Topology *topology = (LL::Topology *)topo;
	return topology->isRankMember(rank);
}

/// \brief is coordinate in topology
///
/// \param[in] topo	Opaque memory for topology
/// \param[in] c0	Coord to test
/// \return	boolean indicating coord is a member of topology
///
extern "C" bool LL_Topology_isCoordMember(LL_Topology_t *topo, CM_Coord_t *c0) {
	LL::Topology *topology = (LL::Topology *)topo;
	return topology->isCoordMember(c0);
}

/// \brief create topology of ranks local to self
///
/// \param[out] _new	Where to build topology
/// \param[in] topo	Opaque memory for topology
///
extern "C" void LL_Topology_sub_LocalToMe(LL_Topology_t *_new, LL_Topology_t *topo) {
	LL::Topology *topology = (LL::Topology *)topo;
	LL::Topology *__new = (LL::Topology *)_new;
	topology->subTopologyLocalToMe(__new);
}

/// \brief create topology from all Nth ranks globally
///
/// \param[out] _new	Where to build topology
/// \param[in] topo	Opaque memory for topology
/// \param[in] n	Which local rank to select on each node
///
extern "C" void LL_Topology_sub_NthGlobal(LL_Topology_t *_new, LL_Topology_t *topo, int n) {
	LL::Topology *topology = (LL::Topology *)topo;
	LL::Topology *__new = (LL::Topology *)_new;
	topology->subTopologyNthGlobal(__new, n);
}

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
extern "C" void LL_Topology_sub_ReduceDims(LL_Topology_t *_new, LL_Topology_t *topo, CM_Coord_t *fmt) {
	LL::Topology *topology = (LL::Topology *)topo;
	LL::Topology *__new = (LL::Topology *)_new;
	topology->subTopologyReduceDims(__new, fmt);
}

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
extern "C" void LL_Topology_getRankList(LL_Topology_t *topo, size_t max, size_t *ranks, size_t *nranks) {
	LL::Topology *topology = (LL::Topology *)topo;
	topology->getRankList(max, ranks, nranks);
}

/// \brief check if rank range or list can be converted to rectangle
///
/// Since a rectangular segment is consider the optimal state, no
/// other analysis is done. A LL_SINGLE_TOPOLOGY cannot be optimized,
/// either. Optimization levels:
///
///	LL_SINGLE_TOPOLOGY (most)
///	LL_COORD_TOPOLOGY
///	LL_RANGE_TOPOLOGY
///	LL_LIST_TOPOLOGY (least)
///
/// \param[in,out] topo	Opaque memory for topology
/// \return	'true' if topology was changed
///
extern "C" bool LL_Topology_analyze(LL_Topology_t *topo) {
	LL::Topology *topology = (LL::Topology *)topo;
	return topology->analyzeTopology();
}

/// \brief check if topology can be converted to type
///
/// Does not differentiate between invalid conversions and
/// 'null' conversions (same type).
///
/// \param[in,out] topo	Opaque memory for topology
/// \param[in] new_type	Topology type to try and convert into
/// \return	'true' if topology was changed
///
extern "C" bool LL_Topology_convert(LL_Topology_t *topo, LL_TopologyType_t new_type) {
	LL::Topology *topology = (LL::Topology *)topo;
	return topology->convertTopology(new_type);
}

/// \brief produce the intersection of two topologies
///
/// produces: _new = this ./\. other
///
/// \param[out] _new	New topology created there
/// \param[in] topo	Opaque memory for topology
/// \param[in] other	The other topology
///
extern "C" void LL_Topology_intersect(LL_Topology_t *_new, LL_Topology_t *topo, LL_Topology_t *_other) {
	LL::Topology *topology = (LL::Topology *)topo;
	LL::Topology *__new = (LL::Topology *)_new;
	LL::Topology *other = (LL::Topology *)_other;
	topology->intersectTopology(__new, other);
}

/// \brief produce the difference of two topologies
///
/// produces: _new = this .-. other
///
/// \param[out] _new	New topology created there
/// \param[in] topo	Opaque memory for topology
/// \param[in] other	The other topology
///
extern "C" void LL_Topology_subtract(LL_Topology_t *_new, LL_Topology_t *topo, LL_Topology_t *_other) {
	LL::Topology *topology = (LL::Topology *)topo;
	LL::Topology *__new = (LL::Topology *)_new;
	LL::Topology *other = (LL::Topology *)_other;
	topology->subtractTopology(__new, other);
}
