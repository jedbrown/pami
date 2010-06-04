/*
 * \file common/NativeInterface.h
 * \brief Simple all-sided and active-message Native Interface on a p2p protocol.
 */

#ifndef __common_NativeInterface_h__
#define __common_NativeInterface_h__

#include "Global.h"
#include "algorithms/interfaces/NativeInterface.h"
#include "util/ccmi_util.h"

#include "components/devices/MulticastModel.h"
#include "components/devices/MultisyncModel.h"
#include "components/devices/MulticombineModel.h"

#include "components/memory/MemoryAllocator.h"

#undef TRACE_ERR
#define TRACE_ERR(x) //fprintf x

extern PAMI::Global __global;

#define DISPATCH_START 0x10

namespace PAMI
{

  ///
  /// \brief Provide a Native Interface over a point to point protocol
  /// 
  /// \details See classes below
  /// \see PAMI::NativeInterfaceBase
  /// \see PAMI::NativeInterfaceAllsided
  /// \see PAMI::NativeInterfaceActiveMessage
  /// \see PAMI::Protocol::Send::SendPWQ
  /// \see PAMI::CollRegistration::BGQ:CCMIRegistration
  ///
  /// \example of usage 
  /// 
  /// // Define a point to point protocol (
  ///
  /// typedef Protocol::Send::...<> MyProtocol;
  /// 
  /// // Define a SendPWQ over this protocol
  /// typedef PAMI::Protocol::Send::SendPWQ < MyProtocol > MySendPWQ;
  /// 
  /// // Construct the Native Interface - returns the NI dispatch id.
  /// 
  /// NativeInterfaceActiveMessage *ni = new NativeInterfaceActiveMessage(client, context, context_id, client_id, dispatch);
  /// 
  /// // Generate the protocol with the NI dispatch id and NI dispatch function and the NI pointer as a cookie
  /// 
  ///  pami_dispatch_callback_fn dispatch_fn;
  /// dispatch_fn.p2p = NativeInterfaceActiveMessage::dispatch_p2p;
  /// 
  /// MySendPWQ *protocol = (MySendPWQ*) MySendPWQ::generate(dispatch, dispatch_fn,(void*) ni,  ...);
  ///
  /// // Set the p2p protocol back in the Native Interface
  /// 
  /// ni->setProtocol(protocol);
  ///
  /// This sequence is necessary.  We can't pass the protocol on the NI ctor because it's a
  /// chicken-n-egg problem.  NativeInterface  ctor needs a protocol. Protocol ctor needs a 
  /// dispatch function (NI::dispatch_p2p) and cookie (NI*).
  ///

  namespace NativeInterfaceCommon // Common constants
  {
    static size_t _id = DISPATCH_START;
  }


  //
  // \class NativeInterfaceBase
  // \brief The common base for both active message and all sided NI.
  // \details
  //   - multicast_model_available_buffers_only: does not fully 
  //        support PipeWorkQueue's
  //            
  //      - send side pwq is fully supported.
  //      - receive side pwq in not fully supported.  We must have enough buffer 
  //        in the receive pwq when dispatched to give the p2p protocol the 
  //        necessary receive buffer.  We do not allocate receive buffers.
  // 
  // \todo Possible enhancement - fully support receive pwq with an allocator? or
  //  write a receivePWQ p2p protocol extension like sendPWQ.
  //

  template <class T_Protocol>
  class NativeInterfaceBase : public CCMI::Interfaces::NativeInterface
  {
    public:
    // Model-specific interfaces
    static const size_t multicast_sizeof_msg     = 1024;;///\todo T_Mcast::sizeof_msg - arbitrary - figure it out from data structures/protocol
    static const size_t multisync_sizeof_msg     = 1024;;///\todo T_Msync::sizeof_msg - arbitrary - figure it out from data structures/protocol
    static const size_t multicombine_sizeof_msg  = 1024;;///\todo T_Mcomb::sizeof_msg - arbitrary - figure it out from data structures/protocol
    static const size_t manytomany_sizeof_msg    = 1024;;///\todo T_M2many::sizeof_msg - arbitrary - figure it out from data structures/protocol

    static const size_t multicast_model_msgcount_max            = 16; /// \todo arbitrary - figure it out from protocol max header
    static const size_t multicast_model_bytes_max               = (uint32_t) - 1; /// \todo arbitrary - figure it out from protocol max header
    static const size_t multicast_model_connection_id_max       = (uint32_t) - 1; /// \todo arbitrary - figure it out from protocol max header

    inline NativeInterfaceBase():
    CCMI::Interfaces::NativeInterface(__global.mapping.task(),
                                      __global.mapping.size())
    {
      TRACE_ERR((stderr, "<%p>NativeInterfaceBase()\n", this));
      DO_DEBUG((templateName<T_Protocol>()));
    };

    /// Allocation object to store state and user's callback
    class allocObj
    {
    public:
      enum {MULTICAST,MULTICOMBINE,MULTISYNC,MANYTOMANY}  _type;
      union
      {
        uint8_t             _mcast[multicast_sizeof_msg]; // p2p_multicast_statedata_t
        uint8_t             _msync[multisync_sizeof_msg];
        uint8_t             _mcomb[multicombine_sizeof_msg];
      } _state;
      NativeInterfaceBase  *_ni;
      pami_callback_t       _user_callback;
    };

    // Metadata (ours + user's) passed in the header
    typedef struct __attribute__((__packed__)) _metadata
    {
      uint32_t              connection_id;  ///< Collective connection id \todo expand to 64 bit?
      uint32_t              root;           ///< Root of the collective
      uint32_t              sndlen;         ///< Number of bytes of application data
      // Structure used to pass user's msgdata (consider alignment).
      uint32_t              msgcount;       ///< User's msgcount
      pami_quad_t           msginfo[multicast_model_msgcount_max];  ///< User's msginfo
    } metadata_t __attribute__ ((__aligned__ (16)));

    // State (request) implementation.  
    class p2p_multicast_statedata_t : public Queue::Element
    {
      inline p2p_multicast_statedata_t() :
      Queue::Element ()
      {
      };
    public:
      unsigned               connection_id;
      size_t                 bytes;
      PAMI::PipeWorkQueue   *rcvpwq;
      PAMI::PipeWorkQueue   *sndpwq;
      unsigned               doneCountDown;
      pami_callback_t        cb_done;
      metadata_t             meta;
      pami_send_t            parameters;
      typename T_Protocol::sendpwq_t        sendpwq;
    } ;
  }; // NativeInterfaceBase

  //
  // \todo NativeInterfaceAllsided and NativeInterfaceActiveMessage duplicate
  // a lot of code.  Move it to NativeInterfaceBase or templatize it?
  // 
  // \todo Short protocol?  Use immediate() instead of simple() based on length
  // and/or template parms.  SendPWQ already supports immediatePWQ().
  // 
  //

