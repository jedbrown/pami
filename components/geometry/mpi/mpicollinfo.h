/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/geometry/mpi/mpicollinfo.h
 * \brief ???
 */

#ifndef __components_geometry_mpi_mpicollinfo_h__
#define __components_geometry_mpi_mpicollinfo_h__

#include <vector>
#include "components/geometry/CollRegistration.h"
#include "components/devices/mpi/oldmpimulticastmodel.h"
#include "components/devices/mpi/oldmpim2mmodel.h"
#include "components/devices/mpi/mpimessage.h"
#include "components/devices/mpi/mpidevice.h"
#include "SysDep.h"
#include "Mapping.h"
#include "util/compact_attributes.h"

// PGASRT includes
#include "algorithms/protocols/tspcoll/NBCollManager.h"
#include "algorithms/protocols/tspcoll/NBColl.h"

#include "MPINativeInterface.h"

// CCMI includes

typedef XMI::Device::MPIOldmulticastModel<XMI::Device::MPIDevice<XMI::SysDep>,
                                          XMI::Device::MPIMessage> MPIMcastModel;
typedef TSPColl::NBCollManager<MPIMcastModel> XMI_NBCollManager;

typedef XMI::Device::MPIOldm2mModel<XMI::Device::MPIDevice<XMI::SysDep>,
                                    XMI::Device::MPIMessage,
                                    size_t> MPIM2MModel;


#define XMI_COLL_MCAST_CLASS  MPIMcastModel
#define XMI_COLL_SYSDEP_CLASS XMI::SysDep

#include "algorithms/protocols/broadcast/async_impl.h"
#include "algorithms/protocols/broadcast/multi_color_impl.h"
#include "algorithms/protocols/broadcast/async_impl.h"

#include "algorithms/connmgr/ColorGeometryConnMgr.h"
#include "algorithms/protocols/barrier/impl.h"

#include "algorithms/protocols/allreduce/sync_impl.h"
#include "algorithms/protocols/allreduce/async_impl.h"

#include "algorithms/protocols/alltoall/Alltoall.h"

#define OLD_CCMI_BARRIER  1

