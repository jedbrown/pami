/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef __common_mpi_Topology_h__
#define __common_mpi_Topology_h__

////////////////////////////////////////////////////////////////////////
/// \file common/mpi/Topology.h
///
/// \brief Coordinate/Address Topology Class
///
/// Some Basic Assertions:
///	* We never have a XMI_COORD_TOPOLOGY unless we also have XMI_N_TORUS_NETWORK
///
////////////////////////////////////////////////////////////////////////

#include "common/TopologyInterface.h"
#include "util/common.h"
#include "Mapping.h"

#define net_coord(n)	u.n_torus.coords[n]

// returns pointer to xmi_coord_t
#define MY_COORDS	&my_coords

#define RANK2COORDS(rank, coords)	\
	({ XMI_SUCCESS; })
//	mapping->task2network(rank, coords, XMI_N_TORUS_NETWORK)

static xmi_network __dummy_net; // never really used

#define COORDS2RANK(coords, rank)	\
	({ assert(0); XMI_SUCCESS; })
//	mapping->network2task(coords, rank, &__dummy_net)

#define IS_LOCAL_PEER(rank)	\
	true
//	assume all are local for now?
//	mapping->isPeer(mapping->task(), rank)

#define XMI_TOPOLOGY_CLASS XMI::Topology

namespace XMI {
	class Topology : public Interface::Topology<XMI::Topology> {
		static XMI::Mapping *mapping;
		static xmi_coord_t my_coords;

		#define b0000	0
		#define b0001	1
		#define b0010	2
		#define b0011	3
		#define b0100	4
		#define b0101	5
		#define b0110	6
		#define b0111	7
		#define b1000	8
		#define b1001	9
		#define b1010	10
		#define b1011	11
		#define b1100	12
		#define b1101	13
		#define b1110	14
		#define b1111	15

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
			unsigned x;
			for (x = 0; x < mapping->torusDims(); ++x) {
				if (c0->net_coord(x) != c1->net_coord(x)) {
					return false;
				}
			}
			return true;
		}

		/// \brief is the coordinate in this topology
		///
		/// \param[in] c0	coordinate
		/// \param[in] ndims	number of dimensions that are significant
		/// \return	boolean indicating if the coordinate lies in this
		///		topology, according to significant dimensions.
		///
		bool __isMemberCoord(const xmi_coord_t *c0, unsigned ndims) {
			unsigned x;
			for (x = 0; x < ndims; ++x) {
				if (c0->net_coord(x) < topo_lldim(x) ||
						c0->net_coord(x) > topo_urdim(x)) {
					return false;
				}
			}
			return true;
		}