  //
  // \class NativeInterfaceAllsided
  // \brief A pseudo all-sided native interface based on a p2p protocol
  // \details
  //   - 'all sided' multicast because it does not call a user receive callback, 
  // however it does require the user to ensure the 'receive' multicasts are 
  // called before the 'send'/root multicast so that data pwq's are available.
  // 
  //

  template <class T_Protocol>
  class NativeInterfaceAllsided : public NativeInterfaceBase<T_Protocol>
  {
  public:

    /// \brief ctor
    inline NativeInterfaceAllsided(pami_client_t client, pami_context_t context, size_t context_id, size_t client_id, size_t &dispatch);
    /// Virtual interfaces (from base \see CCMI::Interfaces::NativeInterface)
    virtual inline pami_result_t multicast    (pami_multicast_t    *);
    virtual inline pami_result_t multisync    (pami_multisync_t    *);
    virtual inline pami_result_t multicombine (pami_multicombine_t *);
    virtual inline pami_result_t manytomany   (pami_manytomany_t   *)
    {
      PAMI_abort();
      return PAMI_ERROR;
    }
    /// \brief initial to set the mcast dispatch
    virtual inline pami_result_t setDispatch (pami_dispatch_callback_fn fn, void *cookie);

    inline pami_result_t multicast    (uint8_t (&)[NativeInterfaceBase<T_Protocol>::multicast_sizeof_msg],    pami_multicast_t    *);
    inline pami_result_t multisync    (uint8_t (&)[NativeInterfaceBase<T_Protocol>::multisync_sizeof_msg],    pami_multisync_t    *);
    inline pami_result_t multicombine (uint8_t (&)[NativeInterfaceBase<T_Protocol>::multicombine_sizeof_msg], pami_multicombine_t *);
    inline pami_result_t manytomany   (uint8_t (&)[NativeInterfaceBase<T_Protocol>::manytomany_sizeof_msg],   pami_manytomany_t   *)
    {
      PAMI_abort();
      return PAMI_ERROR;
    }

    /// \brief Multicast model constants/attributes
    static const bool   multicast_model_active_message          = false;
    static const bool   multicast_model_available_buffers_only  = true;

  protected:

    /// \brief NativeInterfaceAllsided done function - free allocation and call client's done
    static void ni_client_done(pami_context_t  context,
                               void          *rdata,
                               pami_result_t   res);

  public:
    ///
    /// \brief Received a p2p dispatch from another root (static function).
    ///
    /// This function must be passed to the P2P protocol constructor
    ///
    /// \todo If we had a protocol::setDispatch after ctor, we could do this internally.
    ///
    static void dispatch_p2p(pami_context_t       context_hdl,
                             void               * cookie,
                             void               * header,
                             size_t               header_size,
                             void               * data,
                             size_t               data_size,
                             pami_recv_t        * recv);

    /// \brief set the protocol pointer.    We can't pass the protocol on the NI ctor because it's a
    /// chicken-n-egg problem.  NI ctor needs a protocol. Protocol ctor needs a dispatch fn (NI::dispatch_p2p) and cookie (NI*).
    ///
    /// I choose to ctor the NI, ctor the protocol, then manually set it with NI::setProtocol().
    ///
    inline void setProtocol(T_Protocol* protocol)
    {
      TRACE_ERR((stderr, "<%p>NativeInterfaceAllsided::setProtocol(%p)\n", this, protocol));
      _protocol = protocol;
    }

  private:
    ///
    /// \brief common internal impl of postMulticast over p2p
    ///
    pami_result_t postMulticast_impl(uint8_t (&state)[NativeInterfaceBase<T_Protocol>::multicast_sizeof_msg],
                                     pami_multicast_t *mcast);

    ///
    /// \brief Received a p2p dispatch from another root (member function).
    ///
    void dispatch(  pami_context_t       context_hdl,
                    void               * header,
                    size_t               header_size,
                    void               * data,
                    size_t               data_size,
                    pami_recv_t        * recv);

    ///
    /// \brief Called as each p2p send is done.
    ///
    static void sendDone ( pami_context_t   context,
                           void          *  cookie,
                           pami_result_t    result );



    PAMI::MemoryAllocator < sizeof(typename NativeInterfaceBase<T_Protocol>::allocObj), 16 > _allocator;  // Allocator


    T_Protocol             *_protocol;

    pami_client_t           _client;
    pami_context_t          _context;
    size_t                  _contextid;
    size_t                  _clientid;

    pami_dispatch_multicast_fn            _mcast_dispatch_function;
    void                                * _mcast_dispatch_arg;
    size_t                                _mcast_dispatch;
    Queue                                 _recvQ;
    //std::map<unsigned,NativeInterfaceBase<T_Protocol>::p2p_multicast_statedata_t*, std::less<unsigned>, __gnu_cxx::malloc_allocator<std::pair<const unsigned,NativeInterfaceBase<T_Protocol>::p2p_multicast_statedata_t*> > >      _recvQ;            // _recvQ[connection_id]
  }; // class NativeInterfaceAllsided

  //
  // \class NativeInterfaceActiveMessage
  // \brief An active-message native interface based on a p2p protocol
  // \details
  //   - inherits from all-sided and adds dispatch/active message capability
  //   - active message model
  // 
  //
  template <class T_Protocol>
  class NativeInterfaceActiveMessage : public NativeInterfaceAllsided<T_Protocol>
  {
  public:

    /// \brief ctor
    inline NativeInterfaceActiveMessage(pami_client_t client, pami_context_t context, size_t context_id, size_t client_id, size_t &dispatch);
    /// Virtual interfaces (from base \see CCMI::Interfaces::NativeInterfaceActiveMessage)
    virtual inline pami_result_t multicast    (pami_multicast_t    *);
    virtual inline pami_result_t multisync    (pami_multisync_t    *);
    virtual inline pami_result_t multicombine (pami_multicombine_t *);
    virtual inline pami_result_t manytomany   (pami_manytomany_t   *)
    {
      PAMI_abort();
      return PAMI_ERROR;
    }
    /// \brief initialize to set the mcast dispatch
    virtual inline pami_result_t setDispatch (pami_dispatch_callback_fn fn, void *cookie);

    inline pami_result_t multicast    (uint8_t (&)[NativeInterfaceBase<T_Protocol>::multicast_sizeof_msg],    pami_multicast_t    *);
    inline pami_result_t multisync    (uint8_t (&)[NativeInterfaceBase<T_Protocol>::multisync_sizeof_msg],    pami_multisync_t    *);
    inline pami_result_t multicombine (uint8_t (&)[NativeInterfaceBase<T_Protocol>::multicombine_sizeof_msg], pami_multicombine_t *);
    inline pami_result_t manytomany   (uint8_t (&)[NativeInterfaceBase<T_Protocol>::manytomany_sizeof_msg],   pami_manytomany_t   *)
    {
      PAMI_abort();
      return PAMI_ERROR;
    }

    /// \brief Multicast model constants/attributes
    static const bool   multicast_model_active_message          = true;
    static const bool   multicast_model_available_buffers_only  = true;


  protected:

    /// \brief NativeInterfaceActiveMessage done function - free allocation and call client's done
    static void ni_client_done(pami_context_t  context,
                               void          *rdata,
                               pami_result_t   res);

