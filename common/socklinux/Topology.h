/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef __common_socklinux_Topology_h__
#define __common_socklinux_Topology_h__

////////////////////////////////////////////////////////////////////////
/// \file common/socklinux/Topology.h
///
/// \brief Coordinate/Address Topology Class
///
/// Some Basic Assertions:
///	* We never have a XMI_COORD_TOPOLOGY unless we also have XMI_N_TORUS_NETWORK
///
////////////////////////////////////////////////////////////////////////

#include "common/TopologyInterface.h"
#include "Mapping.h"
#include "util/common.h"

namespace XMI {
	class Topology : public Interface::Topology<XMI::Topology> {
		static XMI::Mapping *mapping;
		
		/// \brief how a topology represents each type
		union topology_u {
			size_t _rank;	///< XMI_SINGLE_TOPOLOGY - the rank
			struct {	///< XMI_RANGE_TOPOLOGY
				size_t _first;	///< first rank in range
				size_t _last;	///< last rank in range
			} _rankrange;
			size_t *_ranklist; ///< XMI_LIST_TOPOLOGY - the rank array
			struct {	///< XMI_COORD_TOPOLOGY
				xmi_coord_t _llcorner; ///< lower-left coord
				xmi_coord_t _urcorner; ///< upper-right coord
				unsigned char _istorus[XMI_MAX_DIMS]; ///< existence of torus links
			} _rectseg;
		};
		/// \brief these defines simplify access to above union, and
		///        also isolate the code below from changes above.
		#define topo_rank	__topo._rank
		#define topo_first	__topo._rankrange._first
		#define topo_last	__topo._rankrange._last
		#define topo_range(n)	__topo._rankrange._first + n
		#define topo_ranklist	__topo._ranklist
		#define topo_list(n)	__topo._ranklist[n]
		#define topo_llcoord	__topo._rectseg._llcorner
		#define topo_urcoord	__topo._rectseg._urcorner
		#define topo_istorus	__topo._rectseg._istorus
		#define topo_lldim(n)	__topo._rectseg._llcorner.net_coord(n)
		#define topo_urdim(n)	__topo._rectseg._urcorner.net_coord(n)
		#define topo_hastorus(n)	__topo._rectseg._istorus[n]

		/// \brief are the two coords located on the same node
		///
		/// \param[in] c0	first coordinate
		/// \param[in] c1	second coordinate
		/// \return	boolean indicate locality of two coords
		///
		bool __isLocalCoord(const xmi_coord_t *c0,
					const xmi_coord_t *c1) {
                        // Asset nothing local for now
			return false;
		}

		/// \brief is the coordinate in this topology
		///
		/// \param[in] c0	coordinate
		/// \param[in] ndims	number of dimensions that are significant
		/// \return	boolean indicating if the coordinate lies in this
		///		topology, according to significant dimensions.
		///
		bool __isMemberCoord(const xmi_coord_t *c0, unsigned ndims) {
			return XMI_UNIMPL;
		}

		/// \brief create topology of only ranks local to self
		///
		/// \param[out] _new	place to construct new topology
		/// \return	nothing, but _new may be XMI_EMPTY_TOPOLOGY
		///
		void __subTopologyLocalToMe(XMI::Topology *_new) {
		      _new->__type = XMI_EMPTY_TOPOLOGY;
		      _new->__size = 0;
		}

		/// \brief create topology of all Nth ranks on all nodes
		///
		/// This implemention is only for XMI_COORD_TOPOLOGY.
		/// Essentially a horizontal slice of ranks. The resulting
		/// topology will have no two ranks sharing the same node.
		///
		/// \param[out] _new	space to construct new topology
		/// \param[in] n	which local rank to select
		/// \return	nothing, but _new may be XMI_EMPTY_TOPOLOGY
		///
		void __subTopologyNthGlobal(XMI::Topology *_new, int n) {
		      _new->__type = XMI_EMPTY_TOPOLOGY;
		      _new->__size = 0;
		}