		/// \brief create topology of only ranks local to self
		///
		/// \param[out] _new	place to construct new topology
		/// \return	nothing, but _new may be XMI_EMPTY_TOPOLOGY
		///
		void __subTopologyLocalToMe(XMI::Topology *_new) {
			if (likely(__type == XMI_COORD_TOPOLOGY)) {
				if (__isMemberCoord(MY_COORDS,
						mapping->torusDims())) {
					_new->__type = XMI_COORD_TOPOLOGY;
					_new->topo_llcoord = *MY_COORDS;
					_new->topo_urcoord = *MY_COORDS;
					// might be able to get better torus info from mapping
					memset(_new->topo_istorus, 0, mapping->torusDims());
					size_t s = 1;
					unsigned x;
					for (x = mapping->torusDims(); x < mapping->globalDims(); ++x) {
						_new->topo_lldim(x) = topo_lldim(x);
						_new->topo_urdim(x) = topo_urdim(x);
						_new->topo_hastorus(x) = topo_hastorus(x);
						s *= (topo_urdim(x) - topo_lldim(x) + 1);
					}
					_new->__size = s;
					return;
				}
			} else {
				// the hard way...
				if (__type == XMI_SINGLE_TOPOLOGY) {
					if (IS_LOCAL_PEER(topo_rank)) {
						*_new = *this;
						return;
					}
				} else {
					size_t s = 0, z;
					mapping->nodePeers(z);
					size_t *rl = (size_t *)malloc(z * sizeof(*rl));
					size_t *rp = rl;
					if (__type == XMI_RANGE_TOPOLOGY) {
						size_t r;
						for (r = topo_first; r <= topo_last; ++r) {
							if (IS_LOCAL_PEER(r)) {
								++s;
								*rp++ = r;
							}
						}
					} else if (__type == XMI_LIST_TOPOLOGY) {
						unsigned i;
						for (i = 0; i < __size; ++i) {
							if (IS_LOCAL_PEER(topo_list(i))) {
								++s;
								*rp++ = topo_list(i);
							}
						}
					}
					if (s) {
						// convert "rl" to range if possible.
						// ...or even coords?
						_new->__type = XMI_LIST_TOPOLOGY;
						_new->__size = s;
						_new->topo_ranklist = rl;
						return;
					} else {
						free(rl);
					}
				}
			}
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
			// What order do we sequence multiple "local" dimensions???
			*_new = *this;
			size_t s = __sizeRange(&topo_llcoord,
						&topo_urcoord,
						mapping->torusDims());
			unsigned x;
			for (x = mapping->torusDims(); x < mapping->globalDims(); ++x) {
				unsigned ll = topo_lldim(x);
				unsigned ur = topo_urdim(x);
				int nn = ur - ll + 1;
				if (likely(n < nn)) {
					_new->topo_lldim(x) =
					_new->topo_urdim(x) = ll + n;
					n = 0;
				} else {
					_new->topo_lldim(x) =
					_new->topo_urdim(x) = ll + n % nn;
					n /= nn;
				}
			}
			if (n == 0) { // no overflow on num local nodes...
				_new->__size = s;
				return;
			}
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
			*_new = *this;
			size_t s = 1;
			unsigned x;
			for (x = 0; x < mapping->globalDims(); ++x) {
				if (fmt->net_coord(x) == (unsigned)-1) {
					_new->topo_lldim(x) = topo_lldim(x);
					_new->topo_urdim(x) = topo_urdim(x);
					_new->topo_hastorus(x) = topo_hastorus(x);
					s *= (topo_urdim(x) - topo_lldim(x) + 1);
				} else {
					XMI_assert_debugf(fmt->net_coord(x) >= topo_lldim(x) &&
						fmt->net_coord(x) <= topo_urdim(x), "selected coordinate is out of range\n");
					_new->topo_lldim(x) =
					_new->topo_urdim(x) = fmt->net_coord(x);
					_new->topo_hastorus(x) = 0;
				}
			}
			// cannot result in empty...
			_new->__size = s;
		}

		/// \brief iterate to next coord, [0] is most-significant dim
		///
		/// \param[in,out] c0	coordinate to "increment"
		/// \return	boolean indicating overflow (end of count)
		///
		bool __nextCoord(xmi_coord_t *c0, unsigned ndims) {
			unsigned x;
			for (x = ndims; x > 0;) {
				--x;
				c0->net_coord(x) += 1;
				if (c0->net_coord(x) <= topo_urdim(x)) {
					return true;
				}
				c0->net_coord(x) = topo_lldim(x);
			}
			return false;	// wrapped all dims - end of iteration
		}

		/// \brief initialize a coordinate range to a coord
		///
		/// \param[out] ll	lower-left corner of range
		/// \param[out] ur	upper-right corner of range
		/// \param[in] c0	initial coordinate
		///
		static void __initRange(xmi_coord_t *ll, xmi_coord_t *ur,
					xmi_coord_t *c0, unsigned ndims) {
			unsigned x;
			for (x = 0; x < ndims; ++x) {
				ur->net_coord(x) = ll->net_coord(x) = c0->net_coord(x);
			}
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
			size_t s = 1;
			unsigned x;
			for (x = 0; x < ndims; ++x) {
				unsigned n = ur->net_coord(x) - ll->net_coord(x) + 1;
				s *= n;
			}
			return s;
		}