  public:
    ///
    /// \brief Received a p2p dispatch from another root (static function).
    ///
    /// This function must be passed to the P2P protocol constructor
    ///
    /// \todo If we had a protocol::setDispatch after ctor, we could do this internally.
    ///
    static void dispatch_p2p(pami_context_t       context_hdl,
                             void               * cookie,
                             void               * header,
                             size_t               header_size,
                             void               * data,
                             size_t               data_size,
                             pami_recv_t        * recv);

    /// \brief set the protocol pointer.    We can't pass the protocol on the NI ctor because it's a
    /// chicken-n-egg problem.  NI ctor needs a protocol. Protocol ctor needs a dispatch fn (NI::dispatch_p2p) and cookie (NI*).
    ///
    /// I choose to ctor the NI, ctor the protocol, then manually set it with NI::setProtocol().
    ///
    inline void setProtocol(T_Protocol* protocol)
    {
      TRACE_ERR((stderr, "<%p>NativeInterfaceActiveMessage::setProtocol(%p)\n", this, protocol));
      _protocol = protocol;
    }

  private:
    ///
    /// \brief common internal impl of postMulticast over p2p
    ///
    pami_result_t postMulticast_impl(uint8_t (&state)[NativeInterfaceBase<T_Protocol>::multicast_sizeof_msg],
                                     pami_multicast_t *mcast);

    ///
    /// \brief Received a p2p dispatch from another root (member function).
    ///
    void dispatch(  pami_context_t       context_hdl,
                    void               * header,
                    size_t               header_size,
                    void               * data,
                    size_t               data_size,
                    pami_recv_t        * recv);

    ///
    /// \brief Called as each p2p send is done.
    ///
    static void sendDone ( pami_context_t   context,
                           void          *  cookie,
                           pami_result_t    result );



    PAMI::MemoryAllocator < sizeof(typename NativeInterfaceBase<T_Protocol>::allocObj), 16 > _allocator;  // Allocator


    T_Protocol             *_protocol;

    pami_client_t           _client;
    pami_context_t          _context;
    size_t                  _contextid;
    size_t                  _clientid;

    pami_dispatch_multicast_fn            _mcast_dispatch_function;
    void                                * _mcast_dispatch_arg;
    size_t                                _mcast_dispatch;
  }; // class NativeInterfaceActiveMessage


















  //
  // Inline implementations
  //
  template <class T_Protocol>
  inline NativeInterfaceAllsided<T_Protocol>::NativeInterfaceAllsided(pami_client_t client, pami_context_t context, size_t context_id, size_t client_id, size_t &dispatch):
  NativeInterfaceBase<T_Protocol>(),
  _allocator(),
  _protocol(NULL), /// must be set by setProtocol() before using the NI.
  _client(client),
  _context(context),
  _contextid(context_id),
  _clientid(client_id)
  {
    TRACE_ERR((stderr, "<%p>NativeInterfaceAllsided()\n", this));
    DO_DEBUG((templateName<T_Protocol>()));
  };

  template <class T_Protocol>
  inline void NativeInterfaceAllsided<T_Protocol>::ni_client_done(pami_context_t  context,
                                                                  void          *rdata,
                                                                  pami_result_t   res)
  {
    typename NativeInterfaceBase<T_Protocol>::allocObj             *obj = (typename NativeInterfaceBase<T_Protocol>::allocObj*)rdata;
    NativeInterfaceAllsided *ni   = (NativeInterfaceAllsided *)obj->_ni;

    TRACE_ERR((stderr, "<%p>NativeInterfaceAllsided::ni_client_done(%p, %p, %d) calling %p(%p)\n",
               ni, context, rdata, res,
               obj->_user_callback.function, obj->_user_callback.clientdata));

    if(obj->_type == NativeInterfaceBase<T_Protocol>::allocObj::MULTICAST)
    { 
      // Punch the produce button on the rcvpwq
      typename NativeInterfaceBase<T_Protocol>::p2p_multicast_statedata_t *state = (typename NativeInterfaceBase<T_Protocol>::p2p_multicast_statedata_t*)obj->_state._mcast;
       TRACE_ERR((stderr, "<%p>NativeInterfaceAllsided::ni_client_done pwq<%p>->produce(%zu)\n",
                  ni, state->rcvpwq, state->bytes));
       if(state->rcvpwq) state->rcvpwq->produceBytes(state->bytes); /// \todo ? is this always the right byte count?
       if(state->sndpwq) state->sndpwq->consumeBytes(state->bytes); /// \todo ? is this always the right byte count?
    }

    if (obj->_user_callback.function)
      obj->_user_callback.function(context,
                                   obj->_user_callback.clientdata,
                                   res);
#ifdef CCMI_DEBUG
    typename NativeInterfaceBase<T_Protocol>::p2p_multicast_statedata_t* receive_state = (typename NativeInterfaceBase<T_Protocol>::p2p_multicast_statedata_t*)_recvQ.peekHead();
    while (receive_state && receive_state->connection_id != connection_id)
      receive_state = (typename NativeInterfaceBase<T_Protocol>::p2p_multicast_statedata_t*)_recvQ.nextElem(receive_state);
    PAMI_assert(receive_state!=obj->_state._mcast); // all-sided and sync'd by MU so this shouldn't still be queued
#endif

    ni->_allocator.returnObject(obj);
  }

  template <class T_Protocol>
  inline pami_result_t NativeInterfaceAllsided<T_Protocol>::setDispatch (pami_dispatch_callback_fn fn, void *cookie)
  {

    TRACE_ERR((stderr, "<%p>NativeInterfaceAllsided::setDispatch(%p, %p)\n",
               this, fn.multicast,  cookie));

    if (fn.multicast == NULL) // we allow a no-op dispatch on allsided.
      return PAMI_SUCCESS;

    PAMI_abortf("<%p>NativeInterfaceAllsided::setDispatch(%p, %p)\n", this, fn.multicast,  cookie);
    return PAMI_ERROR;
  }

  template <class T_Protocol>
  inline pami_result_t NativeInterfaceAllsided<T_Protocol>::multicast (pami_multicast_t *mcast)
  {
    typename NativeInterfaceBase<T_Protocol>::allocObj *req          = (typename NativeInterfaceBase<T_Protocol>::allocObj *)_allocator.allocateObject();
    req->_type              = NativeInterfaceBase<T_Protocol>::allocObj::MULTICAST;
    req->_ni               = this;
    req->_user_callback    = mcast->cb_done;
    TRACE_ERR((stderr, "<%p>NativeInterfaceAllsided::multicast(%p/%p) connection id %u, msgcount %u, bytes %zu\n", this, req, mcast, mcast->connection_id, mcast->msgcount, mcast->bytes));
    DO_DEBUG((templateName<T_Protocol>()));

    //  \todo:  this copy will cause a latency hit, maybe we need to change postMultisync
    //          interface so we don't need to copy
    pami_multicast_t  m     = *mcast;

    m.dispatch =  _mcast_dispatch; // \todo ? Not really used in C++ objects?
    m.client   =  _clientid;   // \todo ? Why doesn't caller set this?
    m.context  =  _contextid;// \todo ? Why doesn't caller set this?

    m.cb_done.function     =  ni_client_done;
    m.cb_done.clientdata   =  req;

    postMulticast_impl(req->_state._mcast, &m);
    return PAMI_SUCCESS;
  }