		/// \brief Create new Rectangular Segment topology from subset of this
		///
		/// preserve coord if "fmt" coord is "-1",
		/// otherwise squash coord into 1-dim at "fmt" coord.
		///
		/// \param[out] _new	Storage for new topology
		/// \param[in] fmt	Coords to collapse/preserve
		///
		void __subTopologyReduceDims(XMI::Topology *_new, xmi_coord_t *fmt) {
		      _new->__type = XMI_EMPTY_TOPOLOGY;
		      _new->__size = 0;
		}

		/// \brief iterate to next coord, [0] is most-significant dim
		///
		/// \param[in,out] c0	coordinate to "increment"
		/// \return	boolean indicating overflow (end of count)
		///
		bool __nextCoord(xmi_coord_t *c0, unsigned ndims) {
			return false;
		}

		/// \brief initialize a coordinate range to a coord
		///
		/// \param[out] ll	lower-left corner of range
		/// \param[out] ur	upper-right corner of range
		/// \param[in] c0	initial coordinate
		///
		static void __initRange(xmi_coord_t *ll, xmi_coord_t *ur,
					xmi_coord_t *c0, unsigned ndims) {
			return;
		}

		/// \brief compute size of rectangular segment
		///
		/// \param[in] ll	lower-left corner of range
		/// \param[in] ur	upper-right corner of range
		/// \param[in] ndims	number of significant dimensions
		/// \return	size of rectangular segment
		///
		static size_t __sizeRange(xmi_coord_t *ll, xmi_coord_t *ur,
									unsigned ndims) {
			return 0;
		}

		/// \brief expand range to include coord
		///
		/// \param[in,out] ll	lower-left corner of range
		/// \param[in,out] ur	upper-right corner of range
		/// \param[in] c0	coord to be included
		///
		static void __bumpRange(xmi_coord_t *ll, xmi_coord_t *ur,
						xmi_coord_t *c0, unsigned ndims) {
			return;
		}

		/// \brief Analyze a Rank Range to see if it is a rectangular segment.
		///
		/// \return 'true' means "this" was altered!
		bool __analyzeCoordsRange() {
			return false;
		}

		/// \brief Analyze a Rank List to see if it is a rectangular segment.
		///
		/// Assumes that rank list contains no duplicates.
		///
		/// \return 'true' means "this" was altered!
		bool __analyzeCoordsList() {
			return false;	
		}

		/// \brief Analyze a Rank List to see if it is a range.
		///
		/// Assumes that rank list contains no duplicates.
		///
		/// \return 'true' means "this" was altered!
		bool __analyzeRangeList() {
			return false;
		}

		/// \brief compare coords for less-than
		///
		/// checks if 'c0' is entirely less-than 'c1'
		///
		/// \param[in] c0	Left-hand coord to test
		/// \param[in] c1	Right-hand coord to test
		/// \param[in] ndims	Number of significant dimensions
		/// \return	boolean indicating [c0] < [c1]
		///
		static bool __coordLT(xmi_coord_t *c0, xmi_coord_t *c1,
								unsigned ndims) {
			return false;
		}

		/// \brief return MAX of two coords
		///
		/// _new = MAX(c0, c1)
		///
		/// \param[in] _new	Result coord
		/// \param[in] c0	First coord
		/// \param[in] c1	Second coord
		/// \param[in] ndims	Number of dimension to use
		///
		/// Leaves un-used dimensions uninitialized
		///
		static void __coordMAX(xmi_coord_t *_new,
				xmi_coord_t *c0, xmi_coord_t *c1,
								unsigned ndims) {
			return;
		}

		/// \brief return MIN of two coords
		///
		/// _new = MIN(c0, c1)
		///
		/// \param[in] _new	Result coord
		/// \param[in] c0	First coord
		/// \param[in] c1	Second coord
		/// \param[in] ndims	Number of dimension to use
		///
		/// Leaves un-used dimensions uninitialized
		///
		static void __coordMIN(xmi_coord_t *_new,
				xmi_coord_t *c0, xmi_coord_t *c1,
								unsigned ndims) {
			return;
		}

