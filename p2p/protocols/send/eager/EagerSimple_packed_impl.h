/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file p2p/protocols/send/eager/EagerSimple_packed_impl.h
/// \brief Eager 'short' protocol implementation.
///
#ifndef __p2p_protocols_send_eager_EagerSimple_packed_impl_h__
#define __p2p_protocols_send_eager_EagerSimple_packed_impl_h__

#ifndef __p2p_protocols_send_eager_EagerSimple_h__
#error "implementation #include'd before definition"
#endif

#include "util/trace.h"
#undef  DO_TRACE_ENTEREXIT
#undef  DO_TRACE_DEBUG
#define DO_TRACE_ENTEREXIT 0
#define DO_TRACE_DEBUG     0


using namespace PAMI::Protocol::Send;

template < class T_Model, configuration_t T_Option >
inline pami_result_t EagerSimple<T_Model, T_Option>::send_packed (eager_state_t           * state,
                                                                  pami_task_t             task,
                                                                  size_t                  offset,
                                                                  pami_send_t           * parameters)
{
  TRACE_FN_ENTER();

  // This branch should be resolved at compile time and optimized out.
  if (sizeof(packed_metadata_t) <= T_Model::packet_model_metadata_bytes)
    {
      TRACE_FORMAT("protocol metadata (%ld bytes) fits in the packet metadata (%zu bytes)", sizeof(packed_metadata_t), T_Model::packet_model_metadata_bytes);

      // Initialize the short protocol metadata on the stack to copy
      // into the packet metadata.
      packed_metadata_t metadata;
      metadata.data_bytes   = parameters->send.data.iov_len;
      metadata.header_bytes = parameters->send.header.iov_len;
      metadata.origin       = _origin;

      array_t<iovec, 2> * resized = (array_t<iovec, 2> *) & (parameters->send.header);

      _short_model.postPacket (state->origin.packed.state,
                               send_complete, (void *) state,
                               task, offset,
                               (void *) & metadata,
                               sizeof (packed_metadata_t),
                               resized->array);
    }
  else
    {
      TRACE_STRING("'short' protocol special case, protocol metadata does not fit in the packet metadata");

      // Initialize the short protocol metadata to be sent with the
      // application metadata and application data in the packet payload.
      state->origin.packed.metadata.data_bytes   = parameters->send.data.iov_len;
      state->origin.packed.metadata.header_bytes = parameters->send.header.iov_len;
      state->origin.packed.metadata.origin       = _origin;

      state->origin.packed.v[0].iov_base = (void *) & (state->origin.packed.metadata);
      state->origin.packed.v[0].iov_len  = sizeof (packed_metadata_t);
      state->origin.packed.v[1].iov_base = parameters->send.header.iov_base;
      state->origin.packed.v[1].iov_len  = parameters->send.header.iov_len;
      state->origin.packed.v[2].iov_base = parameters->send.data.iov_base;
      state->origin.packed.v[2].iov_len  = parameters->send.data.iov_len;

      _short_model.postPacket (state->origin.packed.state,
                               send_complete, (void *) state,
                               task, offset,
                               NULL, 0,
                               &state->origin.packed.v, 3);
    }
#if 0
  if (unlikely(parameters->events.remote_fn != NULL))
    {
      send_remotefn (task, offset,
                     parameters->events.remote_fn,
                     parameters->events.cookie);
    }
#endif
  TRACE_FN_EXIT();
  return PAMI_SUCCESS;
};

template < class T_Model, configuration_t T_Option >
inline int EagerSimple<T_Model, T_Option>::dispatch_packed (void   * metadata,
                                                            void   * payload,
                                                            size_t   bytes,
                                                            void   * recv_func_parm,
                                                            void   * cookie)
{
  TRACE_FN_ENTER();
  EagerSimpleProtocol * eager = (EagerSimpleProtocol *) recv_func_parm;
  uint8_t stack[T_Model::packet_model_payload_bytes];

  if (T_Model::read_is_required_packet_model)
    {
      payload = (void *) & stack[0];
      eager->_short_model.device.read (payload, bytes, cookie);
    }

  packed_metadata_t * packed_metadata;
  void * header;

  // This branch should be resolved at compile time and optimized out.
  if (sizeof(packed_metadata_t) > T_Model::packet_model_metadata_bytes)
    {
      packed_metadata = (packed_metadata_t *) payload;
      header = (void *) (packed_metadata + 1);
    }
  else
    {
      packed_metadata = (packed_metadata_t *) metadata;
      header = payload;
    }

  void * data = (void *) (((uint8_t *) header) + packed_metadata->header_bytes);

  TRACE_FORMAT("origin = 0x%08x, bytes = %d", packed_metadata->origin, packed_metadata->data_bytes);

  // Invoke the registered dispatch function.
  eager->_dispatch_fn (eager->_context,               // Communication context
                       eager->_cookie,                // Dispatch cookie
                       header,                        // Application metadata
                       packed_metadata->header_bytes, // Application metadata bytes
                       data,                          // Application data
                       packed_metadata->data_bytes,   // Application data bytes
                       packed_metadata->origin,       // Origin endpoint for the transfer
                       (pami_recv_t *) NULL);         // Synchronous receive

  TRACE_FN_EXIT();
  return 0;
};

#undef DO_TRACE_ENTEREXIT
#undef DO_TRACE_DEBUG

#endif // __p2p_protocols_send_eager_EagerSimple_packed_impl_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
