/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/interfaces/GeometryInterface.h
 * \brief ???
 */

#ifndef __algorithms_interfaces_GeometryInterface_h__
#define __algorithms_interfaces_GeometryInterface_h__

#include "sys/xmi.h"
#include "util/compact_attributes.h"
#include "util/queue/MatchQueue.h"
//  Need to make this work with config.h!
//  hitting some circ dependencies in ccmi...need to solve
//.#include "components/mapping/mpi/mpimapping.h"


//#include "config.h"

#if 0
#define RECTANGLE_TYPE void*
//#define RECTANGLE_TYPE CCMI::Schedule::Rectangle*

#define EXECUTOR_TYPE  void*
//#define EXECUTOR_TYPE  CCMI::Executor::Executor*

//#define CCMI_EXECUTOR_TYPE CCMI_Executor_t*

//#define COMPOSITE_TYPE CCMI::Executor::Composite*

#define CCMI_GEOMETRY void*
//#define CCMI_GEOMETRY CCMI_Geometry_t*
#endif

#define CCMI_EXECUTOR_TYPE void*
#define COMPOSITE_TYPE void*





namespace XMI
{
  namespace Geometry
  {
    typedef enum
    {
      XMI_GKEY_COLLFACTORY     = 0,
      XMI_GKEY_BARRIEREXECUTOR,
      XMI_GKEY_LOCALBARRIEREXECUTOR
    }keys_t;



    template <class T_Geometry, class T_Mapping>
      class Geometry
    {
    public:
      inline Geometry(T_Mapping   *mapping,
                      xmi_task_t  *ranks,
                      xmi_task_t  nranks,
                      unsigned     comm,
                      unsigned     numcolors,
                      bool         globalcontext)
      {
      }
      inline Geometry (T_Mapping *mapping,
                       unsigned   comm,
                       int        numranges,
                       xmi_geometry_range_t rangelist[])
      {
      }

      // These methods were originally from the CCMI Geometry class
      inline int                        getColorsArray();
      inline void                       setAsyncAllreduceMode(unsigned value);
      inline unsigned                   getAsyncAllreduceMode();
      inline unsigned                   incrementAllreduceIteration();
      inline unsigned                   comm();
      inline xmi_task_t                 *ranks();
      inline xmi_task_t                 *ranks_sizet();
      inline xmi_task_t                 nranks();
      inline xmi_task_t                 myIdx();
      inline void                       generatePermutation();
      inline void                       freePermutation();
      inline unsigned                  *permutation();
      inline xmi_topology_t* getTopology(int topo_num);
      inline int myTopologyId();
      inline int getNumTopos();
      inline void                       generatePermutation_sizet();
      inline void                       freePermutation_sizet();
      inline xmi_task_t                 *permutation_sizet();
/** \todo  need to replace by attributes */
#if 1
      // no need for these as they are embedded inside attributes
      inline bool                       isRectangle();
      inline bool                       isTorus();
      inline bool                       isTree();
      inline bool                       isGlobalContext();
      inline bool                       isGI();
#endif
      inline unsigned                   getNumColors();
      inline unsigned                   getAllreduceIteration();
      inline void                       freeAllocations ();
      inline void                       setGlobalContext(bool context);
      inline void                       setNumColors(unsigned numcolors);
      inline MatchQueue                &asyncBcastPostQ();
      inline MatchQueue                &asyncBcastUnexpQ();

      // These are CCMI typed methods that introduce CCMI dependencies on
      // the geometry interface
      // Do these belong in some kind of cache object or elsewhere?
#if 0
      inline void                       setBarrierExecutor (EXECUTOR_TYPE bar);
      inline RECTANGLE_TYPE             rectangle();
      inline RECTANGLE_TYPE             rectangle_mesh();
      inline EXECUTOR_TYPE              getLocalBarrierExecutor ();
      inline void                       setLocalBarrierExecutor (EXECUTOR_TYPE bar);
      inline EXECUTOR_TYPE              getCollectiveExecutor (unsigned color=0);
      inline void                       setCollectiveExecutor (EXECUTOR_TYPE exe,
                                                               unsigned color=0);
      inline void                      *getBarrierExecutor();
#endif

      inline CCMI_EXECUTOR_TYPE         getAllreduceCompositeStorage(unsigned i);
      inline CCMI_EXECUTOR_TYPE         getAllreduceCompositeStorage ();
      inline COMPOSITE_TYPE             getAllreduceComposite(unsigned i);
      inline COMPOSITE_TYPE             getAllreduceComposite();
      inline void                       setAllreduceComposite(COMPOSITE_TYPE c);
      inline void                       setAllreduceComposite(COMPOSITE_TYPE c,
                                                              unsigned i);


