///
/// \file common/bgq/Topology.h
/// \brief XMI Blue Gene\Q specific topology implementation.
///
#ifndef   __components_topology_bgq_bgqtopology_h__
#define   __components_topology_bgq_bgqtopology_h__

#include <stdlib.h>
#include <string.h>

#include "common/Topology.h"

#define XMI_TOPOLOGY_CLASS XMI::Topology

namespace XMI
{
    class Topology : public Interface::Topology<XMI::Topology>
    {
      public:
        inline Topology():
            Interface::Topology<XMI::Topology>()
        {}

        inline Topology(xmi_coord_t *ll, xmi_coord_t *ur,
                           unsigned char *tl = NULL):
            Interface::Topology<XMI::Topology>(ll, ur, tl)
        {}

        inline Topology(size_t rank):
            Interface::Topology<XMI::Topology>(rank)
        {}
        inline Topology(size_t rank0, size_t rankn):
            Interface::Topology<XMI::Topology>(rank0, rankn)
        {}

        inline Topology(size_t *ranks, size_t nranks):
            Interface::Topology<XMI::Topology>(ranks, nranks)
        {}

        inline unsigned size_of_impl()
        {
          return sizeof(Topology);
        }
        inline size_t size_impl()
        {
          return 0;
        }
        inline xmi_topology_type_t type_impl()
        {
          return XMI_EMPTY_TOPOLOGY;
        }
        inline size_t index2Rank(size_t ix)
        {
          return 0;
        }
        inline size_t rank2Index(size_t rank)
        {
          return 0;
        }
        inline xmi_result_t rankRange(size_t *first, size_t *last)
        {
          return XMI_UNIMPL;
        }
        inline xmi_result_t rankList(size_t **list)
        {
          return XMI_UNIMPL;
        }
        inline xmi_result_t rectSeg(xmi_coord_t **ll, xmi_coord_t **ur,
                                    unsigned char **tl = NULL)
        {
          return XMI_UNIMPL;
        }

        inline xmi_result_t rectSeg(xmi_coord_t *ll, xmi_coord_t *ur,
                                    unsigned char *tl = NULL)
        {
          return XMI_UNIMPL;
        }
        inline bool isLocal()
        {
          return false;
        }
        inline bool isLocalToMe()
        {
          return false;
        }
        inline bool isGlobal()
        {
          return false;
        }
        inline bool isRectSeg()
        {
          return false;
        }
        inline void getNthDims(unsigned n, unsigned *c0, unsigned *cn,
                               unsigned char *tl = NULL)
        {
        }
        inline bool isRankMember(size_t rank)
        {
          return false;
        }
        inline bool isCoordMember(xmi_coord_t *c0)
        {
          return false;
        }
        inline void subTopologyLocalToMe(XMI::Topology *_new)
        {
        }
        inline void subTopologyNthGlobal(XMI::Topology *_new, int n)
        {
        }
        inline void subTopologyReduceDims(XMI::Topology *_new, xmi_coord_t *fmt)
        {
        }
        inline void getRankList(size_t max, size_t *ranks, size_t *nranks)
        {
        }
        inline bool analyzeTopology()
        {
          return false;
        }
        inline bool convertTopology(xmi_topology_type_t new_type)
        {
          return false;
        }
        inline void unionTopology(XMI::Topology *_new, XMI::Topology *other)
        {
        }
        inline void intersectTopology(XMI::Topology *_new, XMI::Topology *other)
        {
        }
        inline void subtractTopology(XMI::Topology *_new, XMI::Topology *other)
        {
        }

        static void static_init(XMI::Mapping *map)
        {
        }

  }; // end class Topology
}; // end namespace XMI

#endif // __components_topology_bgq_bgqtopology_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
