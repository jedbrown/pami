/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/geometry/OldCCMICollRegistration.h
 * \brief ???
 */

#ifndef __algorithms_geometry_OldCCMICollRegistration_h__
#define __algorithms_geometry_OldCCMICollRegistration_h__

#include <map>
#include <vector>
#include "algorithms/interfaces/CollRegistrationInterface.h"
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
#include "algorithms/protocols/broadcast/old_multi_color_impl.h"


namespace PAMI
{
  namespace CollRegistration
  {
    template <class T_Geometry,
              class T_Mcast,
              class T_M2M,
              class T_Device>
    class OldCCMIRegistration :
      public CollRegistration<PAMI::CollRegistration::OldCCMIRegistration<T_Geometry,
                                                                         T_Mcast,
                                                                         T_M2M,
                                                                         T_Device>,
                              T_Geometry>
      {
      public:
        inline OldCCMIRegistration(pami_client_t                        client,
                                   pami_context_t                       context,
                                   size_t                               context_id,
                                   T_Device                            &dev,
                                   std::map<unsigned, pami_geometry_t> *geometry_map):
        CollRegistration<PAMI::CollRegistration::OldCCMIRegistration<T_Geometry,
                                                                     T_Mcast,
                                                                     T_M2M,
                                                                     T_Device>,
                         T_Geometry> (),
        _client(client),
        _context(context),
        _context_id(context_id),
        _reduce_val(0),
        _dev(dev),
        _geometry_map(geometry_map),
        _barrier(dev),
        _broadcast(dev),
        _ringbroadcast(dev),
        _alltoallv(dev),
        _shortbinomallreduce(dev),
        _binomallreduce(dev),
        _ringallreduce(dev),
        _ambionmialbroadcast(dev),
        _barrier_registration(&_barrier,(pami_mapidtogeometry_fn)mapidtogeometry),
        _broadcast_connmgr(65535),
        _broadcast_registration(&_broadcast,&_broadcast_connmgr,_context_id),
        _ringbcast_connmgr(65535),
        _ringbcast_registration(&_ringbroadcast,&_ringbcast_connmgr,65535),
        _alltoallv_registration(&_alltoallv),
        _cf(0,0),
        _shortbinomallreduce_registration(&_shortbinomallreduce,(pami_mapidtogeometry_fn)mapidtogeometry,_cf),
        _binomallreduce_registration(&_binomallreduce,(pami_mapidtogeometry_fn)mapidtogeometry,_cf),
        _ringallreduce_registration(&_ringallreduce,(pami_mapidtogeometry_fn)mapidtogeometry,_cf),
        _ambcast_registration(&_ambionmialbroadcast,__global.mapping.size())
          {
          }

        inline pami_result_t analyze_impl(size_t context_id, T_Geometry *geometry, int phase)
        {
    if (phase != 0) return PAMI_SUCCESS;
          _barrier_registration.generate(&_barrier_composite,
                                         sizeof(CCMI_Executor_t),
                                         _context,
                                         (pami_geometry_t)geometry,
                                         NULL);
          geometry->setKey((size_t)0, /// \todo does NOT support multicontext
                           PAMI::Geometry::PAMI_CKEY_BARRIERCOMPOSITE0,
                           (void*)&_barrier_composite);

          geometry->addCollective(PAMI_XFER_BARRIER,&_barrier_registration,_context_id);
          geometry->addCollective(PAMI_XFER_BROADCAST,&_broadcast_registration,_context_id);
          geometry->addCollective(PAMI_XFER_BROADCAST,&_ringbcast_registration,_context_id);
          geometry->addCollective(PAMI_XFER_ALLTOALLV,&_alltoallv_registration,_context_id);
          geometry->addCollective(PAMI_XFER_ALLREDUCE,&_shortbinomallreduce_registration,_context_id);
          geometry->addCollective(PAMI_XFER_ALLREDUCE,&_binomallreduce_registration,_context_id);
          geometry->addCollective(PAMI_XFER_ALLREDUCE,&_ringallreduce_registration,_context_id);
          geometry->addCollective(PAMI_XFER_AMBROADCAST,&_ambcast_registration,_context_id);
          return PAMI_SUCCESS;
        }
        inline pami_result_t analyze_local_impl(size_t context_id,T_Geometry *geometry, uint64_t *out)
          {
            *out = _reduce_val;
            return analyze(context_id, geometry, 0);
          }

        inline pami_result_t analyze_global_impl(size_t context_id,T_Geometry *geometry, uint64_t *in)
          {
            return PAMI_SUCCESS;
          }
    public:
      pami_client_t              _client;
      pami_context_t             _context;
      size_t                     _context_id;
      uint64_t                   _reduce_val;
      T_Device                  &_dev;

      // Map of geometry id's to geometry for this client
      std::map<unsigned, pami_geometry_t> *_geometry_map;
        
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

      CCMI::ConnectionManager::ColorGeometryConnMgr           _broadcast_connmgr;
      CCMI::Adaptor::Broadcast::OldBinomialBcastFactory       _broadcast_registration;

      CCMI::ConnectionManager::ColorGeometryConnMgr           _ringbcast_connmgr;
      CCMI::Adaptor::Broadcast::OldRingBcastFactory           _ringbcast_registration;

      CCMI::Adaptor::Alltoall::AlltoallFactory                _alltoallv_registration;

      CCMI::Adaptor::ConfigFlags                              _cf;
      CCMI::Adaptor::Allreduce::ShortBinomial::Factory        _shortbinomallreduce_registration;
      CCMI::Adaptor::Allreduce::OldBinomial::Factory          _binomallreduce_registration;
      CCMI::Adaptor::Allreduce::Ring::Factory                 _ringallreduce_registration;

      CCMI::Adaptor::Broadcast::AsyncBinomialFactory          _ambcast_registration;


    };
  };
};
#endif