      static inline T_Geometry         *getCachedGeometry (unsigned comm);
      static inline void                updateCachedGeometry (T_Geometry *geometry,
                                                              unsigned comm);

      // These methods were originally from the PGASRT Communicator class
      inline xmi_task_t                 size       (void);
      inline xmi_task_t                 rank       (void);
      inline xmi_task_t                 virtrank   (void);
      inline xmi_task_t                 absrankof  (int rank);
      inline xmi_task_t                 virtrankof (int rank);
      inline void                       setKey(keys_t key, void*value);
      inline void                      *getKey(keys_t key);
    }; // class Geometry

    template <class T_Geometry, class T_Mapping>
      inline int Geometry<T_Geometry, T_Mapping>::getColorsArray()
    {
      return static_cast<T_Geometry*>(this)->getColorsArray_impl();
    }


    template <class T_Geometry, class T_Mapping>
      inline xmi_topology_t* Geometry<T_Geometry, T_Mapping>::getTopology(int topo_num)
    {
      return static_cast<T_Geometry*>(this)->getTopology_impl(topo_num);
    }
    template <class T_Geometry, class T_Mapping>
      inline int Geometry<T_Geometry, T_Mapping>::myTopologyId()
    {
      return static_cast<T_Geometry*>(this)->myTopologyId_impl();
    }
    template <class T_Geometry, class T_Mapping>
      inline int Geometry<T_Geometry, T_Mapping>::getNumTopos()
    {
      return static_cast<T_Geometry*>(this)->getNumTopos_impl();
    }


    template <class T_Geometry, class T_Mapping>
      inline void Geometry<T_Geometry, T_Mapping>::setAsyncAllreduceMode(unsigned value)
    {
      static_cast<T_Geometry*>(this)->setAsyncAllreduceMode_impl(value);
    }

    template <class T_Geometry, class T_Mapping>
      inline unsigned Geometry<T_Geometry, T_Mapping>::getAsyncAllreduceMode()
    {
      return static_cast<T_Geometry*>(this)->getAsyncAllreduceMode_impl();
    }


    template <class T_Geometry, class T_Mapping>
      inline unsigned Geometry<T_Geometry, T_Mapping>::incrementAllreduceIteration()
    {
      return static_cast<T_Geometry*>(this)->incrementAllreduceIteration_impl();
    }

    template <class T_Geometry, class T_Mapping>
      inline unsigned Geometry<T_Geometry, T_Mapping>::comm()
    {
      return static_cast<T_Geometry*>(this)->comm_impl();
    }

    template <class T_Geometry, class T_Mapping>
      inline xmi_task_t *Geometry<T_Geometry, T_Mapping>::ranks()
    {
      return static_cast<T_Geometry*>(this)->ranks_impl();
    }

    template <class T_Geometry, class T_Mapping>
      inline xmi_task_t *Geometry<T_Geometry, T_Mapping>::ranks_sizet()
    {
      return static_cast<T_Geometry*>(this)->ranks_sizet_impl();
    }

    template <class T_Geometry, class T_Mapping>
      inline xmi_task_t Geometry<T_Geometry, T_Mapping>::nranks()
    {
      return static_cast<T_Geometry*>(this)->nranks_impl();
    }

    template <class T_Geometry, class T_Mapping>
      inline xmi_task_t Geometry<T_Geometry, T_Mapping>::myIdx()
    {
      return static_cast<T_Geometry*>(this)->myIdx_impl();
    }

    template <class T_Geometry, class T_Mapping>
      inline void Geometry<T_Geometry, T_Mapping>::generatePermutation()
    {
      return static_cast<T_Geometry*>(this)->generatePermutation_impl();
    }

    template <class T_Geometry, class T_Mapping>
      inline void Geometry<T_Geometry, T_Mapping>::freePermutation()
    {
      return static_cast<T_Geometry*>(this)->freePermutation_impl();
    }

    template <class T_Geometry, class T_Mapping>
      inline xmi_task_t *Geometry<T_Geometry, T_Mapping>::permutation()
    {
      return static_cast<T_Geometry*>(this)->permutation_impl();
    }

    template <class T_Geometry, class T_Mapping>
      inline void Geometry<T_Geometry, T_Mapping>::generatePermutation_sizet()
    {
      return static_cast<T_Geometry*>(this)->generatePermutation_sizet_impl();
    }

    template <class T_Geometry, class T_Mapping>
      inline void Geometry<T_Geometry, T_Mapping>::freePermutation_sizet()
    {
      return static_cast<T_Geometry*>(this)->freePermutation_sizet_impl();
    }

