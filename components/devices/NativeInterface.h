/* * \file components/devices/NativeInterface.h
 * \brief ???
 */

#ifndef __components_devices_NativeInterface_h__
#define __components_devices_NativeInterface_h__

#include "algorithms/interfaces/NativeInterface.h"
#include "util/ccmi_util.h"

#include "components/devices/MulticastModel.h"
#include "components/devices/MultisyncModel.h"
#include "components/devices/MulticombineModel.h"
#include "components/devices/ManytomanyModel.h"
#include "components/memory/MemoryAllocator.h"

#undef TRACE_ERR
#define TRACE_ERR(x) //fprintf x

#define DISPATCH_START_DEVICE  16
#define DISPATCH_START_COLLSHM 32

namespace PAMI
{
  namespace Device
  {
    typedef enum DeviceNativeInterfaceSemantics
    {
      ActiveMessage=0,
      AllSided,
    }DeviceNativeInterfaceSemantics;

    template <class T_Device, class T_Mcast, class T_Msync, class T_Mcomb,  int T_Semantics=ActiveMessage>
    class DeviceNativeInterface : public CCMI::Interfaces::NativeInterface
    {
    public:
      inline DeviceNativeInterface(T_Device       &device,
                                   pami_client_t   client,
                                   pami_context_t  context,
                                   size_t          context_id,
                                   size_t          client_id,
                                   pami_task_t     taskid,
                                   size_t          num_tasks);

      virtual inline pami_result_t setMulticastDispatch (pami_dispatch_multicast_fn  fn,
                                                         void                       *cookie)
        {
          return _mcast.registerMcastRecvFunction(_dispatch,fn,cookie);
        }
      virtual inline pami_result_t setManytomanyDispatch(pami_dispatch_manytomany_fn fn, void *cookie)
      {
        PAMI_abort();
        return PAMI_ERROR;
      }
      virtual inline pami_result_t setSendDispatch(pami_dispatch_p2p_fn  fn,
                                                   void                      *cookie)
        {
          PAMI_abort();
          return PAMI_ERROR;
        }
      virtual inline pami_result_t setSendPWQDispatch(pami_dispatch_p2p_fn fn,
                                                      void *cookie)
        {
          PAMI_abort();
          return PAMI_ERROR;
        }
      virtual inline pami_result_t multicast    (pami_multicast_t    *, void *devinfo=NULL);
      virtual inline pami_result_t multisync    (pami_multisync_t    *, void *devinfo=NULL);
      virtual inline pami_result_t multicombine (pami_multicombine_t *, void *devinfo=NULL);
      virtual inline pami_result_t manytomany   (pami_manytomany_t   *, void *devinfo=NULL)
        {
          PAMI_abort();
          return PAMI_ERROR;
        }
      virtual inline pami_result_t send (pami_send_t * parameters)
        {
          PAMI_abort();
          return PAMI_ERROR;
        }
      virtual inline pami_result_t sendPWQ(pami_context_t       context,
                                           pami_send_t         *parameters,
                                           PAMI::PipeWorkQueue *pwq)
        {
          PAMI_abort();
          return PAMI_ERROR;
        }

      // Model-specific interfaces
      inline pami_result_t multicast    (uint8_t (&)[T_Mcast::sizeof_msg], pami_multicast_t    *, void *devinfo=NULL);
      inline pami_result_t multisync    (uint8_t (&)[T_Msync::sizeof_msg], pami_multisync_t    *, void *devinfo=NULL);
      inline pami_result_t multicombine (uint8_t (&)[T_Mcomb::sizeof_msg], pami_multicombine_t *, void *devinfo=NULL);
      static const size_t  multicast_sizeof_msg     = T_Mcast::sizeof_msg;
      static const size_t  multisync_sizeof_msg     = T_Msync::sizeof_msg;
      static const size_t  multicombine_sizeof_msg  = T_Mcomb::sizeof_msg;

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
          uint8_t              _mcast[T_Mcast::sizeof_msg];
          uint8_t              _msync[T_Msync::sizeof_msg];
          uint8_t              _mcomb[T_Mcomb::sizeof_msg];
        } _state;
        DeviceNativeInterface *_ni;
        pami_callback_t        _user_callback;
      };

