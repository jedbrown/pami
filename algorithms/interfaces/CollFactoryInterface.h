/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/interfaces/CollFactoryInterface.h
 * \brief ???
 */

#ifndef __algorithms_interfaces_CollFactoryInterface_h__
#define __algorithms_interfaces_CollFactoryInterface_h__

#include "sys/xmi.h"

namespace XMI
{
  namespace CollFactory
  {
    template <class T_Collfactory>
    class CollFactory
    {
    public:
      inline CollFactory()
        {
        }
      inline xmi_result_t  algorithms_num  (xmi_xfer_type_t collective,
                                            int *lists_lengths);
      inline xmi_result_t algorithms_info  (xmi_xfer_type_t collective,
                                            xmi_algorithm_t *alglist,
                                            xmi_metadata_t *mdata,
                                            int algorithm_type,
                                            int num);

      inline xmi_result_t  collective      (xmi_xfer_t           *collective);
      inline xmi_result_t  ibroadcast      (xmi_broadcast_t      *broadcast);
      inline xmi_result_t  iallreduce      (xmi_allreduce_t      *allreduce);
      inline xmi_result_t  ireduce         (xmi_reduce_t         *reduce);
      inline xmi_result_t  iallgather      (xmi_allgather_t      *allgather);
      inline xmi_result_t  iallgatherv     (xmi_allgatherv_t     *allgatherv);
      inline xmi_result_t  iallgatherv_int (xmi_allgatherv_int_t *allgatherv_int);
      inline xmi_result_t  iscatter        (xmi_scatter_t        *scatter);
      inline xmi_result_t  iscatterv       (xmi_scatterv_t       *scatterv);
      inline xmi_result_t  iscatterv_int   (xmi_scatterv_int_t   *scatterv);
      inline xmi_result_t  ibarrier        (xmi_barrier_t        *barrier);
      inline xmi_result_t  ialltoall       (xmi_alltoall_t       *alltoall);
      inline xmi_result_t  ialltoallv      (xmi_alltoallv_t      *alltoallv);
      inline xmi_result_t  ialltoallv_int  (xmi_alltoallv_int_t  *alltoallv_int);
      inline xmi_result_t  iscan           (xmi_scan_t           *scan);
      inline xmi_result_t  ambroadcast     (xmi_ambroadcast_t    *ambroadcast);
      inline xmi_result_t  amscatter       (xmi_amscatter_t      *amscatter);
      inline xmi_result_t  amgather        (xmi_amgather_t       *amgather);
      inline xmi_result_t  amreduce        (xmi_amreduce_t       *amreduce);
    }; // class CollFactory

    template <class T_Collfactory>
    inline xmi_result_t
    CollFactory<T_Collfactory>::collective(xmi_xfer_t *collective)
    {
      return static_cast<T_Collfactory*>(this)->collective_impl(collective);
    }

    template <class T_Collfactory>
    inline xmi_result_t CollFactory<T_Collfactory>::algorithms_num(xmi_xfer_type_t collective,
                                                                     int *lists_lengths)
    {
      return static_cast<T_Collfactory*>(this)->algorithms_num_impl(collective, lists_lengths);
    }

    template <class T_Collfactory>
    inline xmi_result_t CollFactory<T_Collfactory>::algorithms_info(xmi_xfer_type_t collective,
                                                                   xmi_algorithm_t *alglist,
                                                                   xmi_metadata_t *mdata,
                                                                   int algorithm_type,
                                                                   int num)
    {
      return static_cast<T_Collfactory*>(this)->algorithms_info_impl(collective, alglist, mdata, algorithm_type, num);
    }


    template <class T_Collfactory>
    inline xmi_result_t
    CollFactory<T_Collfactory>::ibroadcast(xmi_broadcast_t *broadcast)
    {
      return static_cast<T_Collfactory*>(this)->ibroadcast_impl(broadcast);
    }

    template <class T_Collfactory>
    inline xmi_result_t
    CollFactory<T_Collfactory>::iallreduce(xmi_allreduce_t *allreduce)
    {
      return static_cast<T_Collfactory*>(this)->iallreduce_impl(allreduce);
    }

