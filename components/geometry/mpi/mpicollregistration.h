#ifndef   __xmi_mpicollregistration__h__
#define   __xmi_mpicollregistration__h__

#define XMI_COLLREGISTRATION_CLASS XMI::CollRegistration::MPI

#include <map>
#include <vector>
#include "components/geometry/mpi/mpicollinfo.h"
#include "components/memory/MemoryAllocator.h"

namespace XMI
{
  namespace CollRegistration
  {
    template <class T_Geometry, class T_Collfactory, class T_Device, class T_Sysdep>
    class MPI : public CollRegistration<XMI::CollRegistration::MPI<T_Geometry, T_Collfactory, T_Device, T_Sysdep>, T_Geometry, T_Collfactory>
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
        _ccmiambroadcast(dev, sd)
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

        }

      inline T_Collfactory * analyze_impl(T_Geometry *geometry)
      {
	XMI_COLLFACTORY_CLASS *f=(XMI_COLLFACTORY_CLASS *)_fact_alloc.allocateObject();
	new(f)XMI_COLLFACTORY_CLASS();
	f->setGeometry(geometry, &_nbCollMgr, _dev);
	f->add_collective(XMI_XFER_BROADCAST, &_pgbroadcast);
	f->add_collective(XMI_XFER_ALLGATHER, &_pgallgather);
	f->add_collective(XMI_XFER_ALLGATHERV, &_pgallgatherv);
	f->add_collective(XMI_XFER_SCATTER, &_pgscatter);
	f->add_collective(XMI_XFER_SCATTERV, &_pgscatterv);
	f->add_collective(XMI_XFER_ALLREDUCE, &_pgallreduce);
	f->add_collective(XMI_XFER_BARRIER, &_pgbarrier);
	return f;
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
      XMI::CollInfo::CCMIAmbroadcastInfo<T_Device, T_Sysdep>  _ccmiambroadcast;

      RegQueue          _broadcasts;
      RegQueue          _allgathers;
      RegQueue          _allgathervs;
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
