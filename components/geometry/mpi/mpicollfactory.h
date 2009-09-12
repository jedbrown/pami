#ifndef   __xmi_mpicollfactory__h__
#define   __xmi_mpicollfactory__h__


#define XMI_COLLFACTORY_CLASS XMI::CollFactory::MPI

#include "sys/xmi.h"
#include "components/geometry/CollFactory.h"
#include "components/geometry/mpi/mpicollinfo.h"

namespace XMI
{
  namespace CollFactory
  {
    class MPI : public CollFactory<XMI::CollFactory::MPI>
    {
    public:
      inline MPI():
        CollFactory<XMI::CollFactory::MPI>()
        {
        }

      inline xmi_result_t  algorithm_impl(xmi_xfer_type_t       collective,
                                          xmi_algorithm_t      *alglist,
                                          size_t               *num)
        {
          switch (collective)
              {
                  case XMI_XFER_BROADCAST:
                    break;
                  case XMI_XFER_ALLREDUCE:
                    break;
                  case XMI_XFER_REDUCE:
                    break;
                  case XMI_XFER_ALLGATHER:
                    break;
                  case XMI_XFER_ALLGATHERV:
                    break;
                  case XMI_XFER_ALLGATHERV_INT:
                    break;
                  case XMI_XFER_SCATTER:
                    break;
                  case XMI_XFER_SCATTERV:
                    break;
                  case XMI_XFER_SCATTERV_INT:
                    break;
                  case XMI_XFER_BARRIER:
                    break;
                  case XMI_XFER_ALLTOALL:
                    break;
                  case XMI_XFER_ALLTOALLV:
                    break;
                  case XMI_XFER_ALLTOALLV_INT:
                    break;
                  case XMI_XFER_SCAN:
                    break;
                  case XMI_XFER_AMBROADCAST:
                    break;
                  case XMI_XFER_AMSCATTER:
                    break;
                  case XMI_XFER_AMGATHER:
                    break;
                  case XMI_XFER_AMREDUCE:
                    break;
                  default:
                    return XMI_UNIMPL;
              }
          return XMI_SUCCESS;
        }

      inline size_t        num_algorithm_impl   (xmi_xfer_type_t           collective)
        {
          switch (collective)
              {
                  case XMI_XFER_BROADCAST:
                    return _broadcasts.size();
                    break;
                  case XMI_XFER_ALLREDUCE:
                    return _allreduces.size();
                    break;
                  case XMI_XFER_REDUCE:
                    return 0;
                    break;
                  case XMI_XFER_ALLGATHER:
                    return _allgathers.size();
                    break;
                  case XMI_XFER_ALLGATHERV:
                    return _allgathervs.size();
                    break;
                  case XMI_XFER_ALLGATHERV_INT:
                    return 0;
                    break;
                  case XMI_XFER_SCATTER:
                    return _scatters.size();
                    break;
                  case XMI_XFER_SCATTERV:
                    return _scattervs.size();
                    break;
                  case XMI_XFER_SCATTERV_INT:
                    return 0;
                    break;
                  case XMI_XFER_BARRIER:
                    return _barriers.size();
                    break;
                  case XMI_XFER_ALLTOALL:
                    return 0;
                    break;
                  case XMI_XFER_ALLTOALLV:
                    return 0;
                    break;
                  case XMI_XFER_ALLTOALLV_INT:
                    return 0;
                    break;
                  case XMI_XFER_SCAN:
                    return 0;
                    break;
                  case XMI_XFER_AMBROADCAST:
                    return 0;
                    break;
                  case XMI_XFER_AMSCATTER:
                    return 0;
                    break;
                  case XMI_XFER_AMGATHER:
                    return 0;
                    break;
                  case XMI_XFER_AMREDUCE:
                    return 0;
                    break;
                  default:
                    assert(0);
              }
          return XMI_UNIMPL;
        }