      PAMI::MemoryAllocator<sizeof(allocObj),16> _allocator;  // Allocator

      pami_result_t              _mcast_status;
      pami_result_t              _msync_status;
      pami_result_t              _mcomb_status;

      T_Mcast                    _mcast;
      T_Msync                    _msync;
      T_Mcomb                    _mcomb;

      unsigned                   _dispatch;
      pami_client_t              _client;
      pami_context_t             _context;
      size_t                     _contextid;
      size_t                     _clientid;
    }; // class DeviceNativeInterface

    ///////////////////////////////////////////////////////////////////////////////
    // Inline implementations
    ///////////////////////////////////////////////////////////////////////////////
    template <class T_Device, class T_Mcast, class T_Msync, class T_Mcomb, int T_Semantics>
    DeviceNativeInterface<T_Device,T_Mcast,T_Msync,T_Mcomb, T_Semantics>::DeviceNativeInterface(T_Device       &device,
                                                                                                pami_client_t   client,
                                                                                                pami_context_t  context,
                                                                                                size_t          context_id,
                                                                                                size_t          client_id,
                                                                                                pami_task_t     task,
                                                                                                size_t          size):
      CCMI::Interfaces::NativeInterface(task, size),
      _allocator(),
      _mcast_status(PAMI_SUCCESS),
      _msync_status(PAMI_SUCCESS),
      _mcomb_status(PAMI_SUCCESS),
      _mcast(device,_mcast_status),
      _msync(device,_msync_status),
      _mcomb(device,_mcomb_status),
      _dispatch(DISPATCH_START_DEVICE),
      _client(client),
      _context(context),
      _contextid(context_id),
      _clientid(client_id)
    {
      PAMI_assert(_mcast_status == PAMI_SUCCESS);
      PAMI_assert(_msync_status == PAMI_SUCCESS);
      PAMI_assert(_mcomb_status == PAMI_SUCCESS);
    }

    template <class T_Device, class T_Mcast, class T_Msync, class T_Mcomb, int T_Semantics>
    inline void DeviceNativeInterface<T_Device,
                                      T_Mcast,
                                      T_Msync,
                                      T_Mcomb,
                                      T_Semantics>::ni_client_done(pami_context_t  context,
                                                                   void          *rdata,
                                                                   pami_result_t   res)
    {
      allocObj              *obj  = (allocObj*)rdata;
      DeviceNativeInterface *ni   = obj->_ni;
      if (obj->_user_callback.function)
        obj->_user_callback.function(context,
                                     obj->_user_callback.clientdata,
                                     res);
      ni->_allocator.returnObject(obj);
    }

    template <class T_Device, class T_Mcast, class T_Msync, class T_Mcomb, int T_Semantics>
    inline pami_result_t DeviceNativeInterface<T_Device,
                                               T_Mcast,
                                               T_Msync,
                                               T_Mcomb,
                                               T_Semantics>::multicast (pami_multicast_t *mcast,
                                                                        void             *devinfo)
    {
      allocObj *req          = (allocObj *)_allocator.allocateObject();
      req->_ni               = this;
      req->_user_callback    = mcast->cb_done;

      pami_multicast_t  m    = *mcast;
      m.dispatch             =  _dispatch;
      m.client               =  _clientid;
      m.context              =  _contextid;

      m.cb_done.function     =  ni_client_done;
      m.cb_done.clientdata   =  req;
      return _mcast.postMulticast(req->_state._mcast, &m, devinfo);
    }

    template <class T_Device, class T_Mcast, class T_Msync, class T_Mcomb, int T_Semantics>
    inline pami_result_t DeviceNativeInterface<T_Device,
                                               T_Mcast,
                                               T_Msync,
                                               T_Mcomb,
                                               T_Semantics>::multisync(pami_multisync_t *msync,
                                                                       void             *devinfo)
    {
      allocObj         *req   = (allocObj *)_allocator.allocateObject();
      req->_ni                = this;
      req->_user_callback     = msync->cb_done;

      pami_multisync_t  m     = *msync;
      m.client                =  _clientid;
      m.context               =  _contextid;
      m.cb_done.function      =  ni_client_done;
      m.cb_done.clientdata    =  req;
      _msync.postMultisync(req->_state._msync, &m, devinfo);
      return PAMI_SUCCESS;
    }


    template <class T_Device, class T_Mcast, class T_Msync, class T_Mcomb, int T_Semantics>
    inline pami_result_t DeviceNativeInterface<T_Device,
                                               T_Mcast,
                                               T_Msync,
                                               T_Mcomb,
                                               T_Semantics>::multicombine (pami_multicombine_t *mcomb,
                                                                           void                *devinfo)
    {
      allocObj            *req = (allocObj *)_allocator.allocateObject();
      req->_ni                 = this;
      req->_user_callback      = mcomb->cb_done;

      pami_multicombine_t  m   = *mcomb;
      m.client                 =  _clientid;
      m.context                =  _contextid;
      m.cb_done.function       =  ni_client_done;
      m.cb_done.clientdata     =  req;
      return _mcomb.postMulticombine(req->_state._mcomb, &m, devinfo);
    }

    template <class T_Device, class T_Mcast, class T_Msync, class T_Mcomb, int T_Semantics>
    inline pami_result_t  DeviceNativeInterface<T_Device,T_Mcast,T_Msync,T_Mcomb, T_Semantics>::multicast (uint8_t (&state)[T_Mcast::sizeof_msg],
                                                                                                           pami_multicast_t *mcast,
                                                                                                           void             *devinfo)
    {
      mcast->dispatch =  _dispatch;
      return _mcast.postMulticast_impl(state, mcast, devinfo);
    }

    template <class T_Device, class T_Mcast, class T_Msync, class T_Mcomb, int T_Semantics>
    inline pami_result_t  DeviceNativeInterface<T_Device,T_Mcast,T_Msync,T_Mcomb, T_Semantics>::multisync (uint8_t (&state)[T_Msync::sizeof_msg],
                                                                                                           pami_multisync_t *msync,
                                                                                                           void             *devinfo)
    {
      return _msync.postMultisync_impl(state, msync, devinfo);
    }

    template <class T_Device, class T_Mcast, class T_Msync, class T_Mcomb, int T_Semantics>
    inline pami_result_t  DeviceNativeInterface<T_Device,T_Mcast,T_Msync,T_Mcomb, T_Semantics>::multicombine (uint8_t (&state)[T_Mcomb::sizeof_msg],
                                                                                                              pami_multicombine_t *mcomb,
                                                                                                              void                *devinfo)
    {
      return _mcomb.postMulticombine_impl(state, mcomb, devinfo);
    }





    template <class T_Model>
    class CSNativeInterface : public CCMI::Interfaces::NativeInterface
    {
    public:
      inline CSNativeInterface(T_Model       &model,
                               pami_client_t  client,
                               size_t         client_id,
                               pami_context_t context,
                               size_t         context_id,
                               pami_task_t    tasks,
                               size_t         num_tasks);
      virtual inline pami_result_t multicast    (pami_multicast_t    *, void *devinfo=NULL);
      virtual inline pami_result_t multisync    (pami_multisync_t    *, void *devinfo=NULL);
      virtual inline pami_result_t multicombine (pami_multicombine_t *, void *devinfo=NULL);
      virtual inline pami_result_t manytomany (pami_manytomany_t *, void *devinfo=NULL)
        {
          PAMI_abort();
          return PAMI_ERROR;
        }
      inline pami_result_t multicast    (uint8_t (&)[T_Model::sizeof_multicast_msg], pami_multicast_t    *, void *devinfo=NULL);
      inline pami_result_t multisync    (uint8_t (&)[T_Model::sizeof_multisync_msg], pami_multisync_t    *, void *devinfo=NULL);
      inline pami_result_t multicombine (uint8_t (&)[T_Model::sizeof_multicombine_msg], pami_multicombine_t *, void *devinfo=NULL);

      static const size_t sizeof_multicast_msg     = T_Model::sizeof_multicast_msg;
      static const size_t sizeof_multisync_msg     = T_Model::sizeof_multisync_msg;
      static const size_t sizeof_multicombine_msg  = T_Model::sizeof_multicombine_msg;

    protected:
      static void ni_client_done(pami_context_t  context,
                                 void          *rdata,
                                 pami_result_t   res);
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

      PAMI::MemoryAllocator < sizeof(allocObj), 16 > _allocator;
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
    inline CSNativeInterface<T_Model>::CSNativeInterface(T_Model         &model,
                                                         pami_client_t     client,
                                                         size_t            client_id,
                                                         pami_context_t    context,
                                                         size_t            context_id,
                                                         pami_task_t       task_id,
                                                         size_t            num_tasks):
      CCMI::Interfaces::NativeInterface(task_id, num_tasks),
      _allocator(),
      _model(model),
      _dispatch(DISPATCH_START_COLLSHM),
      _client(client),
      _context(context),
      _contextid(context_id),
      _clientid(client_id)
    {
    };

    template <class T_Model>
    inline void CSNativeInterface<T_Model>::ni_client_done(pami_context_t  context,
                                                           void          *rdata,
                                                           pami_result_t   res)
    {
      allocObj             *obj = (allocObj*)rdata;
      CSNativeInterface    *ni   = obj->_ni;

      if (obj->_user_callback.function)
        obj->_user_callback.function(context,
                                     obj->_user_callback.clientdata,
                                     res);

      ni->_allocator.returnObject(obj);
    }

    template <class T_Model>
    inline pami_result_t CSNativeInterface<T_Model>::multicast (pami_multicast_t *mcast,
                                                                void             *devinfo)
    {
      allocObj *req          = (allocObj *)_allocator.allocateObject();
      req->_ni               = this;
      req->_user_callback    = mcast->cb_done;
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

      return _model.postMulticast(req->_state._mcast, &m, devinfo);
    }


    template <class T_Model>
    inline pami_result_t CSNativeInterface<T_Model>::multisync(pami_multisync_t *msync,
                                                               void             *devinfo)
    {
      allocObj *req          = (allocObj *)_allocator.allocateObject();
      req->_ni               = this;
      req->_user_callback    = msync->cb_done;
      DO_DEBUG((templateName<T_Model>()));

      // pami_multisync_t  m     = *msync;
      pami_multisync_t&  m     = *msync;

      m.client   =  _clientid;
      m.context  =  _contextid;

      m.cb_done.function     =  ni_client_done;
      m.cb_done.clientdata   =  req;
      _model.postMultisync(req->_state._msync, &m, devinfo);
      return PAMI_SUCCESS;
    }


    template <class T_Model>
    inline pami_result_t CSNativeInterface<T_Model>::multicombine (pami_multicombine_t *mcomb,
                                                                   void                *devinfo)
    {
      allocObj *req          = (allocObj *)_allocator.allocateObject();
      req->_ni               = this;
      req->_user_callback    = mcomb->cb_done;
      DO_DEBUG((templateName<T_Model>()));

      // pami_multicombine_t&  m     = *mcomb;
      pami_multicombine_t&  m     = *mcomb;

      m.client   =  _clientid;
      m.context  =  _contextid;

      m.cb_done.function     =  ni_client_done;
      m.cb_done.clientdata   =  req;

      return _model.postMulticombine(req->_state._mcomb, &m, devinfo);
    }

    template <class T_Model>
    inline pami_result_t CSNativeInterface<T_Model>::multicast (uint8_t (&state)[T_Model::sizeof_multicast_msg],
                                                                pami_multicast_t *mcast,
                                                                void *devinfo)
    {
      DO_DEBUG((templateName<T_Model>()));

      mcast->dispatch =  _dispatch;

      return _model.postMulticast_impl(state, mcast, devinfo);
    }

    template <class T_Model>
    inline pami_result_t CSNativeInterface<T_Model>::multisync (uint8_t (&state)[T_Model::sizeof_multisync_msg],
                                                                pami_multisync_t *msync,
                                                                void             *devinfo)
    {
      DO_DEBUG((templateName<T_Model>()));

      return _model.postMultisync_impl(state, msync,devinfo);
    }

    template <class T_Model>
    inline pami_result_t CSNativeInterface<T_Model>::multicombine (uint8_t (&state)[T_Model::sizeof_multicombine_msg],
                                                                   pami_multicombine_t *mcomb,
                                                                   void                *devinfo)
    {
      DO_DEBUG((templateName<T_Model>()));

      return _model.postMulticombine_impl(state, mcomb, devinfo);
    }
  };
};

#endif