  template <class T_Protocol>
  inline pami_result_t NativeInterfaceAllsided<T_Protocol>::multisync(pami_multisync_t *msync)
  {
    typename NativeInterfaceBase<T_Protocol>::allocObj *req          = (typename NativeInterfaceBase<T_Protocol>::allocObj *)_allocator.allocateObject();
    req->_type              = NativeInterfaceBase<T_Protocol>::allocObj::MULTISYNC;
    req->_ni               = this;
    req->_user_callback    = msync->cb_done;
    TRACE_ERR((stderr, "<%p>NativeInterfaceAllsided::multisync(%p/%p) connection id %u\n", this, msync, req, msync->connection_id));
    DO_DEBUG((templateName<T_Protocol>()));

    pami_multisync_t  m     = *msync;

    m.client   =  _clientid;
    m.context  =  _contextid;

    m.cb_done.function     =  ni_client_done;
    m.cb_done.clientdata   =  req;
    //_msync.postMultisync(req->_state._msync, &m);
    return PAMI_SUCCESS;
  }


  template <class T_Protocol>
  inline pami_result_t NativeInterfaceAllsided<T_Protocol>::multicombine (pami_multicombine_t *mcomb)
  {
    typename NativeInterfaceBase<T_Protocol>::allocObj *req          = (typename NativeInterfaceBase<T_Protocol>::allocObj *)_allocator.allocateObject();
    req->_type              = NativeInterfaceBase<T_Protocol>::allocObj::MULTICOMBINE;
    req->_ni               = this;
    req->_user_callback    = mcomb->cb_done;
    TRACE_ERR((stderr, "<%p>NativeInterfaceAllsided::multicombine(%p/%p) connection id %u, count %zu, dt %#X, op %#X\n", this, mcomb, req, mcomb->connection_id, mcomb->count, mcomb->dtype, mcomb->optor));
    DO_DEBUG((templateName<T_Protocol>()));

    pami_multicombine_t  m     = *mcomb;

    m.client   =  _clientid;
    m.context  =  _contextid;

    m.cb_done.function     =  ni_client_done;
    m.cb_done.clientdata   =  req;

    //_mcomb.postMulticombine(req->_state._mcomb, &m);
    return PAMI_SUCCESS;
  }

  template <class T_Protocol>
  inline pami_result_t NativeInterfaceAllsided<T_Protocol>::postMulticast_impl(uint8_t (&state)[NativeInterfaceBase<T_Protocol>::multicast_sizeof_msg],
                                                                               pami_multicast_t *mcast)
  {
    pami_result_t result;
    COMPILE_TIME_ASSERT(sizeof(typename NativeInterfaceBase<T_Protocol>::p2p_multicast_statedata_t) <= NativeInterfaceBase<T_Protocol>::multicast_sizeof_msg);

//      PAMI_assert(!multicast_model_all_sided || mcast->src != ... not sure what I was trying to assert but think about it...
    typename NativeInterfaceBase<T_Protocol>::p2p_multicast_statedata_t *state_data = (typename NativeInterfaceBase<T_Protocol>::p2p_multicast_statedata_t*) & state;

    // Get the source data buffer/length and validate (assert) inputs
    size_t length = mcast->bytes;
    PAMI::PipeWorkQueue *pwq = (PAMI::PipeWorkQueue *)mcast->src;

    TRACE_ERR((stderr, "<%p>:NativeInterfaceAllsided::postMulticast_impl() dispatch %zu, connection_id %#X, msgcount %d/%p, bytes %zu/%p/%p\n",
               this, mcast->dispatch, mcast->connection_id,
               mcast->msgcount, mcast->msginfo,
               mcast->bytes, pwq, pwq ? pwq->bufferToConsume() : NULL));

    state_data->connection_id = mcast->connection_id;
    state_data->rcvpwq        = (PAMI::PipeWorkQueue*)mcast->dst;
    state_data->sndpwq        = (PAMI::PipeWorkQueue*)mcast->src;
    state_data->bytes         = length;
    // Save the user's done callback
    state_data->cb_done = mcast->cb_done;

    // Get the msginfo buffer/length and validate (assert) inputs
    void* msgdata = (void*)mcast->msginfo;

    PAMI_assert(NativeInterfaceBase<T_Protocol>::multicast_model_msgcount_max >= mcast->msgcount);

    state_data->meta.connection_id = mcast->connection_id;
    state_data->meta.root = this->myrank();
    state_data->meta.sndlen = length;
    state_data->meta.msgcount = mcast->msgcount;
    if (mcast->msgcount) memcpy(state_data->meta.msginfo, msgdata, mcast->msgcount * sizeof(typename NativeInterfaceBase<T_Protocol>::metadata_t().msginfo));

    TRACE_ERR((stderr, "<%p>:NativeInterfaceAllsided::postMulticast_impl() state %p/%p\n",this,state,state_data));
    if(state_data->rcvpwq) _recvQ.pushTail(state_data); // only use recvQ if this mcast expects to receives data.

    // Destinations may return now and wait for data to be dispatched 
    /// \todo Handle metadata only?
    /// \todo handle both pwq's null? Barrier?  
    if (!pwq) return PAMI_SUCCESS;

    void* payload = NULL;

    if (length)
      payload = (void*)pwq->bufferToConsume();

    // calc how big our msginfo needs to be
    size_t msgsize =  sizeof(typename NativeInterfaceBase<T_Protocol>::metadata_t().connection_id) + sizeof(typename NativeInterfaceBase<T_Protocol>::metadata_t().root) + sizeof(typename NativeInterfaceBase<T_Protocol>::metadata_t().sndlen) +
                      sizeof(typename NativeInterfaceBase<T_Protocol>::metadata_t().msgcount) + (mcast->msgcount * sizeof(typename NativeInterfaceBase<T_Protocol>::metadata_t().msginfo));

    // Send the multicast to each destination

    // \todo indexToRank() doesn't always work so convert a local copy to a list topology...
    PAMI::Topology l_dst_participants = *((PAMI::Topology*)mcast->dst_participants);
    l_dst_participants.convertTopology(PAMI_LIST_TOPOLOGY);

    pami_task_t *taskList = NULL;
    l_dst_participants.rankList(&taskList);
    size_t  size    = l_dst_participants.size();

    state_data->doneCountDown = size;

    state_data->parameters.send.hints = (pami_send_hint_t)
    {
      0
    };
    state_data->parameters.send.data.iov_base = payload;
    state_data->parameters.send.data.iov_len = length;
    state_data->parameters.send.header.iov_base = &state_data->meta;
    state_data->parameters.send.header.iov_len = msgsize;
    state_data->parameters.send.dispatch = mcast->dispatch;
    state_data->parameters.events.cookie = state_data;
    state_data->parameters.events.local_fn = sendDone;
    state_data->parameters.events.remote_fn = NULL;

    TRACE_ERR((stderr, "<%p>:NativeInterfaceAllsided::<%p>send() data %zu, header %zu\n", this, _protocol, state_data->parameters.send.data.iov_len, state_data->parameters.send.header.iov_len));

    for (unsigned i = 0; i < size; ++i)
    {
      if (taskList[i] == this->myrank()) // Am I a destination?
      {
        state_data->doneCountDown--;// don't send to myself
        PAMI::PipeWorkQueue *rcvpwq = (PAMI::PipeWorkQueue *)mcast->dst;

        TRACE_ERR((stderr, "<%p>:NativeInterfaceAllsided<%d>::pwq<%p>\n", this, __LINE__,rcvpwq));
        if (rcvpwq && (rcvpwq->bytesAvailableToProduce() >= length)) // copy it if desired
        {
          memcpy(rcvpwq->bufferToProduce(), payload, length);
//          rcvpwq->produceBytes(length);
        }
        continue;
      }

      result = PAMI_Endpoint_create ((pami_client_t) mcast->client, /// \todo client is ignored on the endpoint?  client isn't a pami_client_t
                                     taskList[i],
                                     mcast->context, /// \todo what context do I target?
                                     &state_data->parameters.send.dest);

      TRACE_ERR((stderr, "<%p>:NativeInterfaceAllsided::<%p>send(%u(%zu,%zu))\n", this, _protocol, state_data->parameters.send.dest, (size_t) taskList[i], mcast->context));

      if (result == PAMI_SUCCESS) result = _protocol->simplePWQ(&state_data->sendpwq,_context, &state_data->parameters, pwq);

      TRACE_ERR((stderr, "<%p>:NativeInterfaceAllsided::send(%zu) result %u\n", this, (size_t) taskList[i], result));
    }


    TRACE_ERR((stderr, "<%p>:NativeInterfaceAllsided::postMulticast_impl() dispatch %zu, connection_id %#X exit\n",
               this, mcast->dispatch, mcast->connection_id));

    return PAMI_SUCCESS;

  }; // NativeInterfaceAllsided<T_Protocol>::postMulticast_impl