	public:
		static void static_init(XMI::Mapping *map) {
			mapping = map;
		}

		/// \brief default constructor (XMI_EMPTY_TOPOLOGY)
		///
		Topology() {
			__type = XMI_EMPTY_TOPOLOGY;
			__size = 0;
		}

		/// \brief rectangular segment with torus (XMI_COORD_TOPOLOGY)
		///
		/// Assumes no torus links if no 'tl' param.
		///
		/// \param[in] ll	lower-left coordinate
		/// \param[in] ur	upper-right coordinate
		/// \param[in] tl	optional, torus links flags
		///
		Topology(xmi_coord_t *ll, xmi_coord_t *ur,
						unsigned char *tl = NULL) {
                        // Should probably abort
		}

		/// \brief single rank constructor (XMI_SINGLE_TOPOLOGY)
		///
		/// \param[in] rank	The rank
		///
		Topology(size_t rank) {
			__type = XMI_SINGLE_TOPOLOGY;
			__size = 1;
			__topo._rank = rank;
		}

		/// \brief rank range constructor (XMI_RANGE_TOPOLOGY)
		///
		/// \param[in] rank0	first rank in range
		/// \param[in] rankn	last rank in range
		///
		Topology(size_t rank0, size_t rankn) {
			__type = XMI_RANGE_TOPOLOGY;
			__size = rankn - rank0 + 1;
			__topo._rankrange._first = rank0;
			__topo._rankrange._last = rankn;
			// should we do this automatically, or let caller?
			// (void)__analyzeCoordsRange();
		}

		/// \brief rank list constructor (XMI_LIST_TOPOLOGY)
		///
		/// caller must not free ranks[]!
		///
		/// \param[in] ranks	array of ranks
		/// \param[in] nranks	size of array
		///
		/// \todo create destructor to free list, or establish rules
		///
		Topology(size_t *ranks, size_t nranks) {
			__type = XMI_LIST_TOPOLOGY;
			__size = nranks;
			topo_ranklist = ranks;
			// should we do this automatically, or let caller?
			// (void)__analyzeCoordsList();
		}

		/// \brief accessor for size of a Topology object
		/// \return	size of XMI::Topology
		static const unsigned size_of_impl() { return sizeof(Topology); }

		/// \brief number of ranks in topology
		/// \return	number of ranks
		size_t size_impl() { return __size; }

		/// \brief type of topology
		/// \return	topology type
		xmi_topology_type_t type_impl() { return __type; }

		/// \brief Nth rank in topology
		///
		/// \param[in] ix	Which rank to select
		/// \return	Nth rank or (size_t)-1 if does not exist
		///
		size_t index2Rank_impl(size_t ix) {
			size_t rank = 0;
			if (ix < __size) switch (__type) {
			case XMI_SINGLE_TOPOLOGY:
				return topo_rank;
				break;
			case XMI_RANGE_TOPOLOGY:
				return topo_range(ix);
				break;
			case XMI_LIST_TOPOLOGY:
				return topo_list(ix);
				break;
			case XMI_COORD_TOPOLOGY:
			        return XMI_UNIMPL;
				break;
			case XMI_EMPTY_TOPOLOGY:
			default:
				break;
			}
			return (size_t)-1;
		}

		/// \brief determine index of rank in topology
		///
		/// This is the inverse function to index2Rank(ix) above.
		///
		/// \param[in] rank	Which rank to get index for
		/// \return	index of rank (rank(ix) == rank) or (size_t)-1
		///
		size_t rank2Index_impl(size_t rank) {
			size_t x, ix, nn;
			xmi_coord_t c0;
			xmi_result_t rc;
			switch (__type) {
			case XMI_SINGLE_TOPOLOGY:
				if (topo_rank == rank) {
					return 0;
				}
				break;
			case XMI_RANGE_TOPOLOGY:
				if (rank >= topo_first && rank <= topo_last) {
					return rank - topo_first;
				}
				break;
			case XMI_LIST_TOPOLOGY:
				for (x = 0; x < __size; ++x) {
					if (rank == topo_list(x)) {
						return x;
					}
				}
				break;
			case XMI_COORD_TOPOLOGY:
			        return XMI_UNIMPL;
				break;
			case XMI_EMPTY_TOPOLOGY:
			default:
				break;
			}
			return (size_t)-1;
		}

