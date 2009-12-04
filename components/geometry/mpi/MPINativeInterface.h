
#ifndef   __mpi_native_interface_h__
#define   __mpi_native_interface_h__

#include "common/GlobalInterface.h"
#include "common/mpi/Context.h"
#include "common/mpi/Global.h"
#include "Mapping.h"
#include "algorithms/interfaces/NativeInterface.h"

extern XMI::Global __global;

#define DISPATCH_START 3

namespace XMI {

  template <class T_Device>
  class MPINativeInterface : public CCMI::Interfaces::NativeInterface {
  public:
  MPINativeInterface(T_Device *dev): CCMI::Interfaces::NativeInterface(__global.mapping.task(), __global.mapping.size()), _device(dev), _protocol(&_status) {}
  
    /// \brief this call is called when the native interface is initialized
    virtual void setDispatch (xmi_dispatch_callback_fn fn, void *cookie) {
      static size_t dispatch = DISPATCH_START;

      //_device->registerMcastRecvFunction (dispatch, fn.multicast, cookie);

      new (&_protocol) P2pDispatchMulticastProtocol ( dispatch,       
						      fn.multicast,
						      cookie,
						      _device,
						      NULL,
						      NULL,
						      0, 
						      & _status);
      
      CCMI_assert (_status = XMI_SUCCESS);
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
