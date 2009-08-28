///
/// \file xmi/mpi/mpitopology.h
/// \brief XMI MPI specific topology implementation.
///
#ifndef   __xmi_mpi_mpitopology_h__
#define   __xmi_mpi_mpitopology_h__

#include <stdlib.h>
#include <string.h>

#include "components/topology/Topology.h"

#define XMI_TOPOLOGY_CLASS XMI::Topology::MPI

namespace XMI
{
    namespace Topology
    {
        class MPI : public Topology<XMI::Topology::MPI>
        {
        public:
            inline MPI():
                Topology<XMI::Topology::MPI>()
                {}
             
            inline MPI(xmi_coord_t *ll, xmi_coord_t *ur,
                       unsigned char *tl = NULL):
                Topology<XMI::Topology::MPI>(ll,ur,tl)
                {}

            inline MPI(size_t rank):
                Topology<XMI::Topology::MPI>(rank)
                {}
            inline MPI(size_t rank0, size_t rankn):
                Topology<XMI::Topology::MPI>(rank0, rankn)
                {}

            inline MPI(size_t *ranks, size_t nranks):
                Topology<XMI::Topology::MPI>(ranks, nranks)
                {}

            inline unsigned size_of_impl()
                {
                }
            inline size_t size_impl()
                {
                }
            inline XMI_TopologyType_t type_impl()
                {
                }
            inline size_t index2Rank(size_t ix)
                {
                }
            inline size_t rank2Index(size_t rank)
                {
                }
            inline xmi_result_t rankRange(size_t *first, size_t *last)
                {
                }
            inline xmi_result_t rankList(size_t **list)
                {
                }
            inline xmi_result_t rectSeg(xmi_coord_t **ll, xmi_coord_t **ur,
                                      unsigned char **tl = NULL)
                {
                }

            inline xmi_result_t rectSeg(xmi_coord_t *ll, xmi_coord_t *ur,
                                      unsigned char *tl = NULL)
                {
                }
            inline bool isLocal()
                {
                }
            inline bool isLocalToMe()
                {
                }
            inline bool isGlobal()
                {
                }
            inline bool isRectSeg()
                {
                }
            inline void getNthDims(unsigned n, unsigned *c0, unsigned *cn,
                                   unsigned char *tl = NULL)
                {
                }
            inline bool isRankMember(size_t rank)
                {
                }
            inline bool isCoordMember(xmi_coord_t *c0)
                {
                }
            inline void subTopologyLocalToMe(XMI::Topology::MPI *_new)
                {
                }
            inline void subTopologyNthGlobal(XMI::Topology::MPI *_new, int n)
                {
                }
            inline void subTopologyReduceDims(XMI::Topology::MPI *_new, xmi_coord_t *fmt)
                {
                }
            inline void getRankList(size_t max, size_t *ranks, size_t *nranks)
                {
                }
            inline bool analyzeTopology()
                {
                }
            inline bool convertTopology(XMI_TopologyType_t new_type)
                {
                }
            inline void unionTopology(XMI::Topology::MPI *_new, XMI::Topology::MPI *other)
                {
                }
            inline void intersectTopology(XMI::Topology::MPI *_new, XMI::Topology::MPI *other)
                {
                }
            inline void subtractTopology(XMI::Topology::MPI *_new, XMI::Topology::MPI *other)
                {
                }
        
        }; // end XMI::Topology::MPI
    }; // end namespace Topology
}; // end namespace XMI

#endif // __xmi_mpi_mpitopology_h__