		/// \brief expand range to include coord
		///
		/// \param[in,out] ll	lower-left corner of range
		/// \param[in,out] ur	upper-right corner of range
		/// \param[in] c0	coord to be included
		///
		static void __bumpRange(xmi_coord_t *ll, xmi_coord_t *ur,
						xmi_coord_t *c0, unsigned ndims) {
			unsigned x;
			for (x = 0; x < ndims; ++x) {
				if (c0->net_coord(x) < ll->net_coord(x)) {
					ll->net_coord(x) = c0->net_coord(x);
				} else if (c0->net_coord(x) > ur->net_coord(x)) {
					ur->net_coord(x) = c0->net_coord(x);
				}
			}
		}

		/// \brief Analyze a Rank Range to see if it is a rectangular segment.
		///
		/// \return 'true' means "this" was altered!
		bool __analyzeCoordsRange() {
			xmi_result_t rc;
			xmi_coord_t ll, ur;
			xmi_coord_t c0;
			size_t r = topo_first;
			rc = RANK2COORDS(r, &c0);
			__initRange(&ll, &ur, &c0, mapping->globalDims());
			for (r += 1; r <= topo_last; ++r) {
				RANK2COORDS(r, &c0);
				__bumpRange(&ll, &ur, &c0, mapping->globalDims());
			}
			size_t s = __sizeRange(&ll, &ur, mapping->globalDims());
			if (s == __size) {
				__type = XMI_COORD_TOPOLOGY;
				topo_llcoord = ll;
				topo_urcoord = ur;
				// can we get real torus info from mapping???
				// for now, assume no torus links
				memset(topo_istorus, 0, mapping->globalDims());
				return true;
			}
			return false;
		}

		/// \brief Analyze a Rank List to see if it is a rectangular segment.
		///
		/// Assumes that rank list contains no duplicates.
		///
		/// \return 'true' means "this" was altered!
		bool __analyzeCoordsList() {
			xmi_result_t rc;
			xmi_coord_t ll, ur;
			xmi_coord_t c0;
			unsigned i = 0;
			rc = RANK2COORDS(topo_list(i), &c0);
			__initRange(&ll, &ur, &c0, mapping->globalDims());
			for (i += 1; i < __size; ++i) {
				RANK2COORDS(topo_list(i), &c0);
				__bumpRange(&ll, &ur, &c0, mapping->globalDims());
			}
			size_t s = __sizeRange(&ll, &ur, mapping->globalDims());
			if (s == __size) {
				__type = XMI_COORD_TOPOLOGY;
				topo_llcoord = ll;
				topo_urcoord = ur;
				// can we get real torus info from mapping???
				// for now, assume no torus links
				memset(topo_istorus, 0, mapping->globalDims());
				return true;
			}
			return false;
		}

		/// \brief Analyze a Rank List to see if it is a range.
		///
		/// Assumes that rank list contains no duplicates.
		///
		/// \return 'true' means "this" was altered!
		bool __analyzeRangeList() {
			size_t min, max;
			unsigned i = 0;
			min = max = topo_list(i);
			for (i += 1; i < __size; ++i) {
				if (topo_list(i) < min) {
					min = topo_list(i);
				} else if (topo_list(i) > max) {
					max = topo_list(i);
				}
			}
			if (__size == max - min + 1) {
				__type = XMI_RANGE_TOPOLOGY;
				topo_first = min;
				topo_last = max;
				return true;
			}
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
			unsigned x;
			for (x = 0; x < ndims; ++x) {
				if (c0->net_coord(x) >= c1->net_coord(x)) {
					return false;
				}
			}
			return true;
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
			unsigned x;
			for (x = 0; x < ndims; ++x) {
				_new->net_coord(x) = (c0->net_coord(x) > c1->net_coord(x) ?
					c0->net_coord(x) : c1->net_coord(x));
			}
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
			unsigned x;
			for (x = 0; x < ndims; ++x) {
				_new->net_coord(x) = (c0->net_coord(x) < c1->net_coord(x) ?
					c0->net_coord(x) : c1->net_coord(x));
			}
		}