		/// \brief return range
		///
		/// \param[out] first	Where to put first rank in range
		/// \param[out] last	Where to put last rank in range
		/// \return	XMI_SUCCESS, or XMI_UNIMPL if not a range topology
		///
		xmi_result_t rankRange_impl(size_t *first, size_t *last) {
			if (__type != XMI_RANGE_TOPOLOGY) {
				return XMI_UNIMPL;
			}
			*first = topo_first;
			*last = topo_last;
			return XMI_SUCCESS;
		}

		/// \brief return rank list
		///
		/// \param[out] list	pointer to list stored here
		/// \return	XMI_SUCCESS, or XMI_UNIMPL if not a list topology
		///
		xmi_result_t rankList_impl(size_t **list) {
			if (__type != XMI_LIST_TOPOLOGY) {
				return XMI_UNIMPL;
			}
			*list = topo_ranklist;
			return XMI_SUCCESS;
		}

		/// \brief return rectangular segment coordinates
		///
		/// Warning! This returns pointers to the Topology internals!
		/// This can result in corruption of a topology if mis-used.
		///
		/// \param[out] ll	lower-left coord pointer storage
		/// \param[out] ur	upper-right coord pointer storage
		/// \param[out] tl	optional, torus links flags
		/// \return	XMI_SUCCESS, or XMI_UNIMPL if not a coord topology
		///
		xmi_result_t rectSeg_impl(xmi_coord_t **ll, xmi_coord_t **ur,
						unsigned char **tl = NULL) {
			return XMI_UNIMPL;
		}

		/// \brief return rectangular segment coordinates
		///
		/// This method copies data to callers buffers. It is safer
		/// as the caller cannot directly modify the topology.
		///
		/// \param[out] ll	lower-left coord pointer storage
		/// \param[out] ur	upper-right coord pointer storage
		/// \param[out] tl	optional, torus links flags
		/// \return	XMI_SUCCESS, or XMI_UNIMPL if not a coord topology
		///
		xmi_result_t rectSeg_impl(xmi_coord_t *ll, xmi_coord_t *ur,
						unsigned char *tl = NULL) {
			return XMI_UNIMPL;
		}

		/// \brief does topology consist entirely of ranks local to eachother
		///
		/// \return boolean indicating locality of ranks
		///
		bool isLocal_impl() {
                        return false; 
		}

		/// \brief does topology consist entirely of ranks local to self
		///
		/// \return boolean indicating locality of ranks
		///
		bool isLocalToMe_impl() {
			return false;
		}

		/// \brief does topology consist entirely of ranks that do not share nodes
		///
		/// \return boolean indicating locality of ranks
		///
		bool isGlobal_impl() {
			// is this too difficult/expensive? is it needed?
			// Note, this is NOT simply "!isLocal()"...
			XMI_abortf("Topology::isGlobal not implemented\n");
		}

		/// \brief is topology a rectangular segment
		/// \return	boolean indicating rect seg topo
		bool isRectSeg_impl() { return (__type == XMI_COORD_TOPOLOGY); }

		/// \brief extract Nth dimensions from coord topology
		///
		/// \param[in] n	Which dim to extract
		/// \param[out] c0	lower value for dim range
		/// \param[out] cn	upper value for dim range
		/// \param[out] tl	optional, torus link flag
		///
		void getNthDims_impl(unsigned n, unsigned *c0, unsigned *cn,
						unsigned char *tl = NULL) {
		}

