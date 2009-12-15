/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/geometry/lapiunix/lapiunixcollinfo.h
 * \brief ???
 */

#ifndef __components_geometry_lapiunix_lapiunixcollinfo_h__
#define __components_geometry_lapiunix_lapiunixcollinfo_h__

#include <vector>
#include "components/geometry/CollRegistration.h"
#include "components/devices/lapiunix/oldlapiunixmulticastmodel.h"
#include "components/devices/lapiunix/oldlapiunixm2mmodel.h"
#include "components/devices/lapiunix/lapiunixmessage.h"
#include "components/devices/lapiunix/lapiunixdevice.h"
#include "SysDep.h"
#include "Mapping.h"


// PGASRT includes
#include "algorithms/protocols/tspcoll/NBCollManager.h"
#include "algorithms/protocols/tspcoll/NBColl.h"

#include "lapiNativeInterface.h"

// CCMI includes

typedef XMI::Device::LAPIOldmulticastModel<XMI::Device::LAPIDevice<XMI::SysDep>,
                                          XMI::Device::LAPIMessage> LAPIMcastModel;
typedef TSPColl::NBCollManager<LAPIMcastModel> XMI_NBCollManager;

typedef XMI::Device::LAPIOldm2mModel<XMI::Device::LAPIDevice<XMI::SysDep>,
                                    XMI::Device::LAPIMessage,
                                    size_t> LAPIM2MModel;


#define XMI_COLL_MCAST_CLASS  LAPIMcastModel
#define XMI_COLL_SYSDEP_CLASS XMI::SysDep

#include "algorithms/protocols/broadcast/async_impl.h"
#include "algorithms/protocols/broadcast/multi_color_impl.h"
#include "algorithms/protocols/broadcast/async_impl.h"

#include "algorithms/connmgr/ColorGeometryConnMgr.h"
#include "algorithms/protocols/barrier/impl.h"

#include "algorithms/protocols/allreduce/sync_impl.h"
#include "algorithms/protocols/allreduce/async_impl.h"

#include "algorithms/protocols/alltoall/Alltoall.h"


typedef CCMI::Adaptor::A2AProtocol <LAPIM2MModel, XMI::SysDep, size_t> AlltoallProtocol;
typedef CCMI::Adaptor::AlltoallFactory <LAPIM2MModel, XMI::SysDep, size_t> AlltoallFactory;


namespace XMI
{
  namespace CollInfo
  {
    typedef enum
    {
      CI_BROADCAST0=0,
      CI_BROADCAST1,
      CI_BROADCAST2,
      CI_ALLGATHER0,
      CI_ALLGATHERV0,
      CI_SCATTER0,
      CI_SCATTERV0,
      CI_ALLREDUCE0,
      CI_ALLREDUCE1,
      CI_ALLREDUCE2,
      CI_BARRIER0,
      CI_BARRIER1,
      CI_AMBROADCAST0,
      CI_ALLTOALLV0,
    }collinfo_type_t;


    template <class T_Device>
    class CollInfo
    {
    public:
      CollInfo(T_Device *dev) {}
      collinfo_type_t _colltype;
      xmi_metadata_t _metadata;
    };

    template <class T_Device>
    class PGBroadcastInfo:public CollInfo<T_Device>
    {
    public:
      PGBroadcastInfo(T_Device *dev):
        CollInfo<T_Device>(dev),
        _model(*dev){}
      LAPIMcastModel _model;
    };

    template <class T_Device>
    class PGAllgatherInfo:public CollInfo<T_Device>
    {
    public:
      PGAllgatherInfo(T_Device *dev):
        CollInfo<T_Device>(dev),
	_model(*dev){}
      LAPIMcastModel _model;
    };

    template <class T_Device>
    class PGAllgathervInfo:public CollInfo<T_Device>
    {
    public:
      PGAllgathervInfo(T_Device *dev):
        CollInfo<T_Device>(dev),
	_model(*dev){}
      LAPIMcastModel _model;
    };

    template <class T_Device>
    class PGScatterInfo:public CollInfo<T_Device>
    {
    public:
      PGScatterInfo(T_Device *dev):
        CollInfo<T_Device>(dev),
	_smodel(*dev),
	_bmodel(*dev){}

      LAPIMcastModel _smodel;
      LAPIMcastModel _bmodel;
    };

    template <class T_Device>
    class PGScattervInfo:public CollInfo<T_Device>
    {
    public:

      PGScattervInfo(T_Device *dev):
        CollInfo<T_Device>(dev),
	_smodel(*dev),
	_bmodel(*dev){}

      LAPIMcastModel _smodel;
      LAPIMcastModel _bmodel;
    };

    template <class T_Device>
    class PGAllreduceInfo:public CollInfo<T_Device>
    {
    public:
      PGAllreduceInfo(T_Device *dev):
        CollInfo<T_Device>(dev),
	_model(*dev){}

      LAPIMcastModel _model;
    };

    template <class T_Device>
    class PGBarrierInfo:public CollInfo<T_Device>
    {
    public:
      PGBarrierInfo(T_Device *dev):
        CollInfo<T_Device>(dev),
	_model(*dev){}

      LAPIMcastModel _model;
    };

