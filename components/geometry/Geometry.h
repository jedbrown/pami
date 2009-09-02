#ifndef   __xmi_geometry__h__
#define   __xmi_geometry__h__

#include "sys/xmi.h"
#include "util/queue/MatchQueue.h"
//#include "algorithms/schedule/Rectangle.h"

#define RECTANGLE_TYPE void*
//#define RECTANGLE_TYPE CCMI::Schedule::Rectangle*

#define EXECUTOR_TYPE  void*
//#define EXECUTOR_TYPE  CCMI::Executor::Executor*

#define CCMI_EXECUTOR_TYPE void*
//#define CCMI_EXECUTOR_TYPE CCMI_Executor_t*

#define COMPOSITE_TYPE void*
//#define COMPOSITE_TYPE CCMI::Executor::Composite*

#define CCMI_GEOMETRY void*
//#define CCMI_GEOMETRY CCMI_Geometry_t*

#define COLLECTIVE_MAPPING void*
//#define COLLECTIVE_MAPPING XMI::CollectiveMapping*

#define MCAST_INTERFACE void*
//#define MCAST_INTERFACE CCMI::MultiSend::OldMulticastInterface *

#define PGASNBCOLL void*
//#define PGASNBCOLL NBColl*

namespace XMI
{
    namespace Geometry
    {
        template <class T_Geometry>
        class Geometry
        {
        public:
            inline Geometry(COLLECTIVE_MAPPING  *mapping,
                            unsigned            *ranks,
                            unsigned             nranks,
                            unsigned             comm,
                            unsigned             numcolors,
                            bool                 globalcontext);
            // These methods were originally from the CCMI Geometry class
            inline int                        getColorsArray();
            inline void                       setAsyncAllreduceMode(unsigned value);
            inline unsigned                   getAsyncAllreduceMode();
            inline unsigned                   incrementAllreduceIteration();
            inline unsigned                   comm();
            inline unsigned                  *ranks();
            inline unsigned                   nranks();
            inline int                        myIdx();
            inline void                       generatePermutation();
            inline void                       freePermutation();
            inline unsigned                  *permutation();
            inline bool                       isRectangle();
            inline bool                       isTorus();
            inline bool                       isTree();
            inline bool                       isGlobalContext();
            inline bool                       isGI();
            inline unsigned                   getNumColors();
            inline RECTANGLE_TYPE             rectangle();
            inline RECTANGLE_TYPE             rectangle_mesh();
            inline EXECUTOR_TYPE              getBarrierExecutor();
            inline void                       setBarrierExecutor (EXECUTOR_TYPE bar);
            inline EXECUTOR_TYPE              getLocalBarrierExecutor ();
            inline void                       setLocalBarrierExecutor (EXECUTOR_TYPE bar);
            inline unsigned                   getAllreduceIteration();
            inline CCMI_EXECUTOR_TYPE         getAllreduceCompositeStorage ();
            inline CCMI_EXECUTOR_TYPE         getAllreduceCompositeStorage(unsigned i);
            inline COMPOSITE_TYPE             getAllreduceComposite();
            inline COMPOSITE_TYPE             getAllreduceComposite(unsigned i);
            inline void                       setAllreduceComposite(COMPOSITE_TYPE c);
            inline void                       setAllreduceComposite(COMPOSITE_TYPE c, 
                                                                    unsigned i);
            inline void                       freeAllocations ();
            inline EXECUTOR_TYPE              getCollectiveExecutor (unsigned color=0);
            inline void                       setCollectiveExecutor (EXECUTOR_TYPE exe,
                                                                     unsigned color=0);
            inline void                       setGlobalContext(bool context);
            inline void                       setNumColors(unsigned numcolors);
            inline CCMI::MatchQueue          &asyncBcastPostQ();
            inline CCMI::MatchQueue          &asyncBcastUnexpQ();
#if 0
            static inline CCMI_GEOMETRY       getCachedGeometry (unsigned comm);
            static inline void                updateCachedGeometry (CCMI_GEOMETRY geometry, 
                                                                    unsigned comm);
#endif
            // These methods were originally from the PGASRT Communicator class            
            inline int                        size       (void);
            inline int                        rank       (void);
            inline int                        split      (int color, int rank, int * proclist);
            inline int                        absrankof  (int rank);
            inline int                        virtrankof (int rank);
            inline PGASNBCOLL                 ibarrier    (MCAST_INTERFACE mcast_iface,
                                                           void (*cb_complete)(void *)=NULL,
                                                           void *arg=NULL);
            inline void                       barrier     (MCAST_INTERFACE mcast_iface,
                                                           void (*cb_complete)(void *)=NULL,
                                                           void *arg=NULL);
            inline PGASNBCOLL                 iallgather  (MCAST_INTERFACE mcast_iface,
                                                           const void *s,void *d, size_t l,
                                                           void (*cb_complete)(void *)=NULL,
                                                           void *arg=NULL);
            inline void                       allgather   (MCAST_INTERFACE mcast_iface,
                                                           const void *s, void *d,size_t l,
                                                           void (*cb_complete)(void *)=NULL,
                                                           void *arg=NULL);
            inline PGASNBCOLL                 iallgatherv (MCAST_INTERFACE mcast_iface,
                                                           const void *s, void *d, size_t *l,
                                                           void (*cb_complete)(void *)=NULL,
                                                           void *arg=NULL);
            inline void                       allgatherv  (MCAST_INTERFACE mcast_iface,
                                                           const void *s, void *d, size_t *l,
                                                           void (*cb_complete)(void *)=NULL,
                                                           void *arg=NULL);
            inline PGASNBCOLL                 ibcast      (MCAST_INTERFACE mcast_iface,
                                                           int root, const void *s, void *d,
                                                           size_t l,
                                                           void (*cb_complete)(void *)=NULL,
                                                           void *arg=NULL);
            inline void                       bcast       (MCAST_INTERFACE mcast_iface,
                                                           int root, const void *s, void *d,
                                                           size_t l,
                                                           void (*cb_complete)(void *)=NULL,
                                                           void *arg=NULL);
            inline PGASNBCOLL                 iallreduce  (MCAST_INTERFACE mcast_iface,
                                                           const void        * s,
                                                           void              * d, 
                                                           xmi_op              op,
                                                           xmi_dt              dtype, 
                                                           unsigned            nelems,
                                                           void (*cb_complete)(void *)=NULL,
                                                           void *arg=NULL);
            inline void                       allreduce   (MCAST_INTERFACE mcast_iface,
                                                           const void        * s, 
                                                           void              * d, 
                                                           xmi_op             op,
                                                           xmi_dt             dtype, 
                                                           unsigned            nelems,
                                                           void (*cb_complete)(void *)=NULL,
                                                           void *arg=NULL);
            inline PGASNBCOLL                 iscatter   (MCAST_INTERFACE info_barrier,
                                                          MCAST_INTERFACE info_scatter,
                                                          int root, const void *s, void *d,
                                                          size_t l,
                                                          void (*cb_complete)(void *)=NULL,
                                                          void *arg=NULL);
            inline void                       scatter    (MCAST_INTERFACE info_barrier,
                                                          MCAST_INTERFACE info_scatter,
                                                          int root, const void *s, void *d,
                                                          size_t l,
                                                          void (*cb_complete)(void *)=NULL,
                                                          void *arg=NULL);
            inline PGASNBCOLL                 iscatterv  (MCAST_INTERFACE info_barrier,
                                                          MCAST_INTERFACE info_scatter,
                                                          int root, const void *s, void *d,
                                                          size_t *l,
                                                          void (*cb_complete)(void *)=NULL,
                                                          void *arg=NULL);
            inline void                       scatterv   (MCAST_INTERFACE info_barrier,
                                                          MCAST_INTERFACE info_scatter,
                                                          int root, const void *s, void *d,
                                                          size_t *l,
                                                          void (*cb_complete)(void *)=NULL,
                                                          void *arg=NULL);
            inline void                       gather     (MCAST_INTERFACE mcast_iface,
                                                          int root, const void *s,
                                                          void *d, size_t l);
            inline void                       gatherv    (MCAST_INTERFACE mcast_iface,
                                                          int root, const void *s,
                                                          void *d, size_t *l);
            inline void                       nbwait     (PGASNBCOLL);
        }; // class Geometry

