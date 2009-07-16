/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef __ll_cpp_topology_h__
#define __ll_cpp_topology_h__

////////////////////////////////////////////////////////////////////////
/// \file sysdep/Topology.h
///
/// \brief Coordinate/Address Topology Class
///
/// Some Basic Assertions:
///	* We never have a LL_COORD_TOPOLOGY unless we also have CM_TORUS_NETWORK
///
////////////////////////////////////////////////////////////////////////

#include "ll_topology.h"

#include "TopologyImpl.h"

namespace LL {

class Topology : public _TopologyImpl {

public:
	/// \brief default constructor (LL_EMPTY_TOPOLOGY)
	///
	Topology() : _TopologyImpl() {}

	/// \brief rectangular segment with torus (LL_COORD_TOPOLOGY)
	///
	/// Assumes no torus links if no 'tl' param.
	///
	/// \param[in] ll	lower-left coordinate
	/// \param[in] ur	upper-right coordinate
	/// \param[in] tl	optional, torus links flags
	///
	Topology(CM_Coord_t *ll, CM_Coord_t *ur,
					unsigned char *tl = NULL);

	/// \brief single rank constructor (LL_SINGLE_TOPOLOGY)
	///
	/// \param[in] rank	The rank
	///
	Topology(size_t rank);

	/// \brief rank range constructor (LL_RANGE_TOPOLOGY)
	///
	/// \param[in] rank0	first rank in range
	/// \param[in] rankn	last rank in range
	///
	Topology(size_t rank0, size_t rankn);

	/// \brief rank list constructor (LL_LIST_TOPOLOGY)
	///
	/// caller must not free ranks[]!
	///
	/// \param[in] ranks	array of ranks
	/// \param[in] nranks	size of array
	///
	/// \todo create destructor to free list, or establish rules
	///
	Topology(size_t *ranks, size_t nranks);

	/// \brief accessor for size of a Topology object
	/// \return	size of LL::Topology
	static const unsigned size_of() { return sizeof(Topology); }

	/// \brief number of ranks in topology
	/// \return	number of ranks
	size_t size();

	/// \brief type of topology
	/// \return	topology type
	LL_TopologyType_t type();

	/// \brief Nth rank in topology
	///
	/// \param[in] ix	Which rank to select
	/// \return	Nth rank or (size_t)-1 if does not exist
	///
	size_t index2Rank(size_t ix);

	/// \brief determine index of rank in topology
	///
	/// This is the inverse function to index2Rank(ix) above.
	///
	/// \param[in] rank	Which rank to get index for
	/// \return	index of rank (rank(ix) == rank) or (size_t)-1
	///
	size_t rank2Index(size_t rank);

	/// \brief return range
	///
	/// \param[out] first	Where to put first rank in range
	/// \param[out] last	Where to put last rank in range
	/// \return	CM_SUCCESS, or CM_UNIMPL if not a range topology
	///
	CM_Result rankRange(size_t *first, size_t *last);

	/// \brief return rank list
	///
	/// \param[out] list	pointer to list stored here
	/// \return	CM_SUCCESS, or CM_UNIMPL if not a list topology
	///
	CM_Result rankList(size_t **list);

	/// \brief return rectangular segment coordinates
	///
	/// Warning! This returns pointers to the Topology internals!
	/// This can result in corruption of a topology if mis-used.
	///
	/// \param[out] ll	lower-left coord pointer storage
	/// \param[out] ur	upper-right coord pointer storage
	/// \param[out] tl	optional, torus links flags
	/// \return	CM_SUCCESS, or CM_UNIMPL if not a coord topology
	///
	CM_Result rectSeg(CM_Coord_t **ll, CM_Coord_t **ur,
					unsigned char **tl = NULL);

	/// \brief return rectangular segment coordinates
	///
	/// This method copies data to callers buffers. It is safer
	/// as the caller cannot directly modify the topology.
	///
	/// \param[out] ll	lower-left coord pointer storage
	/// \param[out] ur	upper-right coord pointer storage
	/// \param[out] tl	optional, torus links flags
	/// \return	CM_SUCCESS, or CM_UNIMPL if not a coord topology
	///
	CM_Result rectSeg(CM_Coord_t *ll, CM_Coord_t *ur,
					unsigned char *tl = NULL);

