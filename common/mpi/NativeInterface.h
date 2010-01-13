/*
 * \file components/geometry/mpi/MPINativeInterface.h
 * \brief ???
 */

#ifndef __components_geometry_mpi_MPINativeInterface_h__
#define __components_geometry_mpi_MPINativeInterface_h__

#include "common/GlobalInterface.h"
#include "Global.h"
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

#if 0
  typedef XMI::Device::MPIMessage MPIMessage;
  typedef XMI::Device::MPIPacketModel<T_Device,MPIMessage> MPIPacketModel;
  typedef XMI::Protocol::Send::Eager <MPIPacketModel,T_Device> EagerMPI;
  typedef XMI::Protocol::MPI::P2pDispatchMulticastProtocol<T_Device,EagerMPI,XMI::Device::MPIBcastMdl> P2pDispatchMulticastProtocol;
#endif

  public:
  MPINativeInterface(T_Device *dev, xmi_client_t client, xmi_context_t context, size_t context_id): CCMI::Interfaces::NativeInterface(__global.mapping.task(), __global.mapping.size()), _device(dev), /*_protocol(_status),*/ _dispatch(0), _client(client), _context(context), _contextid(context_id) {}

    /// \brief this call is called when the native interface is initialized
    virtual void setDispatch (xmi_dispatch_callback_fn fn, void *cookie) {
      static size_t dispatch = DISPATCH_START;

      //fprintf (stderr, "In set dispatch, id = %d\n", dispatch);
      /*new (&_protocol) P2pDispatchMulticastProtocol(dispatch, fn.multicast, cookie, *_device, _client, _context, _contextid, _status);
      //_protocol.registerMcastRecvFunction (dispatch, fn.multicast, cookie);*/

      xmi_dispatch_hint_t        options;
      memset(&options, 0x00, sizeof(options));
      options.type = XMI_MULTICAST;
      options.config = NULL;
      options.hint.multicast.global = 1;
      options.hint.multicast.one_sided = 1;
      options.hint.multicast.active_message = 1;
      XMI_Dispatch_set_new (_context, dispatch, fn, cookie, options);

      CCMI_assert (_status == XMI_SUCCESS);
      _dispatch = dispatch;
      dispatch ++;
    }

    virtual xmi_result_t multicast (xmi_multicast_t *mcast) {

#if 0
      if (mcast->src_participants == NULL)
	printf ("%d: Calling xmi_multicast recv\n", myrank());
      else
	printf ("%d: Calling xmi_multicast send\n", myrank());
#endif

      mcast->dispatch =  _dispatch;
      mcast->client   =  _client;
      mcast->context  =  _contextid;
      return XMI_Multicast (mcast);
    }
    virtual xmi_result_t multysync    (xmi_multisync_t *msync) { return XMI_Multisync (msync); }
    virtual xmi_result_t multicombine (xmi_multicombine_t *mcombine) { return XMI_Multicombine (mcombine); }

  private:
    T_Device                 *_device;

    //    P2pDispatchMulticastProtocol      _protocol;
    xmi_result_t                      _status;
    unsigned                          _dispatch;
    xmi_client_t                      _client;
    xmi_context_t                     _context;
    size_t                            _contextid;
  };
};

#endif
