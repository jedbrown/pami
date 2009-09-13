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



    class CollInfo
    {
    public:
      collinfo_type_t _colltype;
    };
    class PGBroadcastInfo:public CollInfo
    {
    public:
      inline xmi_result_t start(xmi_broadcast_t *broadcast)
        {

        }
        

      MPIMcastModel _model;
    };
    class PGAllgatherInfo:public CollInfo
    {
    public:
      MPIMcastModel _model;
    };

    class PGAllgathervInfo:public CollInfo
    {
    public:
      MPIMcastModel _model;
    };

    class PGScatterInfo:public CollInfo
    {
    public:
      MPIMcastModel _smodel;
      MPIMcastModel _bmodel;
    };
    
    class PGScattervInfo:public CollInfo
    {
    public:
      MPIMcastModel _smodel;
      MPIMcastModel _bmodel;
    };

    class PGAllreduceInfo:public CollInfo
    {
    public:
      MPIMcastModel _model;
    };

    class PGBarrierInfo:public CollInfo
    {
    public:
      MPIMcastModel _model;
    };
    typedef std::vector<CollInfo*> RegQueue;
  };
};
typedef std::vector<XMI::CollInfo::CollInfo*> RegQueue;

#endif
