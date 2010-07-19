/*
 * \file common/lapiunix/CSNativeInterface.h
 * \brief ???
 */

#ifndef __common_lapiunix_CSNativeInterface_h__
#define __common_lapiunix_CSNativeInterface_h__

#include "Global.h"
#include "algorithms/interfaces/NativeInterface.h"
#include "util/ccmi_util.h"

#include "components/devices/MulticastModel.h"
#include "components/devices/MultisyncModel.h"
#include "components/devices/MulticombineModel.h"

#include "components/memory/MemoryAllocator.h"

#ifndef TRACE_ERR
  #define TRACE_ERR(x) // fprintf x
#endif
extern PAMI::Global __global;

#define DISPATCH_START 0x10

namespace PAMI
{


  template <class T_Model>
  class CSNativeInterface : public CCMI::Interfaces::NativeInterface
  {
  public:
    inline CSNativeInterface(T_Model& model, pami_client_t client, size_t client_id, pami_context_t context, size_t context_id);

    /// Virtual interfaces (from base \see CCMI::Interfaces::NativeInterface)
    virtual inline pami_result_t multicast    (pami_multicast_t    *);
    virtual inline pami_result_t multisync    (pami_multisync_t    *);
    virtual inline pami_result_t multicombine (pami_multicombine_t *);
    virtual inline pami_result_t manytomany (pami_manytomany_t *)
    {
      PAMI_abort();
      return PAMI_ERROR;
    }


    // Model-specific interfaces
    inline pami_result_t multicast    (uint8_t (&)[T_Model::sizeof_multicast_msg], pami_multicast_t    *);
    inline pami_result_t multisync    (uint8_t (&)[T_Model::sizeof_multisync_msg], pami_multisync_t    *);
    inline pami_result_t multicombine (uint8_t (&)[T_Model::sizeof_multicombine_msg], pami_multicombine_t *);

    static const size_t sizeof_multicast_msg     = T_Model::sizeof_multicast_msg;
    static const size_t sizeof_multisync_msg     = T_Model::sizeof_multisync_msg;
    static const size_t sizeof_multicombine_msg  = T_Model::sizeof_multicombine_msg;