        template <class T_Geometry>
        inline int Geometry<T_Geometry>::getColorsArray()
        {
            return static_cast<T_Geometry*>(this)->getColorsArray_impl();
        }

        template <class T_Geometry>
        inline void Geometry<T_Geometry>::setAsyncAllreduceMode(unsigned value)
        {
            static_cast<T_Geometry*>(this)->setAsyncAllreduceMode_impl(value);
        }

        template <class T_Geometry>
        inline unsigned Geometry<T_Geometry>::getAsyncAllreduceMode()
        {
            return static_cast<T_Geometry*>(this)->getAsyncAllreduceMode_impl();
        }


        template <class T_Geometry>
        inline unsigned Geometry<T_Geometry>::incrementAllreduceIteration()
        {
            return static_cast<T_Geometry*>(this)->incrementAllreduceIteration_impl();
        }

        template <class T_Geometry>
        inline unsigned Geometry<T_Geometry>::comm()
        {
            return static_cast<T_Geometry*>(this)->comm_impl();
        }

        template <class T_Geometry>
        inline unsigned *Geometry<T_Geometry>::ranks()
        {
            return static_cast<T_Geometry*>(this)->ranks_impl();
        }

        template <class T_Geometry>
        inline unsigned Geometry<T_Geometry>::nranks()
        {
            return static_cast<T_Geometry*>(this)->nranks_impl();
        }

