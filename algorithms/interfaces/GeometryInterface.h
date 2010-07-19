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

#include <pami.h>
#include "util/compact_attributes.h"
#include "util/queue/MatchQueue.h"
#include "Mapping.h"
#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include "GeometryPlatform.h"

#define CCMI_EXECUTOR_TYPE void*
#define COMPOSITE_TYPE void*

#undef TRACE_ERR
#define TRACE_ERR(x) //fprintf x

namespace PAMI
{
  namespace Geometry
  {
    typedef enum
    {
      PAMI_GKEY_COLLFACTORY     = 0,
      PAMI_GKEY_BARRIERCOMPOSITE0,     // ?
      PAMI_GKEY_BARRIERCOMPOSITE1,     // ?
      PAMI_GKEY_LOCALBARRIERCOMPOSITE, // local sub-topologies only in the geometry
      PAMI_GKEY_GLOBALBARRIERCOMPOSITE, // global sub-topologes only in the geometry
      PAMI_GKEY_PLATEXTENSIONS
      PAMI_GKEY_GEOMETRYCSNI           // native interface for coll shm device
    }keys_t;

    template <class T_Geometry>
      class Geometry
    {
    public:
      inline Geometry(Mapping     *mapping,
                      pami_task_t  *ranks,
                      pami_task_t  nranks,
                      unsigned     comm,
                      unsigned     numcolors,
                      bool         globalcontext)
      {
        TRACE_ERR((stderr, "<%p>%s\n", this, __PRETTY_FUNCTION__));
      }
      inline Geometry (Geometry  *parent,
                       Mapping   *mapping,
                       unsigned   comm,
                       int        numranges,
                       pami_geometry_range_t rangelist[])
      {
        TRACE_ERR((stderr, "<%p>%s\n", this, __PRETTY_FUNCTION__));
      }

      // These methods were originally from the CCMI Geometry class
      inline int                        getColorsArray();
      inline void                       setAsyncAllreduceMode(unsigned value);
      inline unsigned                   getAsyncAllreduceMode();
      inline unsigned                   incrementAllreduceIteration();
      inline unsigned                   comm();
      inline pami_task_t                 *ranks();
      inline pami_task_t                 *ranks_sizet();
      inline pami_task_t                 nranks();
      inline pami_task_t                 myIdx();
      inline void                       generatePermutation();
      inline void                       freePermutation();
      inline unsigned                  *permutation();
      inline pami_topology_t* getTopology(int topo_num);
      inline int myTopologyId();
      inline int getNumTopos();
      inline void                       generatePermutation_sizet();
      inline void                       freePermutation_sizet();
      inline pami_task_t                 *permutation_sizet();
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
      inline MatchQueue                &asyncCollectivePostQ();
      inline MatchQueue                &asyncCollectiveUnexpQ();

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
      inline pami_task_t                 size       (void);
      inline pami_task_t                 rank       (void);
      inline pami_task_t                 virtrank   (void);
      inline pami_task_t                 absrankof  (int rank);
      inline pami_task_t                 virtrankof (int rank);
      inline void                       setKey(keys_t key, void*value);
      inline void                      *getKey(keys_t key);

      // API support
      inline pami_result_t algorithms_num(pami_xfer_type_t  colltype,
                                         size_t             *lists_lengths,
                                         size_t           context_id);

      inline pami_result_t algorithms_info (pami_xfer_type_t   colltype,
                                           pami_algorithm_t  *algs0,
                                           pami_metadata_t   *mdata0,
                                           size_t               num0,
                                           pami_algorithm_t  *algs1,
                                           pami_metadata_t   *mdata1,
                                           size_t               num1,
                                           size_t            context_id);



      // List management
      inline pami_result_t addCollective(pami_xfer_type_t                            xfer_type,
                                        CCMI::Adaptor::CollectiveProtocolFactory  *factory,
                                        size_t                                     context_id);

      inline pami_result_t addCollectiveCheck(pami_xfer_type_t                            xfer_type,
                                             CCMI::Adaptor::CollectiveProtocolFactory  *factory,
                                             size_t                                     context_id);



    }; // class Geometry