	/// \brief does topology consist entirely of ranks local to eachother
	///
	/// \return boolean indicating locality of ranks
	///
	bool isLocal();

	/// \brief does topology consist entirely of ranks local to self
	///
	/// \return boolean indicating locality of ranks
	///
	bool isLocalToMe();

	/// \brief does topology consist entirely of ranks that do not share nodes
	///
	/// \return boolean indicating locality of ranks
	///
	bool isGlobal();

	/// \brief is topology a rectangular segment
	///
	/// \return	boolean indicating rect seg topo
	///
	bool isRectSeg();

	/// \brief extract Nth dimensions from coord topology
	///
	/// \param[in] n	Which dim to extract
	/// \param[out] c0	lower value for dim range
	/// \param[out] cn	upper value for dim range
	/// \param[out] tl	optional, torus link flag
	///
	void getNthDims(unsigned n, unsigned *c0, unsigned *cn,
					unsigned char *tl = NULL);

	/// \brief is rank in topology
	///
	/// \param[in] rank	Rank to test
	/// \return	boolean indicating rank is in topology
	///
	bool isRankMember(size_t rank);

	/// \brief is coordinate in topology
	///
	/// \param[in] c0	Coord to test
	/// \return	boolean indicating coord is a member of topology
	///
	bool isCoordMember(CM_Coord_t *c0);

	/// \brief create topology of ranks local to self
	///
	/// \param[out] _new	Where to build topology
	///
	void subTopologyLocalToMe(LL::Topology *_new);

	/// \brief create topology from all Nth ranks globally
	///
	/// \param[out] _new	Where to build topology
	/// \param[in] n	Which local rank to select on each node
	///
	void subTopologyNthGlobal(LL::Topology *_new, int n);

	/// \brief reduce dimensions of topology (cube -> plane, etc)
	///
	/// The 'fmt' param is a pattern indicating which dimensions
	/// to squash, and what coord to squash into. A dim in 'fmt'
	/// having "-1" will be preserved, while all others will be squashed
	/// into a dimension of size 1 having the value specified.
	///
	/// \param[out] _new	where to build new topology
	/// \param[in] fmt	how to reduce dimensions
	///
	void subTopologyReduceDims(LL::Topology *_new, CM_Coord_t *fmt);

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
	/// \param[in] max	size of caller-allocated array
	/// \param[out] ranks	array where rank list is placed
	/// \param[out] nranks	actual number of ranks put into array
	///
	void getRankList(size_t max, size_t *ranks, size_t *nranks);

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
	/// \return	'true' if topology was changed
	///
	bool analyzeTopology();

	/// \brief check if topology can be converted to type
	///
	/// Does not differentiate between invalid conversions and
	/// 'null' conversions (same type).
	///
	/// \param[in] new_type	Topology type to try and convert into
	/// \return	'true' if topology was changed
	///
	bool convertTopology(LL_TopologyType_t new_type);

	/// \brief produce the union of two topologies
	///
	/// produces: _new = this .U. other
	///
	/// \param[out] _new	New topology created there
	/// \param[in] other	The other topology
	///
	void unionTopology(Topology *_new, Topology *other);

	/// \brief produce the intersection of two topologies
	///
	/// produces: _new = this ./\. other
	///
	/// \param[out] _new	New topology created there
	/// \param[in] other	The other topology
	///
	void intersectTopology(Topology *_new, Topology *other);

	/// \brief produce the difference of two topologies
	///
	/// produces: _new = this .-. other
	///
	/// \param[out] _new	New topology created there
	/// \param[in] other	The other topology
	///
	void subtractTopology(Topology *_new, Topology *other);

}; /* class Topology */

}; /* namespace LL */