    template <class T_Device, class T_Sysdep>
    class CCMIAmbroadcastInfo:public CollInfo<T_Device>
    {
    public:
      CCMIAmbroadcastInfo(T_Device *dev, T_Sysdep * sd):
        CollInfo<T_Device>(dev),
	_model(*dev),
        _bcast_registration(sd, &_model, __global.mapping.size())
        {
        }
      XMI_Request_t                                   _request;
      LAPIMcastModel                                   _model;
      CCMI::Adaptor::Broadcast::AsyncBinomialFactory  _bcast_registration;
    };

#if  OLD_CCMI_BARRIER
    template <class T_Device, class T_Sysdep>
    class CCMIBinomBarrierInfo:public CollInfo<T_Device>
    {
    public:
      CCMIBinomBarrierInfo(T_Device *dev,
                           T_Sysdep * sd,
                           xmi_mapidtogeometry_fn fcn):
        CollInfo<T_Device>(dev),
	_model(*dev),
        _barrier_registration(&_model,
                              sd,
                              fcn)
        {
        }
      XMI_Request_t                                  _request;
      LAPIMcastModel                                  _model;
      CCMI::Adaptor::Barrier::BinomialBarrierFactory _barrier_registration;
      CCMI_Executor_t                                _barrier_executor;
    };
#else
    template <class T_Device, class T_Sysdep>
    class CCMIBinomBarrierInfo:public CollInfo<T_Device>
    {
    public:
      CCMIBinomBarrierInfo(T_Device *dev,
                           T_Sysdep * sd,
                           xmi_mapidtogeometry_fn fcn,
			   xmi_client_t           client,
			   xmi_context_t          context,
			   size_t                 context_id):
        CollInfo<T_Device>(dev),
	_minterface(dev, client, context, context_id),
        _barrier_registration(&_minterface,
                              fcn),
        _client(client),
        _context(context),
	_contextid (context_id)
        {
          xmi_metadata_t *meta = &(this->_metadata);
          strcpy(meta->name, "CCMI_BinomBarrier");
        }
      XMI_Request_t                                  _request;
      lapiNativeInterface<T_Device>                   _minterface;
      CCMI::Adaptor::Barrier::BinomialBarrierFactory _barrier_registration;
      CCMI_Executor_t                                _barrier_executor;
      xmi_client_t                                   _client;
      xmi_context_t                                  _context;
      size_t                                         _contextid;
    };
#endif

    template <class T_Device, class T_Sysdep>
    class CCMIBinomBroadcastInfo:public CollInfo<T_Device>
    {
    public:
      CCMIBinomBroadcastInfo(T_Device *dev,
                             T_Sysdep * sd,
                             xmi_mapidtogeometry_fn fcn):
        CollInfo<T_Device>(dev),
        _model(*dev),
        _connmgr(65535),
        _broadcast_registration(sd,
                                &_model,
                                &_connmgr,
                                65535)
        {
        }
      XMI_Request_t                                           _request;
      LAPIMcastModel                                           _model;
      CCMI::ConnectionManager::ColorGeometryConnMgr<T_Sysdep> _connmgr;
      CCMI::Adaptor::Broadcast::BinomialBcastFactory          _broadcast_registration;
    };

    template <class T_Device, class T_Sysdep>
    class CCMIRingBroadcastInfo:public CollInfo<T_Device>
    {
    public:
      CCMIRingBroadcastInfo(T_Device *dev,
                             T_Sysdep * sd,
                             xmi_mapidtogeometry_fn fcn):
        CollInfo<T_Device>(dev),
        _model(*dev),
        _connmgr(65535),
        _broadcast_registration(sd,
                                &_model,
                                &_connmgr,
                                65535)
        {
        }
      XMI_Request_t                                           _request;
      LAPIMcastModel                                           _model;
      CCMI::ConnectionManager::ColorGeometryConnMgr<T_Sysdep> _connmgr;
      CCMI::Adaptor::Broadcast::RingBcastFactory              _broadcast_registration;
    };


    template <class T_Device, class T_Sysdep>
    class CCMIRingAllreduceInfo:public CollInfo<T_Device>
    {
    public:
      CCMIRingAllreduceInfo(T_Device *dev,
                            T_Sysdep * sd,
                            xmi_mapidtogeometry_fn fcn):
        CollInfo<T_Device>(dev),
        _model(*dev),
        _cf(0,0),
        _allreduce_registration(sd,
                                &_model,
                                fcn,
                                _cf)
        {
        }
      XMI_Request_t                                           _request;
      LAPIMcastModel                                           _model;
      CCMI::Adaptor::ConfigFlags                              _cf;
      CCMI::Adaptor::Allreduce::Ring::Factory                 _allreduce_registration;
    };

   template <class T_Device, class T_Sysdep>
    class CCMIBinomialAllreduceInfo:public CollInfo<T_Device>
    {
    public:
      CCMIBinomialAllreduceInfo(T_Device *dev,
                                T_Sysdep * sd,
                                xmi_mapidtogeometry_fn fcn):
        CollInfo<T_Device>(dev),
        _model(*dev),
        _cf(0,0),
        _allreduce_registration(sd,
                                &_model,
                                fcn,
                                _cf)
        {
        }
      XMI_Request_t                                           _request;
      LAPIMcastModel                                           _model;
      CCMI::Adaptor::ConfigFlags                              _cf;
      CCMI::Adaptor::Allreduce::Binomial::Factory             _allreduce_registration;
    };


    template <class T_Device, class T_Sysdep>
    class CCMIAlltoallvInfo:public CollInfo<T_Device>
    {
    public:
      CCMIAlltoallvInfo(T_Device *dev,
                       T_Sysdep * sd):
        CollInfo<T_Device>(dev),
        _model(*dev),
        _alltoallv_registration(&_model,
                                sd)
        {
        }
      XMI_Request_t     _request;
      LAPIM2MModel       _model;
      AlltoallFactory   _alltoallv_registration;
    };


  };
};
typedef XMI::Device::LAPIDevice<XMI::SysDep> LAPIDevice;
typedef std::vector<XMI::CollInfo::CollInfo<LAPIDevice> *> RegQueue;

#endif