    template <class T_Collfactory>
    inline xmi_result_t
    CollFactory<T_Collfactory>::ireduce(xmi_reduce_t *reduce)
    {
      return static_cast<T_Collfactory*>(this)->ireduce_impl(reduce);
    }

    template <class T_Collfactory>
    inline xmi_result_t
    CollFactory<T_Collfactory>::iallgather(xmi_allgather_t *allgather)
    {
      return static_cast<T_Collfactory*>(this)->iallgather_impl(allgather);
    }

    template <class T_Collfactory>
    inline xmi_result_t
    CollFactory<T_Collfactory>::iallgatherv(xmi_allgatherv_t *allgatherv)
    {
      return static_cast<T_Collfactory*>(this)->iallgatherv_impl(allgatherv);
    }

    template <class T_Collfactory>
    inline xmi_result_t
    CollFactory<T_Collfactory>::iallgatherv_int(xmi_allgatherv_int_t *allgatherv_int)
    {
      return static_cast<T_Collfactory*>(this)->iallgatherv_int_impl(allgatherv_int);
    }

    template <class T_Collfactory>
    inline xmi_result_t
    CollFactory<T_Collfactory>::iscatter(xmi_scatter_t *scatter)
    {
      return static_cast<T_Collfactory*>(this)->iscatter_impl(scatter);
    }

    template <class T_Collfactory>
    inline xmi_result_t
    CollFactory<T_Collfactory>::iscatterv(xmi_scatterv_t *scatterv)
    {
      return static_cast<T_Collfactory*>(this)->iscatterv_impl(scatterv);
    }

    template <class T_Collfactory>
    inline xmi_result_t
    CollFactory<T_Collfactory>::iscatterv_int(xmi_scatterv_int_t *scatterv_int)
    {
      return static_cast<T_Collfactory*>(this)->iscatterv_int_impl(scatterv_int);
    }

    template <class T_Collfactory>
    inline xmi_result_t
    CollFactory<T_Collfactory>::ibarrier(xmi_barrier_t *barrier)
    {
      return static_cast<T_Collfactory*>(this)->ibarrier_impl(barrier);
    }

    template <class T_Collfactory>
    inline xmi_result_t
    CollFactory<T_Collfactory>::ialltoall(xmi_alltoall_t *alltoall)
    {
      return static_cast<T_Collfactory*>(this)->ialltoall_impl(alltoall);
    }

    template <class T_Collfactory>
    inline xmi_result_t
    CollFactory<T_Collfactory>::ialltoallv(xmi_alltoallv_t *alltoallv)
    {
      return static_cast<T_Collfactory*>(this)->ialltoallv_impl(alltoallv);
    }

    template <class T_Collfactory>
    inline xmi_result_t
    CollFactory<T_Collfactory>::ialltoallv_int(xmi_alltoallv_int_t *alltoallv_int)
    {
      return static_cast<T_Collfactory*>(this)->ialltoallv_int_impl(alltoallv_int);
    }

    template <class T_Collfactory>
    inline xmi_result_t
    CollFactory<T_Collfactory>::iscan(xmi_scan_t *scan)
    {
      return static_cast<T_Collfactory*>(this)->iscan_impl(scan);
    }

    template <class T_Collfactory>
    inline xmi_result_t
    CollFactory<T_Collfactory>::ambroadcast(xmi_ambroadcast_t *ambroadcast)
    {
      return static_cast<T_Collfactory*>(this)->ambroadcast_impl(ambroadcast);
    }

    template <class T_Collfactory>
    inline xmi_result_t
    CollFactory<T_Collfactory>::amscatter(xmi_amscatter_t *amscatter)
    {
      return static_cast<T_Collfactory*>(this)->amscatter_impl(amscatter);
    }

    template <class T_Collfactory>
    inline xmi_result_t
    CollFactory<T_Collfactory>::amgather(xmi_amgather_t *amgather)
    {
      return static_cast<T_Collfactory*>(this)->amgather_impl(amgather);
    }

    template <class T_Collfactory>
    inline xmi_result_t
    CollFactory<T_Collfactory>::amreduce(xmi_amreduce_t *amreduce)
    {
      return static_cast<T_Collfactory*>(this)->amreduce_impl(amreduce);
    }
  }; // namespace CollFactory
}; // namespace XMI

#endif
