#ifndef   __xmi_mpicollinfo__h__
#define   __xmi_mpicollinfo__h__

#include <vector>
#include "components/geometry/CollRegistration.h"
#include "components/devices/mpi/oldmpimulticastmodel.h"
#include "components/devices/mpi/mpimessage.h"
#include "collectives/components/devices/mpi/mpidevice.h"
#include "components/memory/heap/HeapMemoryManager.h"
#include "components/mapping/mpi/mpimapping.h"
#include "components/sysdep/mpi/mpisysdep.h"
#include "algorithms/protocols/tspcoll/NBCollManager.h"
#include "algorithms/protocols/tspcoll/NBColl.h"

typedef XMI::Device::MPIOldmulticastModel<XMI::Device::MPIDevice<XMI::SysDep::MPISysDep>, XMI::Device::MPIMessage> MPIMcastModel;
typedef TSPColl::NBCollManager<MPIMcastModel> XMI_NBCollManager;


namespace XMI
{
  namespace CollInfo
  {
    typedef enum
    {
      CI_BROADCAST0=0,
      CI_ALLGATHER0,
      CI_ALLGATHERV0,
      CI_SCATTER0,
      CI_SCATTERV0,
      CI_ALLREDUCE0,
      CI_BARRIER0,
    }collinfo_type_t;


    template <class T_Device>
    class CollInfo
    {
    public:
      CollInfo(T_Device *dev) {}
      collinfo_type_t _colltype;
    };
    
    template <class T_Device>
    class PGBroadcastInfo:public CollInfo<T_Device>
    {
    public:
    PGBroadcastInfo(T_Device *dev):
      CollInfo<T_Device>(dev),
      _model(*dev){}
      MPIMcastModel _model;
    };

    template <class T_Device>
    class PGAllgatherInfo:public CollInfo<T_Device>
    {
    public:
    PGAllgatherInfo(T_Device *dev):
      CollInfo<T_Device>(dev),
	_model(*dev){}
      MPIMcastModel _model;
    };

    template <class T_Device>
    class PGAllgathervInfo:public CollInfo<T_Device>
    {
    public:
    PGAllgathervInfo(T_Device *dev):
      CollInfo<T_Device>(dev),
	_model(*dev){}
      MPIMcastModel _model;
    };

    template <class T_Device>
    class PGScatterInfo:public CollInfo<T_Device>
    {
    public:
    PGScatterInfo(T_Device *dev):
      CollInfo<T_Device>(dev),
	_smodel(*dev),
	_bmodel(*dev){}
      
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
	_bmodel(*dev){}
      
      MPIMcastModel _smodel;
      MPIMcastModel _bmodel;
    };

    template <class T_Device>
    class PGAllreduceInfo:public CollInfo<T_Device>
    {
    public:
    PGAllreduceInfo(T_Device *dev):
      CollInfo<T_Device>(dev),
	_model(*dev){}

      MPIMcastModel _model;
    };

    template <class T_Device>
    class PGBarrierInfo:public CollInfo<T_Device>
    {
    public:
    PGBarrierInfo(T_Device *dev):
      CollInfo<T_Device>(dev),
	_model(*dev){}

      MPIMcastModel _model;
    };

  };
};
typedef XMI::Device::MPIDevice<XMI::SysDep::MPISysDep> MPIDevice;
typedef std::vector<XMI::CollInfo::CollInfo<MPIDevice> *> RegQueue;

#endif