  ///
  /// \brief Received a p2p dispatch from another root (static function).
  /// Call the member function on the protocol.
  ///
  template <class T_Protocol>
  inline void NativeInterfaceAllsided<T_Protocol>::dispatch_p2p(pami_context_t       context_hdl,  /**< IN:  communication context handle */
                                                                void               * cookie,       /**< IN:  dispatch cookie (pointer to protocol object)*/
                                                                void               * header,       /**< IN:  header address  */
                                                                size_t               header_size,  /**< IN:  header size     */
                                                                void               * data,         /**< IN:  address of PAMI pipe  buffer, valid only if non-NULL        */
                                                                size_t               data_size,    /**< IN:  number of byts of message data, valid regarldless of message type */
                                                                pami_recv_t        * recv)         /**< OUT: receive message structure, only needed if addr is non-NULL */
  {
    TRACE_ERR((stderr, "<%p>NativeInterfaceAllsided::dispatch_p2p header size %zu, data size %zu\n", cookie, header_size, data_size));
    NativeInterfaceAllsided<T_Protocol> *p = (NativeInterfaceAllsided<T_Protocol> *)cookie;
    p->dispatch(context_hdl,
                header,
                header_size,
                data,
                data_size,
                recv);
  }
  ///
  /// \brief Received a p2p dispatch from another root (member function).
  /// Call user's dispatch
  /// \see PAMI::Protocol::Send::Send::..?
  ///
  template <class T_Protocol>
  inline void NativeInterfaceAllsided<T_Protocol>::dispatch(pami_context_t       context_hdl,  /**< IN:  communication context handle */
                                                            void               * header,       /**< IN:  header address  */
                                                            size_t               header_size,  /**< IN:  header size     */
                                                            void               * data,         /**< IN:  address of PAMI pipe  buffer, valid only if non-NULL        */
                                                            size_t               data_size,    /**< IN:  number of byts of message data, valid regarldless of message type */
                                                            pami_recv_t        * recv)         /**< OUT: receive message structure, only needed if addr is non-NULL */
  {

    unsigned connection_id = ((typename NativeInterfaceBase<T_Protocol>::metadata_t*)header)->connection_id;
    size_t bytes           = ((typename NativeInterfaceBase<T_Protocol>::metadata_t*)header)->sndlen;
    //size_t root            = ((NativeInterfaceBase<T_Protocol>::metadata_t*)header)->root;
    PAMI::PipeWorkQueue   *rcvpwq;
    pami_callback_t       cb_done;

    typename NativeInterfaceBase<T_Protocol>::p2p_multicast_statedata_t* receive_state = (typename NativeInterfaceBase<T_Protocol>::p2p_multicast_statedata_t*)_recvQ.peekHead();
    // probably the head, but (unlikely) search if it isn't
    while (receive_state && receive_state->connection_id != connection_id)
      receive_state = (typename NativeInterfaceBase<T_Protocol>::p2p_multicast_statedata_t*)_recvQ.nextElem(receive_state);
    PAMI_assert(receive_state); // all-sided and sync'd by MU so this shouldn't be unexpected data

    bytes   = receive_state->bytes;
    rcvpwq  = receive_state->rcvpwq;
    cb_done = receive_state->cb_done;
    TRACE_ERR((stderr, "<%p>NativeInterfaceAllsided::dispatch() header size %zu, data size %zu/%zu, connection_id %u, root %u, recv %p, receive_state %p\n", this, header_size, data_size, bytes, connection_id, ((typename NativeInterfaceBase<T_Protocol>::metadata_t*)header)->root, recv, receive_state));

    _recvQ.deleteElem(receive_state);

    // I don't think send/recv lets us receive less than was sent, so assert they gave us enough buffer/pwq...
    PAMI_assert_debugf(bytes == data_size, "bytes %zu == %zu data_size\n", bytes, data_size);

    // No data or immediate data? We're done.
    if ((bytes == 0) || (recv == NULL) || (data != NULL))
    {
      TRACE_ERR((stderr, "<%p>NativeInterfaceAllsided::dispatch() immediate\n", this));

      if (data && bytes)
      {
        /// \todo An assertion probably isn't the best choice...
        TRACE_ERR((stderr, "<%p>:NativeInterfaceAllsided<%d>::pwq<%p>\n", this, __LINE__,rcvpwq));
        PAMI_assertf(rcvpwq->bytesAvailableToProduce() >= data_size, "dst %zu >= data_size %zu\n", rcvpwq->bytesAvailableToProduce(), data_size);
        memcpy(rcvpwq->bufferToProduce(), data, bytes);
//        rcvpwq->produceBytes(data_size);
      }

      // call original done
      /** \todo fix or remove this hack */
      if (cb_done.function)
        (cb_done.function)(NULL,//PAMI_Client_getcontext(_client,_contextid),
                           cb_done.clientdata, PAMI_SUCCESS);

      return;
    }

    // multicast_model_available_buffers_only semantics: If you're receiving data then the pwq must be available
    TRACE_ERR((stderr, "<%p>:NativeInterfaceAllsided<%d>::pwq<%p>\n", this, __LINE__,rcvpwq));
    PAMI_assert(multicast_model_available_buffers_only && (rcvpwq->bytesAvailableToProduce() >= data_size));

    recv->addr     = rcvpwq->bufferToProduce();
    recv->type     = PAMI_BYTE;
    recv->offset   = 0;
    recv->local_fn = cb_done.function;
    recv->cookie   = cb_done.clientdata;



  }
  template <class T_Protocol>
  inline void NativeInterfaceAllsided<T_Protocol>::sendDone ( pami_context_t   context,
                                                              void          *  cookie,
                                                              pami_result_t    result )
  {

    typename NativeInterfaceBase<T_Protocol>::p2p_multicast_statedata_t *state_data = (typename NativeInterfaceBase<T_Protocol>::p2p_multicast_statedata_t*)cookie;

    TRACE_ERR((stderr, "<%p>:NativeInterfaceAllsided::sendDone countDown %u\n", cookie, state_data->doneCountDown));

    if (--state_data->doneCountDown == 0)
    {
      // call original done
      if (state_data->cb_done.function)
        (state_data->cb_done.function)(context,
                                       state_data->cb_done.clientdata, PAMI_SUCCESS);

      return;
    }
  }

