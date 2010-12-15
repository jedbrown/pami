/*
 * \file common/bgq/NativeInterface.h
 * \brief ???
 */

#ifndef __common_bgq_NativeInterface_h__
#define __common_bgq_NativeInterface_h__

#include "Global.h"
#include "algorithms/interfaces/NativeInterface.h"
#include "util/ccmi_debug.h"
#include "util/ccmi_util.h"

#include "components/devices/MulticastModel.h"
#include "components/devices/MultisyncModel.h"
#include "components/devices/MulticombineModel.h"

#include "components/memory/MemoryAllocator.h"

#include "util/trace.h"

#ifdef CCMI_TRACE_ALL
#define DO_TRACE_ENTEREXIT 1
#define DO_TRACE_DEBUG     1
#else
#define DO_TRACE_ENTEREXIT 0
#define DO_TRACE_DEBUG     0
#endif


extern PAMI::Global __global;

namespace PAMI
{


  template <class T_Device, class T_Mcast, class T_Msync, class T_Mcomb>
  class BGQNativeInterfaceAS : public CCMI::Interfaces::NativeInterface
  {
    public:
      /// \brief ctor that takes pointers to multi* objects
      inline BGQNativeInterfaceAS(T_Mcast *mcast, T_Msync *msync, T_Mcomb *mcomb, pami_client_t client, pami_context_t context, size_t context_id, size_t client_id, int *dispatch_id);

      /// \brief ctor that constructs multi* objects internally
      inline BGQNativeInterfaceAS(T_Device &device, pami_client_t client, pami_context_t context, size_t context_id, size_t client_id, int *dispatch_id);

      /// Virtual interfaces (from base \see CCMI::Interfaces::NativeInterface)
      virtual inline pami_result_t multicast    (pami_multicast_t    *, void *devinfo = NULL);
      virtual inline pami_result_t multisync    (pami_multisync_t    *, void *devinfo = NULL);
      virtual inline pami_result_t multicombine (pami_multicombine_t *, void *devinfo = NULL);
      virtual inline pami_result_t manytomany (pami_manytomany_t *, void *devinfo = NULL)
      {
        PAMI_abort();
        return PAMI_ERROR;
      }

      // Model-specific interfaces
      inline pami_result_t multicast    (uint8_t (&)[T_Mcast::sizeof_msg], pami_multicast_t    *, void *devinfo = NULL);
      inline pami_result_t multisync    (uint8_t (&)[T_Msync::sizeof_msg], pami_multisync_t    *, void *devinfo = NULL);
      inline pami_result_t multicombine (uint8_t (&)[T_Mcomb::sizeof_msg], pami_multicombine_t *, void *devinfo = NULL);

