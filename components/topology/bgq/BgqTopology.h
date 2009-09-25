///
/// \file components/topology/bgq/BgqTopology.h
/// \brief XMI Blue Gene\Q specific topology implementation.
///
#ifndef   __components_topology_bgq_bgqtopology_h__
#define   __components_topology_bgq_bgqtopology_h__

#include <stdlib.h>
#include <string.h>

#include "components/topology/Topology.h"

#define XMI_TOPOLOGY_CLASS XMI::Topology::BgqTopology

namespace XMI
{
  namespace Topology
  {
    class BgqTopology : public Topology<XMI::Topology::BgqTopology>
    {
      public:
        inline BgqTopology():
            Topology<XMI::Topology::BgqTopology>()
        {}

        inline BgqTopology(xmi_coord_t *ll, xmi_coord_t *ur,
                           unsigned char *tl = NULL):
            Topology<XMI::Topology::BgqTopology>(ll, ur, tl)
        {}

        inline BgqTopology(size_t rank):
            Topology<XMI::Topology::BgqTopology>(rank)
        {}
        inline BgqTopology(size_t rank0, size_t rankn):
            Topology<XMI::Topology::BgqTopology>(rank0, rankn)
        {}

        inline BgqTopology(size_t *ranks, size_t nranks):
            Topology<XMI::Topology::BgqTopology>(ranks, nranks)
        {}

        inline unsigned size_of_impl()
        {
          return sizeof(BgqTopology);
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
        inline void subTopologyLocalToMe(XMI::Topology::BgqTopology *_new)
        {
        }
        inline void subTopologyNthGlobal(XMI::Topology::BgqTopology *_new, int n)
        {
        }
        inline void subTopologyReduceDims(XMI::Topology::BgqTopology *_new, xmi_coord_t *fmt)
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
        inline void unionTopology(XMI::Topology::BgqTopology *_new, XMI::Topology::BgqTopology *other)
        {
        }
        inline void intersectTopology(XMI::Topology::BgqTopology *_new, XMI::Topology::BgqTopology *other)
        {
        }
        inline void subtractTopology(XMI::Topology::BgqTopology *_new, XMI::Topology::BgqTopology *other)
        {
        }

    }; // end XMI::Topology::BgqTopology
  }; // end namespace Topology
}; // end namespace XMI

#endif // __components_topology_bgq_bgqtopology_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