	public:
		static void static_init(XMI::Mapping *map) {
			mapping = map;
			RANK2COORDS(mapping->task(), &my_coords);
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
			__type = XMI_COORD_TOPOLOGY;
			topo_llcoord = *ll;
			topo_urcoord = *ur;
			if (tl) {
				memcpy(topo_istorus, tl, mapping->globalDims());
			} else {
				memset(topo_istorus, 0, mapping->globalDims());
			}
			__size = __sizeRange(ll, ur, mapping->globalDims());
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
				// probably not used?
				// assume last dim is least-significant
				xmi_coord_t c0;
				xmi_result_t rc;
				c0 = topo_llcoord;
				unsigned x;
				for (x = mapping->globalDims(); x > 0 && ix > 0;) {
					--x;
					unsigned ll = topo_lldim(x);
					unsigned ur = topo_urdim(x);
					int nn = ur - ll + 1;
					c0.net_coord(x) = ll + ix % nn;
					ix /= nn;
				}
				rc = COORDS2RANK(&c0, &rank);
				return rank;
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
				// probably not used?
				// assume last dim is least-significant
				rc = RANK2COORDS(rank, &c0);
				ix = 0;
				nn = 0;
				for (x = 0; x < mapping->globalDims(); ++x) {
					unsigned ll = topo_lldim(x);
					unsigned ur = topo_urdim(x);
					if (c0.net_coord(x) < ll || c0.net_coord(x) > ur) {
						return (size_t)-1;
					}
					ix *= nn;
					ix += c0.net_coord(x) - ll;
					nn = ur - ll + 1;
				}
				return ix;
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
			if (__type != XMI_COORD_TOPOLOGY) {
				return XMI_UNIMPL;
			}
			*ll = &topo_llcoord;
			*ur = &topo_urcoord;
			if (tl) {
				*tl = topo_istorus;
			}
			return XMI_SUCCESS;
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
			if (__type != XMI_COORD_TOPOLOGY) {
				return XMI_UNIMPL;
			}
			*ll = topo_llcoord;
			*ur = topo_urcoord;
			if (tl) {
				memcpy(tl, topo_istorus, mapping->globalDims());
			}
			return XMI_SUCCESS;
		}

		/// \brief does topology consist entirely of ranks local to eachother
		///
		/// \return boolean indicating locality of ranks
		///
		bool isLocal_impl() {
			xmi_result_t rc;
			if (likely(__type == XMI_COORD_TOPOLOGY)) {
				return __isLocalCoord(&topo_llcoord,
							&topo_urcoord);
			} else {
				// the hard way...
				if (__type == XMI_SINGLE_TOPOLOGY) {
					// a rank is always local to itself...
					return true;
				} else if (__type == XMI_RANGE_TOPOLOGY) {
					// would it be faster to just compare ranks in vnpeers list?
					// can't do that here since this wants to know if
					// ranks are all local to each other... not self.
					xmi_coord_t c0, c1;
					rc = RANK2COORDS(topo_first, &c0);
					XMI_assert_debugf(rc == XMI_SUCCESS, "RANK2COORDS failed\n");
					size_t r;
					for (r = topo_first + 1; r <= topo_last; ++r) {
						RANK2COORDS(r, &c1);
						if (!__isLocalCoord(&c0, &c1)) {
							return false;
						}
					}
					return true;
				} else if (__type == XMI_LIST_TOPOLOGY) {
					xmi_coord_t c0, c1;
					rc = RANK2COORDS(topo_list(0), &c0);
					XMI_assert_debugf(rc == XMI_SUCCESS, "RANK2COORDS failed\n");
					unsigned i;
					for (i = 1; i < __size; ++i) {
						RANK2COORDS(topo_list(i), &c1);
						if (!__isLocalCoord(&c0, &c1)) {
							return false;
						}
					}
					return true;
				}
			}
			// i.e. XMI_EMPTY_TOPOLOGY
			return false;
		}