        template <class T_Geometry>
        inline int Geometry<T_Geometry>::myIdx()
        {
            return static_cast<T_Geometry*>(this)->myIdx_impl();
        }

        template <class T_Geometry>
        inline void Geometry<T_Geometry>::generatePermutation()
        {
            return static_cast<T_Geometry*>(this)->generatePermutation_impl();
        }

        template <class T_Geometry>
        inline void Geometry<T_Geometry>::freePermutation()
        {
            return static_cast<T_Geometry*>(this)->freePermutation_impl();
        }

        template <class T_Geometry>
        inline unsigned *Geometry<T_Geometry>::permutation()
        {
            return static_cast<T_Geometry*>(this)->permutation_impl();
        }

        template <class T_Geometry>
        inline bool Geometry<T_Geometry>::isRectangle()
        {
            return static_cast<T_Geometry*>(this)->isRectangle_impl();
        }

        template <class T_Geometry>
        inline bool Geometry<T_Geometry>::isTorus()
        {
            return static_cast<T_Geometry*>(this)->isTorus_impl();
        }

        template <class T_Geometry>
        inline bool Geometry<T_Geometry>::isTree()
        {
            return static_cast<T_Geometry*>(this)->isTree_impl();
        }

        template <class T_Geometry>
        inline bool Geometry<T_Geometry>::isGlobalContext()
        {
            return static_cast<T_Geometry*>(this)->isGlobalContext_impl();
        }

        template <class T_Geometry>
        inline bool Geometry<T_Geometry>::isGI()
        {
            return static_cast<T_Geometry*>(this)->isGI_impl();
        }

        template <class T_Geometry>
        inline unsigned Geometry<T_Geometry>::getNumColors()
        {
            return static_cast<T_Geometry*>(this)->getNumColors_impl();
        }

        template <class T_Geometry>
        inline RECTANGLE_TYPE Geometry<T_Geometry>::rectangle()
        {
            return static_cast<T_Geometry*>(this)->rectangle_impl();
        }

        template <class T_Geometry>
        inline RECTANGLE_TYPE Geometry<T_Geometry>::rectangle_mesh()
        {
            return static_cast<T_Geometry*>(this)->rectangle_mesh_impl();
        }

        template <class T_Geometry>
        inline EXECUTOR_TYPE Geometry<T_Geometry>::getBarrierExecutor()
        {
            return static_cast<T_Geometry*>(this)->getBarrierExecutor_impl();
        }