    template <class T_Geometry>
    inline int Geometry<T_Geometry>::getColorsArray()
    {
      return static_cast<T_Geometry*>(this)->getColorsArray_impl();
    }


    template <class T_Geometry>
    inline pami_topology_t* Geometry<T_Geometry>::getTopology(int topo_num)
    {
      return static_cast<T_Geometry*>(this)->getTopology_impl(topo_num);
    }
    template <class T_Geometry>
    inline int Geometry<T_Geometry>::myTopologyId()
    {
      return static_cast<T_Geometry*>(this)->myTopologyId_impl();
    }
    template <class T_Geometry>
    inline int Geometry<T_Geometry>::getNumTopos()
    {
      return static_cast<T_Geometry*>(this)->getNumTopos_impl();
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
    inline pami_task_t *Geometry<T_Geometry>::ranks()
    {
      return static_cast<T_Geometry*>(this)->ranks_impl();
    }

    template <class T_Geometry>
    inline pami_task_t *Geometry<T_Geometry>::ranks_sizet()
    {
      return static_cast<T_Geometry*>(this)->ranks_sizet_impl();
    }

    template <class T_Geometry>
    inline pami_task_t Geometry<T_Geometry>::nranks()
    {
      return static_cast<T_Geometry*>(this)->nranks_impl();
    }

    template <class T_Geometry>
    inline pami_task_t Geometry<T_Geometry>::myIdx()
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
    inline pami_task_t *Geometry<T_Geometry>::permutation()
    {
      return static_cast<T_Geometry*>(this)->permutation_impl();
    }

    template <class T_Geometry>
    inline void Geometry<T_Geometry>::generatePermutation_sizet()
    {
      return static_cast<T_Geometry*>(this)->generatePermutation_sizet_impl();
    }

    template <class T_Geometry>
    inline void Geometry<T_Geometry>::freePermutation_sizet()
    {
      return static_cast<T_Geometry*>(this)->freePermutation_sizet_impl();
    }

    template <class T_Geometry>
    inline pami_task_t *Geometry<T_Geometry>::permutation_sizet()
    {
      return static_cast<T_Geometry*>(this)->permutation_sizet_impl();
    }

/** \todo replace by attributes */
#if 1
    // to be removed eventually
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
#endif
    template <class T_Geometry>
    inline unsigned Geometry<T_Geometry>::getNumColors()
    {
      return static_cast<T_Geometry*>(this)->getNumColors_impl();
    }

    template <class T_Geometry>
    inline unsigned Geometry<T_Geometry>::getAllreduceIteration()
    {
      return static_cast<T_Geometry*>(this)->getAllreduceIteration_impl();
    }

    template <class T_Geometry>
    inline void Geometry<T_Geometry>::freeAllocations ()
    {
      return static_cast<T_Geometry*>(this)->freeAllocations_impl();
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
    inline MatchQueue &Geometry<T_Geometry>::asyncCollectivePostQ()
    {
      return static_cast<T_Geometry*>(this)->asyncCollectivePostQ_impl();
    }

    template <class T_Geometry>
    inline MatchQueue &Geometry<T_Geometry>::asyncCollectiveUnexpQ()
    {
      return static_cast<T_Geometry*>(this)->asyncCollectiveUnexpQ_impl();
    }
#if 0
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
    inline void * Geometry<T_Geometry>::getBarrierExecutor()
    {
      return static_cast<T_Geometry*>(this)->getBarrierExecutor_impl();
    }

#endif
    template <class T_Geometry>
    inline CCMI_EXECUTOR_TYPE Geometry<T_Geometry>::getAllreduceCompositeStorage(unsigned i)
    {
      return static_cast<T_Geometry*>(this)->getAllreduceCompositeStorage_impl(i);
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
    inline CCMI_EXECUTOR_TYPE Geometry<T_Geometry>::getAllreduceCompositeStorage ()
    {
      return static_cast<T_Geometry*>(this)->getAllreduceCompositeStorage_impl();
    }

    template <class T_Geometry>
    inline COMPOSITE_TYPE Geometry<T_Geometry>::getAllreduceComposite()
    {
      return static_cast<T_Geometry*>(this)->getAllreduceComposite_impl();
    }

    template <class T_Geometry>
    inline T_Geometry *Geometry<T_Geometry>::getCachedGeometry (unsigned comm)
    {
      return T_Geometry::getCachedGeometry_impl(comm);
    }

    template <class T_Geometry>
    inline void Geometry<T_Geometry>::updateCachedGeometry (T_Geometry *geometry,
                                                                         unsigned comm)
    {
      return T_Geometry::updateCachedGeometry_impl(geometry, comm);
    }

    // These methods were originally from the PGASRT Communicator class
    template <class T_Geometry>
    inline pami_task_t  Geometry<T_Geometry>::size       (void)
    {
      return static_cast<T_Geometry*>(this)->size_impl();
    }
    template <class T_Geometry>
    inline pami_task_t  Geometry<T_Geometry>::rank       (void)
    {
      return static_cast<T_Geometry*>(this)->rank_impl();
    }
    template <class T_Geometry>
    inline pami_task_t   Geometry<T_Geometry>::virtrank()
    {
      return static_cast<T_Geometry*>(this)->virtrank_impl();
    }
    template <class T_Geometry>
    inline pami_task_t  Geometry<T_Geometry>::absrankof  (int rank)
    {
      return static_cast<T_Geometry*>(this)->absrankof_impl(rank);
    }
    template <class T_Geometry>
    inline pami_task_t Geometry<T_Geometry>::virtrankof (int rank)
    {
      return static_cast<T_Geometry*>(this)->virtrankof_impl(rank);
    }
    template <class T_Geometry>
    inline void                        Geometry<T_Geometry>::setKey (keys_t key, void *value)
    {
      static_cast<T_Geometry*>(this)->setKey_impl(key, value);
    }
    template <class T_Geometry>
    inline void*                       Geometry<T_Geometry>::getKey (keys_t key)
    {
      return static_cast<T_Geometry*>(this)->getKey_impl(key);
    }

    template <class T_Geometry>
    inline pami_result_t Geometry<T_Geometry>::algorithms_num(pami_xfer_type_t  colltype,
                                                             size_t             *lists_lengths,
                                                             size_t           context_id)
    {
      return static_cast<T_Geometry*>(this)->algorithms_num_impl(colltype,lists_lengths,context_id);
    }

    template <class T_Geometry>
    inline pami_result_t  Geometry<T_Geometry>::algorithms_info (pami_xfer_type_t   colltype,
                                                                pami_algorithm_t  *algs0,
                                                                pami_metadata_t   *mdata0,
                                                                size_t               num0,
                                                                pami_algorithm_t  *algs1,
                                                                pami_metadata_t   *mdata1,
                                                                size_t               num1,
                                                                size_t            context_id)
    {
      return static_cast<T_Geometry*>(this)->algorithms_info_impl(colltype,
                                                                  algs0,
                                                                  mdata0,
                                                                  num0,
                                                                  algs1,
                                                                  mdata1,
                                                                  num1,
                                                                  context_id);
    }

    template <class T_Geometry>
    inline pami_result_t Geometry<T_Geometry>::addCollective(pami_xfer_type_t                            xfer_type,
                                                            CCMI::Adaptor::CollectiveProtocolFactory  *factory,
                                                            size_t                                     context_id)
    {
      return static_cast<T_Geometry*>(this)->addCollective_impl(xfer_type,factory,context_id);
    }


    template <class T_Geometry>
    inline pami_result_t Geometry<T_Geometry>::addCollectiveCheck(pami_xfer_type_t                            xfer_type,
                                                                 CCMI::Adaptor::CollectiveProtocolFactory  *factory,
                                                                 size_t                                     context_id)
    {
      return static_cast<T_Geometry*>(this)->addCollectiveCheck_impl(xfer_type,factory,context_id);
    }




  }; // namespace Geometry
}; // namespace PAMI

#undef TRACE_ERR

#endif
