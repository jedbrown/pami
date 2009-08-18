/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef __xmi_topologyimpl_h__
#define __xmi_topologyimpl_h__

////////////////////////////////////////////////////////////////////////
/// \file generic/TopologyImpl.h
///
/// \brief Coordinate/Address Topology Class
///
/// Some Basic Assertions:
///	* We never have a XMI_COORD_TOPOLOGY unless we also have XMI_TORUS_NETWORK
///
////////////////////////////////////////////////////////////////////////
namespace XMI 
{
  class _TopologyImpl 
  {
    /// \brief are the two coords located on the same node
    ///
    /// \param[in] c0	first coordinate
    /// \param[in] c1	second coordinate
    /// \return	boolean indicate locality of two coords
    ///
    bool __isLocalCoord(const XMI_Coord_t *c0,
			const XMI_Coord_t *c1) 
    {
      return true;
    }

    /// \brief is the coordinate in this topology
    ///
    /// \param[in] c0	coordinate
    /// \param[in] ndims	number of dimensions that are significant
    /// \return	boolean indicating if the coordinate lies in this
    ///		topology, according to significant dimensions.
    ///
    bool __isMemberCoord(const XMI_Coord_t *c0, unsigned ndims) 
    {
      return true;
    }
    
    /// \brief create topology of only ranks local to self
    ///
    /// \param[out] _new	place to construct new topology
    /// \return	nothing, but _new may be XMI_EMPTY_TOPOLOGY
    ///
    void __subTopologyLocalToMe(XMI::_TopologyImpl *_new) 
    {
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
    void __subTopologyNthGlobal(XMI::_TopologyImpl *_new, int n) 
    {

    }

    /// \brief Create new Rectangular Segment topology from subset of this
    ///
    /// preserve coord if "fmt" coord is "-1",
    /// otherwise squash coord into 1-dim at "fmt" coord.
    ///
    /// \param[out] _new	Storage for new topology
    /// \param[in] fmt	Coords to collapse/preserve
    ///
    void __subTopologyReduceDims(XMI::_TopologyImpl *_new, XMI_Coord_t *fmt) 
    {

    }

    /// \brief iterate to next coord, [0] is most-significant dim
    ///
    /// \param[in,out] c0	coordinate to "increment"
    /// \return	boolean indicating overflow (end of count)
    ///
    bool __nextCoord(XMI_Coord_t *c0, unsigned ndims) 
    {
      return false;	// wrapped all dims - end of iteration
    }

    /// \brief initialize a coordinate range to a coord
    ///
    /// \param[out] ll	lower-left corner of range
    /// \param[out] ur	upper-right corner of range
    /// \param[in] c0	initial coordinate
    ///
    static void __initRange(XMI_Coord_t *ll, XMI_Coord_t *ur,
			    XMI_Coord_t *c0, unsigned ndims) 
    {
    }

    /// \brief compute size of rectangular segment
    ///
    /// \param[in] ll	lower-left corner of range
    /// \param[in] ur	upper-right corner of range
    /// \param[in] ndims	number of significant dimensions
    /// \return	size of rectangular segment
    ///
    static size_t __sizeRange(XMI_Coord_t *ll, XMI_Coord_t *ur,
			      unsigned ndims) 
    {
	return 0;
    }

    /// \brief expand range to include coord
    ///
    /// \param[in,out] ll	lower-left corner of range
    /// \param[in,out] ur	upper-right corner of range
    /// \param[in] c0	coord to be included
    ///
    static void __bumpRange(XMI_Coord_t *ll, XMI_Coord_t *ur,
			    XMI_Coord_t *c0, unsigned ndims) 
    {
    }

    /// \brief Analyze a Rank Range to see if it is a rectangular segment.
    ///
    /// \return 'true' means "this" was altered!
    bool __analyzeCoordsRange() 
    {
      return false;
    }

    /// \brief Analyze a Rank List to see if it is a rectangular segment.
    ///
    /// Assumes that rank list contains no duplicates.
    ///
    /// \return 'true' means "this" was altered!
    bool __analyzeCoordsList() 
    {
      return false;
    }

    /// \brief Analyze a Rank List to see if it is a range.
    ///
    /// Assumes that rank list contains no duplicates.
    ///
    /// \return 'true' means "this" was altered!
    bool __analyzeRangeList() 
    {
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
    static bool __coordLT(XMI_Coord_t *c0, XMI_Coord_t *c1,
			  unsigned ndims) 
    {
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
    static void __coordMAX(XMI_Coord_t *_new,
			   XMI_Coord_t *c0, XMI_Coord_t *c1,
			   unsigned ndims) 
    {
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
    static void __coordMIN(XMI_Coord_t *_new,
			   XMI_Coord_t *c0, XMI_Coord_t *c1,
			   unsigned ndims) 
    {
    }

  public:
    /// \brief default constructor (XMI_EMPTY_TOPOLOGY)
    ///
    _TopologyImpl() 
      {
      }

    /// \brief rectangular segment with torus (XMI_COORD_TOPOLOGY)
    ///
    /// Assumes no torus links if no 'tl' param.
    ///
    /// \param[in] ll	lower-left coordinate
    /// \param[in] ur	upper-right coordinate
    /// \param[in] tl	optional, torus links flags
    ///
    _TopologyImpl(XMI_Coord_t *ll, XMI_Coord_t *ur,
		  unsigned char *tl = NULL) 
      {

      }

    /// \brief single rank constructor (XMI_SINGLE_TOPOLOGY)
    ///
    /// \param[in] rank	The rank
    ///
    _TopologyImpl(size_t rank) 
      {
      }

    /// \brief rank range constructor (XMI_RANGE_TOPOLOGY)
    ///
    /// \param[in] rank0	first rank in range
    /// \param[in] rankn	last rank in range
    ///
    _TopologyImpl(size_t rank0, size_t rankn) 
      {
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
    _TopologyImpl(size_t *ranks, size_t nranks) 
      {
      }

    /// \brief accessor for size of a Topology object
    /// \return	size of XMI::Topology
    static const unsigned size_of() { return sizeof(_TopologyImpl); }

    /// \brief number of ranks in topology
    /// \return	number of ranks
    size_t size() { return __size; }

    /// \brief type of topology
    /// \return	topology type
    XMI_TopologyType_t type() { return __type; }

    /// \brief Nth rank in topology
    ///
    /// \param[in] ix	Which rank to select
    /// \return	Nth rank or (size_t)-1 if does not exist
    ///
    size_t index2Rank(size_t ix) 
    {
	return 0;
    }

    /// \brief determine index of rank in topology
    ///
    /// This is the inverse function to index2Rank(ix) above.
    ///
    /// \param[in] rank	Which rank to get index for
    /// \return	index of rank (rank(ix) == rank) or (size_t)-1
    ///
    size_t rank2Index(size_t rank) 
    {
	return 0;
    }

    /// \brief return range
    ///
    /// \param[out] first	Where to put first rank in range
    /// \param[out] last	Where to put last rank in range
    /// \return	XMI_SUCCESS, or XMI_UNIMPL if not a range topology
    ///
    XMI_Result rankRange(size_t *first, size_t *last) 
    {
	return XMI_ERROR;
    }

    /// \brief return rank list
    ///
    /// \param[out] list	pointer to list stored here
    /// \return	XMI_SUCCESS, or XMI_UNIMPL if not a list topology
    ///
    XMI_Result rankList(size_t **list) 
    {
	return XMI_ERROR;
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
    XMI_Result rectSeg(XMI_Coord_t **ll, XMI_Coord_t **ur,
		      unsigned char **tl = NULL) 
    {
	return XMI_ERROR;
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
    XMI_Result rectSeg(XMI_Coord_t *ll, XMI_Coord_t *ur,
		      unsigned char *tl = NULL) 
    {
      return XMI_SUCCESS;
    }

    /// \brief does topology consist entirely of ranks local to eachother
    ///
    /// \return boolean indicating locality of ranks
    ///
    bool isLocal() 
    {
      return false;
    }

    /// \brief does topology consist entirely of ranks local to self
    ///
    /// \return boolean indicating locality of ranks
    ///
    bool isLocalToMe() 
    {
      return false;
    }

    /// \brief does topology consist entirely of ranks that do not share nodes
    ///
    /// \return boolean indicating locality of ranks
    ///
    bool isGlobal() 
    {
      // is this too difficult/expensive? is it needed?
      // Note, this is NOT simply "!isLocal()"...
      XMI_abortf("Topology::isGlobal not implemented\n");
    }

    /// \brief is topology a rectangular segment
    /// \return	boolean indicating rect seg topo
    bool isRectSeg() { return (__type == XMI_COORD_TOPOLOGY); }

    /// \brief extract Nth dimensions from coord topology
    ///
    /// \param[in] n	Which dim to extract
    /// \param[out] c0	lower value for dim range
    /// \param[out] cn	upper value for dim range
    /// \param[out] tl	optional, torus link flag
    ///
    void getNthDims(unsigned n, unsigned *c0, unsigned *cn,
		    unsigned char *tl = NULL) 
    {
    }

    /// \brief is rank in topology
    ///
    /// \param[in] rank	Rank to test
    /// \return	boolean indicating rank is in topology
    ///
    bool isRankMember(size_t rank) 
    {
      // i.e. XMI_EMPTY_TOPOLOGY
      return false;
    }

    /// \brief is coordinate in topology
    ///
    /// \param[in] c0	Coord to test
    /// \return	boolean indicating coord is a member of topology
    ///
    bool isCoordMember(XMI_Coord_t *c0) 
    {
	return false;
    }

    /// \brief create topology of ranks local to self
    ///
    /// \param[out] _new	Where to build topology
    ///
    void subTopologyLocalToMe(XMI::_TopologyImpl *_new) 
    {
    }

    /// \brief create topology from all Nth ranks globally
    ///
    /// \param[out] _new	Where to build topology
    /// \param[in] n	Which local rank to select on each node
    ///
    void subTopologyNthGlobal(XMI::_TopologyImpl *_new, int n) 
    {
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
    void subTopologyReduceDims(XMI::_TopologyImpl *_new, XMI_Coord_t *fmt) 
    {
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
    void getRankList(size_t max, size_t *ranks, size_t *nranks) 
    {
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
    bool analyzeTopology() 
    {
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
    bool convertTopology(XMI_TopologyType_t new_type) 
    {
	return false;
    }

    /// \brief produce the union of two topologies
    ///
    /// produces: _new = this .U. other
    ///
    /// \param[out] _new	New topology created there
    /// \param[in] other	The other topology
    ///
    void unionTopology(_TopologyImpl *_new, _TopologyImpl *other) 
    {

    }

    /// \brief produce the intersection of two topologies
    ///
    /// produces: _new = this ./\. other
    ///
    /// \param[out] _new	New topology created there
    /// \param[in] other	The other topology
    ///
    void intersectTopology(_TopologyImpl *_new, _TopologyImpl *other) 
    {

    }

    /// \brief produce the difference of two topologies
    ///
    /// produces: _new = this .-. other
    ///
    /// \param[out] _new	New topology created there
    /// \param[in] other	The other topology
    ///
    void subtractTopology(_TopologyImpl *_new, _TopologyImpl *other) 
    {
    }

  private:
    size_t	      __size;	///< number of ranks in this topology
    XMI_TopologyType_t __type;	///< type of topology this is

  }; // class _TopologyImpl

}; /* namespace XMI */

#endif /* __xmi_topologyimpl_h__ */