        template <class T_Geometry>
        inline void Geometry<T_Geometry>::setBarrierExecutor (EXECUTOR_TYPE bar)
        {
            return static_cast<T_Geometry*>(this)->setBarrierExecutor_impl(bar);
        }

        template <class T_Geometry>
        inline EXECUTOR_TYPE Geometry<T_Geometry>::getLocalBarrierExecutor ()
        {
            return static_cast<T_Geometry*>(this)->getLocalBarrierExecutor_impl();
        }

        template <class T_Geometry>
        inline void Geometry<T_Geometry>::setLocalBarrierExecutor (EXECUTOR_TYPE bar)
        {
            return static_cast<T_Geometry*>(this)->setLocalBarrierExecutor_impl(bar);
        }

        template <class T_Geometry>
        inline unsigned Geometry<T_Geometry>::getAllreduceIteration()
        {
            return static_cast<T_Geometry*>(this)->getAllreduceIteration_impl();
        }

        template <class T_Geometry>
        inline CCMI_EXECUTOR_TYPE Geometry<T_Geometry>::getAllreduceCompositeStorage ()
        {
            return static_cast<T_Geometry*>(this)->getAllreduceCompositeStorage_impl();
        }

        template <class T_Geometry>
        inline CCMI_EXECUTOR_TYPE Geometry<T_Geometry>::getAllreduceCompositeStorage(unsigned i)
        {
            return static_cast<T_Geometry*>(this)->getAllreduceCompositeStorage_impl(i);
        }

        template <class T_Geometry>
        inline COMPOSITE_TYPE Geometry<T_Geometry>::getAllreduceComposite()
        {
            return static_cast<T_Geometry*>(this)->_impl();
        }

        template <class T_Geometry>
        inline COMPOSITE_TYPE Geometry<T_Geometry>::getAllreduceComposite(unsigned i)
        {
            return static_cast<T_Geometry*>(this)->getAllreduceComposite_impl(i);
        }

        template <class T_Geometry>
        inline void Geometry<T_Geometry>::setAllreduceComposite(COMPOSITE_TYPE c)
        {
            return static_cast<T_Geometry*>(this)->setAllreduceComposite_impl(c);
        }

        template <class T_Geometry>
        inline void Geometry<T_Geometry>::setAllreduceComposite(COMPOSITE_TYPE c, 
                                                                unsigned i)
        {
            return static_cast<T_Geometry*>(this)->setAllreduceComposite_impl(c, i);
        }

        template <class T_Geometry>
        inline void Geometry<T_Geometry>::freeAllocations ()
        {
            return static_cast<T_Geometry*>(this)->freeAllocations_impl();
        }

        template <class T_Geometry>
        inline EXECUTOR_TYPE Geometry<T_Geometry>::getCollectiveExecutor (unsigned color)
        {
            return static_cast<T_Geometry*>(this)->getCollectiveExecutor_impl(color);
        }

        template <class T_Geometry>
        inline void Geometry<T_Geometry>::setCollectiveExecutor (EXECUTOR_TYPE exe,
                                                                 unsigned color)
        {
            return static_cast<T_Geometry*>(this)->setCollectiveExecutor_impl(exe, color);
        }

        template <class T_Geometry>
        inline void Geometry<T_Geometry>::setGlobalContext(bool context)
        {
            return static_cast<T_Geometry*>(this)->setGlobalContext_impl(context);
        }

        template <class T_Geometry>
        inline void Geometry<T_Geometry>::setNumColors(unsigned numcolors)
        {
            return static_cast<T_Geometry*>(this)->setNumColors_impl(numcolors);
        }

        template <class T_Geometry>
        inline CCMI::MatchQueue &Geometry<T_Geometry>::asyncBcastPostQ()
        {
            return static_cast<T_Geometry*>(this)->asyncBcastPostQ_impl();
        }

