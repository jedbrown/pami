/*
 * \file common/lapiunix/NativeInterface.h
 * \brief ???
 */

#ifndef __common_lapiunix_NativeInterface_h__
#define __common_lapiunix_NativeInterface_h__

#include "Global.h"
#include "algorithms/interfaces/NativeInterface.h"
#include "util/ccmi_util.h"

#include "components/devices/MulticastModel.h"
#include "components/devices/MultisyncModel.h"
#include "components/devices/MulticombineModel.h"

#include "components/memory/MemoryAllocator.h"

#ifndef TRACE_ERR
  #define TRACE_ERR(x) //fprintf x
#endif

extern PAMI::Global __global;

#define DISPATCH_START 0x10

namespace PAMI
{
  template <class T_Device, class T_Mcast, class T_Msync, class T_Mcomb>
  class LAPINativeInterface : public CCMI::Interfaces::NativeInterface
  {
  public:
    inline LAPINativeInterface(T_Device &device, pami_client_t client, pami_context_t context, size_t context_id, size_t client_id);

    /// \brief this call is called when the native interface is
    /// initialized to set the mcast dispatch
    virtual inline pami_result_t setDispatch (pami_dispatch_callback_fn fn, void *cookie);

    /// Virtual interfaces (from base \see CCMI::Interfaces::NativeInterface)
    virtual inline pami_result_t multicast    (pami_multicast_t    *);
    virtual inline pami_result_t multisync    (pami_multisync_t    *);
    virtual inline pami_result_t multicombine (pami_multicombine_t *);

    // Model-specific interfaces
    inline pami_result_t multicast    (uint8_t (&)[T_Mcast::sizeof_msg], pami_multicast_t    *);
    inline pami_result_t multisync    (uint8_t (&)[T_Msync::sizeof_msg], pami_multisync_t    *);
    inline pami_result_t multicombine (uint8_t (&)[T_Mcomb::sizeof_msg], pami_multicombine_t *);

    static const size_t multicast_sizeof_msg     = T_Mcast::sizeof_msg;
    static const size_t multisync_sizeof_msg     = T_Msync::sizeof_msg;
    static const size_t multicombine_sizeof_msg  = T_Mcomb::sizeof_msg;

  private:
    /// \brief NativeInterface done function - free allocation and call client's done
    static void ni_client_done(pami_context_t  context,
                               void          *rdata,
                               pami_result_t   res);

    /// Allocation object to store state and user's callback
    class allocObj
    {
  public:
      union
      {
        uint8_t             _mcast[T_Mcast::sizeof_msg];
        uint8_t             _msync[T_Msync::sizeof_msg];
        uint8_t             _mcomb[T_Mcomb::sizeof_msg];
      } _state;
      LAPINativeInterface *_ni;
      pami_callback_t      _user_callback;
    };

    PAMI::MemoryAllocator<sizeof(allocObj),16> _allocator;  // Allocator

    pami_result_t              _mcast_status;
    pami_result_t              _msync_status;
    pami_result_t              _mcomb_status;

    T_Mcast                   _mcast;
    T_Msync                   _msync;
    T_Mcomb                   _mcomb;

    unsigned                  _dispatch;
    pami_client_t              _client;
    pami_context_t             _context;
    size_t                    _contextid;
    size_t                    _clientid;
  }; // class LAPINativeInterface

  ///////////////////////////////////////////////////////////////////////////////
  // Inline implementations
  ///////////////////////////////////////////////////////////////////////////////
  template <class T_Device, class T_Mcast, class T_Msync, class T_Mcomb>
  LAPINativeInterface<T_Device,T_Mcast,T_Msync,T_Mcomb>::LAPINativeInterface(T_Device      &device,
                       pami_client_t   client,
                       pami_context_t  context,
                       size_t         context_id,
                       size_t         client_id):
      CCMI::Interfaces::NativeInterface(__global.mapping.task(),
                                        __global.mapping.size()),
  _allocator(),

  _mcast_status(PAMI_SUCCESS),
  _msync_status(PAMI_SUCCESS),
  _mcomb_status(PAMI_SUCCESS),

  _mcast(device,_mcast_status),
  _msync(device,_msync_status),
  _mcomb(device,_mcomb_status),

      _dispatch(0),
      _context(context),
      _contextid(context_id),
      _clientid(client_id)
      {
    TRACE_ERR((stderr, "<%p>%s %d %d %d\n", this, __PRETTY_FUNCTION__,
               _mcast_status, _msync_status, _mcomb_status));

    PAMI_assert(_mcast_status == PAMI_SUCCESS);
    PAMI_assert(_msync_status == PAMI_SUCCESS);
    PAMI_assert(_mcomb_status == PAMI_SUCCESS);
  }

  template <class T_Device, class T_Mcast, class T_Msync, class T_Mcomb>
  inline void LAPINativeInterface<T_Device,T_Mcast,T_Msync,T_Mcomb>::ni_client_done(pami_context_t  context,
                                                                                   void          *rdata,
                                                                                   pami_result_t   res)
  {
    allocObj           *obj = (allocObj*)rdata;
    LAPINativeInterface *ni   = obj->_ni;

    TRACE_ERR((stderr, "<%p>LAPINativeInterface::ni_client_done(%p, %p, %d) calling %p(%p)\n",
               ni, context, rdata, res,
               obj->_user_callback.function, obj->_user_callback.clientdata));

    if (obj->_user_callback.function)
      obj->_user_callback.function(context,
                                   obj->_user_callback.clientdata,
                                   res);
    ni->_allocator.returnObject(obj);
      }

