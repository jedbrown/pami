#ifndef   __xmi_mpicollregistration__h__
#define   __xmi_mpicollregistration__h__

#define XMI_COLLREGISTRATION_CLASS XMI::CollRegistration::MPI

#include <map>
#include "components/geometry/CollRegistration.h"
#include "components/devices/mpi/oldmpimulticastmodel.h"
#include "components/devices/mpi/mpimessage.h"
#include "collectives/components/devices/mpi/mpidevice.h"
#include "components/memory/heap/HeapMemoryManager.h"
#include "components/mapping/mpi/mpimapping.h"
#include "components/sysdep/mpi/mpisysdep.h"

#include "algorithms/protocols/tspcoll/NBColl.h"


typedef XMI::Device::MPIOldmulticastModel<XMI::Device::MPIDevice<XMI::SysDep::MPISysDep>, XMI::Device::MPIMessage> MPIMcastModel;
typedef TSPColl::NBCollManager<MPIMcastModel> XMI_NBCollManager;


namespace XMI
{
  namespace CollRegistration
  {
    template <class T_Geometry, class T_Collfactory>
    class MPI : public CollRegistration<XMI::CollRegistration::MPI<T_Geometry, T_Collfactory>, T_Geometry, T_Collfactory>
    {
    public:
      inline MPI(XMI::Device::MPIDevice<XMI::SysDep::MPISysDep> *dev):
        CollRegistration<XMI::CollRegistration::MPI<T_Geometry, T_Collfactory>, T_Geometry, T_Collfactory>()
        {
          // Pgasrt broadcast call
          XMI_NBCollManager::instance()->multisend_reg(TSPColl::BcastTag, &_pgasrt_broadcast);
          
          
          
#if 0
//          XMI::Device::MPIDevice<MPISysDep> *dev
//          XMI::Device::MPIDevice *dev
          CCMI::Adaptor::Generic::OldMulticastImpl *minfo = 
            new(registration) CCMI::Adaptor::Generic::OldMulticastImpl();
          COMPILE_TIME_ASSERT(sizeof(*minfo) < sizeof (*registration));
          minfo->initialize(_g_generic_adaptor);
          TSPColl::NBCollManager::instance()->multisend_reg(TSPColl::BcastTag, minfo);		    
          return XMI_SUCCESS;
#endif
          
          
          
          
        }

      inline T_Collfactory * analyze_impl(T_Geometry *geometry)
        {
          return XMI_UNIMPL;
        }

    public:
      std::map<MPIMcastModel*, int> collectiveid;
      MPIMcastModel _ccmi_broadcast;
      MPIMcastModel _pgasrt_broadcast;
      MPIMcastModel _pgasrt_allgather;
      MPIMcastModel _pgasrt_allgatherv;
      MPIMcastModel _pgasrt_scatter;
      MPIMcastModel _pgasrt_scatterv;
      MPIMcastModel _ccmi_allreduce;
      MPIMcastModel _pgasrt_allreduce;
      MPIMcastModel _ccmi_alltoall;
      MPIMcastModel _ccmi_barrier;
      MPIMcastModel _pgasrt_barrier;
      MPIMcastModel _ccmi_ambroadcast;

      
//      XMI::Device::MPIOldmulticastModel<XMI::Device::MPIDevice<XMI::SysDep::MPISysDep>, XMI::Device::MPIMessage> _ccmi_mcast;
//      XMI::Device::MPIOldmulticastModel<XMI::Device::MPIDevice<XMI::SysDep::MPISysDep>, XMI::Device::MPIMessage> _pgasrt_mcast;

      
    }; // class Collregistration
  };  // namespace Collregistration
}; // namespace XMI
#endif
