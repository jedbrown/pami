#ifndef   __xmi_mpicollregistration__h__
#define   __xmi_mpicollregistration__h__

#define XMI_COLLREGISTRATION_CLASS XMI::CollRegistration::MPI

#include <map>
#include <vector>
#include "components/geometry/mpi/mpicollinfo.h"

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
          _nbColl.instance()->multisend_reg(TSPColl::BcastTag, &_pgbroadcast._model);
	  _broadcasts.push_back(&_pgbroadcast);

          _nbColl.instance()->multisend_reg(TSPColl::AllgatherTag, &_pgallgather._model);
	  _allgathers.push_back(&_pgallgather);

          _nbColl.instance()->multisend_reg(TSPColl::AllgathervTag, &_pgallgatherv._model);
	  _allgathervs.push_back(&_pgallgatherv);

          _nbColl.instance()->multisend_reg(TSPColl::ScatterTag, &_pgscatter._smodel);
          _nbColl.instance()->multisend_reg(TSPColl::BarrierTag, &_pgscatter._bmodel);
	  _scatters.push_back(&_pgscatter);

          _nbColl.instance()->multisend_reg(TSPColl::ScattervTag, &_pgscatterv._smodel);
          _nbColl.instance()->multisend_reg(TSPColl::BarrierTag, &_pgscatterv._bmodel);
	  _scattervs.push_back(&_pgscatterv);

          _nbColl.instance()->multisend_reg(TSPColl::ShortAllreduceTag, &_pgallreduce._model);
	  _allreduces.push_back(&_pgallreduce);

          _nbColl.instance()->multisend_reg(TSPColl::BarrierTag, &_pgbarrier._model);
	  _barriers.push_back(&_pgbarrier);

          return XMI_SUCCESS;
        }
      
      inline T_Collfactory * analyze_impl(T_Geometry *geometry)
        {
          return NULL;
        }

    public:
      XMI_NBCollManager _nbColl;
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
      


      MPIMcastModel     _ccmi_broadcast;      
      MPIMcastModel     _ccmi_allreduce;
      MPIMcastModel     _ccmi_alltoall;
      MPIMcastModel     _ccmi_barrier;
      MPIMcastModel     _ccmi_ambroadcast;

    }; // class Collregistration
  };  // namespace Collregistration
}; // namespace XMI
#endif