      static const size_t multicast_sizeof_msg     = T_Mcast::sizeof_msg;
      static const size_t multisync_sizeof_msg     = T_Msync::sizeof_msg;
      static const size_t multicombine_sizeof_msg  = T_Mcomb::sizeof_msg;

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
            uint8_t             _mcast[T_Mcast::sizeof_msg];
            uint8_t             _msync[T_Msync::sizeof_msg];
            uint8_t             _mcomb[T_Mcomb::sizeof_msg];
          } _state;
          BGQNativeInterfaceAS *_ni;
          pami_callback_t       _user_callback;
      };
      PAMI::MemoryAllocator < sizeof(allocObj), 16 > _allocator;  // Allocator

      pami_result_t              _mcast_status;
      pami_result_t              _msync_status;
      pami_result_t              _mcomb_status;

      T_Mcast                 _mcast;
      T_Msync                 _msync;
      T_Mcomb                 _mcomb;

      unsigned                _dispatch;
      pami_client_t           _client;
      pami_context_t          _context;
      size_t                  _contextid;
      size_t                  _clientid;
      int                    *_dispatch_id;
  }; // class BGQNativeInterfaceAS

  template <class T_Device, class T_Mcast, class T_Msync, class T_Mcomb>
  class BGQNativeInterface : public BGQNativeInterfaceAS<T_Device, T_Mcast, T_Msync, T_Mcomb>
  {
    public:
      /// \brief ctor that constructs multi* objects internally
      inline BGQNativeInterface(T_Device &device, pami_client_t client, pami_context_t context, size_t context_id, size_t client_id, int *dispatch_id);

      /// \brief this call is called when the native interface is
      /// initialized to set the mcast dispatch
      virtual inline pami_result_t setMulticastDispatch (pami_dispatch_multicast_function fn, void *cookie);
      virtual inline pami_result_t setManytomanyDispatch(pami_dispatch_manytomany_function fn, void *cookie)
      {
        PAMI_abort();
        return PAMI_ERROR;
      }

  }; // class BGQNativeInterface

  ///////////////////////////////////////////////////////////////////////////////
  // Inline implementations
  ///////////////////////////////////////////////////////////////////////////////
  template <class T_Device, class T_Mcast, class T_Msync, class T_Mcomb>
  inline BGQNativeInterfaceAS<T_Device, T_Mcast, T_Msync, T_Mcomb>::BGQNativeInterfaceAS(T_Mcast *mcast, T_Msync *msync, T_Mcomb *mcomb, pami_client_t client, pami_context_t context, size_t context_id, size_t client_id, int *dispatch_id):
      CCMI::Interfaces::NativeInterface(__global.mapping.task(),
                                        __global.mapping.size()),
      _allocator(),
      _mcast(*mcast),
      _msync(*msync),
      _mcomb(*mcomb),
      _dispatch(-1U),
      _client(client),
      _context(context),
      _contextid(context_id),
      _clientid(client_id),
      _dispatch_id(dispatch_id)
  {
    TRACE_FN_ENTER();
    TRACE_FN_EXIT();
  };

  template <class T_Device, class T_Mcast, class T_Msync, class T_Mcomb>
  BGQNativeInterfaceAS<T_Device, T_Mcast, T_Msync, T_Mcomb>::BGQNativeInterfaceAS(T_Device      &device,
      pami_client_t  client,
      pami_context_t context,
      size_t         context_id,
      size_t         client_id,
      int           *dispatch_id):
      CCMI::Interfaces::NativeInterface(__global.mapping.task(),
                                        __global.mapping.size()),
      _allocator(),

      _mcast_status(PAMI_SUCCESS),
      _msync_status(PAMI_SUCCESS),
      _mcomb_status(PAMI_SUCCESS),

      _mcast(device, _mcast_status),
      _msync(device, _msync_status),
      _mcomb(device, _mcomb_status),

      _dispatch(-1U),
      _client(client),
      _context(context),
      _contextid(context_id),
      _clientid(client_id),
      _dispatch_id(dispatch_id)
  {
    TRACE_FN_ENTER();
    TRACE_FORMAT( "<%p>%s %d %d %d", this, __PRETTY_FUNCTION__,
                  _mcast_status, _msync_status, _mcomb_status);

    CCMI::Interfaces::NativeInterface::_status = (pami_result_t) (_mcast_status | _msync_status | _mcomb_status);

    TRACE_FN_EXIT();
  };

  template <class T_Device, class T_Mcast, class T_Msync, class T_Mcomb>
  BGQNativeInterface<T_Device, T_Mcast, T_Msync, T_Mcomb>::BGQNativeInterface(T_Device      &device,
                                                                              pami_client_t  client,
                                                                              pami_context_t context,
                                                                              size_t         context_id,
                                                                              size_t         client_id,
                                                                              int           *dispatch_id) :
      BGQNativeInterfaceAS<T_Device, T_Mcast, T_Msync, T_Mcomb>(device, client, context, context_id, client_id, dispatch_id)
  {
    TRACE_FN_ENTER();
    DO_DEBUG((templateName<T_Device>()));
    TRACE_FN_EXIT();
  }


  template <class T_Device, class T_Mcast, class T_Msync, class T_Mcomb>
  inline void BGQNativeInterfaceAS<T_Device, T_Mcast, T_Msync, T_Mcomb>::ni_client_done(pami_context_t  context,
      void          *rdata,
      pami_result_t   res)
  {
    TRACE_FN_ENTER();
    allocObj             *obj = (allocObj*)rdata;
    BGQNativeInterfaceAS *ni   = obj->_ni;

    TRACE_FORMAT( "<%p> %p, %p, %d calling %p(%p)",
                  ni, context, rdata, res,
                  obj->_user_callback.function, obj->_user_callback.clientdata);

    if (obj->_user_callback.function)
      obj->_user_callback.function(context,
                                   obj->_user_callback.clientdata,
                                   res);

    ni->_allocator.returnObject(obj);
    TRACE_FN_EXIT();
  }

  /// \brief this call is called when the native interface is initialized
  template <class T_Device, class T_Mcast, class T_Msync, class T_Mcomb>
  inline pami_result_t BGQNativeInterface<T_Device, T_Mcast, T_Msync, T_Mcomb>::setMulticastDispatch (pami_dispatch_multicast_function fn, void *cookie)
  {
    TRACE_FN_ENTER();
    this->_dispatch = (*this->_dispatch_id)--;

    TRACE_FORMAT( "<%p> %p, %p id=%u",
                  this, fn,  cookie,  this->_dispatch);
    DO_DEBUG((templateName<T_Mcast>()));

    pami_result_t result = this->_mcast.registerMcastRecvFunction(this->_dispatch, fn, cookie);

    PAMI_assert(result == PAMI_SUCCESS);

    TRACE_FN_EXIT();
    return result;
  }

  template <class T_Device, class T_Mcast, class T_Msync, class T_Mcomb>
  inline pami_result_t BGQNativeInterfaceAS<T_Device, T_Mcast, T_Msync, T_Mcomb>::multicast (pami_multicast_t *mcast, void *devinfo)
  {
    TRACE_FN_ENTER();
    allocObj *req          = (allocObj *)_allocator.allocateObject();
    req->_ni               = this;
    req->_user_callback    = mcast->cb_done;
    TRACE_FORMAT( "<%p> %p/%p connection id %u, msgcount %u, bytes %zu, devinfo %p", this, mcast, req, mcast->connection_id, mcast->msgcount, mcast->bytes, devinfo);
    DO_DEBUG((templateName<T_Mcast>()));

    //  \todo:  this copy will cause a latency hit, maybe we need to change postMultisync
    //          interface so we don't need to copy
    pami_multicast_t  m     = *mcast;

    m.dispatch =  _dispatch; // \todo ? Not really used in C++ objects?
    m.client   =  _clientid;   // \todo ? Why doesn't caller set this?
    m.context  =  _contextid;// \todo ? Why doesn't caller set this?

    m.cb_done.function     =  ni_client_done;
    m.cb_done.clientdata   =  req;

    TRACE_FN_EXIT();
    return _mcast.postMulticast_impl(req->_state._mcast, &m, devinfo);
  }


  template <class T_Device, class T_Mcast, class T_Msync, class T_Mcomb>
  inline pami_result_t BGQNativeInterfaceAS<T_Device, T_Mcast, T_Msync, T_Mcomb>::multisync(pami_multisync_t *msync, void *devinfo)
  {
    TRACE_FN_ENTER();
    allocObj *req          = (allocObj *)_allocator.allocateObject();
    req->_ni               = this;
    req->_user_callback    = msync->cb_done;
    TRACE_FORMAT( "<%p> %p/%p connection id %u, devinfo %p", this, msync, req, msync->connection_id, devinfo);
    DO_DEBUG((templateName<T_Msync>()));

    pami_multisync_t  m     = *msync;

    m.client   =  _clientid;
    m.context  =  _contextid;

    m.cb_done.function     =  ni_client_done;
    m.cb_done.clientdata   =  req;
    _msync.postMultisync(req->_state._msync, &m, devinfo);
    TRACE_FN_EXIT();
    return PAMI_SUCCESS;
  }


  template <class T_Device, class T_Mcast, class T_Msync, class T_Mcomb>
  inline pami_result_t BGQNativeInterfaceAS<T_Device, T_Mcast, T_Msync, T_Mcomb>::multicombine (pami_multicombine_t *mcomb, void *devinfo)
  {
    TRACE_FN_ENTER();
    allocObj *req          = (allocObj *)_allocator.allocateObject();
    req->_ni               = this;
    req->_user_callback    = mcomb->cb_done;
    TRACE_FORMAT( "<%p> %p/%p connection id %u, count %zu, dt %#X, op %#X, devinfo %p", this, mcomb, req, mcomb->connection_id, mcomb->count, mcomb->dtype, mcomb->optor, devinfo);
    DO_DEBUG((templateName<T_Mcomb>()));

    pami_multicombine_t  m     = *mcomb;

    m.client   =  _clientid;
    m.context  =  _contextid;

    m.cb_done.function     =  ni_client_done;
    m.cb_done.clientdata   =  req;

    TRACE_FN_EXIT();
    return _mcomb.postMulticombine(req->_state._mcomb, &m, devinfo);
  }

  template <class T_Device, class T_Mcast, class T_Msync, class T_Mcomb>
  inline pami_result_t BGQNativeInterfaceAS<T_Device, T_Mcast, T_Msync, T_Mcomb>::multicast (uint8_t (&state)[T_Mcast::sizeof_msg],
      pami_multicast_t *mcast, void *devinfo)
  {
    TRACE_FN_ENTER();
    TRACE_FORMAT( "<%p> %p,%p connection id %u, msgcount %u, bytes %zu, devinfo %p", this, &state, mcast, mcast->connection_id, mcast->msgcount, mcast->bytes, devinfo);
    DO_DEBUG((templateName<T_Mcast>()));

    mcast->dispatch =  _dispatch;

    TRACE_FN_EXIT();
    return _mcast.postMulticast_impl(state, mcast, devinfo);
  }

  template <class T_Device, class T_Mcast, class T_Msync, class T_Mcomb>
  inline pami_result_t BGQNativeInterfaceAS<T_Device, T_Mcast, T_Msync, T_Mcomb>::multisync (uint8_t (&state)[T_Msync::sizeof_msg],
      pami_multisync_t *msync, void *devinfo)
  {
    TRACE_FN_ENTER();
    TRACE_FORMAT( "<%p> %p,%p connection id %u, devinfo %p", this, &state, msync, msync->connection_id, devinfo);
    DO_DEBUG((templateName<T_Msync>()));

    TRACE_FN_EXIT();
    return _msync.postMultisync_impl(state, msync, devinfo);
  }

  template <class T_Device, class T_Mcast, class T_Msync, class T_Mcomb>
  inline pami_result_t BGQNativeInterfaceAS<T_Device, T_Mcast, T_Msync, T_Mcomb>::multicombine (uint8_t (&state)[T_Mcomb::sizeof_msg],
      pami_multicombine_t *mcomb, void *devinfo)
  {
    TRACE_FN_ENTER();
    TRACE_FORMAT( "<%p> %p,%p connection id %u, count %zu, dt %#X, op %#X, devinfo %p", this, &state, mcomb, mcomb->connection_id, mcomb->count, mcomb->dtype, mcomb->optor, devinfo);
    DO_DEBUG((templateName<T_Mcomb>()));

    TRACE_FN_EXIT();
    return _mcomb.postMulticombine_impl(state, mcomb, devinfo);
  }


  template <class T_Device1, class T_Device2, class T_Mcast1, class T_Mcast2, class T_Msync, class T_Mcomb>
  class BGQNativeInterfaceASMultiDevice : public BGQNativeInterfaceAS <T_Device1, T_Mcast1, T_Msync, T_Mcomb>
  {

      /// Allocation object to store state and user's callback
      class allocObjMD
      {
        public:
          struct _state
          {
            uint8_t             _mcast1[T_Mcast1::sizeof_msg];
            uint8_t             _mcast2[T_Mcast2::sizeof_msg];
          } _state;
          BGQNativeInterfaceASMultiDevice *_ni;
          pami_callback_t       _user_callback;

          int done_count;
          int completion_count;
          unsigned total_bytes;
          unsigned  is_master;
          PAMI::PipeWorkQueue *src;
          PAMI::Topology       _src_topo_local;
          PAMI::Topology       _dst_topo_local;
      };

      /// \brief NativeInterface done function - free allocation and call client's done
      static void ni_multi_client_done(pami_context_t  context,
                                       void          *rdata,
                                       pami_result_t   res);

      pami_result_t              _mcast2_status;
      T_Mcast2                 _mcast2;

    public:

      BGQNativeInterfaceASMultiDevice(T_Device1      &device1,
                                      T_Device2    &device2,                                                   pami_client_t  client,
                                      pami_context_t context,
                                      size_t         context_id,
                                      size_t         client_id,
                                      int           *dispatch_id);

      BGQNativeInterfaceASMultiDevice(T_Device1      &device1,
                                      pami_client_t  client,
                                      pami_context_t context,
                                      size_t         context_id,
                                      size_t         client_id,
                                      int           *dispatch_id);

      virtual pami_result_t multicast (pami_multicast_t *mcast, void *devinfo);
  };

  template <class T_Device1, class T_Device2, class T_Mcast1, class T_Mcast2, class T_Msync, class T_Mcomb>
  BGQNativeInterfaceASMultiDevice<T_Device1, T_Device2, T_Mcast1, T_Mcast2, T_Msync, T_Mcomb>::
  BGQNativeInterfaceASMultiDevice(T_Device1      &device1,
                                  T_Device2    &device2,                                                 pami_client_t  client,
                                  pami_context_t context,
                                  size_t         context_id,
                                  size_t         client_id,
                                  int           *dispatch_id)
      : BGQNativeInterfaceAS<T_Device1, T_Mcast1, T_Msync, T_Mcomb>(device1, client, context, context_id, client_id, dispatch_id),
      _mcast2_status(PAMI_SUCCESS),
      _mcast2(device2, _mcast2_status)
  {
    TRACE_FN_ENTER();
    TRACE_FORMAT( "<%p> status %u", this, _mcast2_status);
    this->CCMI::Interfaces::NativeInterface::_status = (pami_result_t) (this->CCMI::Interfaces::NativeInterface::_status | _mcast2_status);
    TRACE_FN_EXIT();
  }

  template <class T_Device1, class T_Device2, class T_Mcast1, class T_Mcast2, class T_Msync, class T_Mcomb>
  BGQNativeInterfaceASMultiDevice<T_Device1, T_Device2, T_Mcast1, T_Mcast2, T_Msync, T_Mcomb>::
  BGQNativeInterfaceASMultiDevice(T_Device1      &device1,
                                  pami_client_t  client,
                                  pami_context_t context,
                                  size_t         context_id,
                                  size_t         client_id,
                                  int           *dispatch_id)
      : BGQNativeInterfaceAS<T_Device1, T_Mcast1, T_Msync, T_Mcomb>(device1, client, context, context_id, client_id, dispatch_id),
      _mcast2_status(PAMI_SUCCESS),
      _mcast2(*(device1.getProgressDevice()), _mcast2_status)
  {
    TRACE_FN_ENTER();
    TRACE_FORMAT( "<%p> status %u", this, _mcast2_status);
    this->CCMI::Interfaces::NativeInterface::_status = (pami_result_t) (this->CCMI::Interfaces::NativeInterface::_status | _mcast2_status);
    TRACE_FN_EXIT();
  }

  template <class T_Device1, class T_Device2, class T_Mcast1, class T_Mcast2, class T_Msync, class T_Mcomb>
  inline pami_result_t BGQNativeInterfaceASMultiDevice<T_Device1, T_Device2, T_Mcast1, T_Mcast2, T_Msync, T_Mcomb>::multicast (pami_multicast_t *mcast, void *devinfo)
  {
    TRACE_FN_ENTER();
    allocObjMD *req          = (allocObjMD *)(this->_allocator).allocateObject();

    req->_ni               = this;
    req->_user_callback    = mcast->cb_done;
    req->done_count = 0;
    TRACE_FORMAT( "<%p> %p/%p connection id %u, msgcount %u, bytes %zu, devinfo %p", this, mcast, req, mcast->connection_id, mcast->msgcount, mcast->bytes, devinfo);
    DO_DEBUG((templateName<T_Mcast1>()));
    DO_DEBUG((templateName<T_Mcast2>()));


    pami_multicast_t  m_local     = *mcast;
    pami_multicast_t  m_global    = *mcast;
    m_local.dispatch = m_global.dispatch =  this->_dispatch; // \todo ? Not really used in C++ objects?
    m_local.client   = m_global.dispatch =  this->_clientid;   // \todo ? Why doesn't caller set this?
    m_local.context  = m_global.context =   this->_contextid;// \todo ? Why doesn't caller set this?

    m_local.cb_done.function     = m_global.cb_done.function =  ni_multi_client_done;
    m_local.cb_done.clientdata   = m_global.cb_done.clientdata =  req;

    //fprintf(stderr, "src pwq:%p recv pwq:%p passed in native interface\n", mcast->src, mcast->dst);


    //fprintf(stderr,"posting multicast from the native interface\n");
    //if (local_root == __global.mapping.task()) /* master */

    /* Check for the master/root */

    PAMI::Topology *root_topo = (PAMI::Topology*)mcast->src_participants;
    PAMI_assert(root_topo);
    TRACE_FORMAT( "<%p> root %p/%u myrank %u", this, root_topo, root_topo ? root_topo->index2Rank(0) : -1, this->myrank());

    if (root_topo->index2Rank(0) == this->myrank())
      {
        req->is_master = 1;
        req->total_bytes = mcast->bytes;
        req->src = (PAMI::PipeWorkQueue*) mcast->src;

        unsigned any_local_dst_procs = 0, any_nw_dst_procs = 0;

        PAMI::Topology *dst_topology = (PAMI::Topology *)mcast->dst_participants;

        if (dst_topology == NULL)
          {
            any_local_dst_procs = any_nw_dst_procs = 0;
          }
        else
          {
            PAMI_assert (dst_topology->type() == PAMI_AXIAL_TOPOLOGY);

            pami_coord_t *ll = NULL;
            pami_coord_t *ur = NULL;
            pami_coord_t *ref = NULL;
            unsigned char *isTorus = NULL;

            pami_result_t result = PAMI_SUCCESS;
            result = dst_topology->axial(&ll, &ur, &ref, &isTorus);
            PAMI_assert(result == PAMI_SUCCESS);


            if ((ur->u.n_torus.coords[LOCAL_DIM] - ll->u.n_torus.coords[LOCAL_DIM]) > 0)
              any_local_dst_procs  = 1;

            if ((ll->u.n_torus.coords[0] == ur->u.n_torus.coords[0]) &&
                (ll->u.n_torus.coords[1] == ur->u.n_torus.coords[1]) &&
                (ll->u.n_torus.coords[2] == ur->u.n_torus.coords[2]) &&
                (ll->u.n_torus.coords[3] == ur->u.n_torus.coords[3]) &&
                (ll->u.n_torus.coords[4] == ur->u.n_torus.coords[4]))
              any_nw_dst_procs = 0;
            else
              any_nw_dst_procs = 1;

          }

        //fprintf(stderr,"any_nw_dst_procs:%d any_local_dst_pocs:%d\n", any_local_dst_procs, any_nw_dst_procs);

        this->_mcast.setLocalMulticast(false);
        this->_mcast.callConsumeBytesOnMaster(false);


        req->completion_count = 0;

        if (any_local_dst_procs)
          {
            //fprintf(stderr,"[%zu]master posting local multicast\n", __global.mapping.task());
            req->completion_count++;
            _mcast2.postMulticast_impl(req->_state._mcast2, &m_local, devinfo);
          }

        if (any_nw_dst_procs)
          {
            //fprintf(stderr,"[%zu] master posting nw multicast\n", __global.mapping.task());
            req->completion_count++;
            //this->_mcast.callConsumeBytesOnMaster(false);
            this->_mcast.postMulticast_impl(req->_state._mcast1, &m_global, devinfo);
          }

        //this->_mcast.setLocalMulticast(false);
      }
    else /* is not the master/root */
      {
        req->completion_count = 1;
        req->is_master = 0;
        req->src = NULL;

        //pami_task_t root[1];
        pami_task_t root;
        pami_coord_t root_coord;
        root = root_topo->index2Rank(0);
        __global.mapping.task2network(root, &root_coord, PAMI_N_TORUS_NETWORK);

        if (root_coord.u.n_torus.coords[5] == __global.mapping.t()) //network receiver is the same peer as root
          {
            //fprintf (stderr,"[%zu] non master posting MU Multicast \n", __global.mapping.task());
            this->_mcast.postMulticast_impl(req->_state._mcast1, &m_global, devinfo);

          }
        else //shmem
          {
            //fprintf (stderr,"[%zu] non master posting Local Multicast \n", __global.mapping.task());
            _mcast2.postMulticast_impl(req->_state._mcast2, &m_local, devinfo);
          }
      }

    TRACE_FN_EXIT();
    return PAMI_SUCCESS;
  }


  template <class T_Device1, class T_Device2, class T_Mcast1, class T_Mcast2, class T_Msync, class T_Mcomb>
  inline void BGQNativeInterfaceASMultiDevice<T_Device1, T_Device2, T_Mcast1, T_Mcast2, T_Msync, T_Mcomb>::ni_multi_client_done(pami_context_t  context,
      void          *rdata,
      pami_result_t   res)
  {
    TRACE_FN_ENTER();
    allocObjMD             *obj = (allocObjMD*)rdata;
    BGQNativeInterfaceASMultiDevice *ni   = obj->_ni;

    TRACE_FORMAT( "<%p> %p, %p, %d calling %p(%p)",
                  ni, context, rdata, res,
                  obj->_user_callback.function, obj->_user_callback.clientdata);

    ++obj->done_count;
    //printf("calling ni_multi_client_done:%d\n", obj->done_count);

    if (obj->done_count == obj->completion_count)
      {
        //Call pipework queue consume bytes
        if (obj->src) obj->src->consumeBytes(obj->total_bytes);

        if (obj->_user_callback.function)
          obj->_user_callback.function(context,
                                       obj->_user_callback.clientdata,
                                       res);

        ni->_allocator.returnObject(obj);
      }

    TRACE_FN_EXIT();
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
