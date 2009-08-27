/**
 * \file components/Geometry.h

 * \brief Geometry object
 * This object is a message layer approximation of communicators
 *  - Stores ranks and number of ranks in this particular group
 *  - Stores an ID which must be the same on all the ranks in the group
 *  - Has flags for tree, GI, rectangle, and torus
 *  - Has flag for global context - needed in MPI to determine if this is
 *    comm_world or a comm dupe of comm_world in multi-threaded environments
 *  - Keeps a barrier for this group of nodes
 */

#ifndef   __xmi_geometry__h__
#define   __xmi_geometry__h__

#include "sys/xmi.h"
#include "util/queue/MatchQueue.h"

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

    }; // namespace Geometry
}; // namespace XMI

#endif
