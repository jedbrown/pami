///
/// \file components/context/lapiunix/bgpcontext.h
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
          return XMI_UNIMPL;
        }

        inline xmi_result_t send_impl (xmi_send_immediate_t * parameters)
        {
          return XMI_UNIMPL;
        }

        inline xmi_result_t send_impl (xmi_send_typed_t * parameters)
        {
          return XMI_UNIMPL;
        }

        inline xmi_result_t dispatch_impl (size_t                     id,
                                           xmi_dispatch_callback_fn   fn,
                                           void                     * cookie,
                                           xmi_send_hint_t            options)
        {
          if (_dispatch[id] == NULL) return XMI_ERROR;

          // For now, only enable dma short/eager sends.
          XMI::Protocol::Send::Eager <ShmemModel, ShmemDevice, ShmemMessage> * eager;
          posix_memalign ((void **)&eager, 16,
                          sizeof(XMI::Protocol::Send::Eager <ShmemModel,
                                                             ShmemDevice,
                                                             ShmemMessage>));
          new (eager) XMI::Protocol::Send::Eager <ShmemModel,ShmemDevice,ShmemMessage>
                                                 (_shmem, _mapping, result);
#if 0
          factory = new (registration)
            Protocol::Send::CDI::EagerFactory <ShmemModel, ShmemDevice, ShmemMessage>
                                              (configuration->cb_recv_short,
                                               configuration->cb_recv_short_clientdata,
                                               configuration->cb_recv,
                                               configuration->cb_recv_clientdata,
                                               msgr->shmem(),
                                               msgr->mapping(),
                                               status,
                                               DCMF_PROTOCOL_HAS_NO_CHANNELS);
#endif

          return XMI_UNIMPL;
        }


        inline xmi_result_t geometry_algorithms_num_impl (xmi_context_t context,
                                                          xmi_geometry_t geometry,
                                                          xmi_xfer_type_t ctype,
                                                          int *lists_lengths)
        {
          return XMI_UNIMPL;
        }

        inline xmi_result_t geometry_algorithm_info_impl (xmi_context_t context,
                                                          xmi_geometry_t geometry,
                                                          xmi_xfer_type_t type,
                                                          xmi_algorithm_t algorithm,
                                                          int algorithm_type,
                                                          xmi_metadata_t *mdata)
        {
          return XMI_UNIMPL;
        }


      private:

        xmi_client_t _client;

        void * _dispatch[1024];

    }; // end XMI::Context::BGP
  }; // end namespace Context
}; // end namespace XMI

#endif // __xmi_bgp_bgpcontext_h__