      inline xmi_result_t  collective_impl      (xmi_xfer_t           *collective)
        {
          switch (collective->xfer_type)
              {
                  case XMI_XFER_BROADCAST:
                    return ibroadcast_impl(&collective->xfer_broadcast);
                    break;
                  case XMI_XFER_ALLREDUCE:
                    return iallreduce_impl(&collective->xfer_allreduce);
                    break;
                  case XMI_XFER_REDUCE:
                    return ireduce_impl(&collective->xfer_reduce);
                    break;
                  case XMI_XFER_ALLGATHER:
                    return iallgather_impl(&collective->xfer_allgather);
                    break;
                  case XMI_XFER_ALLGATHERV:
                    return iallgatherv_impl(&collective->xfer_allgatherv);
                    break;
                  case XMI_XFER_ALLGATHERV_INT:
                    return iallgatherv_int_impl(&collective->xfer_allgatherv_int);
                    break;
                  case XMI_XFER_SCATTER:
                    return iscatter_impl(&collective->xfer_scatter);
                    break;
                  case XMI_XFER_SCATTERV:
                    return iscatterv_impl(&collective->xfer_scatterv);
                    break;
                  case XMI_XFER_SCATTERV_INT:
                    return iscatterv_int_impl(&collective->xfer_scatterv_int);
                    break;
                  case XMI_XFER_BARRIER:
                    return ibarrier_impl(&collective->xfer_barrier);
                    break;
                  case XMI_XFER_ALLTOALL:
                    return ialltoall_impl(&collective->xfer_alltoall);
                    break;
                  case XMI_XFER_ALLTOALLV:
                    return ialltoallv_impl(&collective->xfer_alltoallv);
                    break;
                  case XMI_XFER_ALLTOALLV_INT:
                    return ialltoallv_int_impl(&collective->xfer_alltoallv_int);
                    break;
                  case XMI_XFER_SCAN:
                    return iscan_impl(&collective->xfer_scan);
                    break;
                  case XMI_XFER_AMBROADCAST:
                    return ambroadcast_impl(&collective->xfer_ambroadcast);
                    break;
                  case XMI_XFER_AMSCATTER:
                    return amscatter_impl(&collective->xfer_amscatter);
                    break;
                  case XMI_XFER_AMGATHER:
                    return amgather_impl(&collective->xfer_amgather);
                    break;
                  case XMI_XFER_AMREDUCE:
                    return amreduce_impl(&collective->xfer_amreduce);
                    break;
                  default:
                    return XMI_UNIMPL;
              }
          return XMI_UNIMPL;
        }

      inline xmi_result_t  ibroadcast_impl      (xmi_broadcast_t      *broadcast)
        {
          XMI::CollInfo::PGBroadcastInfo *info = (XMI::CollInfo::PGBroadcastInfo *)_broadcasts[broadcast->algorithm];
          info->start(broadcast);
          return XMI_UNIMPL;
        }

      inline xmi_result_t  iallreduce_impl      (xmi_allreduce_t      *allreduce)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  ireduce_impl         (xmi_reduce_t         *reduce)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  iallgather_impl      (xmi_allgather_t      *allgather)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  iallgatherv_impl     (xmi_allgatherv_t     *allgatherv)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  iallgatherv_int_impl (xmi_allgatherv_int_t *allgatherv_int)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  iscatter_impl        (xmi_scatter_t        *scatter)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  iscatterv_impl       (xmi_scatterv_t       *scatterv)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  iscatterv_int_impl   (xmi_scatterv_int_t   *scatterv)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  ibarrier_impl        (xmi_barrier_t        *barrier)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  ialltoall_impl       (xmi_alltoall_t       *alltoall)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  ialltoallv_impl      (xmi_alltoallv_t      *alltoallv)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  ialltoallv_int_impl  (xmi_alltoallv_int_t  *alltoallv_int)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  iscan_impl           (xmi_scan_t           *scan)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  ambroadcast_impl     (xmi_ambroadcast_t    *ambroadcast)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  amscatter_impl       (xmi_amscatter_t      *amscatter)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  amgather_impl        (xmi_amgather_t       *amgather)
        {
          return XMI_UNIMPL;
        }

      inline xmi_result_t  amreduce_impl        (xmi_amreduce_t       *amreduce)
        {
          return XMI_UNIMPL;
        }
      RegQueue          _broadcasts;
      RegQueue          _allgathers;
      RegQueue          _allgathervs;
      RegQueue          _scatters;
      RegQueue          _scattervs;
      RegQueue          _allreduces;
      RegQueue          _barriers;
      
    }; // class CollFactory
  };  // namespace CollFactory
}; // namespace XMI


#endif