        template <class T_Geometry>
        inline CCMI::MatchQueue &Geometry<T_Geometry>::asyncBcastUnexpQ()
        {
            return static_cast<T_Geometry*>(this)->asyncBcastUnexpQ_impl();
        }
#if 0
        template <class T_Geometry>
        static inline CCMI_GEOMETRY Geometry<T_Geometry>::getCachedGeometry (unsigned comm)
        {
            return static_cast<T_Geometry*>(this)->getCachedGeometry_impl(comm);
        }

        template <class T_Geometry>
        static inline void Geometry<T_Geometry>::updateCachedGeometry (CCMI_GEOMETRY geometry, 
                                                                       unsigned comm)
        {
            return static_cast<T_Geometry*>(this)->updateCachedGeometry_impl(geometry, comm);
        }
#endif

        // These methods were originally from the PGASRT Communicator class
        template <class T_Geometry>
        inline int                        Geometry<T_Geometry>::size       (void)
        {
            return static_cast<T_Geometry*>(this)->size_impl();
        }
        template <class T_Geometry>
        inline int                        Geometry<T_Geometry>::rank       (void)
        {
            return static_cast<T_Geometry*>(this)->rank_impl();
        }
        template <class T_Geometry>
        inline int                        Geometry<T_Geometry>::split      (int color, int rank, int * proclist)
        {
            return static_cast<T_Geometry*>(this)->split_impl();
        }
        template <class T_Geometry>
        inline int                        Geometry<T_Geometry>::absrankof  (int rank)
        {
            return static_cast<T_Geometry*>(this)->absrankof_impl();
        }
        template <class T_Geometry>
        inline int                        Geometry<T_Geometry>::virtrankof (int rank)
        {
            return static_cast<T_Geometry*>(this)->virtrankof_impl();
        }
        template <class T_Geometry>
        inline PGASNBCOLL                 Geometry<T_Geometry>::ibarrier    (MCAST_INTERFACE mcast_iface,
                                                                             void (*cb_complete)(void *),
                                                                             void *arg)
        {
            return static_cast<T_Geometry*>(this)->ibarrier_impl();
        }
        template <class T_Geometry>
        inline void                       Geometry<T_Geometry>::barrier     (MCAST_INTERFACE mcast_iface,
                                                                             void (*cb_complete)(void *),
                                                                             void *arg)
        {
            return static_cast<T_Geometry*>(this)->barrier_impl();
        }
        template <class T_Geometry>
        inline PGASNBCOLL                 Geometry<T_Geometry>::iallgather  (MCAST_INTERFACE mcast_iface,
                                                                             const void *s,void *d, size_t l,
                                                                             void (*cb_complete)(void *),
                                                                             void *arg)
        {
            return static_cast<T_Geometry*>(this)->iallgather_impl();
        }
        template <class T_Geometry>
        inline void                       Geometry<T_Geometry>::allgather   (MCAST_INTERFACE mcast_iface,
                                                                             const void *s, void *d,size_t l,
                                                                             void (*cb_complete)(void *),
                                                                             void *arg)
        {
            return static_cast<T_Geometry*>(this)->allgather_impl();
        }
        template <class T_Geometry>
        inline PGASNBCOLL                 Geometry<T_Geometry>::iallgatherv (MCAST_INTERFACE mcast_iface,
                                                                             const void *s, void *d, size_t *l,
                                                                             void (*cb_complete)(void *),
                                                                             void *arg)
        {
            return static_cast<T_Geometry*>(this)->iallgatherv_impl();
        }
        template <class T_Geometry>
        inline void                       Geometry<T_Geometry>::allgatherv  (MCAST_INTERFACE mcast_iface,
                                                                             const void *s, void *d, size_t *l,
                                                                             void (*cb_complete)(void *),
                                                                             void *arg)
        {
            return static_cast<T_Geometry*>(this)->allgatherv_impl();
        }
        template <class T_Geometry>
        inline PGASNBCOLL                 Geometry<T_Geometry>::ibcast      (MCAST_INTERFACE mcast_iface,
                                                                             int root, const void *s, void *d,
                                                                             size_t l,
                                                                             void (*cb_complete)(void *),
                                                                             void *arg)
        {
            return static_cast<T_Geometry*>(this)->ibcast_impl();
        }
        template <class T_Geometry>
        inline void                       Geometry<T_Geometry>::bcast       (MCAST_INTERFACE mcast_iface,
                                                                             int root, const void *s, void *d,
                                                                             size_t l,
                                                                             void (*cb_complete)(void *),
                                                                             void *arg)
        {
            return static_cast<T_Geometry*>(this)->bcast_impl();
        }
        template <class T_Geometry>
        inline PGASNBCOLL                 Geometry<T_Geometry>::iallreduce  (MCAST_INTERFACE mcast_iface,
                                                                             const void        * s,
                                                                             void              * d, 
                                                                             xmi_op              op,
                                                                             xmi_dt              dtype, 
                                                                             unsigned            nelems,
                                                                             void (*cb_complete)(void *),
                                                                             void *arg)
        {
            return static_cast<T_Geometry*>(this)->iallreduce_impl();
        }
        template <class T_Geometry>
        inline void                       Geometry<T_Geometry>::allreduce   (MCAST_INTERFACE mcast_iface,
                                                                             const void        * s, 
                                                                             void              * d, 
                                                                             xmi_op             op,
                                                                             xmi_dt             dtype, 
                                                                             unsigned           nelems,
                                                                             void (*cb_complete)(void *),
                                                                             void *arg)
        {
            return static_cast<T_Geometry*>(this)->allreduce_impl();
        }
        template <class T_Geometry>
        inline PGASNBCOLL                 Geometry<T_Geometry>::iscatter   (MCAST_INTERFACE info_barrier,
                                                                            MCAST_INTERFACE info_scatter,
                                                                            int root, const void *s, void *d,
                                                                            size_t l,
                                                                            void (*cb_complete)(void *),
                                                                            void *arg)
        {
            return static_cast<T_Geometry*>(this)->iscatter_impl();
        }
        template <class T_Geometry>
        inline void                       Geometry<T_Geometry>::scatter    (MCAST_INTERFACE info_barrier,
                                                                            MCAST_INTERFACE info_scatter,
                                                                            int root, const void *s, void *d,
                                                                            size_t l,
                                                                            void (*cb_complete)(void *),
                                                                            void *arg)
        {
            return static_cast<T_Geometry*>(this)->scatter_impl();
        }
        template <class T_Geometry>
        inline PGASNBCOLL                 Geometry<T_Geometry>::iscatterv  (MCAST_INTERFACE info_barrier,
                                                                            MCAST_INTERFACE info_scatter,
                                                                            int root, const void *s, void *d,
                                                                            size_t *l,
                                                                            void (*cb_complete)(void *),
                                                                            void *arg)
        {
            return static_cast<T_Geometry*>(this)->iscatterv_impl();
        }
        template <class T_Geometry>
        inline void                       Geometry<T_Geometry>::scatterv   (MCAST_INTERFACE info_barrier,
                                                                            MCAST_INTERFACE info_scatter,
                                                                            int root, const void *s, void *d,
                                                                            size_t *l,
                                                                            void (*cb_complete)(void *),
                                                                            void *arg)
        {
            return static_cast<T_Geometry*>(this)->scatterv_impl();
        }
        template <class T_Geometry>
        inline void                       Geometry<T_Geometry>::gather     (MCAST_INTERFACE mcast_iface,
                                                                            int root, const void *s,
                                                                            void *d, size_t l)
        {
            return static_cast<T_Geometry*>(this)->gather_impl();
        }
        template <class T_Geometry>
        inline void                       Geometry<T_Geometry>::gatherv    (MCAST_INTERFACE mcast_iface,
                                                                            int root, const void *s,
                                                                            void *d, size_t *l)
        {
            return static_cast<T_Geometry*>(this)->gatherv_impl();
        }
        template <class T_Geometry>
        inline void                       Geometry<T_Geometry>::nbwait     (PGASNBCOLL)
        {
            return static_cast<T_Geometry*>(this)->nbwait_impl();
        }

        

    }; // namespace Geometry
}; // namespace XMI

#endif
