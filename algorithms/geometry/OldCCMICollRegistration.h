/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file common/default/CollRegistration.h
 * \brief ???
 */

#ifndef __common_oldccmi_CollRegistration_h__
#define __common_oldccmi_CollRegistration_h__

#include <map>
#include <vector>
#include "algorithms/interfaces/CollRegistrationInterface.h"
#include "SysDep.h"
#include "Global.h"


#include "TypeDefs.h"
#include "algorithms/protocols/broadcast/async_impl.h"
#include "algorithms/protocols/broadcast/multi_color_impl.h"
#include "algorithms/protocols/broadcast/async_impl.h"
#include "algorithms/connmgr/ColorGeometryConnMgr.h"
#include "algorithms/protocols/barrier/impl.h"
#include "algorithms/protocols/allreduce/sync_impl.h"
#include "algorithms/protocols/allreduce/async_impl.h"
#include "algorithms/protocols/alltoall/impl.h"



namespace XMI
{
  extern std::map<unsigned, xmi_geometry_t> geometry_map;
  namespace CollRegistration
  {
    template <class T_Geometry,
              class T_Mcast,
              class T_M2M,
              class T_Device,
              class T_Sysdep>
    class OldCCMIRegistration :
      public CollRegistration<XMI::CollRegistration::OldCCMIRegistration<T_Geometry,
                                                                         T_Mcast,
                                                                         T_M2M,
                                                                         T_Device,
                                                                         T_Sysdep>,
                              T_Geometry>
      {
      public:
        inline OldCCMIRegistration(xmi_client_t       client,
                                   xmi_context_t      context,
                                   size_t             context_id,
                                   T_Sysdep          &sd,
                                   T_Device          &dev):
        CollRegistration<XMI::CollRegistration::OldCCMIRegistration<T_Geometry,
                                                                    T_Mcast,
                                                                    T_M2M,
                                                                    T_Device,
                                                                    T_Sysdep>,
                         T_Geometry> (),
        _client(client),
        _context(context),
        _context_id(context_id),
        _sd(sd),
        _dev(dev),
        _barrier(dev),
        _broadcast(dev),
        _ringbroadcast(dev),
        _alltoallv(dev),
        _shortbinomallreduce(dev),
        _binomallreduce(dev),
        _ringallreduce(dev),
        _ambionmialbroadcast(dev),
        _barrier_registration(&_barrier,&_sd,(xmi_mapidtogeometry_fn)mapidtogeometry),
        _broadcast_connmgr(65535),
        _broadcast_registration(&_sd,&_broadcast,&_broadcast_connmgr,_context_id),
        _ringbcast_connmgr(65535),
        _ringbcast_registration(&_sd,&_ringbroadcast,&_ringbcast_connmgr,65535),
        _alltoallv_registration(&_alltoallv,&_sd),
        _cf(0,0),
        _shortbinomallreduce_registration(&_sd,&_shortbinomallreduce,(xmi_mapidtogeometry_fn)mapidtogeometry,_cf),
        _binomallreduce_registration(&_sd,&_binomallreduce,(xmi_mapidtogeometry_fn)mapidtogeometry,_cf),
        _ringallreduce_registration(&_sd,&_ringallreduce,(xmi_mapidtogeometry_fn)mapidtogeometry,_cf),
        _ambcast_registration(&_sd,&_ambionmialbroadcast,__global.mapping.size())
          {
          }

        inline xmi_result_t analyze_impl(size_t context_id, T_Geometry *geometry)
        {
          _barrier_registration.generate(&_barrier_composite,
                                         sizeof(CCMI_Executor_t),
                                         _context,
                                         (xmi_geometry_t)geometry,
                                         NULL);
          geometry->setKey(XMI::Geometry::XMI_GKEY_BARRIERCOMPOSITE0,
                           (void*)&_barrier_composite);

          geometry->addCollective(XMI_XFER_BARRIER,&_barrier_registration,_context_id);
          geometry->addCollective(XMI_XFER_BROADCAST,&_broadcast_registration,_context_id);
          geometry->addCollective(XMI_XFER_BROADCAST,&_ringbcast_registration,_context_id);
          geometry->addCollective(XMI_XFER_ALLTOALLV,&_alltoallv_registration,_context_id);
          geometry->addCollective(XMI_XFER_ALLREDUCE,&_shortbinomallreduce_registration,_context_id);
          geometry->addCollective(XMI_XFER_ALLREDUCE,&_binomallreduce_registration,_context_id);
          geometry->addCollective(XMI_XFER_ALLREDUCE,&_ringallreduce_registration,_context_id);
          geometry->addCollective(XMI_XFER_AMBROADCAST,&_ambcast_registration,_context_id);
          return XMI_SUCCESS;
        }

      static xmi_geometry_t mapidtogeometry (int comm)
        {
          xmi_geometry_t g = geometry_map[comm];
          return g;
        }
    public:
      xmi_client_t               _client;
      xmi_context_t              _context;
      size_t                     _context_id;
      T_Sysdep                  &_sd;
      T_Device                  &_dev;

      T_Mcast                    _barrier;
      T_Mcast                    _broadcast;
      T_Mcast                    _ringbroadcast;
      T_M2M                      _alltoallv;
      T_Mcast                    _shortbinomallreduce;
      T_Mcast                    _binomallreduce;
      T_Mcast                    _ringallreduce;
      T_Mcast                    _ambionmialbroadcast;

      CCMI::Adaptor::Barrier::OldBinomialBarrierFactory       _barrier_registration;
      CCMI_Executor_t                                         _barrier_composite;

      CCMI::ConnectionManager::ColorGeometryConnMgr<T_Sysdep> _broadcast_connmgr;
      CCMI::Adaptor::Broadcast::OldBinomialBcastFactory       _broadcast_registration;

      CCMI::ConnectionManager::ColorGeometryConnMgr<T_Sysdep> _ringbcast_connmgr;
      CCMI::Adaptor::Broadcast::OldRingBcastFactory           _ringbcast_registration;

      CCMI::Adaptor::Alltoall::AlltoallFactory                _alltoallv_registration;

      CCMI::Adaptor::ConfigFlags                              _cf;
      CCMI::Adaptor::Allreduce::ShortBinomial::Factory        _shortbinomallreduce_registration;
      CCMI::Adaptor::Allreduce::Binomial::Factory             _binomallreduce_registration;
      CCMI::Adaptor::Allreduce::Ring::Factory                 _ringallreduce_registration;

      CCMI::Adaptor::Broadcast::AsyncBinomialFactory          _ambcast_registration;


    };
  };
};
#endif