  template <class T_Protocol>
  inline NativeInterfaceActiveMessage<T_Protocol>::NativeInterfaceActiveMessage(pami_client_t client, pami_context_t context, size_t context_id, size_t client_id, size_t &dispatch):
  NativeInterfaceAllsided<T_Protocol>(client, context, context_id, client_id, dispatch),
  _mcast_dispatch(NativeInterfaceCommon::_id++)
  {
    TRACE_ERR((stderr, "<%p>NativeInterfaceActiveMessage()\n", this));
    DO_DEBUG((templateName<T_Protocol>()));
    dispatch = _mcast_dispatch;
  };

  template <class T_Protocol>
  inline void NativeInterfaceActiveMessage<T_Protocol>::ni_client_done(pami_context_t  context,
                                                                       void          *rdata,
                                                                       pami_result_t   res)
  {
    typename NativeInterfaceBase<T_Protocol>::allocObj             *obj = (typename NativeInterfaceBase<T_Protocol>::allocObj*)rdata;
    NativeInterfaceActiveMessage *ni   = (NativeInterfaceActiveMessage *)obj->_ni;

    TRACE_ERR((stderr, "<%p>NativeInterfaceActiveMessage::ni_client_done(%p, %p, %d) calling %p(%p)\n",
               ni, context, rdata, res,
               obj->_user_callback.function, obj->_user_callback.clientdata));

    if(obj->_type == NativeInterfaceBase<T_Protocol>::allocObj::MULTICAST)
    { // Punch the produce button on the pwq
      typename NativeInterfaceBase<T_Protocol>::p2p_multicast_statedata_t *state = (typename NativeInterfaceBase<T_Protocol>::p2p_multicast_statedata_t*)obj->_state._mcast;
       TRACE_ERR((stderr, "<%p>NativeInterfaceActiveMessage::ni_client_done pwq<%p>->produce(%zu)\n",
                  ni, state->rcvpwq, state->bytes));
       if(state->rcvpwq) state->rcvpwq->produceBytes(state->bytes); /// \todo ? is this always the right byte count?
       if(state->sndpwq) state->sndpwq->consumeBytes(state->bytes); /// \todo ? is this always the right byte count?
    }

    if (obj->_user_callback.function)
      obj->_user_callback.function(context,
                                   obj->_user_callback.clientdata,
                                   res);

    ni->_allocator.returnObject(obj);
  }

  template <class T_Protocol>
  inline pami_result_t NativeInterfaceActiveMessage<T_Protocol>::setDispatch (pami_dispatch_callback_fn fn, void *cookie)
  {

    TRACE_ERR((stderr, "<%p>NativeInterfaceActiveMessage::setDispatch(%p, %p) id=%zu\n",
               this, fn.multicast,  cookie,  _mcast_dispatch));
    DO_DEBUG((templateName<T_Protocol>()));

    this->_mcast_dispatch_arg = cookie;
    this->_mcast_dispatch_function = fn.multicast;

    /// \todo I would really like to set the p2p protocol dispatch now, but that isn't defined...
    /// so my caller must construct the p2p protocol with my dispatch function and this ptr.

    return PAMI_SUCCESS;
  }

  template <class T_Protocol>
  inline pami_result_t NativeInterfaceActiveMessage<T_Protocol>::multicast (pami_multicast_t *mcast)
  {
    typename NativeInterfaceBase<T_Protocol>::allocObj *req          = (typename NativeInterfaceBase<T_Protocol>::allocObj *)_allocator.allocateObject();
    req->_type              = NativeInterfaceBase<T_Protocol>::allocObj::MULTICAST;
    req->_ni               = this;
    req->_user_callback    = mcast->cb_done;
    TRACE_ERR((stderr, "<%p>NativeInterfaceActiveMessage::multicast(%p/%p) connection id %u, msgcount %u, bytes %zu\n", this, req, mcast, mcast->connection_id, mcast->msgcount, mcast->bytes));
    DO_DEBUG((templateName<T_Protocol>()));

    //  \todo:  this copy will cause a latency hit, maybe we need to change postMultisync
    //          interface so we don't need to copy
    pami_multicast_t  m     = *mcast;

    m.dispatch =  _mcast_dispatch; // \todo ? Not really used in C++ objects?
    m.client   =  _clientid;   // \todo ? Why doesn't caller set this?
    m.context  =  _contextid;// \todo ? Why doesn't caller set this?

    m.cb_done.function     =  ni_client_done;
    m.cb_done.clientdata   =  req;

    postMulticast_impl(req->_state._mcast, &m);
    return PAMI_SUCCESS;
  }


  template <class T_Protocol>
  inline pami_result_t NativeInterfaceActiveMessage<T_Protocol>::multisync(pami_multisync_t *msync)
  {
    typename NativeInterfaceBase<T_Protocol>::allocObj *req          = (typename NativeInterfaceBase<T_Protocol>::allocObj *)_allocator.allocateObject();
    req->_type              = NativeInterfaceBase<T_Protocol>::allocObj::MULTISYNC;
    req->_ni               = this;
    req->_user_callback    = msync->cb_done;
    TRACE_ERR((stderr, "<%p>NativeInterfaceActiveMessage::multisync(%p/%p) connection id %u\n", this, msync, req, msync->connection_id));
    DO_DEBUG((templateName<T_Protocol>()));

    pami_multisync_t  m     = *msync;

    m.client   =  _clientid;
    m.context  =  _contextid;

    m.cb_done.function     =  ni_client_done;
    m.cb_done.clientdata   =  req;
    //_msync.postMultisync(req->_state._msync, &m);
    return PAMI_SUCCESS;
  }


