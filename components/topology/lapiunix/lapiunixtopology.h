///
/// \file components/topology/lapi/lapitopology.h
/// \brief XMI LAPI specific topology implementation.
///
#ifndef   __xmi_lapi_lapitopology_h__
#define   __xmi_lapi_lapitopology_h__

#include <stdlib.h>
#include <string.h>

#include "components/topology/Topology.h"
#include "components/mapping/lapiunix/lapiunixmapping.h"

#define XMI_TOPOLOGY_CLASS XMI::Topology::LAPI

namespace XMI
{
    namespace Topology
    {
        class LAPI : public Topology<XMI::Topology::LAPI>
        {
        public:
            inline LAPI():
                Topology<XMI::Topology::LAPI>()
                {}

            inline LAPI(xmi_coord_t *ll, xmi_coord_t *ur,
                       unsigned char *tl = NULL):
                Topology<XMI::Topology::LAPI>(ll,ur,tl)
                {}

            inline LAPI(size_t rank):
                Topology<XMI::Topology::LAPI>(rank)
                {}
            inline LAPI(size_t rank0, size_t rankn):
                Topology<XMI::Topology::LAPI>(rank0, rankn)
                {}

            inline LAPI(size_t *ranks, size_t nranks):
                Topology<XMI::Topology::LAPI>(ranks, nranks)
                {}


            static void static_init(XMI::Mapping::LAPIMapping *map)
            {
            }
            inline unsigned size_of_impl()
                {
		  return 0;
                }
            inline size_t size_impl()
                {
		  return -1;
                }
            inline xmi_topology_type_t type_impl()
                {
		  return XMI_EMPTY_TOPOLOGY;
                }
            inline size_t index2Rank(size_t ix)
                {
		  return -1;
                }
            inline size_t rank2Index(size_t rank)
                {
		  return -1;
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
            inline void subTopologyLocalToMe(XMI::Topology::LAPI *_new)
                {
                }
            inline void subTopologyNthGlobal(XMI::Topology::LAPI *_new, int n)
                {
                }
            inline void subTopologyReduceDims(XMI::Topology::LAPI *_new, xmi_coord_t *fmt)
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
            inline void unionTopology(XMI::Topology::LAPI *_new, XMI::Topology::LAPI *other)
                {
                }
            inline void intersectTopology(XMI::Topology::LAPI *_new, XMI::Topology::LAPI *other)
                {
                }
            inline void subtractTopology(XMI::Topology::LAPI *_new, XMI::Topology::LAPI *other)
                {
                }

        }; // end XMI::Topology::LAPI
    }; // end namespace Topology
}; // end namespace XMI

#endif // __xmi_lapi_lapitopology_h__
