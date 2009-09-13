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
    template <class T_Geometry, class T_Collfactory, class T_Device>
    class MPI : public CollRegistration<XMI::CollRegistration::MPI<T_Geometry, T_Collfactory, T_Device>, T_Geometry, T_Collfactory, T_Device>
    {
    public:
      inline MPI():
        CollRegistration<XMI::CollRegistration::MPI<T_Geometry, T_Collfactory, T_Device>, T_Geometry, T_Collfactory, T_Device>()
        {
        }
      inline xmi_result_t setup_impl(T_Device *dev)
        {
          // Register and link each collective into a queue for analysis
	  _nbColl.initialize();
	  
	  _pgbroadcast._colltype=XMI::CollInfo::CI_BROADCAST0;
          _nbColl.instance()->multisend_reg(TSPColl::BcastTag, &_pgbroadcast._model);
	  _broadcasts.push_back(&_pgbroadcast);

	  _pgallgather._colltype=XMI::CollInfo::CI_ALLGATHER0;
          _nbColl.instance()->multisend_reg(TSPColl::AllgatherTag, &_pgallgather._model);
	  _allgathers.push_back(&_pgallgather);

	  _pgallgatherv._colltype=XMI::CollInfo::CI_ALLGATHERV0;
          _nbColl.instance()->multisend_reg(TSPColl::AllgathervTag, &_pgallgatherv._model);
	  _allgathervs.push_back(&_pgallgatherv);

	  _pgscatter._colltype=XMI::CollInfo::CI_SCATTER0;
          _nbColl.instance()->multisend_reg(TSPColl::ScatterTag, &_pgscatter._smodel);
          _nbColl.instance()->multisend_reg(TSPColl::BarrierTag, &_pgscatter._bmodel);
	  _scatters.push_back(&_pgscatter);

	  _pgscatterv._colltype=XMI::CollInfo::CI_SCATTERV0;
          _nbColl.instance()->multisend_reg(TSPColl::ScattervTag, &_pgscatterv._smodel);
          _nbColl.instance()->multisend_reg(TSPColl::BarrierTag, &_pgscatterv._bmodel);
	  _scattervs.push_back(&_pgscatterv);

	  _pgallreduce._colltype=XMI::CollInfo::CI_ALLREDUCE0;
          _nbColl.instance()->multisend_reg(TSPColl::ShortAllreduceTag, &_pgallreduce._model);
	  _allreduces.push_back(&_pgallreduce);

	  _pgbarrier._colltype=XMI::CollInfo::CI_BARRIER0;
          _nbColl.instance()->multisend_reg(TSPColl::BarrierTag, &_pgbarrier._model);
	  _barriers.push_back(&_pgbarrier);

          return XMI_SUCCESS;
        }
      
      inline T_Collfactory * analyze_impl(T_Geometry *geometry)
      {

	XMI_COLLFACTORY_CLASS *f=(XMI_COLLFACTORY_CLASS *)_fact_alloc.allocateObject();
	new(f)XMI_COLLFACTORY_CLASS();
	f->setGeometry(geometry, &_nbColl);
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
      XMI_NBCollManager                _nbColl;
      XMI::CollInfo::PGBroadcastInfo   _pgbroadcast;
      XMI::CollInfo::PGAllgatherInfo   _pgallgather;
      XMI::CollInfo::PGAllgathervInfo  _pgallgatherv;
      XMI::CollInfo::PGScatterInfo     _pgscatter;
      XMI::CollInfo::PGScattervInfo    _pgscatterv;
      XMI::CollInfo::PGAllreduceInfo   _pgallreduce;
      XMI::CollInfo::PGBarrierInfo     _pgbarrier;
      
      RegQueue          _broadcasts;
      RegQueue          _allgathers;
      RegQueue          _allgathervs;
      RegQueue          _scatters;
      RegQueue          _scattervs;
      RegQueue          _allreduces;
      RegQueue          _barriers;
      
      MemoryAllocator<sizeof(XMI_COLLFACTORY_CLASS), 16> _fact_alloc;

      MPIMcastModel     _ccmi_broadcast;      
      MPIMcastModel     _ccmi_allreduce;
      MPIMcastModel     _ccmi_alltoall;
      MPIMcastModel     _ccmi_barrier;
      MPIMcastModel     _ccmi_ambroadcast;

    }; // class Collregistration
  };  // namespace Collregistration
}; // namespace XMI
#endif