    template <class T_Geometry, class T_Mapping>
      inline xmi_task_t *Geometry<T_Geometry, T_Mapping>::permutation_sizet()
    {
      return static_cast<T_Geometry*>(this)->permutation_sizet_impl();
    }

/** \todo replace by attributes */
#if 1
    // to be removed eventually
    template <class T_Geometry, class T_Mapping>
      inline bool Geometry<T_Geometry, T_Mapping>::isRectangle()
    {
      return static_cast<T_Geometry*>(this)->isRectangle_impl();
    }

    template <class T_Geometry, class T_Mapping>
      inline bool Geometry<T_Geometry, T_Mapping>::isTorus()
    {
      return static_cast<T_Geometry*>(this)->isTorus_impl();
    }

    template <class T_Geometry, class T_Mapping>
      inline bool Geometry<T_Geometry, T_Mapping>::isTree()
    {
      return static_cast<T_Geometry*>(this)->isTree_impl();
    }

    template <class T_Geometry, class T_Mapping>
      inline bool Geometry<T_Geometry, T_Mapping>::isGlobalContext()
    {
      return static_cast<T_Geometry*>(this)->isGlobalContext_impl();
    }

    template <class T_Geometry, class T_Mapping>
      inline bool Geometry<T_Geometry, T_Mapping>::isGI()
    {
      return static_cast<T_Geometry*>(this)->isGI_impl();
    }
#endif
    template <class T_Geometry, class T_Mapping>
      inline unsigned Geometry<T_Geometry, T_Mapping>::getNumColors()
    {
      return static_cast<T_Geometry*>(this)->getNumColors_impl();
    }

    template <class T_Geometry, class T_Mapping>
      inline unsigned Geometry<T_Geometry, T_Mapping>::getAllreduceIteration()
    {
      return static_cast<T_Geometry*>(this)->getAllreduceIteration_impl();
    }

    template <class T_Geometry, class T_Mapping>
      inline void Geometry<T_Geometry, T_Mapping>::freeAllocations ()
    {
      return static_cast<T_Geometry*>(this)->freeAllocations_impl();
    }
    template <class T_Geometry, class T_Mapping>
      inline void Geometry<T_Geometry, T_Mapping>::setGlobalContext(bool context)
    {
      return static_cast<T_Geometry*>(this)->setGlobalContext_impl(context);
    }

    template <class T_Geometry, class T_Mapping>
      inline void Geometry<T_Geometry, T_Mapping>::setNumColors(unsigned numcolors)
    {
      return static_cast<T_Geometry*>(this)->setNumColors_impl(numcolors);
    }

    template <class T_Geometry, class T_Mapping>
      inline MatchQueue &Geometry<T_Geometry, T_Mapping>::asyncBcastPostQ()
    {
      return static_cast<T_Geometry*>(this)->asyncBcastPostQ_impl();
    }

    template <class T_Geometry, class T_Mapping>
      inline MatchQueue &Geometry<T_Geometry, T_Mapping>::asyncBcastUnexpQ()
    {
      return static_cast<T_Geometry*>(this)->asyncBcastUnexpQ_impl();
    }
#if 0
    template <class T_Geometry, class T_Mapping>
      inline RECTANGLE_TYPE Geometry<T_Geometry, T_Mapping>::rectangle()
    {
      return static_cast<T_Geometry*>(this)->rectangle_impl();
    }

    template <class T_Geometry, class T_Mapping>
      inline RECTANGLE_TYPE Geometry<T_Geometry, T_Mapping>::rectangle_mesh()
    {
      return static_cast<T_Geometry*>(this)->rectangle_mesh_impl();
    }

    template <class T_Geometry, class T_Mapping>
      inline void Geometry<T_Geometry, T_Mapping>::setBarrierExecutor (EXECUTOR_TYPE bar)
    {
      return static_cast<T_Geometry*>(this)->setBarrierExecutor_impl(bar);
    }

    template <class T_Geometry, class T_Mapping>
      inline EXECUTOR_TYPE Geometry<T_Geometry, T_Mapping>::getLocalBarrierExecutor ()
    {
      return static_cast<T_Geometry*>(this)->getLocalBarrierExecutor_impl();
    }

    template <class T_Geometry, class T_Mapping>
      inline void Geometry<T_Geometry, T_Mapping>::setLocalBarrierExecutor (EXECUTOR_TYPE bar)
    {
      return static_cast<T_Geometry*>(this)->setLocalBarrierExecutor_impl(bar);
    }

    template <class T_Geometry, class T_Mapping>
      inline EXECUTOR_TYPE Geometry<T_Geometry, T_Mapping>::getCollectiveExecutor (unsigned color)
    {
      return static_cast<T_Geometry*>(this)->getCollectiveExecutor_impl(color);
    }

