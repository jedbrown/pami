/*
 * \file common/lapiunix/NativeInterface.h
 * \brief ???
 */

#ifndef __common_lapiunix_NativeInterface_h__
#define __common_lapiunix_NativeInterface_h__

#include "common/GlobalInterface.h"
#include "Global.h"
#include "algorithms/interfaces/NativeInterface.h"
#include "util/ccmi_util.h"

#include "components/devices/lapiunix/lapiunixmessage.h"
#include "components/devices/lapiunix/lapiunixmultisyncmodel.h"


extern XMI::Global __global;

#define DISPATCH_START 3

namespace XMI
{
  template <class T_Device>
  class LAPINativeInterface : public CCMI::Interfaces::NativeInterface
  {
    typedef Device::LAPIMSyncMessage LAPIMSyncMessage;
    typedef Device::LAPIMultisyncModel<T_Device,LAPIMSyncMessage> LAPIMultisyncModel;
    typedef XMI::Device::LAPIMessage LAPIMessage;

  public:
    LAPINativeInterface(T_Device      *dev,
                        xmi_context_t  context,
                        size_t         context_id,
                        size_t         client_id):
      CCMI::Interfaces::NativeInterface(__global.mapping.task(),
                                        __global.mapping.size()),
      _msyncAlloc(),
      _device(dev),
      _msync(*_device, _msync_status),
      _dispatch(0),
      _context(context),
      _clientid(client_id)
      _contextid(context_id)
      {
      }

    /// \brief this call is called when the native interface is initialized
    virtual xmi_result_t setDispatch (xmi_dispatch_callback_fn fn, void *cookie)
      {
        static size_t dispatch = DISPATCH_START;

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
	return XMI_SUCCESS;
      }

    virtual xmi_result_t multicast (xmi_multicast_t *mcast)
      {
        mcast->dispatch =  _dispatch;
        mcast->client   =  _clientid;
        mcast->context  =  _contextid;
        return XMI_Multicast (mcast);
      }



    // Multisync Code
    // Including memory allocation/free


    class msyncObj
    {
    public:
      uint8_t             _state[sizeof(LAPIMSyncMessage)];
      LAPINativeInterface *_ni;
      xmi_callback_t      _user_callback;
    };

    static void msync_client_done(xmi_context_t  context,
                                  void          *rdata,
                                  xmi_result_t   res)
      {
        msyncObj           *mobj = (msyncObj*)rdata;
        LAPINativeInterface *ni   = mobj->_ni;
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

    T_Device                 *_device;
    LAPIMultisyncModel        _msync;
    xmi_result_t              _status;
    xmi_result_t              _msync_status;
    unsigned                  _dispatch;
    xmi_context_t             _context;
    size_t                    _clientid;
    size_t                    _contextid;
  };
};

#endif
