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
            inline MatchQueue               &asyncBcastPostQ_impl()
                {
                }
            inline MatchQueue               &asyncBcastUnexpQ_impl()
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
            // These methods were originally from the PGASRT Communicator class            
            inline int                        size_impl       (void)
                {

                }
            inline int                        rank_impl       (void)
                {

                }
            inline int                        split_impl      (int color, int rank, int * proclist)
                {

                }
            inline int                        absrankof_impl  (int rank)
                {

                }
            inline int                        virtrankof_impl (int rank)
                {

                }
            inline PGASNBCOLL                 ibarrier_impl    (MCAST_INTERFACE mcast_iface,
                                                                void (*cb_complete)(void *)=NULL,
                                                                void *arg=NULL)
                {

                }
            inline void                       barrier_impl     (MCAST_INTERFACE mcast_iface,
                                                                void (*cb_complete)(void *)=NULL,
                                                                void *arg=NULL)
                {

                }
            inline PGASNBCOLL                 iallgather_impl  (MCAST_INTERFACE mcast_iface,
                                                                const void *s,void *d, size_t l,
                                                                void (*cb_complete)(void *)=NULL,
                                                                void *arg=NULL)
                {

                }
            inline void                       allgather_impl   (MCAST_INTERFACE mcast_iface,
                                                                const void *s, void *d,size_t l,
                                                                void (*cb_complete)(void *)=NULL,
                                                                void *arg=NULL)
                {

                }
            inline PGASNBCOLL                 iallgatherv_impl (MCAST_INTERFACE mcast_iface,
                                                                const void *s, void *d, size_t *l,
                                                                void (*cb_complete)(void *)=NULL,
                                                                void *arg=NULL)
                {

                }
            inline void                       allgatherv_impl  (MCAST_INTERFACE mcast_iface,
                                                                const void *s, void *d, size_t *l,
                                                                void (*cb_complete)(void *)=NULL,
                                                                void *arg=NULL)
                {

                }
            inline PGASNBCOLL                 ibcast_impl      (MCAST_INTERFACE mcast_iface,
                                                                int root, const void *s, void *d,
                                                                size_t l,
                                                                void (*cb_complete)(void *)=NULL,
                                                                void *arg=NULL)
                {

                }
            inline void                       bcast_impl       (MCAST_INTERFACE mcast_iface,
                                                                int root, const void *s, void *d,
                                                                size_t l,
                                                                void (*cb_complete)(void *)=NULL,
                                                                void *arg=NULL)
                {

                }
            inline PGASNBCOLL                 iallreduce_impl  (MCAST_INTERFACE mcast_iface,
                                                                const void        * s,
                                                                void              * d, 
                                                                xmi_op              op,
                                                                xmi_dt              dtype, 
                                                                unsigned            nelems,
                                                                void (*cb_complete)(void *)=NULL,
                                                                void *arg=NULL)
                {

                }
            inline void                       allreduce_impl   (MCAST_INTERFACE mcast_iface,
                                                                const void        * s, 
                                                                void              * d, 
                                                                xmi_op              op,
                                                                xmi_dt              dtype, 
                                                                unsigned            nelems,
                                                                void (*cb_complete)(void *)=NULL,
                                                                void *arg=NULL)
                {

                }
            inline PGASNBCOLL                 iscatter_impl   (MCAST_INTERFACE info_barrier,
                                                               MCAST_INTERFACE info_scatter,
                                                               int root, const void *s, void *d,
                                                               size_t l,
                                                               void (*cb_complete)(void *)=NULL,
                                                               void *arg=NULL)
                {

                }
            inline void                       scatter_impl    (MCAST_INTERFACE info_barrier,
                                                               MCAST_INTERFACE info_scatter,
                                                               int root, const void *s, void *d,
                                                               size_t l,
                                                               void (*cb_complete)(void *)=NULL,
                                                               void *arg=NULL)
                {

                }
            inline PGASNBCOLL                 iscatterv_impl  (MCAST_INTERFACE info_barrier,
                                                               MCAST_INTERFACE info_scatter,
                                                               int root, const void *s, void *d,
                                                               size_t *l,
                                                               void (*cb_complete)(void *)=NULL,
                                                               void *arg=NULL)
                {

                }
            inline void                       scatterv_impl   (MCAST_INTERFACE info_barrier,
                                                               MCAST_INTERFACE info_scatter,
                                                               int root, const void *s, void *d,
                                                               size_t *l,
                                                               void (*cb_complete)(void *)=NULL,
                                                               void *arg=NULL)
                {

                }
            inline void                       gather_impl     (MCAST_INTERFACE mcast_iface,
                                                               int root, const void *s,
                                                               void *d, size_t l)
                {

                }
            inline void                       gatherv_impl    (MCAST_INTERFACE mcast_iface,
                                                               int root, const void *s,
                                                               void *d, size_t *l)
                {

                }
            inline void                       nbwait_impl     (PGASNBCOLL)
                {

                }

            
        }; // class Geometry
    };  // namespace Geometry
}; // namespace XMI


#endif