typedef CCMI::Adaptor::A2AProtocol <MPIM2MModel, XMI::SysDep, size_t> AlltoallProtocol;
typedef CCMI::Adaptor::AlltoallFactory <MPIM2MModel, XMI::SysDep, size_t> AlltoallFactory;


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
      CollInfo(T_Device *dev)
      {
        xmi_ca_unset_all(&_metadata.geometry);
        xmi_ca_unset_all(&_metadata.buffer);
        xmi_ca_unset_all(&_metadata.misc);
      }
      collinfo_type_t _colltype;
      xmi_metadata_t _metadata;
    };

    template <class T_Device>
    class PGBroadcastInfo:public CollInfo<T_Device>
    {
    public:
      PGBroadcastInfo(T_Device *dev):
        CollInfo<T_Device>(dev),
        _model(*dev)
        {
          xmi_metadata_t *meta = &(this->_metadata);
          strcpy(meta->name, "PGAS_Bcast");
        }
      MPIMcastModel _model;
    };

    template <class T_Device>
    class PGAllgatherInfo:public CollInfo<T_Device>
    {
    public:
      PGAllgatherInfo(T_Device *dev):
        CollInfo<T_Device>(dev),
	_model(*dev)
        {
          xmi_metadata_t *meta = &(this->_metadata);
          strcpy(meta->name, "PGAS_Allgather");
        }
      MPIMcastModel _model;
    };

    template <class T_Device>
    class PGAllgathervInfo:public CollInfo<T_Device>
    {
    public:
      PGAllgathervInfo(T_Device *dev):
        CollInfo<T_Device>(dev),
	_model(*dev)
        {
          xmi_metadata_t *meta = &(this->_metadata);
          strcpy(meta->name, "PGAS_Allgatherv");
        }
      MPIMcastModel _model;
    };

    template <class T_Device>
    class PGScatterInfo:public CollInfo<T_Device>
    {
    public:
      PGScatterInfo(T_Device *dev):
        CollInfo<T_Device>(dev),
	_smodel(*dev),
	_bmodel(*dev)
        {
          xmi_metadata_t *meta = &(this->_metadata);
          strcpy(meta->name, "PGAS_Scatter");
        }

      MPIMcastModel _smodel;
      MPIMcastModel _bmodel;
    };

    template <class T_Device>
    class PGScattervInfo:public CollInfo<T_Device>
    {
    public:

      PGScattervInfo(T_Device *dev):
        CollInfo<T_Device>(dev),
	_smodel(*dev),
	_bmodel(*dev)
        {
          xmi_metadata_t *meta = &(this->_metadata);
          strcpy(meta->name, "PGAS_Scatterv");
        }

      MPIMcastModel _smodel;
      MPIMcastModel _bmodel;
    };

    template <class T_Device>
    class PGAllreduceInfo:public CollInfo<T_Device>
    {
    public:
      PGAllreduceInfo(T_Device *dev):
        CollInfo<T_Device>(dev),
	_model(*dev)
        {
          xmi_metadata_t *meta = &(this->_metadata);
          strcpy(meta->name, "PGAS_Allreduce");
        }

      MPIMcastModel _model;
    };

    template <class T_Device>
    class PGBarrierInfo:public CollInfo<T_Device>
    {
    public:
      PGBarrierInfo(T_Device *dev):
        CollInfo<T_Device>(dev),
	_model(*dev)
        {
          xmi_metadata_t *meta = &(this->_metadata);
          strcpy(meta->name, "PGAS_Barrier");
        }

      MPIMcastModel _model;
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
          xmi_metadata_t *meta = &(this->_metadata);
          strcpy(meta->name, "CCMI_AMBroadcast");
        }
      XMI_Request_t                                   _request;
      MPIMcastModel                                   _model;
      CCMI::Adaptor::Broadcast::AsyncBinomialFactory  _bcast_registration;
    };

#if  OLD_CCMI_BARRIER
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
	_model(*dev),
        _barrier_registration(&_model,
                              sd,
                              fcn)
        {
          xmi_metadata_t *meta = &(this->_metadata);
          strcpy(meta->name, "CCMI_BinomBarrier");
        }
      XMI_Request_t                                     _request;
      MPIMcastModel                                     _model;
      CCMI::Adaptor::Barrier::OldBinomialBarrierFactory _barrier_registration;
      CCMI_Executor_t                                   _barrier_executor;
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
      MPINativeInterface<T_Device>                   _minterface;
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
          xmi_metadata_t *meta = &(this->_metadata);
          strcpy(meta->name, "CCMI_BinomBroadcast");
        }
      XMI_Request_t                                           _request;
      MPIMcastModel                                           _model;
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
          xmi_metadata_t *meta = &(this->_metadata);
          strcpy(meta->name, "CCMI_RingBroadcast");
        }
      XMI_Request_t                                           _request;
      MPIMcastModel                                           _model;
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
          xmi_metadata_t *meta = &(this->_metadata);
          strcpy(meta->name, "CCMI_RingAllreduce");
        }
      XMI_Request_t                                           _request;
      MPIMcastModel                                           _model;
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
          xmi_metadata_t *meta = &(this->_metadata);
          strcpy(meta->name, "CCMI_BinomAllreduce");
        }
      XMI_Request_t                                           _request;
      MPIMcastModel                                           _model;
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
          xmi_metadata_t *meta = &(this->_metadata);
          strcpy(meta->name, "CCMI_Alltoallv");
        }
      XMI_Request_t     _request;
      MPIM2MModel       _model;
      AlltoallFactory   _alltoallv_registration;
    };


  };
};
typedef XMI::Device::MPIDevice<XMI::SysDep> MPIDevice;
typedef std::vector<XMI::CollInfo::CollInfo<MPIDevice> *> RegQueue;

#endif
