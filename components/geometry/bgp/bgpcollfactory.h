/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/geometry/bgp/bgpcollfactory.h
 * \brief ???
 */

#ifndef __components_geometry_bgp_bgpcollfactory_h__
#define __components_geometry_bgp_bgpcollfactory_h__


//#define XMI_COLLFACTORY_CLASS XMI::CollFactory::MPI<Device::MPIDevice<SysDep>, SysDep>

#include "sys/xmi.h"
#include "components/geometry/CollFactory.h"
#include "components/geometry/bgp/bgpcollinfo.h"
#include "util/common.h"
#include "algorithms/ccmi.h"

namespace XMI
{
  namespace CollFactory
  {

    template <class T_Device, class T_Sysdep>
    class BGP: public CollFactory<XMI::CollFactory::BGP<T_Device, T_Sysdep> >

    {
    public:
      inline BGP(T_Sysdep *sd):CollFactory<XMI::CollFactory::BGP<T_Device, T_Sysdep>>(),
                               _sd(sd)
        {
        }

      inline RegQueue* getRegQ(xmi_xfer_type_t collective)
      {
        RegQueue *rq = NULL;
        switch (collective)
        {
          case XMI_XFER_BROADCAST:
            rq = &_broadcasts;
            break;
          case XMI_XFER_ALLREDUCE:
            rq = &_allreduces;
            break;
          case XMI_XFER_REDUCE:
            return NULL;
            break;
          case XMI_XFER_ALLGATHER:
            rq = &_allgathers;
            break;
          case XMI_XFER_ALLGATHERV:
            rq = &_allgathervs;
            break;
          case XMI_XFER_ALLGATHERV_INT:
            return NULL;
            break;
          case XMI_XFER_SCATTER:
            rq = &_scatters;
            break;
          case XMI_XFER_SCATTERV:
            rq = &_scattervs;
            break;
          case XMI_XFER_SCATTERV_INT:
            break;
          case XMI_XFER_BARRIER:
            rq = &_barriers;
            break;
          case XMI_XFER_ALLTOALL:
            return NULL;
            break;
          case XMI_XFER_ALLTOALLV:
            rq = &_alltoallvs;
            break;
          case XMI_XFER_ALLTOALLV_INT:
            return NULL;
            break;
          case XMI_XFER_SCAN:
            return NULL;
            break;
          case XMI_XFER_AMBROADCAST:
            return NULL;
            break;
          case XMI_XFER_AMSCATTER:
            return NULL;
            break;
          case XMI_XFER_AMGATHER:
            return NULL;
            break;
          case XMI_XFER_AMREDUCE:
            return NULL;
            break;
          default:
            return NULL;
        }
        return rq;
      }

      inline xmi_result_t algorithms_num_impl (xmi_xfer_type_t collective,
                                               int *lists_lengths)
      {
        RegQueue *rq = getRegQ(collective);
        if(rq == NULL)
          return XMI_UNIMPL;
        lists_lengths[0] = rq->size();
       
        /* we return 0 for now for the "sometimes works" list */
        lists_lengths[1] = 0;
        return XMI_SUCCESS;
      }
      
      inline xmi_result_t algorithms_info_impl(xmi_xfer_type_t collective,
                                               xmi_algorithm_t *alglist,
                                               xmi_metadata_t *mdata,
                                               int algorithm_type,
                                               int num)
      {
        int i;
        RegQueue *rq = (RegQueue *) NULL;

        /* if type is 0, then we want the list of "always works" list */
        if (algorithm_type == 0)
          rq = getRegQ(collective);
        else
        {
#warning need to implement this later
          ; //
        }
       
        if(rq == NULL)
          return XMI_UNIMPL;

        if (num > rq->size())
          return XMI_ERROR;
       
        for(i = 0; i < num; i++)
          alglist[i] = (size_t) i;

        if (mdata)
        {
          for(i = 0; i < num; i++)
            mdata[i].geometry = (*rq)[i]->_metadata.geometry;
          mdata[i].buffer = (*rq)[i]->_metadata.buffer;
          mdata[i].misc = (*rq)[i]->_metadata.misc;
          strcpy(mdata[i].name, (*rq)[i]->_metadata.name);
        }
        return XMI_SUCCESS;
      }
     

      /*
        we need to pass in global barrier instead
      inline xmi_result_t setGeometry(XMI_GEOMETRY_CLASS *geometry,
                                      XMI_NBCollManager  *mgr,
                                      T_Device           *dev,
                                      XMI::CollInfo::CCMIBinomBarrierInfo<T_Device, T_Sysdep> *default_bar)
      {
        _geometry = geometry;
        _dev      = dev;
        return XMI_SUCCESS;
      }
      */
      inline xmi_result_t add_collective(xmi_xfer_type_t collective,
                                         XMI::CollInfo::CollInfo<T_Device>* ci)
      {
        RegQueue *rq = getRegQ(collective);
        if(rq==NULL)
          return XMI_UNIMPL;
        rq->push_back(ci);
        return XMI_SUCCESS;
      }

      inline xmi_result_t  collective_impl(xmi_xfer_t *collective)
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

      inline xmi_result_t ibroadcast_impl(xmi_broadcast_t *broadcast)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t iallreduce_impl(xmi_allreduce_t *allreduce)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t ireduce_impl(xmi_reduce_t *reduce)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t iallgather_impl(xmi_allgather_t *allgather)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t iallgatherv_impl(xmi_allgatherv_t *allgatherv)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t iallgatherv_int_impl (xmi_allgatherv_int_t *allgatherv_int)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t iscatter_impl(xmi_scatter_t *scatter)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t iscatterv_impl(xmi_scatterv_t *scatterv)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t iscatterv_int_impl(xmi_scatterv_int_t *scatterv)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t ibarrier_impl(xmi_barrier_t *barrier)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t ialltoall_impl(xmi_alltoall_t *alltoall)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t ialltoallv_impl(xmi_alltoallv_t *alltoallv)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t ialltoallv_int_impl(xmi_alltoallv_int_t *alltoallv_int)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t iscan_impl(xmi_scan_t *scan)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t ambroadcast_impl(xmi_ambroadcast_t *ambroadcast)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t amscatter_impl(xmi_amscatter_t *amscatter)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t amgather_impl(xmi_amgather_t *amgather)
      {
        return XMI_UNIMPL;
      }

      inline xmi_result_t amreduce_impl(xmi_amreduce_t *amreduce)
      {
        return XMI_UNIMPL;
      }

      XMI_GEOMETRY_CLASS *_geometry;
      T_Device *_dev;
      T_Sysdep *_sd;
      RegQueue _broadcasts;
      RegQueue _ambroadcasts;
      RegQueue _allgathers;
      RegQueue _allgathervs;
      RegQueue _scatters;
      RegQueue _scattervs;
      RegQueue _allreduces;
      RegQueue _barriers;
      RegQueue _alltoallvs;

      //    XMI::CollInfo::CCMIBinomBarrierInfo<T_Device, T_Sysdep>  *_ccmi_bar;
      XMI::MemoryAllocator<sizeof(reqObj), 16> _reqAllocator;

    }; // class CollFactory
  };  // namespace CollFactory
}; // namespace XMI


#endif