		/// \brief does topology consist entirely of ranks local to self
		///
		/// \return boolean indicating locality of ranks
		///
		bool isLocalToMe_impl() {
			if (likely(__type == XMI_COORD_TOPOLOGY)) {
				// does mapping have "me" cached as a xmi_coord_t?
				return __isLocalCoord(&topo_llcoord,
							&topo_urcoord) &&
					__isLocalCoord(MY_COORDS,
							&topo_llcoord);
			} else {
				// the hard way...
				if (__type == XMI_SINGLE_TOPOLOGY) {
					return IS_LOCAL_PEER(__topo._rank);
				} else if (__type == XMI_RANGE_TOPOLOGY) {
					size_t r;
					for (r = topo_first; r <= topo_last; ++r) {
						if (!IS_LOCAL_PEER(r)) {
							return false;
						}
					}
					return true;
				} else if (__type == XMI_LIST_TOPOLOGY) {
					unsigned i;
					for (i = 0; i < __size; ++i) {
						if (!IS_LOCAL_PEER(topo_list(i))) {
							return false;
						}
					}
					return true;
				}
			}
			// i.e. XMI_EMPTY_TOPOLOGY
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
			*c0 = topo_lldim(n);
			*cn = topo_urdim(n);
			if (tl) {
				*tl = topo_hastorus(n);
			}
		}

