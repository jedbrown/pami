/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/geometry/mpi/mpicollregistration.h
 * \brief ???
 */

#ifndef __components_geometry_mpi_mpicollregistration_h__
#define __components_geometry_mpi_mpicollregistration_h__

#define XMI_COLLREGISTRATION_CLASS XMI::CollRegistration::MPI

#include <map>
#include <vector>
#include "components/geometry/mpi/mpicollinfo.h"
#include "components/geometry/mpi/mpicollfactory.h"
#include "components/memory/MemoryAllocator.h"

namespace XMI
{
  extern std::map<unsigned, xmi_geometry_t> geometry_map;
  namespace CollRegistration
  {
    template <class T_Geometry, class T_Collfactory, class T_Device, class T_Sysdep>
    class MPI : public CollRegistration<XMI::CollRegistration::MPI<XMI_GEOMETRY_CLASS, T_Collfactory, T_Device, T_Sysdep>, T_Geometry, T_Collfactory>
    {
    public:
      inline MPI(T_Device *dev, T_Sysdep *sd):
        CollRegistration<XMI::CollRegistration::MPI<T_Geometry, T_Collfactory, T_Device, T_Sysdep>, T_Geometry, T_Collfactory>(),
	_dev(dev),
        _sysdep(sd),
	_pgbroadcast(dev),
	_pgallgather(dev),
	_pgallgatherv(dev),
	_pgscatter(dev),
	_pgscatterv(dev),
	_pgallreduce(dev),
	_pgbarrier(dev),
        _ccmiambroadcast(dev, sd),
        _ccmibarrier(dev, sd, mapidtogeometry),
        _ccmibinombroadcast(dev, sd, mapidtogeometry),
        _ccmiringbroadcast(dev, sd, mapidtogeometry),
        _ccmiringallreduce(dev, sd, mapidtogeometry),
        _ccmibinomialallreduce(dev, sd, mapidtogeometry),
        _ccmialltoallv(dev,sd)
        {
	  // Register and link each collective into a queue for analysis
	  _nbCollMgr.initialize();

	  _pgbroadcast._colltype=XMI::CollInfo::CI_BROADCAST0;
          _nbCollMgr.multisend_reg(TSPColl::BcastTag, &_pgbroadcast._model);
	  _broadcasts.push_back(&_pgbroadcast);

	  _pgallgather._colltype=XMI::CollInfo::CI_ALLGATHER0;
          _nbCollMgr.multisend_reg(TSPColl::AllgatherTag, &_pgallgather._model);
	  _allgathers.push_back(&_pgallgather);

	  _pgallgatherv._colltype=XMI::CollInfo::CI_ALLGATHERV0;
          _nbCollMgr.multisend_reg(TSPColl::AllgathervTag, &_pgallgatherv._model);
	  _allgathervs.push_back(&_pgallgatherv);

	  _pgscatter._colltype=XMI::CollInfo::CI_SCATTER0;
          _nbCollMgr.multisend_reg(TSPColl::ScatterTag, &_pgscatter._smodel);
          _nbCollMgr.multisend_reg(TSPColl::BarrierTag, &_pgscatter._bmodel);
	  _scatters.push_back(&_pgscatter);

	  _pgscatterv._colltype=XMI::CollInfo::CI_SCATTERV0;
          _nbCollMgr.multisend_reg(TSPColl::ScattervTag, &_pgscatterv._smodel);
          _nbCollMgr.multisend_reg(TSPColl::BarrierTag, &_pgscatterv._bmodel);
	  _scattervs.push_back(&_pgscatterv);

	  _pgallreduce._colltype=XMI::CollInfo::CI_ALLREDUCE0;
          _nbCollMgr.multisend_reg(TSPColl::ShortAllreduceTag, &_pgallreduce._model);
	  _allreduces.push_back(&_pgallreduce);

	  _pgbarrier._colltype=XMI::CollInfo::CI_BARRIER0;
          _nbCollMgr.multisend_reg(TSPColl::BarrierTag, &_pgbarrier._model);
	  _barriers.push_back(&_pgbarrier);

          _ccmiambroadcast._colltype=XMI::CollInfo::CI_AMBROADCAST0;
	  _ambroadcasts.push_back(&_ccmiambroadcast);

          _ccmibarrier._colltype=XMI::CollInfo::CI_BARRIER1;
          _barriers.push_back(&_ccmiambroadcast);

          _ccmibinombroadcast._colltype=XMI::CollInfo::CI_BROADCAST1;
          _broadcasts.push_back(&_ccmibinombroadcast);

          _ccmiringbroadcast._colltype=XMI::CollInfo::CI_BROADCAST2;
          _broadcasts.push_back(&_ccmiringbroadcast);

          _ccmiringallreduce._colltype=XMI::CollInfo::CI_ALLREDUCE1;
          _allreduces.push_back(&_ccmiringallreduce);

          _ccmibinomialallreduce._colltype=XMI::CollInfo::CI_ALLREDUCE2;
          _allreduces.push_back(&_ccmibinomialallreduce);

          _ccmialltoallv._colltype=XMI::CollInfo::CI_ALLTOALLV0;
          _alltoallvs.push_back(&_ccmialltoallv);
        }

