/*
 * \file common/mpi/NativeInterface.h
 * \brief ???
 */

#ifndef __common_mpi_NativeInterface_h__
#define __common_mpi_NativeInterface_h__

#include "common/GlobalInterface.h"
#include "Global.h"
#include "algorithms/interfaces/NativeInterface.h"
#include "util/ccmi_util.h"

#include "components/devices/mpi/mpimessage.h"
#include "components/devices/mpi/mpipacketmodel.h"
#include "p2p/protocols/send/eager/Eager.h"
#include "components/devices/mpi/MPIBcastMsg.h"
#include "components/devices/mpi/mpimulticastprotocol.h"
#include "components/devices/mpi/mpimultisyncmodel.h"

extern XMI::Global __global;

#define DISPATCH_START 3

namespace XMI
{
  template <class T_Device>
  class MPINativeInterface : public CCMI::Interfaces::NativeInterface
  {
    typedef Device::MPIMSyncMessage MPIMSyncMessage;
    typedef Device::MPIMultisyncModel<T_Device,MPIMSyncMessage> MPIMultisyncModel;
    typedef XMI::Device::MPIMessage MPIMessage;
    typedef XMI::Device::MPIPacketModel<T_Device,MPIMessage> MPIPacketModel;
    typedef XMI::Protocol::Send::Eager <MPIPacketModel,T_Device> EagerMPI;

  public:
    MPINativeInterface(T_Device      *dev,
                       xmi_client_t   client,
                       xmi_context_t  context,
                       size_t         context_id):
      CCMI::Interfaces::NativeInterface(__global.mapping.task(),
                                        __global.mapping.size()),
      _msyncAlloc(),
      _device(dev), /*_protocol(_status),*/
      _msync(*_device, _msync_status),
      _dispatch(0),
      _client(client),
      _context(context),
      _contextid(context_id)
      {
      }

    /// \brief this call is called when the native interface is initialized
    virtual xmi_result_t setDispatch (xmi_dispatch_callback_fn  fn,
                              void                     *cookie)
      {
      static size_t        dispatch = DISPATCH_START;
      xmi_dispatch_hint_t  options;
      memset(&options, 0x00, sizeof(options));
      options.type                  = XMI_MULTICAST;
      options.config                = NULL;
      options.hint.multicast.global = 1;
      if(fn.multicast == NULL)
        options.hint.multicast.all_sided = 1;
      else
          {
            options.hint.multicast.one_sided = 1;
            options.hint.multicast.active_message = 1;
          }
      XMI_Dispatch_set_new (_context, dispatch, fn, cookie, options);
      CCMI_assert (_status == XMI_SUCCESS);
      _dispatch = dispatch;
      dispatch ++;
      return XMI_SUCCESS;
    }

    virtual xmi_result_t multicast (xmi_multicast_t *mcast)
      {
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

    // Multisync Implementation
    // xmi_client_t       client;	        /**< client to operate within */
    // size_t             context;	        /**< primary context to operate within */
    // void              *request;	        /**< space for operation */
    // xmi_callback_t     cb_done;		/**< User's completion callback */
    // unsigned           connection_id;	/**< (remove?) differentiate data streams */
    // unsigned           roles;		/**< bitmap of roles to perform */
    // xmi_topology_t    *participants;	        /**< Tasks involved in synchronization */
    class msyncObj
      {
      public:
        uint8_t             _state[sizeof(MPIMSyncMessage)];
        MPINativeInterface *_ni;
        xmi_callback_t      _user_callback;
      };

    static void msync_client_done(xmi_context_t  context,
                                  void          *rdata,
                                  xmi_result_t   res)
      {
        msyncObj           *mobj = (msyncObj*)rdata;
        MPINativeInterface *ni   = mobj->_ni;
        if(mobj->_user_callback.function)
          mobj->_user_callback.function(context,
                                        mobj->_user_callback.clientdata,
                                        res);
        ni->_msyncAlloc.returnObject(mobj);
      }

    virtual xmi_result_t multisync    (xmi_multisync_t *msync)
      {
        msyncObj *req          = (msyncObj *)_msyncAlloc.allocateObject();
        req->_ni               = this;
        req->_user_callback    = msync->cb_done;
        //  \todo:  this copy will cause a latency hit, maybe we need to change postMultisync
        //          interface so we don't need to copy
        xmi_multisync_t  m     = *msync;
        m.cb_done.function     =  msync_client_done;
        m.cb_done.clientdata   =  req;
        _msync.postMultisync(req->_state, &m);
        return XMI_SUCCESS;
      }

    virtual xmi_result_t multicombine (xmi_multicombine_t *mcombine)
      {
        return XMI_Multicombine (mcombine);
      }

  private:
    // Allocators
    XMI::MemoryAllocator<sizeof(msyncObj),16> _msyncAlloc;

    T_Device          *_device;
    MPIMultisyncModel  _msync;
    xmi_result_t       _msync_status;
    xmi_result_t       _status;
    unsigned           _dispatch;
    xmi_client_t       _client;
    xmi_context_t      _context;
    size_t             _contextid;





  };
};

#endif
