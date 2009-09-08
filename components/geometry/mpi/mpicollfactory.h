#ifndef   __xmi_mpicollfactory__h__
#define   __xmi_mpicollfactory__h__


#define XMI_COLLFACTORY_CLASS XMI::CollFactory::MPI

#include "sys/xmi.h"
#include "components/geometry/CollFactory.h"

namespace XMI
{
  namespace CollFactory
  {
    template <class T_Geometry, class T_Collreg>
    class MPI : public CollFactory<XMI::CollFactory::MPI<T_Geometry, T_Collreg>, T_Geometry, T_Collreg>
    {
    public:      
      inline MPI(T_Geometry *geometry, T_Collreg *reg):
        CollFactory<XMI::CollFactory::MPI<T_Geometry, T_Collreg>, T_Geometry, T_Collreg>(geometry, reg)
        {
        }

      inline xmi_result_t  algorithm       (xmi_xfer_t           *collective,
                                            xmi_algorithm_t      *alglist,
                                            size_t               *num)
        {
          return XMI_UNIMPL;
        }

      inline size_t        num_algorithm   (xmi_xfer_t           *collective)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  collective      (xmi_xfer_t           *collective)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  ibroadcast      (xmi_broadcast_t      *broadcast)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  iallreduce      (xmi_allreduce_t      *allreduce)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  ireduce         (xmi_reduce_t         *reduce)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  iallgather      (xmi_allgather_t      *allgather)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  iallgatherv     (xmi_allgatherv_t     *allgatherv)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  iallgatherv_int (xmi_allgatherv_int_t *allgatherv_int)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  iscatter        (xmi_scatter_t        *scatter)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  iscatterv       (xmi_scatterv_t       *scatterv)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  iscatterv_int   (xmi_scatterv_int_t   *scatterv)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  ibarrier        (xmi_barrier_t        *barrier)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  ialltoall       (xmi_alltoall_t       *alltoall)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  ialltoallv      (xmi_alltoallv_t      *alltoallv)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  ialltoallv_int  (xmi_alltoallv_int_t  *alltoallv_int)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  iscan           (xmi_scan_t           *scan)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  ambroadcast     (xmi_ambroadcast_t    *ambroadcast)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  amscatter       (xmi_amscatter_t      *amscatter)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  amgather        (xmi_amgather_t       *amgather)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  amreduce        (xmi_amreduce_t       *amreduce)
        {
          return XMI_UNIMPL;
        }
    }; // class CollFactory
  };  // namespace CollFactory
}; // namespace XMI


#endif
