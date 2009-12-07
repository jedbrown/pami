
#ifndef   __mpi_native_interface_h__
#define   __mpi_native_interface_h__

#include "common/GlobalInterface.h"
#include "common/mpi/Context.h"
#include "common/mpi/Global.h"
#include "Mapping.h"
#include "algorithms/interfaces/NativeInterface.h"
#include "util/ccmi_util.h"

#include "components/devices/mpi/mpimessage.h"
#include "components/devices/mpi/mpipacketmodel.h"
//#include "p2p/protocols/send/adaptive/Adaptive.h"
#include "p2p/protocols/send/eager/Eager.h"
//#include "p2p/protocols/send/eager/EagerSimple.h"
//#include "p2p/protocols/send/eager/EagerImmediate.h"
#include "components/devices/mpi/MPIBcastMsg.h"
#include "components/devices/mpi/mpimulticastprotocol.h"

extern XMI::Global __global;

#define DISPATCH_START 3

namespace XMI {

  template <class T_Device>
  class MPINativeInterface : public CCMI::Interfaces::NativeInterface {

  typedef XMI::Device::MPIMessage MPIMessage;
  typedef XMI::Device::MPIPacketModel<T_Device,MPIMessage> MPIPacketModel;
  typedef XMI::Protocol::Send::Eager <MPIPacketModel,T_Device> EagerMPI;
  typedef XMI::Protocol::MPI::P2pDispatchMulticastProtocol<T_Device,EagerMPI,XMI::Device::MPIBcastMdl> P2pDispatchMulticastProtocol;

  public:
  MPINativeInterface(T_Device *dev): CCMI::Interfaces::NativeInterface(__global.mapping.task(), __global.mapping.size()), _device(dev), _protocol(_status) {}
  
    /// \brief this call is called when the native interface is initialized
    virtual void setDispatch (xmi_dispatch_callback_fn fn, void *cookie) {
      static size_t dispatch = DISPATCH_START;

      _protocol.registerMcastRecvFunction (dispatch, fn.multicast, cookie);

      //CCMI_assert (_status == XMI_SUCCESS);
      dispatch ++;
    }

    virtual xmi_result_t multicast    (xmi_multicast_t *mcast) { XMI_Multicast (mcast); }
    virtual xmi_result_t multysync    (xmi_multisync_t *msync) { XMI_Multisync (msync); }
    virtual xmi_result_t multicombine (xmi_multicombine_t *mcombine) { XMI_Multicombine (mcombine); }

  private:
    T_Device                 *_device;

    P2pDispatchMulticastProtocol      _protocol;
    xmi_result_t                      _status;
  };
};

#endif