    /// \brief this call is called when the native interface is initialized
  template <class T_Device, class T_Mcast, class T_Msync, class T_Mcomb>
  inline pami_result_t LAPINativeInterface<T_Device,T_Mcast,T_Msync,T_Mcomb>::setDispatch (pami_dispatch_callback_fn fn, void *cookie)
      {
      static size_t        dispatch = DISPATCH_START;

    TRACE_ERR((stderr, "<%p>LAPINativeInterface::setDispatch(%p, %p) id=%zu\n",
               this, fn.multicast,  cookie,  dispatch));

    pami_result_t result = _mcast.registerMcastRecvFunction(dispatch, fn.multicast, cookie);

      _dispatch = dispatch;
      dispatch ++;
    return result;
    }

  template <class T_Device, class T_Mcast, class T_Msync, class T_Mcomb>
  inline pami_result_t LAPINativeInterface<T_Device,T_Mcast,T_Msync,T_Mcomb>::multicast (pami_multicast_t *mcast)
      {
    allocObj *req          = (allocObj *)_allocator.allocateObject();
    req->_ni               = this;
    req->_user_callback    = mcast->cb_done;
    TRACE_ERR((stderr, "<%p>LAPINativeInterface::multicast(%p) %p\n",
               this, mcast, req));

    //  \todo:  this copy will cause a latency hit, maybe we need to change postMultisync
    //          interface so we don't need to copy
    pami_multicast_t  m     = *mcast;

    m.dispatch =  _dispatch; // \todo ? Not really used in C++ objects?
    m.client   =  _clientid;   // \todo ? Why doesn't caller set this?
    m.context  =  _contextid;// \todo ? Why doesn't caller set this?

    m.cb_done.function     =  ni_client_done;
    m.cb_done.clientdata   =  req;

    return _mcast.postMulticast(req->_state._mcast, &m);
      }


  // Multisync Code

  template <class T_Device, class T_Mcast, class T_Msync, class T_Mcomb>
  inline pami_result_t LAPINativeInterface<T_Device,T_Mcast,T_Msync,T_Mcomb>::multisync(pami_multisync_t *msync)
      {
    allocObj *req          = (allocObj *)_allocator.allocateObject();
        req->_ni               = this;
        req->_user_callback    = msync->cb_done;
    TRACE_ERR((stderr, "<%p>LAPINativeInterface::multisync(%p) %p\n", this, msync, req));

        pami_multisync_t  m     = *msync;

    m.client   =  _clientid;
    m.context  =  _contextid;

    m.cb_done.function     =  ni_client_done;
        m.cb_done.clientdata   =  req;
    _msync.postMultisync(req->_state._msync, &m);
        return PAMI_SUCCESS;
      }


  template <class T_Device, class T_Mcast, class T_Msync, class T_Mcomb>
  inline pami_result_t LAPINativeInterface<T_Device,T_Mcast,T_Msync,T_Mcomb>::multicombine (pami_multicombine_t *mcomb)
  {
    allocObj *req          = (allocObj *)_allocator.allocateObject();
    req->_ni               = this;
    req->_user_callback    = mcomb->cb_done;
    TRACE_ERR((stderr, "<%p>LAPINativeInterface::multicombine(%p) %p\n",
               this, mcomb, req));

    pami_multicombine_t  m     = *mcomb;

    m.client   =  _clientid;
    m.context  =  _contextid;

    m.cb_done.function     =  ni_client_done;
    m.cb_done.clientdata   =  req;

    return _mcomb.postMulticombine(req->_state._mcomb, &m);
  }

  template <class T_Device, class T_Mcast, class T_Msync, class T_Mcomb>
  inline pami_result_t  LAPINativeInterface<T_Device,T_Mcast,T_Msync,T_Mcomb>::multicast (uint8_t (&state)[T_Mcast::sizeof_msg],
                                                                                        pami_multicast_t *mcast)
      {
    TRACE_ERR((stderr, "<%p>LAPINativeInterface::multicast(%p, %p)\n",
               this, &state, mcast));
    mcast->dispatch =  _dispatch;

    return _mcast.postMulticast_impl(state, mcast);
      }

  template <class T_Device, class T_Mcast, class T_Msync, class T_Mcomb>
  inline pami_result_t  LAPINativeInterface<T_Device,T_Mcast,T_Msync,T_Mcomb>::multisync (uint8_t (&state)[T_Msync::sizeof_msg],
                                                                                        pami_multisync_t *msync)
  {
    TRACE_ERR((stderr, "<%p>LAPINativeInterface::multisync(%p, %p)\n",
               this, &state, msync));

    return _msync.postMultisync_impl(state, msync);
  }

  template <class T_Device, class T_Mcast, class T_Msync, class T_Mcomb>
  inline pami_result_t  LAPINativeInterface<T_Device,T_Mcast,T_Msync,T_Mcomb>::multicombine (uint8_t (&state)[T_Mcomb::sizeof_msg],
                                                                                           pami_multicombine_t *mcomb)
  {
    TRACE_ERR((stderr, "<%p>LAPINativeInterface::multicombine(%p, %p)\n",
               this, &state, mcomb));

    return _mcomb.postMulticombine_impl(state, mcomb);
  }




};

#endif