  protected:
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
        uint8_t             _mcast[T_Model::sizeof_multicast_msg];
        uint8_t             _msync[T_Model::sizeof_multisync_msg];
        uint8_t             _mcomb[T_Model::sizeof_multicombine_msg];
      } _state;
      CSNativeInterface     *_ni;
      pami_callback_t       _user_callback;
    };

    PAMI::MemoryAllocator < sizeof(allocObj), 16 > _allocator;  // Allocator
    T_Model                 &_model;

    unsigned                _dispatch;
    pami_client_t           _client;
    pami_context_t          _context;
    size_t                  _contextid;
    size_t                  _clientid;
  }; // class CSNativeInterface

  ///////////////////////////////////////////////////////////////////////////////
  // Inline implementations
  ///////////////////////////////////////////////////////////////////////////////
  template <class T_Model>
  inline CSNativeInterface<T_Model>::CSNativeInterface(T_Model& model, pami_client_t client, size_t client_id, pami_context_t context, size_t context_id):
  CCMI::Interfaces::NativeInterface(__global.mapping.task(),
                                    __global.mapping.size()),
  _allocator(),
  _model(model),
  _dispatch(DISPATCH_START),
  _client(client),
  _context(context),
  _contextid(context_id),
  _clientid(client_id)
  {
    // TRACE_ERR((stderr, "<%p>%s\n", this, __PRETTY_FUNCTION__));

  };

  template <class T_Model>
  inline void CSNativeInterface<T_Model>::ni_client_done(pami_context_t  context,
                                                          void          *rdata,
                                                         pami_result_t   res)
  {
    allocObj             *obj = (allocObj*)rdata;
    CSNativeInterface    *ni   = obj->_ni;

    TRACE_ERR((stderr, "<%p>CSNativeInterface::ni_client_done(%p, %p, %d) calling %p(%p)\n",
               ni, context, rdata, res,
               obj->_user_callback.function, obj->_user_callback.clientdata));

    if (obj->_user_callback.function)
      obj->_user_callback.function(context,
                                   obj->_user_callback.clientdata,
                                   res);

    ni->_allocator.returnObject(obj);
  }

  template <class T_Model>
  inline pami_result_t CSNativeInterface<T_Model>::multicast (pami_multicast_t *mcast)
  {
    allocObj *req          = (allocObj *)_allocator.allocateObject();
    req->_ni               = this;
    req->_user_callback    = mcast->cb_done;
    TRACE_ERR((stderr, "<%p>CSNativeInterface::multicast(%p/%p) connection id %u, msgcount %u, bytes %zu\n", this, mcast, req, mcast->connection_id, mcast->msgcount, mcast->bytes));
    DO_DEBUG((templateName<T_Model>()));

    //  \todo:  this copy will cause a latency hit, maybe we need to change postMultisync
    //          interface so we don't need to copy
    // pami_multicast_t  m     = *mcast;
    pami_multicast_t&  m     = *mcast;

    m.dispatch =  _dispatch; // \todo ? Not really used in C++ objects?
    m.client   =  _clientid;   // \todo ? Why doesn't caller set this?
    m.context  =  _contextid;// \todo ? Why doesn't caller set this?

    m.cb_done.function     =  ni_client_done;
    m.cb_done.clientdata   =  req;

    return _model.postMulticast(req->_state._mcast, &m);
  }


  template <class T_Model>
  inline pami_result_t CSNativeInterface<T_Model>::multisync(pami_multisync_t *msync)
  {
    allocObj *req          = (allocObj *)_allocator.allocateObject();
    req->_ni               = this;
    req->_user_callback    = msync->cb_done;
    TRACE_ERR((stderr, "<%p>CSNativeInterface::multisync(%p/%p) connection id %u\n", this, msync, req, msync->connection_id));
    DO_DEBUG((templateName<T_Model>()));

    // pami_multisync_t  m     = *msync;
    pami_multisync_t&  m     = *msync;

    m.client   =  _clientid;
    m.context  =  _contextid;

    m.cb_done.function     =  ni_client_done;
    m.cb_done.clientdata   =  req;
    _model.postMultisync(req->_state._msync, &m);
    return PAMI_SUCCESS;
  }


  template <class T_Model>
  inline pami_result_t CSNativeInterface<T_Model>::multicombine (pami_multicombine_t *mcomb)
  {
    allocObj *req          = (allocObj *)_allocator.allocateObject();
    req->_ni               = this;
    req->_user_callback    = mcomb->cb_done;
    TRACE_ERR((stderr, "<%p>CSNativeInterface::multicombine(%p/%p) connection id %u, count %zu, dt %#X, op %#X\n", this, mcomb, req, mcomb->connection_id, mcomb->count, mcomb->dtype, mcomb->optor));
    DO_DEBUG((templateName<T_Model>()));

    // pami_multicombine_t&  m     = *mcomb;
    pami_multicombine_t&  m     = *mcomb;

    m.client   =  _clientid;
    m.context  =  _contextid;

    m.cb_done.function     =  ni_client_done;
    m.cb_done.clientdata   =  req;

    return _model.postMulticombine(req->_state._mcomb, &m);
  }

  template <class T_Model>
  inline pami_result_t CSNativeInterface<T_Model>::multicast (uint8_t (&state)[T_Model::sizeof_multicast_msg],
                                                                     pami_multicast_t *mcast)
  {
    TRACE_ERR((stderr, "<%p>CSNativeInterface::multicast(%p,%p) connection id %u, msgcount %u, bytes %zu\n", this, &state, mcast, mcast->connection_id, mcast->msgcount, mcast->bytes));
    DO_DEBUG((templateName<T_Model>()));

    mcast->dispatch =  _dispatch;

    return _model.postMulticast_impl(state, mcast);
  }

  template <class T_Model>
  inline pami_result_t CSNativeInterface<T_Model>::multisync (uint8_t (&state)[T_Model::sizeof_multisync_msg],
                                                                     pami_multisync_t *msync)
  {
    TRACE_ERR((stderr, "<%p>CSNativeInterface::multisync(%p,%p) connection id %u\n", this, &state, msync, msync->connection_id));
    DO_DEBUG((templateName<T_Model>()));

    return _model.postMultisync_impl(state, msync);
  }

  template <class T_Model>
  inline pami_result_t CSNativeInterface<T_Model>::multicombine (uint8_t (&state)[T_Model::sizeof_multicombine_msg],
                                                                        pami_multicombine_t *mcomb)
  {
    TRACE_ERR((stderr, "<%p>CSNativeInterface::multicombine(%p,%p) connection id %u, count %zu, dt %#X, op %#X\n", this, &state, mcomb, mcomb->connection_id, mcomb->count, mcomb->dtype, mcomb->optor));
    DO_DEBUG((templateName<T_Model>()));

    return _model.postMulticombine_impl(state, mcomb);
  }


};

#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
