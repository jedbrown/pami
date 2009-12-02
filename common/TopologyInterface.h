///
/// \file common/TopologyInterface.h
/// \brief XMI topology implementation.
///

#ifndef __common_TopologyInterface_h__
#define __common_TopologyInterface_h__

#include <stdlib.h>
#include <string.h>

#include "sys/xmi.h"

namespace XMI
{
    namespace Interface
    {
        template <class T_Topology>
        class Topology
        {
        public:
            /// \brief default constructor (XMI_EMPTY_TOPOLOGY)
            ///
            inline Topology() {}

            /// \brief rectangular segment with torus (XMI_COORD_TOPOLOGY)
            ///
            /// Assumes no torus links if no 'tl' param.
            ///
            /// \param[in] ll	lower-left coordinate
            /// \param[in] ur	upper-right coordinate
            /// \param[in] tl	optional, torus links flags
            ///
            inline Topology(xmi_coord_t *ll, xmi_coord_t *ur,
                            unsigned char *tl = NULL) {}

            /// \brief single rank constructor (XMI_SINGLE_TOPOLOGY)
            ///
            /// \param[in] rank	The rank
            ///
            inline Topology(xmi_task_t rank) {}

            /// \brief rank range constructor (XMI_RANGE_TOPOLOGY)
            ///
            /// \param[in] rank0	first rank in range
            /// \param[in] rankn	last rank in range
            ///
            inline Topology(xmi_task_t rank0, xmi_task_t rankn) {}

            /// \brief rank list constructor (XMI_LIST_TOPOLOGY)
            ///
            /// caller must not free ranks[]!
            ///
            /// \param[in] ranks	array of ranks
            /// \param[in] nranks	size of array
            ///
            /// \todo create destructor to free list, or establish rules
            ///
            inline Topology(xmi_task_t *ranks, size_t nranks) {}

            /// \brief accessor for size of a Topology object
            /// \return	size of XMI::Topology
            inline unsigned size_of();

            /// \brief number of ranks in topology
            /// \return	number of ranks
            inline size_t size();

            /// \brief type of topology
            /// \return	topology type
            inline xmi_topology_type_t type();

            /// \brief Nth rank in topology
            ///
            /// \param[in] ix	Which rank to select
            /// \return	Nth rank or (size_t)-1 if does not exist
            ///
            inline xmi_task_t index2Rank(size_t ix);

            /// \brief determine index of rank in topology
            ///
            /// This is the inverse function to index2Rank(ix) above.
            ///
            /// \param[in] rank	Which rank to get index for
            /// \return	index of rank (rank(ix) == rank) or (size_t)-1
            ///
            inline size_t rank2Index(xmi_task_t rank);

            /// \brief return range
            ///
            /// \param[out] first	Where to put first rank in range
            /// \param[out] last	Where to put last rank in range
            /// \return	XMI_SUCCESS, or XMI_UNIMPL if not a range topology
            ///
            inline xmi_result_t rankRange(xmi_task_t *first, xmi_task_t *last);

            /// \brief return rank list
            ///
            /// \param[out] list	pointer to list stored here
            /// \return	XMI_SUCCESS, or XMI_UNIMPL if not a list topology
            ///
            inline xmi_result_t rankList(xmi_task_t **list);

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
            inline xmi_result_t rectSeg(xmi_coord_t **ll, xmi_coord_t **ur,
                                        unsigned char **tl = NULL);

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
            inline xmi_result_t rectSeg(xmi_coord_t *ll, xmi_coord_t *ur,
                                        unsigned char *tl = NULL);

            /// \brief does topology consist entirely of ranks local to eachother
            ///
            /// \return boolean indicating locality of ranks
            ///
            inline bool isLocal();

            /// \brief does topology consist entirely of ranks local to self
            ///
            /// \return boolean indicating locality of ranks
            ///
            inline bool isLocalToMe();

            /// \brief does topology consist entirely of ranks that do not share nodes
            ///
            /// \return boolean indicating locality of ranks
            ///
            inline bool isGlobal();