  template <class T_Protocol>
  inline pami_result_t NativeInterfaceActiveMessage<T_Protocol>::multicombine (pami_multicombine_t *mcomb)
  {
    typename NativeInterfaceBase<T_Protocol>::allocObj *req          = (typename NativeInterfaceBase<T_Protocol>::allocObj *)_allocator.allocateObject();
    req->_type              = NativeInterfaceBase<T_Protocol>::allocObj::MULTICOMBINE;
    req->_ni               = this;
    req->_user_callback    = mcomb->cb_done;
    TRACE_ERR((stderr, "<%p>NativeInterfaceActiveMessage::multicombine(%p/%p) connection id %u, count %zu, dt %#X, op %#X\n", this, mcomb, req, mcomb->connection_id, mcomb->count, mcomb->dtype, mcomb->optor));
    DO_DEBUG((templateName<T_Protocol>()));

    pami_multicombine_t  m     = *mcomb;

    m.client   =  _clientid;
    m.context  =  _contextid;

    m.cb_done.function     =  ni_client_done;
    m.cb_done.clientdata   =  req;

    //_mcomb.postMulticombine(req->_state._mcomb, &m);
    return PAMI_SUCCESS;
  }

  template <class T_Protocol>
  inline pami_result_t NativeInterfaceActiveMessage<T_Protocol>::postMulticast_impl(uint8_t (&state)[NativeInterfaceBase<T_Protocol>::multicast_sizeof_msg],
                                                                                    pami_multicast_t *mcast)
  {
    TRACE_ERR((stderr, "<%p>NativeInterfaceAllsided::postMulticast_impl\n", this));
    pami_result_t result;
    COMPILE_TIME_ASSERT(sizeof(typename NativeInterfaceBase<T_Protocol>::p2p_multicast_statedata_t) <= NativeInterfaceBase<T_Protocol>::multicast_sizeof_msg);

//      PAMI_assert(!multicast_model_all_sided || mcast->src != ... not sure what I was trying to assert but think about it...
    typename NativeInterfaceBase<T_Protocol>::p2p_multicast_statedata_t *state_data = (typename NativeInterfaceBase<T_Protocol>::p2p_multicast_statedata_t*) & state;

    // Get the source data buffer/length and validate (assert) inputs
    size_t length = mcast->bytes;
    PAMI::PipeWorkQueue *pwq = (PAMI::PipeWorkQueue *)mcast->src;

    TRACE_ERR((stderr, "<%p>:NativeInterfaceActiveMessage::postMulticast_impl() dispatch %zu, connection_id %#X, msgcount %d/%p, bytes %zu/%p/%p\n",
               this, mcast->dispatch, mcast->connection_id,
               mcast->msgcount, mcast->msginfo,
               mcast->bytes, pwq, pwq ? pwq->bufferToConsume() : NULL));

    void* payload = NULL;

    if (length)
      payload = (void*)pwq->bufferToConsume();

    // Save the user's done callback
    state_data->cb_done = mcast->cb_done;

    // Get the msginfo buffer/length and validate (assert) inputs
    void* msgdata = (void*)mcast->msginfo;

    PAMI_assert(NativeInterfaceBase<T_Protocol>::multicast_model_msgcount_max >= mcast->msgcount);

    state_data->meta.connection_id = mcast->connection_id;
    state_data->meta.root = this->myrank();
    state_data->meta.sndlen = length;
    state_data->meta.msgcount = mcast->msgcount;
    memcpy(state_data->meta.msginfo, msgdata, mcast->msgcount * sizeof(typename NativeInterfaceBase<T_Protocol>::metadata_t().msginfo));

    // calc how big our msginfo needs to beg
    size_t msgsize =  sizeof(typename NativeInterfaceBase<T_Protocol>::metadata_t().connection_id) + sizeof(typename NativeInterfaceBase<T_Protocol>::metadata_t().root) + sizeof(typename NativeInterfaceBase<T_Protocol>::metadata_t().sndlen) +
                      sizeof(typename NativeInterfaceBase<T_Protocol>::metadata_t().msgcount) + (mcast->msgcount * sizeof(typename NativeInterfaceBase<T_Protocol>::metadata_t().msginfo));

    // Send the multicast to each destination

    // \todo indexToRank() doesn't always work so convert a local copy to a list topology...
    PAMI::Topology l_dst_participants = *((PAMI::Topology*)mcast->dst_participants);
    l_dst_participants.convertTopology(PAMI_LIST_TOPOLOGY);

    pami_task_t *taskList = NULL;
    l_dst_participants.rankList(&taskList);
    size_t  size    = l_dst_participants.size();

    state_data->doneCountDown = size;

    state_data->parameters.send.hints = (pami_send_hint_t)
    {
      0
    };
    state_data->parameters.send.data.iov_base = payload;
    state_data->parameters.send.data.iov_len = length;
    state_data->parameters.send.header.iov_base = &state_data->meta;
    state_data->parameters.send.header.iov_len = msgsize;
    state_data->parameters.send.dispatch = _mcast_dispatch;
    state_data->parameters.events.cookie = state_data;
    state_data->parameters.events.local_fn = sendDone;
    state_data->parameters.events.remote_fn = NULL;

    TRACE_ERR((stderr, "<%p>:NativeInterfaceActiveMessage::<%p>send() data %zu, header %zu\n", this, _protocol, state_data->parameters.send.data.iov_len, state_data->parameters.send.header.iov_len));

    for (unsigned i = 0; i < size; ++i)
    {
      if (taskList[i] == this->myrank()) // Am I a destination?
      {
        state_data->doneCountDown--;// don't send to myself
        PAMI::PipeWorkQueue *rcvpwq = (PAMI::PipeWorkQueue *)mcast->dst;

        TRACE_ERR((stderr, "<%p>:NativeInterfaceAllsided<%d>::pwq<%p>\n", this, __LINE__,rcvpwq));
        if (rcvpwq && (rcvpwq->bytesAvailableToProduce() >= length)) // copy it if desired
        {
          memcpy(rcvpwq->bufferToProduce(), payload, length);
//          rcvpwq->produceBytes(length);
        }
        continue;
      }

      result = PAMI_Endpoint_create ((pami_client_t) mcast->client, /// \todo client is ignored on the endpoint?  client isn't a pami_client_t
                                     taskList[i],
                                     mcast->context, /// \todo what context do I target?
                                     &state_data->parameters.send.dest);

      TRACE_ERR((stderr, "<%p>:NativeInterfaceActiveMessage::<%p>send(%u(%zu,%zu))\n", this, _protocol, state_data->parameters.send.dest, (size_t) taskList[i], mcast->context));

      if (result == PAMI_SUCCESS) result = _protocol->simplePWQ(&state_data->sendpwq,_context,&state_data->parameters,pwq);

      TRACE_ERR((stderr, "<%p>:NativeInterfaceActiveMessage::send(%zu) result %u\n", this, (size_t) taskList[i], result));
    }


    TRACE_ERR((stderr, "<%p>:NativeInterfaceActiveMessage::postMulticast_impl() dispatch %zu, connection_id %#X exit\n",
               this, mcast->dispatch, mcast->connection_id));

    return PAMI_SUCCESS;

  }; // NativeInterfaceActiveMessage<T_Protocol>::postMulticast_impl