// This is ugly - but never needs to change
inline LL::Topology::Topology(CM_Coord_t *ll, CM_Coord_t *ur, unsigned char *tl) : LL::_TopologyImpl::_TopologyImpl(ll, ur, tl) {}
inline LL::Topology::Topology(size_t rank) : LL::_TopologyImpl::_TopologyImpl(rank) {}
inline LL::Topology::Topology(size_t rank0, size_t rankn) : LL::_TopologyImpl::_TopologyImpl(rank0, rankn) {}
inline LL::Topology::Topology(size_t *ranks, size_t nranks) : LL::_TopologyImpl::_TopologyImpl(ranks, nranks) {}
inline size_t LL::Topology::size() { return LL::_TopologyImpl::size(); }
inline LL_TopologyType_t LL::Topology::type() { return LL::_TopologyImpl::type(); }
inline size_t LL::Topology::index2Rank(size_t ix) { return LL::_TopologyImpl::index2Rank(ix); }
inline size_t LL::Topology::rank2Index(size_t rank) { return LL::_TopologyImpl::rank2Index(rank); }
inline CM_Result LL::Topology::rankRange(size_t *first, size_t *last) { return LL::_TopologyImpl::rankRange(first, last); }
inline CM_Result LL::Topology::rankList(size_t **list) { return LL::_TopologyImpl::rankList(list); }
inline CM_Result LL::Topology::rectSeg(CM_Coord_t **ll, CM_Coord_t **ur, unsigned char **tl) { return LL::_TopologyImpl::rectSeg(ll, ur, tl); }
inline CM_Result LL::Topology::rectSeg(CM_Coord_t *ll, CM_Coord_t *ur, unsigned char *tl) { return LL::_TopologyImpl::rectSeg(ll, ur, tl); }
inline bool LL::Topology::isLocal() { return LL::_TopologyImpl::isLocal(); }
inline bool LL::Topology::isLocalToMe() { return LL::_TopologyImpl::isLocalToMe(); }
inline bool LL::Topology::isGlobal() { return LL::_TopologyImpl::isGlobal(); }
inline bool LL::Topology::isRectSeg() { return LL::_TopologyImpl::isRectSeg(); }
inline void LL::Topology::getNthDims(unsigned n, unsigned *c0, unsigned *cn, unsigned char *tl) { LL::_TopologyImpl::getNthDims(n, c0, cn, tl); }
inline bool LL::Topology::isRankMember(size_t rank) { return LL::_TopologyImpl::isRankMember(rank); }
inline bool LL::Topology::isCoordMember(CM_Coord_t *c0) { return LL::_TopologyImpl::isCoordMember(c0); }
inline void LL::Topology::subTopologyLocalToMe(LL::Topology *_new) { LL::_TopologyImpl::subTopologyLocalToMe(_new); }
inline void LL::Topology::subTopologyNthGlobal(LL::Topology *_new, int n) { LL::_TopologyImpl::subTopologyNthGlobal(_new, n); }
inline void LL::Topology::subTopologyReduceDims(LL::Topology *_new, CM_Coord_t *fmt) { LL::_TopologyImpl::subTopologyReduceDims(_new, fmt); }
inline void LL::Topology::getRankList(size_t max, size_t *ranks, size_t *nranks) { LL::_TopologyImpl::getRankList(max, ranks, nranks); }
inline bool LL::Topology::analyzeTopology() { return LL::_TopologyImpl::analyzeTopology(); }
inline bool LL::Topology::convertTopology(LL_TopologyType_t new_type) { return LL::_TopologyImpl::convertTopology(new_type); }
inline void LL::Topology::unionTopology(Topology *_new, Topology *other) { LL::_TopologyImpl::unionTopology(_new, other); }
inline void LL::Topology::intersectTopology(Topology *_new, Topology *other) { LL::_TopologyImpl::intersectTopology(_new, other); }
inline void LL::Topology::subtractTopology(Topology *_new, Topology *other) { LL::_TopologyImpl::subtractTopology(_new, other); }

#endif /* __ll_cpp_topology_h__ */