		/// \brief is rank in topology
		///
		/// \param[in] rank	Rank to test
		/// \return	boolean indicating rank is in topology
		///
		bool isRankMember_impl(size_t rank) {
			xmi_result_t rc;
			if (unlikely(__type == XMI_COORD_TOPOLOGY)) {
				return false;
			} else {
				if (__type == XMI_SINGLE_TOPOLOGY) {
					return (rank == __topo._rank);
				} else if (__type == XMI_RANGE_TOPOLOGY) {
					return (rank >= topo_first && rank <= topo_last);
				} else if (__type == XMI_LIST_TOPOLOGY) {
					unsigned i;
					for (i = 0; i < __size; ++i) {
						if (rank == topo_list(i)) {
							return true;
						}
					}
					return false;
				}
			}
			// i.e. XMI_EMPTY_TOPOLOGY
			return false;
		}

		/// \brief is coordinate in topology
		///
		/// \param[in] c0	Coord to test
		/// \return	boolean indicating coord is a member of topology
		///
		bool isCoordMember_impl(xmi_coord_t *c0) {
				return false;
		}

		/// \brief create topology of ranks local to self
		///
		/// \param[out] _new	Where to build topology
		///
		void subTopologyLocalToMe_impl(XMI::Topology *_new) {
				_new->__type = XMI_EMPTY_TOPOLOGY;
				_new->__size = 0;
		}