  ///
  /// \brief Received a p2p dispatch from another root (static function).
  /// Call the member function on the protocol.
  ///
  template <class T_Protocol>
  inline void NativeInterfaceActiveMessage<T_Protocol>::dispatch_p2p(pami_context_t       context_hdl,  /**< IN:  communication context handle */
                                                                     void               * cookie,       /**< IN:  dispatch cookie (pointer to protocol object)*/
                                                                     void               * header,       /**< IN:  header address  */
                                                                     size_t               header_size,  /**< IN:  header size     */
                                                                     void               * data,         /**< IN:  address of PAMI pipe  buffer, valid only if non-NULL        */
                                                                     size_t               data_size,    /**< IN:  number of byts of message data, valid regarldless of message type */
                                                                     pami_recv_t        * recv)         /**< OUT: receive message structure, only needed if addr is non-NULL */
  {
    TRACE_ERR((stderr, "<%p>NativeInterfaceActiveMessage::dispatch_p2p header size %zu, data size %zu\n", cookie, header_size, data_size));
    NativeInterfaceActiveMessage<T_Protocol> *p = (NativeInterfaceActiveMessage<T_Protocol> *)cookie;
    p->dispatch(context_hdl,
                header,
                header_size,
                data,
                data_size,
                recv);
  }
  ///
  /// \brief Received a p2p dispatch from another root (member function).
  /// Call user's dispatch
  /// \see PAMI::Protocol::Send::Send::..?
  ///
  template <class T_Protocol>
  inline void NativeInterfaceActiveMessage<T_Protocol>::dispatch(pami_context_t       context_hdl,  /**< IN:  communication context handle */
                                                                 void               * header,       /**< IN:  header address  */
                                                                 size_t               header_size,  /**< IN:  header size     */
                                                                 void               * data,         /**< IN:  address of PAMI pipe  buffer, valid only if non-NULL        */
                                                                 size_t               data_size,    /**< IN:  number of byts of message data, valid regarldless of message type */
                                                                 pami_recv_t        * recv)         /**< OUT: receive message structure, only needed if addr is non-NULL */
  {

    // Call user's dispatch to get receive pwq and cb_done.
    unsigned connection_id = ((typename NativeInterfaceBase<T_Protocol>::metadata_t*)header)->connection_id;
    size_t bytes           = ((typename NativeInterfaceBase<T_Protocol>::metadata_t*)header)->sndlen;
    size_t root            = ((typename NativeInterfaceBase<T_Protocol>::metadata_t*)header)->root;
    PAMI::PipeWorkQueue   *rcvpwq;
    pami_callback_t       cb_done = {NULL,NULL};

    TRACE_ERR((stderr, "<%p>NativeInterfaceActiveMessage::dispatch() header size %zu, data size %zu/%zu, connection_id %u, root %zu, recv %p\n", this, header_size, data_size, bytes, connection_id, root, recv));

    // tolerate a null dispatch if there's no data (barrier?)
    PAMI_assertf(((_mcast_dispatch_function == NULL) && (data_size == 0)) || (_mcast_dispatch_function != NULL),"fn %p, size %zu\n",_mcast_dispatch_function, data_size);

    if(_mcast_dispatch_function != NULL)
      _mcast_dispatch_function(((typename NativeInterfaceBase<T_Protocol>::metadata_t*)header)->msginfo, ((typename NativeInterfaceBase<T_Protocol>::metadata_t*)header)->msgcount,
                               connection_id, root, bytes, _mcast_dispatch_arg, &bytes,
                               (pami_pipeworkqueue_t**)&rcvpwq, &cb_done);

    TRACE_ERR((stderr, "<%p>NativeInterfaceActiveMessage::dispatch() requested bytes %zu\n", this, bytes));

    // I don't think send/recv lets us receive less than was sent, so assert they gave us enough buffer/pwq...
    PAMI_assert_debugf(bytes == data_size, "bytes %zu == %zu data_size\n", bytes, data_size);

    // No data or immediate data? We're done.
    if ((bytes == 0) || (recv == NULL) || (data != NULL))
    {
      TRACE_ERR((stderr, "<%p>NativeInterfaceActiveMessage::dispatch() immediate\n", this));

      if (data && bytes)
      {
        /// \todo An assertion probably isn't the best choice...
        TRACE_ERR((stderr, "<%p>:NativeInterfaceActiveMessage<%d>::pwq<%p>\n", this, __LINE__,rcvpwq));
        PAMI_assertf(rcvpwq->bytesAvailableToProduce() >= data_size, "dst %zu >= data_size %zu\n", rcvpwq->bytesAvailableToProduce(), data_size);
        memcpy(rcvpwq->bufferToProduce(), data, bytes);
//        rcvpwq->produceBytes(data_size);
      }

      // call original done
      /** \todo fix or remove this hack */
      TRACE_ERR((stderr, "<%p>:NativeInterfaceActiveMessage::done<%p>, cookie<%p>\n", this,cb_done.function,cb_done.clientdata));
      if (cb_done.function)
        (cb_done.function)(NULL,//PAMI_Client_getcontext(_client,_contextid),
                           cb_done.clientdata, PAMI_SUCCESS);

      return;
    }

    // multicast_model_available_buffers_only semantics: If you're receiving data then the pwq must be available
    TRACE_ERR((stderr, "<%p>:NativeInterfaceAllsided<%d>::pwq<%p>\n", this, __LINE__,rcvpwq));
    PAMI_assert(multicast_model_available_buffers_only && (rcvpwq->bytesAvailableToProduce() >= data_size));

    recv->addr     = rcvpwq->bufferToProduce();
    recv->type     = PAMI_BYTE;
    recv->offset   = 0;
    recv->local_fn = cb_done.function;
    recv->cookie   = cb_done.clientdata;

  }
  template <class T_Protocol>
  inline void NativeInterfaceActiveMessage<T_Protocol>::sendDone ( pami_context_t   context,
                                                                   void          *  cookie,
                                                                   pami_result_t    result )
  {

    typename NativeInterfaceBase<T_Protocol>::p2p_multicast_statedata_t *state_data = (typename NativeInterfaceBase<T_Protocol>::p2p_multicast_statedata_t*)cookie;

    TRACE_ERR((stderr, "<%p>:NativeInterfaceActiveMessage::sendDone countDown %u\n", cookie, state_data->doneCountDown));

    if (--state_data->doneCountDown == 0)
    {
      // call original done
      if (state_data->cb_done.function)
        (state_data->cb_done.function)(context,
                                       state_data->cb_done.clientdata, PAMI_SUCCESS);

      return;
    }
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