		/// \brief is rank in topology
		///
		/// \param[in] rank	Rank to test
		/// \return	boolean indicating rank is in topology
		///
		bool isRankMember_impl(size_t rank) {
			xmi_result_t rc;
			if (unlikely(__type == XMI_COORD_TOPOLOGY)) {
				xmi_coord_t c0;
				rc = RANK2COORDS(rank, &c0);
				XMI_assert_debugf(rc == XMI_SUCCESS, "RANK2COORDS failed\n");
				return __isMemberCoord(&c0, mapping->globalDims());
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
			xmi_result_t rc;
			if (likely(__type == XMI_COORD_TOPOLOGY)) {
				return __isMemberCoord(c0, mapping->globalDims());
			} else if (__type == XMI_EMPTY_TOPOLOGY) {
				return false;
			} else {
				// the hard way...
				size_t rank = 0;
				rc = COORDS2RANK(c0, &rank);
				return isRankMember(rank);
			}
		}

		/// \brief create topology of ranks local to self
		///
		/// \param[out] _new	Where to build topology
		///
		void subTopologyLocalToMe_impl(XMI::Topology *_new) {
			__subTopologyLocalToMe(_new);
		}

		/// \brief create topology from all Nth ranks globally
		///
		/// \param[out] _new	Where to build topology
		/// \param[in] n	Which local rank to select on each node
		///
		void subTopologyNthGlobal_impl(XMI::Topology *_new, int n) {
			if (likely(__type == XMI_COORD_TOPOLOGY)) {
				__subTopologyNthGlobal(_new, n);
				// may produce empty topology, if "n" is out of range.
			} else {
				// the hard way... impractical?
				_new->__type = XMI_EMPTY_TOPOLOGY;
				_new->__size = 0;
			}
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
			} else if (__type == XMI_COORD_TOPOLOGY) {
				// the hard way...
				xmi_coord_t c0;
				size_t rank = 0;
				unsigned x;
				// c0 = llcorner;
				c0.network = XMI_N_TORUS_NETWORK;
				for (x = 0; x < mapping->globalDims(); ++x) {
					c0.net_coord(x) = topo_lldim(x);
				}
				x = 0;
				do {
					rc = COORDS2RANK(&c0, &rank);
					ranks[x] = rank;
					++x;
				} while (x < max && __nextCoord(&c0, mapping->globalDims()));
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
			if (__size == 1 && __type != XMI_SINGLE_TOPOLOGY) {
				if (__type == XMI_LIST_TOPOLOGY) {
					// it might not always be desirable to
					// convert a coord to single rank...
					// maybe shouldn't at all...
					size_t rank = 0;
					COORDS2RANK(&topo_llcoord, &rank);
					__type = XMI_SINGLE_TOPOLOGY;
					topo_rank = rank;
					return true;
				} else if (__type == XMI_RANGE_TOPOLOGY) {
					__type = XMI_SINGLE_TOPOLOGY;
					topo_rank = topo_first;
					return true;
				} else if (__type == XMI_LIST_TOPOLOGY) {
					__type = XMI_SINGLE_TOPOLOGY;
					topo_rank = topo_list(0);
					return true;
				}
			} else if (__type == XMI_LIST_TOPOLOGY) {
				// Note, might be able to convert to range...
				return __analyzeCoordsList() ||
					__analyzeRangeList();
			} else if (__type == XMI_RANGE_TOPOLOGY) {
				return __analyzeCoordsRange();
			}
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
					xmi_coord_t c0;
					rc = RANK2COORDS(topo_rank, &c0);
					__type = XMI_COORD_TOPOLOGY;
					topo_llcoord = c0;
					topo_urcoord = c0;
					// can we get real torus info from mapping???
					// for now, assume no torus links
					memset(topo_istorus, 0, mapping->globalDims());
					return true;
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
					return __analyzeCoordsRange();
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
				switch (new_type) {
				case XMI_COORD_TOPOLOGY:
					break;
				case XMI_SINGLE_TOPOLOGY:
					if (__size == 1) {
						rc = COORDS2RANK(&topo_llcoord, &rank);
						__type = XMI_SINGLE_TOPOLOGY;
						topo_rank = rank;
						return true;
					}
					break;
				case XMI_LIST_TOPOLOGY:
					rl = (size_t *)malloc(__size * sizeof(*rl));
					rp = rl;
					c0 = topo_llcoord;
					do {
						rc = COORDS2RANK(&c0, &rank);
						*rp++ = rank;
					} while (__nextCoord(&c0, mapping->globalDims()));
					__type = XMI_LIST_TOPOLOGY;
					topo_ranklist = rl;
					return true;
					break;
				case XMI_RANGE_TOPOLOGY:
					min = (size_t)-1;
					max = 0;
					c0 = topo_llcoord;
					do {
						rc = COORDS2RANK(&c0, &rank);
						if (rank < min) min = rank;
						if (rank > max) max = rank;
					} while (__nextCoord(&c0, mapping->globalDims()));
					if (__size == max - min + 1) {
						__type = XMI_RANGE_TOPOLOGY;
						topo_first = min;
						topo_last = max;
						return true;
					}
					break;
				default:
					break;
				}
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
#if 0

          if (likely(__type == other->__type))
            {
              // size_t s;
              // size_t i, j, k;
              switch (__type)
                {
                  case XMI_COORD_TOPOLOGY:
                    // in many cases we must fall-back to rank lists...
                    // for now, just always fall-back.
                    break;
                  case XMI_SINGLE_TOPOLOGY:
                    break;
                  case XMI_RANGE_TOPOLOGY:
                    break;
                  case XMI_LIST_TOPOLOGY:
                    /// \todo keep this from being O(n^2)
                    break;
                  case XMI_EMPTY_TOPOLOGY:
                  default:
                    break;
                }
            }
          else if (__type == XMI_EMPTY_TOPOLOGY)
            {
              *_new = *other;
              return;
            }
          else if (other->__type == XMI_EMPTY_TOPOLOGY)
            {
              *_new = *this;
              return;
            }
          else if (__type == XMI_SINGLE_TOPOLOGY)
            {
            }
          else if (other->__type == XMI_SINGLE_TOPOLOGY)
            {
            }
          else
            {
              // more complicated scenarios - TBD
              switch (__type)
                {
                  case XMI_COORD_TOPOLOGY:

                    switch (other->__type)
                      {
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

                    switch (other->__type)
                      {
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

                    switch (other->__type)
                      {
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

#endif
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
				// This always results in a rectangle...
				// first, check for disjoint
				if (__coordLT(&topo_urcoord,
						&other->topo_llcoord,
						mapping->globalDims()) ||
					__coordLT(&other->topo_urcoord,
						&topo_llcoord,
						mapping->globalDims())) {
					break;
				}
				_new->__type = XMI_COORD_TOPOLOGY;
				__coordMAX(&_new->topo_llcoord,
					&topo_llcoord,
					&other->topo_llcoord,
					mapping->globalDims());
				__coordMIN(&_new->topo_urcoord,
					&topo_urcoord,
					&other->topo_urcoord,
					mapping->globalDims());
				_new->__size = __sizeRange(&_new->topo_llcoord,
					&_new->topo_urcoord, mapping->globalDims());
				// can we get real torus info from old topology???
				// for now, assume no torus links
				memset(topo_istorus, 0, mapping->globalDims());
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
				// This results in coord range in special cases
				// that we don't check for. We just create a
				// list and then try to convert it to coords.
				s = __size;
				rl = (size_t *)malloc(s * sizeof(*rl));
				k = 0;
				c0 = topo_llcoord;
				do {
					if (other->__isMemberCoord(&c0,
							mapping->globalDims())) {
						continue;
					}
					// keep it
					rc = COORDS2RANK(&c0, &rank);
					if (k == 0) { // first one found...
						__initRange(&ll, &ur, &c0, mapping->globalDims());
						min = max = rank;
					} else {
						__bumpRange(&ll, &ur, &c0, mapping->globalDims());
						if (rank < min) min = rank;
						if (rank > max) max = rank;
					}
					rl[k++] = rank;
				} while (__nextCoord(&c0, mapping->globalDims()));
				if (k == 0) {
					free(rl);
					break;
				}
				_new->__size = k;
				s = __sizeRange(&ll, &ur, mapping->globalDims());
				if (s == k) {
					_new->__type = XMI_COORD_TOPOLOGY;
					_new->topo_llcoord = ll;
					_new->topo_urcoord = ur;
					// can we get real torus info from old topology???
					// for now, assume no torus links
					memset(_new->topo_istorus, 0, mapping->globalDims());
					free(rl);
				} else if (max - min + 1 == k) {
					_new->__type = XMI_RANGE_TOPOLOGY;
					_new->topo_first = min;
					_new->topo_last = max;
					free(rl);
				} else {
					_new->__type = XMI_LIST_TOPOLOGY;
					_new->topo_ranklist = rl;
				}
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
				case b0000:
				case b0001:
				case b0010:
				case b0011:
				case b0100:
				case b0101:
				case b1000:
				case b1001:
				case b1010:
				case b1011:
				case b1100:
				case b1101:
				default:
					// disjoint ranges, nothing removed
					*_new = *this;
					return;
					break;
				case b0110:
					// split into two ranges...
					// convert to list...
					_new->__type = XMI_LIST_TOPOLOGY;
					s = other->topo_first - topo_first +
						topo_last - other->topo_last;
					XMI_assert_debugf(s != 0, "subtraction results in empty topology\n");
					rl = (size_t *)malloc(s * sizeof(*rl));
					k = 0;
					for (j = topo_first; j < other->topo_first; ++j) {
						rl[k++] = j;
					}
					for (j = other->topo_last + 1; j <= topo_last; ++j) {
						rl[k++] = j;
					}
					XMI_assert_debug(k == s);
					_new->__size = s;
					return;
					break;
				case b0111:
					// remove top of range...
					_new->__type = XMI_RANGE_TOPOLOGY;
					_new->topo_first = topo_first;
					_new->topo_last = other->topo_first - 1;
					_new->__size = _new->topo_last - _new->topo_first + 1;
					XMI_assert_debugf(_new->__size != 0, "subtraction results in empty topology\n");
					return;
					break;
				case b1110:
					// remove bottom of range...
					_new->__type = XMI_RANGE_TOPOLOGY;
					_new->topo_first = other->topo_last + 1;
					_new->topo_last = topo_last;
					_new->__size = _new->topo_last - _new->topo_first + 1;
					XMI_assert_debugf(_new->__size != 0, "subtraction results in empty topology\n");
					return;
					break;
				case b1111:
					// remove all...
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
				if (isRankMember(other->topo_rank)) {
					// convert rectangle to list and remove one...
					s = __size;
					rl = (size_t *)malloc(s * sizeof(*rl));
					k = 0;
					c0 = topo_llcoord;
					do {
						rc = COORDS2RANK(&c0, &rank);
						if (rank == other->topo_rank) {
							continue;
						}
						// could try to make a range...
						rl[k++] = rank;
					} while (__nextCoord(&c0, mapping->globalDims()));
					if (k == 0) {
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

#endif // __components_topology_mpi_topology_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