		/// \brief create topology from all Nth ranks globally
		///
		/// \param[out] _new	Where to build topology
		/// \param[in] n	Which local rank to select on each node
		///
		void subTopologyNthGlobal_impl(XMI::Topology *_new, int n) {
				_new->__type = XMI_EMPTY_TOPOLOGY;
				_new->__size = 0;
		}

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
		void subTopologyReduceDims_impl(XMI::Topology *_new, xmi_coord_t *fmt) {
			if (likely(__type == XMI_COORD_TOPOLOGY)) {
				__subTopologyReduceDims(_new, fmt);
			} else {
				// the really hard way... impractical?
				_new->__type = XMI_EMPTY_TOPOLOGY;
				_new->__size = 0;
			}
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
		/// \param[in] max	size of caller-allocated array
		/// \param[out] ranks	array where rank list is placed
		/// \param[out] nranks	actual number of ranks put into array
		///
		void getRankList_impl(size_t max, size_t *ranks, size_t *nranks) {
			xmi_result_t rc;
			*nranks = __size; // might exceed "max" - caller detects error.
			XMI_assert_debugf(max != 0, "getRankList called with no array space\n");
			if (likely(__type == XMI_LIST_TOPOLOGY)) {
				unsigned x;
				for (x = 0; x < __size && x < max; ++x) {
					ranks[x] = topo_list(x);
				}
			} else if (__type == XMI_SINGLE_TOPOLOGY) {
				ranks[0] = __topo._rank;
			} else if (__type == XMI_RANGE_TOPOLOGY) {
				size_t r;
				unsigned x;
				for (x = 0, r = topo_first; r <= topo_last && x < max; ++x, ++r) {
					ranks[x] = r;
				}
			}
		}

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
		/// \return	'true' if topology was changed
		///
		bool analyzeTopology_impl() {
			return false;
		}

		/// \brief check if topology can be converted to type
		///
		/// Does not differentiate between invalid conversions and
		/// 'null' conversions (same type).
		///
		/// \param[in] new_type	Topology type to try and convert into
		/// \return	'true' if topology was changed
		///
		bool convertTopology_impl(xmi_topology_type_t new_type) {
			xmi_result_t rc;
			xmi_coord_t c0;
			size_t rank = 0;
			size_t *rl, *rp;
			size_t min, max;
			switch (__type) {
			case XMI_SINGLE_TOPOLOGY:
				switch (new_type) {
				case XMI_COORD_TOPOLOGY:
					return false;
					break;
				case XMI_RANGE_TOPOLOGY:
					__type = XMI_RANGE_TOPOLOGY;
					topo_first =
					topo_last = topo_rank;
					return true;
					break;
				case XMI_LIST_TOPOLOGY:
					rl = (size_t *)malloc(sizeof(*rl));
					__type = XMI_LIST_TOPOLOGY;
					*rl = topo_rank;
					topo_ranklist = rl;
					return true;
					break;
				case XMI_SINGLE_TOPOLOGY:
					break;
				default:
					break;
				}
				break;
			case XMI_RANGE_TOPOLOGY:
				switch (new_type) {
				case XMI_COORD_TOPOLOGY:
					return false;
					break;
				case XMI_SINGLE_TOPOLOGY:
					if (__size == 1) {
						__type = XMI_SINGLE_TOPOLOGY;
						topo_rank = topo_first;
						return true;
					}
					break;
				case XMI_LIST_TOPOLOGY:
					rl = (size_t *)malloc(__size * sizeof(*rl));
					rp = rl;
					__type = XMI_LIST_TOPOLOGY;
					size_t r;
					for (r = topo_first; r <= topo_last; ++r) {
						*rp++ = r;
					}
					topo_ranklist = rl;
					return true;
					break;
				case XMI_RANGE_TOPOLOGY:
					break;
				default:
					break;
				}
				break;
			case XMI_LIST_TOPOLOGY:
				switch (new_type) {
				case XMI_COORD_TOPOLOGY:
					return __analyzeCoordsList();
					break;
				case XMI_SINGLE_TOPOLOGY:
					if (__size == 1) {
						__type = XMI_SINGLE_TOPOLOGY;
						topo_rank = topo_list(0);
						return true;
					}
					break;
				case XMI_LIST_TOPOLOGY:
					break;
				case XMI_RANGE_TOPOLOGY:
					return __analyzeRangeList();
					break;
				default:
					break;
				}
				break;
			case XMI_COORD_TOPOLOGY:
                        return false;
			break;
			default:
				break;
			}
			return false; // no change - possible error
		}

	/// \brief produce the union of two topologies
	///
	/// produces: _new = this .U. other
	///
	/// \param[out] _new	New topology created there
	/// \param[in] other	The other topology
	///
	void unionTopology_impl(Topology *_new, Topology *other) {
		// for now, assume this isn't used/needed.
		XMI_abortf("Topology::unionTopology not implemented\n");

	}

	/// \brief produce the intersection of two topologies
	///
	/// produces: _new = this ./\. other
	///
	/// \param[out] _new	New topology created there
	/// \param[in] other	The other topology
	///
	void intersectTopology_impl(Topology *_new, Topology *other) {
		if (likely(__type == other->__type)) {
			size_t s;
			size_t i, j, k;
			size_t *rl;
			switch (__type) {
			case XMI_COORD_TOPOLOGY:
	                 	_new->__type = XMI_EMPTY_TOPOLOGY;
		                _new->__size = 0;

				return;
				break;
			case XMI_SINGLE_TOPOLOGY:
				if (other->topo_rank == topo_rank) {
					_new->__type = XMI_SINGLE_TOPOLOGY;
					_new->__size = 1;
					_new->topo_rank = topo_rank;
					return;
				}
				break;
			case XMI_RANGE_TOPOLOGY:
				// This always results in a range...
				if (topo_last < other->topo_first ||
						other->topo_last < topo_first) {
					// disjoint ranges
					break;
				}
				_new->__type = XMI_RANGE_TOPOLOGY;
				_new->topo_first = (topo_first > other->topo_first ?
						topo_first : other->topo_first);
				_new->topo_last = (topo_last < other->topo_last ?
						topo_last : other->topo_last);
				_new->__size = _new->topo_last - _new->topo_first + 1;
				return;
				break;
			case XMI_LIST_TOPOLOGY:
				/// \todo keep this from being O(n^2)
				// guess at size: smallest topology.
				s = (__size < other->__size ?
						__size : other->__size);
				rl = (size_t *)malloc(s * sizeof(*rl));
				k = 0;
				for (i = 0; i < __size; ++i) {
					for (j = 0; j < other->__size; ++j) {
						if (topo_list(i) == other->topo_list(j)) {
							rl[k++] = topo_list(i);
						}
					}
				}
				if (k) {
					_new->__type = XMI_LIST_TOPOLOGY;
					_new->__size = k;
					_new->topo_ranklist = rl;
					return;
				}
				free(rl);
				break;
			case XMI_EMPTY_TOPOLOGY:
			default:
				break;
			}
		} else if (__type == XMI_SINGLE_TOPOLOGY) {
			if (other->isRankMember(topo_rank)) {
				_new->__type = XMI_SINGLE_TOPOLOGY;
				_new->__size = 1;
				_new->topo_rank = topo_rank;
				return;
			}
		} else if (other->__type == XMI_SINGLE_TOPOLOGY) {
			if (isRankMember(other->topo_rank)) {
				_new->__type = XMI_SINGLE_TOPOLOGY;
				_new->__size = 1;
				_new->topo_rank = other->topo_rank;
				return;
			}
		} else if (__type != XMI_EMPTY_TOPOLOGY &&
				other->__type != XMI_EMPTY_TOPOLOGY) {
			// more complicated scenarios - TBD
			switch (__type) {
			case XMI_COORD_TOPOLOGY:
				switch (other->__type) {
				case XMI_LIST_TOPOLOGY:
					break;
				case XMI_RANGE_TOPOLOGY:
					break;
				case XMI_SINGLE_TOPOLOGY: // already handled
				case XMI_COORD_TOPOLOGY:  // already handled
				default:
					break;
				}
			case XMI_LIST_TOPOLOGY:
				switch (other->__type) {
				case XMI_COORD_TOPOLOGY:
					break;
				case XMI_RANGE_TOPOLOGY:
					break;
				case XMI_SINGLE_TOPOLOGY: // already handled
				case XMI_LIST_TOPOLOGY:   // already handled
				default:
					break;
				}
			case XMI_RANGE_TOPOLOGY:
				switch (other->__type) {
				case XMI_COORD_TOPOLOGY:
					break;
				case XMI_LIST_TOPOLOGY:
					break;
				case XMI_SINGLE_TOPOLOGY: // already handled
				case XMI_RANGE_TOPOLOGY:  // already handled
				default:
					break;
				}
			case XMI_SINGLE_TOPOLOGY: // already handled
			default:
				break;
			}
		}
		_new->__type = XMI_EMPTY_TOPOLOGY;
		_new->__size = 0;
	}

	/// \brief produce the difference of two topologies
	///
	/// produces: _new = this .-. other
	///
	/// \param[out] _new	New topology created there
	/// \param[in] other	The other topology
	///
	void subtractTopology_impl(Topology *_new, Topology *other) {
		xmi_result_t rc;
		xmi_coord_t ll, ur, c0;
		size_t rank = 0;
		size_t min = 0, max = 0;
		size_t s;
		size_t i, j, k;
		size_t *rl;
		unsigned flag;
		if (likely(__type == other->__type)) {
			switch (__type) {
			case XMI_COORD_TOPOLOGY:

				return;
				break;
			case XMI_SINGLE_TOPOLOGY:
				if (other->topo_rank != topo_rank) {
					*_new = *this;
					return;
				}
				break;
			case XMI_RANGE_TOPOLOGY:
				// This mostly results in a range...
				flag =	((topo_first >= other->topo_first) << 3) |
					((topo_first <= other->topo_last) << 2) |
					((topo_last >= other->topo_first) << 1) |
					((topo_last <= other->topo_last) << 0);
				switch (flag) {

				default:
					// disjoint ranges, nothing removed
					*_new = *this;
					return;
					break;
				}
				break;
			case XMI_LIST_TOPOLOGY:
				/// \todo keep this from being O(n^2)
				s = __size;
				rl = (size_t *)malloc(s * sizeof(*rl));
				k = 0;
				for (i = 0; i < s; ++i) {
					if (other->isRankMember(topo_list(i))) {
						continue;
					}
					// keep it...
					rl[k++] = topo_list(i);
				}
				if (k == 0) {
					free(rl);
					_new->__type = XMI_EMPTY_TOPOLOGY;
					_new->__size = 0;
					break;
				}
				_new->__type = XMI_LIST_TOPOLOGY;
				_new->__size = k;
				_new->topo_ranklist = rl;
				return;
				break;
			case XMI_EMPTY_TOPOLOGY:
				break;
			default:
				break;
			}
		} else if (__type == XMI_SINGLE_TOPOLOGY) {
			if (other->isRankMember(topo_rank)) {
				_new->__type = XMI_SINGLE_TOPOLOGY;
				_new->__size = 1;
				_new->topo_rank = topo_rank;
				return;
			}
		} else if (other->__type == XMI_SINGLE_TOPOLOGY) {
			switch (__type) {
			case XMI_COORD_TOPOLOGY:
				break;
			case XMI_RANGE_TOPOLOGY:
				if (isRankMember(other->topo_rank)) {
					// convert range into list...
					s = __size;
					rl = (size_t *)malloc(s * sizeof(*rl));
					k = 0;
					for (i = topo_first; i < other->topo_rank; ++i) {
						rl[k++] = i;
					}
					for (++i; i <= topo_last; ++i) {
						rl[k++] = i;
					}
					if (k == 0) { // probably never
						free(rl);
						break;
					} else if (k == 1) {
						_new->__type = XMI_SINGLE_TOPOLOGY;
						_new->__size = 1;
						_new->topo_rank = rl[0];
						free(rl);
						return;
					}
					_new->__type = XMI_LIST_TOPOLOGY;
					_new->__size = k;
					_new->topo_ranklist = rl;
					return;
				} else {
					*_new = *this;
					return;
				}
				break;
			case XMI_LIST_TOPOLOGY:
				s = __size;
				rl = (size_t *)malloc(s * sizeof(*rl));
				k = 0;
				for (i = 0; i < s; ++i) {
					if (topo_list(i) != other->topo_rank) {
						rl[k++] = topo_list(i);
					}
				}
				if (k == 0) { // probably never
					free(rl);
					break;
				} else if (k == 1) {
					_new->__type = XMI_SINGLE_TOPOLOGY;
					_new->__size = 1;
					_new->topo_rank = rl[0];
					free(rl);
					return;
				}
				_new->__type = XMI_LIST_TOPOLOGY;
				_new->__size = k;
				_new->topo_ranklist = rl;
				return;
				break;
			case XMI_EMPTY_TOPOLOGY:
				break;
			case XMI_SINGLE_TOPOLOGY: // cannot happen - already checked
			default:
				break;
			}
		} else if (__type != XMI_EMPTY_TOPOLOGY &&
				other->__type != XMI_EMPTY_TOPOLOGY) {
			// more complicated scenarios - TBD
			switch (__type) {
			case XMI_COORD_TOPOLOGY:
				switch (other->__type) {
				case XMI_LIST_TOPOLOGY:
					break;
				case XMI_RANGE_TOPOLOGY:
					break;
				case XMI_SINGLE_TOPOLOGY: // already handled
				case XMI_COORD_TOPOLOGY:  // already handled
				default:
					break;
				}
			case XMI_LIST_TOPOLOGY:
				switch (other->__type) {
				case XMI_COORD_TOPOLOGY:
					break;
				case XMI_RANGE_TOPOLOGY:
					break;
				case XMI_SINGLE_TOPOLOGY: // already handled
				case XMI_LIST_TOPOLOGY:   // already handled
				default:
					break;
				}
			case XMI_RANGE_TOPOLOGY:
				switch (other->__type) {
				case XMI_COORD_TOPOLOGY:
					break;
				case XMI_LIST_TOPOLOGY:
					break;
				case XMI_SINGLE_TOPOLOGY: // already handled
				case XMI_RANGE_TOPOLOGY:  // already handled
				default:
					break;
				}
			case XMI_SINGLE_TOPOLOGY: // already handled
			default:
				break;
			}
		}
		_new->__type = XMI_EMPTY_TOPOLOGY;
		_new->__size = 0;
	}

	private:
		size_t	__size;		///< number of ranks in this topology
		xmi_topology_type_t __type;	///< type of topology this is
		union topology_u __topo;///< topoloy info

	}; // class Topology

}; // namespace XMI

#endif // __common_socklinux_Topology_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