      inline T_Collfactory * analyze_impl(T_Geometry *geometry)
      {
	XMI_COLLFACTORY_CLASS *f=(XMI_COLLFACTORY_CLASS *)_fact_alloc.allocateObject();
	new(f)XMI_COLLFACTORY_CLASS(_sysdep);
	f->setGeometry(geometry, &_nbCollMgr, _dev, &_ccmibarrier);
	f->add_collective(XMI_XFER_BROADCAST,  &_pgbroadcast);
	f->add_collective(XMI_XFER_ALLGATHER,  &_pgallgather);
	f->add_collective(XMI_XFER_ALLGATHERV, &_pgallgatherv);
	f->add_collective(XMI_XFER_SCATTER,    &_pgscatter);
	f->add_collective(XMI_XFER_SCATTERV,   &_pgscatterv);
	f->add_collective(XMI_XFER_ALLREDUCE,  &_pgallreduce);
	f->add_collective(XMI_XFER_BARRIER,    &_pgbarrier);
        f->add_collective(XMI_XFER_BARRIER,    &_ccmibarrier);
        f->add_collective(XMI_XFER_AMBROADCAST,&_ccmiambroadcast);
        f->add_collective(XMI_XFER_BROADCAST,  &_ccmiringbroadcast);
        f->add_collective(XMI_XFER_BROADCAST,  &_ccmibinombroadcast);
        f->add_collective(XMI_XFER_ALLREDUCE,  &_ccmiringallreduce);
        f->add_collective(XMI_XFER_ALLREDUCE,  &_ccmibinomialallreduce);
        f->add_collective(XMI_XFER_ALLTOALLV,  &_ccmialltoallv);
	return f;
      }

      static xmi_geometry_t mapidtogeometry (int comm)
        {
          xmi_geometry_t g = geometry_map[comm];
          return g;
        }

    public:
      T_Device                        *_dev;
      T_Sysdep                        *_sysdep;
      XMI_NBCollManager                _nbCollMgr;
      MemoryAllocator<sizeof(XMI_COLLFACTORY_CLASS), 16> _fact_alloc;

      XMI::CollInfo::PGBroadcastInfo<T_Device>    _pgbroadcast;
      XMI::CollInfo::PGAllgatherInfo<T_Device>    _pgallgather;
      XMI::CollInfo::PGAllgathervInfo<T_Device>   _pgallgatherv;
      XMI::CollInfo::PGScatterInfo<T_Device>      _pgscatter;
      XMI::CollInfo::PGScattervInfo<T_Device>     _pgscatterv;
      XMI::CollInfo::PGAllreduceInfo<T_Device>    _pgallreduce;
      XMI::CollInfo::PGBarrierInfo<T_Device>      _pgbarrier;
      XMI::CollInfo::CCMIAmbroadcastInfo<T_Device, T_Sysdep>        _ccmiambroadcast;
      XMI::CollInfo::CCMIBinomBarrierInfo<T_Device, T_Sysdep>       _ccmibarrier;
      XMI::CollInfo::CCMIBinomBroadcastInfo<T_Device, T_Sysdep>     _ccmibinombroadcast;
      XMI::CollInfo::CCMIRingBroadcastInfo<T_Device, T_Sysdep>      _ccmiringbroadcast;
      XMI::CollInfo::CCMIRingAllreduceInfo<T_Device, T_Sysdep>      _ccmiringallreduce;
      XMI::CollInfo::CCMIBinomialAllreduceInfo<T_Device, T_Sysdep>  _ccmibinomialallreduce;
      XMI::CollInfo::CCMIAlltoallvInfo<T_Device, T_Sysdep>           _ccmialltoallv;

      RegQueue          _broadcasts;
      RegQueue          _ambroadcasts;
      RegQueue          _allgathers;
      RegQueue          _allgathervs;
      RegQueue          _alltoallvs;
      RegQueue          _scatters;
      RegQueue          _scattervs;
      RegQueue          _allreduces;
      RegQueue          _barriers;

#if 0
      MPIMcastModel     _ccmi_broadcast;
      MPIMcastModel     _ccmi_allreduce;
      MPIMcastModel     _ccmi_alltoall;
      MPIMcastModel     _ccmi_barrier;
      MPIMcastModel     _ccmi_ambroadcast;
#endif

    }; // class Collregistration
  };  // namespace Collregistration
}; // namespace XMI
#endif
