/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file adaptor/geometry/Geometry.h
 * \brief Geometry object
 * This object is a message layer approximation of communicators
 *  - Stores ranks and number of ranks in this particular group
 *  - Stores an ID which must be the same on all the ranks in the group
 *  - Has flags for tree, GI, rectangle, and torus
 *  - Has flag for global context - needed in MPI to determine if this is
 *    comm_world or a comm dupe of comm_world in multi-threaded environments
 *  - Keeps a barrier for this group of nodes
 */


#ifndef   __xmi_commongeometry__h__
#define   __xmi_commongeometry__h__


#define XMI_GEOMETRY_CLASS XMI::Geometry::Common

#include "components/geometry/Geometry.h"


namespace XMI
{
    namespace Geometry
    {
        class Common : public Geometry<XMI::Geometry::Common>
        {
        public:
            inline Common(COLLECTIVE_MAPPING     *mapping,
                          unsigned               *ranks,
                          unsigned                nranks,
                          unsigned                comm,
                          unsigned                numcolors,
                          bool                    globalcontext):
                Geometry<XMI::Geometry::Common>(mapping,ranks,nranks,comm,
                                                numcolors,globalcontext)
                {
                }
            inline int                       getColorsArray_impl()
                {
                }
            inline void                      setAsyncAllreduceMode_impl(unsigned value)
                {
                }
            inline unsigned                  getAsyncAllreduceMode_impl()
                {
                }
            inline unsigned                  incrementAllreduceIteration_impl()
                {
                }
            inline unsigned                  comm_impl()
                {
                }
            inline unsigned                 *ranks_impl()
                {
                }
            inline unsigned                  nranks_impl()
                {
                }
            inline int                       myIdx_impl()
                {
                }
            inline void                      generatePermutation_impl()
                {
                }
            inline void                      freePermutation_impl()
                {
                }
            inline unsigned                 *permutation_impl()
                {
                }
            inline bool                      isRectangle_impl()
                {
                }
            inline bool                      isTorus_impl()
                {
                }
            inline bool                      isTree_impl()
                {
                }
            inline bool                      isGlobalContext_impl()
                {
                }
            inline bool                      isGI_impl()
                {
                }
            inline unsigned                  getNumColors_impl()
                {
                }
            inline RECTANGLE_TYPE            rectangle_impl()
                {
                }
            inline RECTANGLE_TYPE            rectangle_mesh_impl()
                {
                }
            inline EXECUTOR_TYPE             getBarrierExecutor_impl()
                {
                }
            inline void                      setBarrierExecutor_impl(EXECUTOR_TYPE bar)
                {
                }
            inline EXECUTOR_TYPE             getLocalBarrierExecutor_impl()
                {
                }
            inline void                      setLocalBarrierExecutor_impl(EXECUTOR_TYPE bar)
                {
                }
            inline unsigned                  getAllreduceIteration_impl()
                {
                }
            inline CCMI_EXECUTOR_TYPE        getAllreduceCompositeStorage_impl()
                {
                }
            inline CCMI_EXECUTOR_TYPE        getAllreduceCompositeStorage_impl(unsigned i)
                {
                }
            inline COMPOSITE_TYPE            getAllreduceComposite_impl()
                {
                }
            inline COMPOSITE_TYPE            getAllreduceComposite_impl(unsigned i)
                {
                }
            inline void                      setAllreduceComposite_impl(COMPOSITE_TYPE c)
                {
                }
            inline void                      setAllreduceComposite_impl(COMPOSITE_TYPE c, 
                                                                        unsigned i)
                {
                }
            inline void                      freeAllocations_impl()
                {
                }
            inline EXECUTOR_TYPE             getCollectiveExecutor_impl(unsigned color=0)
                {
                }
            inline void                      setCollectiveExecutor_impl(EXECUTOR_TYPE exe,
                                                                        unsigned color=0)
                {
                }
            inline void                      setGlobalContext_impl(bool context)
                {
                }
            inline void                      setNumColors_impl(unsigned numcolors)
                {
                }
            inline CCMI::MatchQueue         &asyncBcastPostQ_impl()
                {
                }
            inline CCMI::MatchQueue         &asyncBcastUnexpQ_impl()
                {
                }
#if 0            
            static inline CCMI_Geometry_t   *getCachedGeometry_impl(unsigned comm)
                {
                }
            static inline void               updateCachedGeometry_impl(CCMI_Geometry_t *geometry, 
                                                                       unsigned comm)
                {
                }
#endif            
        }; // class Geometry
    };  // namespace Geometry
}; // namespace XMI


#endif