            /// \brief is topology a rectangular segment
            ///
            /// \return	boolean indicating rect seg topo
            ///
            inline bool isRectSeg();

            /// \brief extract Nth dimensions from coord topology
            ///
            /// \param[in] n	Which dim to extract
            /// \param[out] c0	lower value for dim range
            /// \param[out] cn	upper value for dim range
            /// \param[out] tl	optional, torus link flag
            ///
            inline void getNthDims(unsigned n, unsigned *c0, unsigned *cn,
                                   unsigned char *tl = NULL);

            /// \brief is rank in topology
            ///
            /// \param[in] rank	Rank to test
            /// \return	boolean indicating rank is in topology
            ///
            inline bool isRankMember(xmi_task_t rank);

            /// \brief is coordinate in topology
            ///
            /// \param[in] c0	Coord to test
            /// \return	boolean indicating coord is a member of topology
            ///
            inline bool isCoordMember(xmi_coord_t *c0);

            /// \brief create topology of ranks local to self
            ///
            /// \param[out] _new	Where to build topology
            ///
            inline void subTopologyLocalToMe(T_Topology *_new);

            /// \brief create topology from all Nth ranks globally
            ///
            /// \param[out] _new	Where to build topology
            /// \param[in] n	Which local rank to select on each node
            ///
            inline void subTopologyNthGlobal(T_Topology *_new, int n);

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
            inline void subTopologyReduceDims(T_Topology *_new, xmi_coord_t *fmt);

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
            inline void getRankList(size_t max, xmi_task_t *ranks, size_t *nranks);

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
            inline bool analyzeTopology();

            /// \brief check if topology can be converted to type
            ///
            /// Does not differentiate between invalid conversions and
            /// 'null' conversions (same type).
            ///
            /// \param[in] new_type	Topology type to try and convert into
            /// \return	'true' if topology was changed
            ///
            inline bool convertTopology(xmi_topology_type_t new_type);

            /// \brief produce the union of two topologies
            ///
            /// produces: _new = this .U. other
            ///
            /// \param[out] _new	New topology created there
            /// \param[in] other	The other topology
            ///
            inline void unionTopology(T_Topology *_new, T_Topology *other);

            /// \brief produce the intersection of two topologies
            ///
            /// produces: _new = this ./\. other
            ///
            /// \param[out] _new	New topology created there
            /// \param[in] other	The other topology
            ///
            inline void intersectTopology(T_Topology *_new, T_Topology *other);

            /// \brief produce the difference of two topologies
            ///
            /// produces: _new = this .-. other
            ///
            /// \param[out] _new	New topology created there
            /// \param[in] other	The other topology
            ///
            inline void subtractTopology(T_Topology *_new, T_Topology *other);
        }; // end class XMI::Interface::Topology

        template <class T_Topology>
	unsigned Topology<T_Topology>::size_of()
        {
            return static_cast<T_Topology*>(this)->size_of_impl();
        }

        template <class T_Topology>
        size_t Topology<T_Topology>::size()
        {
            return static_cast<T_Topology*>(this)->size_impl();
        }

        template <class T_Topology>
	xmi_topology_type_t Topology<T_Topology>::type()
        {
            return static_cast<T_Topology*>(this)->type_impl();
        }

        template <class T_Topology>
	xmi_task_t Topology<T_Topology>::index2Rank(size_t ix)
        {
            return static_cast<T_Topology*>(this)->index2Rank_impl(ix);
        }

        template <class T_Topology>
	size_t Topology<T_Topology>::rank2Index(xmi_task_t rank)
        {
            return static_cast<T_Topology*>(this)->rank2Index_impl(rank);
        }

        template <class T_Topology>
	xmi_result_t Topology<T_Topology>::rankRange(xmi_task_t *first, xmi_task_t *last)
        {
            return static_cast<T_Topology*>(this)->rankRange_impl(first,last);
        }

        template <class T_Topology>
	xmi_result_t Topology<T_Topology>::rankList(xmi_task_t **list)
        {
            return static_cast<T_Topology*>(this)->rankList_impl(list);
        }