    template <class T_Geometry, class T_Mapping>
      inline void Geometry<T_Geometry, T_Mapping>::setCollectiveExecutor (EXECUTOR_TYPE exe,
                                                                          unsigned color)
    {
      return static_cast<T_Geometry*>(this)->setCollectiveExecutor_impl(exe, color);
    }

    template <class T_Geometry, class T_Mapping>
      inline void * Geometry<T_Geometry, T_Mapping>::getBarrierExecutor()
    {
      return static_cast<T_Geometry*>(this)->getBarrierExecutor_impl();
    }

#endif
    template <class T_Geometry, class T_Mapping>
      inline CCMI_EXECUTOR_TYPE Geometry<T_Geometry, T_Mapping>::getAllreduceCompositeStorage(unsigned i)
    {
      return static_cast<T_Geometry*>(this)->getAllreduceCompositeStorage_impl(i);
    }

    template <class T_Geometry, class T_Mapping>
      inline COMPOSITE_TYPE Geometry<T_Geometry, T_Mapping>::getAllreduceComposite(unsigned i)
    {
      return static_cast<T_Geometry*>(this)->getAllreduceComposite_impl(i);
    }

    template <class T_Geometry, class T_Mapping>
      inline void Geometry<T_Geometry, T_Mapping>::setAllreduceComposite(COMPOSITE_TYPE c)
    {
      return static_cast<T_Geometry*>(this)->setAllreduceComposite_impl(c);
    }

    template <class T_Geometry, class T_Mapping>
      inline void Geometry<T_Geometry, T_Mapping>::setAllreduceComposite(COMPOSITE_TYPE c,
                                                                         unsigned i)
    {
      return static_cast<T_Geometry*>(this)->setAllreduceComposite_impl(c, i);
    }

    template <class T_Geometry, class T_Mapping>
      inline CCMI_EXECUTOR_TYPE Geometry<T_Geometry, T_Mapping>::getAllreduceCompositeStorage ()
    {
      return static_cast<T_Geometry*>(this)->getAllreduceCompositeStorage_impl();
    }

    template <class T_Geometry, class T_Mapping>
      inline COMPOSITE_TYPE Geometry<T_Geometry, T_Mapping>::getAllreduceComposite()
    {
      return static_cast<T_Geometry*>(this)->getAllreduceComposite_impl();
    }

    template <class T_Geometry, class T_Mapping>
      inline T_Geometry *Geometry<T_Geometry, T_Mapping>::getCachedGeometry (unsigned comm)
    {
      return T_Geometry::getCachedGeometry_impl(comm);
    }

    template <class T_Geometry, class T_Mapping>
      inline void Geometry<T_Geometry, T_Mapping>::updateCachedGeometry (T_Geometry *geometry,
                                                                         unsigned comm)
    {
      return T_Geometry::updateCachedGeometry_impl(geometry, comm);
    }

    // These methods were originally from the PGASRT Communicator class
    template <class T_Geometry, class T_Mapping>
      inline xmi_task_t  Geometry<T_Geometry, T_Mapping>::size       (void)
    {
      return static_cast<T_Geometry*>(this)->size_impl();
    }
    template <class T_Geometry, class T_Mapping>
      inline xmi_task_t  Geometry<T_Geometry, T_Mapping>::rank       (void)
    {
      return static_cast<T_Geometry*>(this)->rank_impl();
    }
    template <class T_Geometry, class T_Mapping>
      inline xmi_task_t   Geometry<T_Geometry, T_Mapping>::virtrank()
    {
      return static_cast<T_Geometry*>(this)->virtrank_impl();
    }
    template <class T_Geometry, class T_Mapping>
      inline xmi_task_t  Geometry<T_Geometry, T_Mapping>::absrankof  (int rank)
    {
      return static_cast<T_Geometry*>(this)->absrankof_impl(rank);
    }
    template <class T_Geometry, class T_Mapping>
      inline xmi_task_t Geometry<T_Geometry, T_Mapping>::virtrankof (int rank)
    {
      return static_cast<T_Geometry*>(this)->virtrankof_impl(rank);
    }
    template <class T_Geometry, class T_Mapping>
      inline void                        Geometry<T_Geometry, T_Mapping>::setKey (keys_t key, void *value)
    {
      static_cast<T_Geometry*>(this)->setKey_impl(key, value);
    }
    template <class T_Geometry, class T_Mapping>
      inline void*                       Geometry<T_Geometry, T_Mapping>::getKey (keys_t key)
    {
      return static_cast<T_Geometry*>(this)->getKey_impl(key);
    }



  }; // namespace Geometry
}; // namespace XMI

#endif
