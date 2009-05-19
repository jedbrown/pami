#ifndef __ll_topology_h__
#define __ll_topology_h__

#include "./ll_collectives_base.h"

////////////////////////////////////////////////////////////////////////
/// \file sysdep/Topology.h
///
/// \brief Coordinate/Address Topology Class
///
/// This is a strip-down version with only public interfaces
///
/// Some Basic Assertions:
///	* We never have a LL_COORD_TOPOLOGY unless we also have LL_TORUS_NETWORK
///
////////////////////////////////////////////////////////////////////////

namespace LL {

    class Topology {

    public:
        /// \brief default constructor (LL_EMPTY_TOPOLOGY)
        ///
        Topology();

        /// \brief rectangular segment constructor (LL_COORD_TOPOLOGY)
        ///
        /// \param[in] ll	lower-left coordinate
        /// \param[in] ur	upper-right coordinate
        ///
        Topology(LL_NetworkCoord_t *ll, LL_NetworkCoord_t *ur);

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
        static const unsigned size_of();

        /// \brief number of ranks in topology
        /// \return	number of ranks
        size_t size();

        /// \brief type of topology
        /// \return	topology type
        LL_Topology_t type();

        /// \brief Nth rank in topology
        ///
        /// \param[in] ix	Which rank to select
        /// \return	Nth rank or (size_t)-1 if does not exist
        ///
        size_t rank(size_t ix);

        /// \brief return range
        ///
        /// \param[out] first	Where to put first rank in range
        /// \param[out] last	Where to put last rank in range
        /// \return	LL_SUCCESS, or LL_UNIMPL if not a range topology
        ///
        LL_Result rankRange(size_t *first, size_t *last);

        /// \brief return rank list
        ///
        /// \param[out] list	pointer to list stored here
        /// \return	LL_SUCCESS, or LL_UNIMPL if not a list topology
        ///
        LL_Result rankList(size_t **list);

        /// \brief return rectangular segment coordinates
        ///
        /// \param[out] ll	lower-left coord pointer storage
        /// \param[out] ur	upper-right coord pointer storage
        /// \return	LL_SUCCESS, or LL_UNIMPL if not a coord topology
        ///
        LL_Result rectSeg(LL_NetworkCoord_t **ll, LL_NetworkCoord_t **ur);

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
        /// \return	boolean indicating rect seg topo
        bool isRectSeg();

        /// \brief extract Nth dimensions from coord topology
        ///
        /// \param[in] n	Which dim to extract
        /// \param[out] c0	lower value for dim range
        /// \param[out] cn	upper value for dim range
        ///
        void getNthDims(unsigned n, unsigned *c0, unsigned *cn);

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
        bool isCoordMember(LL_NetworkCoord_t *c0);

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
        void subTopologyReduceDims(LL::Topology *_new, LL_NetworkCoord_t *fmt);

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
        bool convertTopology(LL_Topology_t new_type);

    }; /* class Topology */

}; /* namespace LL */

#endif