        template <class T_Topology>
	xmi_result_t Topology<T_Topology>::rectSeg(xmi_coord_t **ll, xmi_coord_t **ur,
                                                   unsigned char **tl)
        {
            return static_cast<T_Topology*>(this)->rectSeg_impl(ll,ur,tl);
        }

        template <class T_Topology>
	xmi_result_t Topology<T_Topology>::rectSeg(xmi_coord_t *ll, xmi_coord_t *ur,
                                                   unsigned char *tl)
        {
            return static_cast<T_Topology*>(this)->rectSeg_impl(ll, ur, tl);
        }

        template <class T_Topology>
	bool Topology<T_Topology>::isLocal()
        {
            return static_cast<T_Topology*>(this)->isLocal_impl();
        }

        template <class T_Topology>
	bool Topology<T_Topology>::isLocalToMe()
        {
            return static_cast<T_Topology*>(this)->isLocalToMe_impl();
        }

        template <class T_Topology>
	bool Topology<T_Topology>::isGlobal()
        {
            return static_cast<T_Topology*>(this)->isGlobal_impl();
        }

        template <class T_Topology>
	bool Topology<T_Topology>::isRectSeg()
        {
            return static_cast<T_Topology*>(this)->isRectSeg_impl();
        }

        template <class T_Topology>
	void Topology<T_Topology>::getNthDims(unsigned n, unsigned *c0, unsigned *cn,
                                              unsigned char *tl)
        {
            return static_cast<T_Topology*>(this)->getNthDims_impl(n,c0,cn,tl);
        }

        template <class T_Topology>
	bool Topology<T_Topology>::isRankMember(xmi_task_t rank)
        {
            return static_cast<T_Topology*>(this)->isRankMember_impl(rank);
        }

        template <class T_Topology>
	bool Topology<T_Topology>::isCoordMember(xmi_coord_t *c0)
        {
            return static_cast<T_Topology*>(this)->isCoordMember_impl(c0);
        }

        template <class T_Topology>
	void Topology<T_Topology>::subTopologyLocalToMe(T_Topology *_new)
        {
            return static_cast<T_Topology*>(this)->subTopologyLocalToMe_impl(_new);
        }

        template <class T_Topology>
	void Topology<T_Topology>::subTopologyNthGlobal(T_Topology *_new, int n)
        {
            return static_cast<T_Topology*>(this)->subTopologyNthGlobal_impl(_new, n);
        }

        template <class T_Topology>
	void Topology<T_Topology>::subTopologyReduceDims(T_Topology *_new, xmi_coord_t *fmt)
        {
            return static_cast<T_Topology*>(this)->subTopologyReduceDims_impl(_new, fmt);
        }

        template <class T_Topology>
	void Topology<T_Topology>::getRankList(size_t max, xmi_task_t *ranks, size_t *nranks)
        {
            return static_cast<T_Topology*>(this)->getRankList_impl(max,ranks,nranks);
        }

        template <class T_Topology>
	bool Topology<T_Topology>::analyzeTopology()
        {
            return static_cast<T_Topology*>(this)->analyzeTopology_impl();
        }

        template <class T_Topology>
	bool Topology<T_Topology>::convertTopology(xmi_topology_type_t new_type)
        {
            return static_cast<T_Topology*>(this)->convertTopology_impl(new_type);
        }

        template <class T_Topology>
	void Topology<T_Topology>::unionTopology(T_Topology *_new, T_Topology *other)
        {
            return static_cast<T_Topology*>(this)->unionTopology_impl(_new, other);
        }

        template <class T_Topology>
	void Topology<T_Topology>::intersectTopology(T_Topology *_new, T_Topology *other)
        {
            return static_cast<T_Topology*>(this)->intersectTopology_impl(_new, other);
        }

        template <class T_Topology>
	void Topology<T_Topology>::subtractTopology(T_Topology *_new, T_Topology *other)
        {
            return static_cast<T_Topology*>(this)->subtractTopology_impl(_new, other);
        }
    }; // end namespace Interface
}; // end namespace XMI

#endif // __xmi_topology_h__
