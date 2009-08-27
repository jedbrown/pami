///
/// \file xmi/bgp/bgpcontext.h
/// \brief XMI BGP specific context implementation.
///
#ifndef   __xmi_bgp_bgpcontext_h__
#define   __xmi_bgp_bgpcontext_h__

#include <stdlib.h>
#include <string.h>

#include "xmi/Context.h"

#define XMI_CONTEXT_CLASS XMI::Context::BGP

namespace XMI
{
  namespace Context
  {
    typedef CDI::Fifo::FifoPacket <16,240> ShmemPacket;
    typedef CDI::Fifo::LinearFifo<Atomic::GccBuiltin,ShmemPacket,16> ShmemFifo;

    typedef CDI::ShmemBaseMessage<ShmemPacket> ShmemMessage;
    typedef CDI::ShmemPacketDevice<ShmemFifo,ShmemPacket> ShmemDevice;
    typedef CDI::ShmemPacketModel<ShmemDevice,ShmemMessage> ShmemModel;
  
    class BGP : public Context<XMI::Context::BGP>
    {
      public:
        inline BGP (xmi_client_t client) :
          Context<XMI::Context::BGP> (client),
          _client (client)
        {
        }
        
        inline xmi_client_t getClientId_impl ()
        {
          return _client;
        }

        inline void destroy_impl ()
        {
        }

        inline xmi_result_t post_impl (xmi_event_function work_fn, void * cookie)
        {
          return XMI_UNIMPL;
        }

        inline size_t advance_impl (size_t maximum, xmi_result_t & result)
        {
          result = XMI_UNIMPL;
          return 0;
        }

        inline xmi_result_t lock_impl ()
        {
          return XMI_UNIMPL;
        }

        inline xmi_result_t trylock_impl ()
        {
          return XMI_UNIMPL;
        }

        inline xmi_result_t unlock_impl ()
        {
          return XMI_UNIMPL;
        }

        inline xmi_result_t send_impl (xmi_send_simple_t * parameters)
        {
          assert (_dispatch[id] != NULL);
          
          XMI::Protocol::Send::Simple * send =
            (XMI::Protocol::Send::Simple *) _dispatch[id];
          send->start (parameters->simple.local_fn,
                       parameters->simple.remote_fn,
                       parameters->cookie,
                       parameters->task,
                       parameters->simple.addr,
                       parameters->simple.bytes,
                       parameters->header.addr,
                       parameters->header.bytes);
          
          return XMI_SUCCESS;
        }

        inline xmi_result_t send_impl (xmi_send_immediate_t * parameters)
        {
          return XMI_UNIMPL;
        }

        inline xmi_result_t send_impl (xmi_send_typed_t * parameters)
        {
          return XMI_UNIMPL;
        }

        inline xmi_result_t dispatch_impl (xmi_dispatch_t             id,
                                           xmi_dispatch_callback_fn   fn,
                                           void                     * cookie,
                                           xmi_send_hint_t            options)
        {
          if (_dispatch[id] != NULL) return XMI_ERROR;

          // Allocate memory for the protocol object.
          _dispatch[id] = (void *) _request.allocateObject ();

          // For now, only enable shmem short/eager sends.
          typedef XMI::Protocol::Send::Eager <ShmemModel, ShmemDevice, ShmemMessage> EagerShmem;
          new (_dispatch[id]) EagerShmem (id, fn, cookie, _shmem, _mapping.rank(), result);

          return XMI_SUCCESS;
        }



        

      private:
      
        xmi_client_t _client;

        void * _dispatch[1024];
        MemoryAllocator<sizeof(CM_Request_t),16> _request;

    }; // end XMI::Context::BGP
  }; // end namespace Context
}; // end namespace XMI

#endif // __xmi_bgp_bgpcontext_h__
