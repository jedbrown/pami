/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file p2p/protocols/send/eager/EagerSimple_data_impl.h
/// \brief Eager protocol 'data flow' implementation.
///
#ifndef __p2p_protocols_send_eager_EagerSimple_data_impl_h__
#define __p2p_protocols_send_eager_EagerSimple_data_impl_h__

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
template <bool T_ContiguousCopy>
inline pami_result_t EagerSimple<T_Model, T_Option>::send_data (eager_state_t     * state,
                                                                pami_task_t         task,
                                                                size_t              offset,
                                                                pami_send_typed_t * parameters)
{
  TRACE_FN_ENTER();

  if (T_ContiguousCopy == true)
    {
      _data_model.postMultiPacket (state->origin.eager.data.contig.state,
                                   send_complete,
                                   (void *) state,
                                   task,
                                   offset,
                                   (void *) & _origin,
                                   sizeof (pami_endpoint_t),
                                   parameters->send.data.iov_base,
                                   parameters->send.data.iov_len);
#if 0
      if (unlikely(parameters->events.remote_fn != NULL))
        {
          send_remotefn (task, offset,
                         parameters->events.remote_fn,
                         parameters->events.cookie);
        }
#endif
    }
  else
    {
      PAMI_abort();


#if 0
      Type::TypeCode * type = (Type::TypeCode *) parameters->typed.type;
      PAMI_assert_debugf(type != NULL, "parameters->typed.type == NULL !");

#ifdef ERROR_CHECKS

      if (! type->IsCompleted())
        {
          //RETURN_ERR_PAMI(PAMI_INVAL, "Using an incompleted type.");
          if (state->info.local_fn != NULL)
            state->info.local_fn (_context, state->info.cookie, PAMI_INVAL);

          TRACE_FN_EXIT();
          return;
        }

#endif

      // Construct a type machine for this transfer.
      new (&(state->typed.machine)) Type::TypeMachine (type);
      state->typed.machine.SetCopyFunc (parameters->typed.data_fn, parameters->typed.data_cookie);
      state->typed.machine.MoveCursor (parameters->typed.offset);

      const size_t atom_size = type->GetAtomSize();

      if (T_Model::packet_model_payload_bytes % atom_size != 0)
        {
          // partial packet data not supported.
          PAMI_abort();
          return;
        }

      state->typed.origin_addr     = parameters->send.data.iov_base;
      state->typed.bytes_remaining = parameters->send.data.iov_len;

      size_t bytes_remaining = parameters->send.data.iov_len;

      if (bytes_remaining <= sizeof(typed_pipeline_t))
        {
          // "short", non-pipelined, pack-n-send
          state->typed.machine.Pack (state->typed.pipeline[0],
                                     state->typed.origin_addr,
                                     bytes_remaining);

          _data_model.postMultiPacket (state->msg[2],
                                       send_complete,
                                       (void *) state,
                                       task,
                                       offset,
                                       (void *) & _origin,
                                       sizeof (pami_endpoint_t),
                                       state->typed.pipeline[0],
                                       bytes_remaining);
        }
      else if (bytes_remaining <= sizeof(typed_pipeline_t)*2)
        {
          // pack-n-send the first full pipeline width _without_ a
          // completion callback.
          state->typed.machine.Pack (state->typed.pipeline[0],
                                     state->typed.origin_addr,
                                     sizeof(typed_pipeline_t));

          _data_model.postMultiPacket (state->msg[2],
                                       NULL,
                                       (void *) NULL,
                                       task,
                                       offset,
                                       (void *) & _origin,
                                       sizeof (pami_endpoint_t),
                                       state->typed.pipeline[0],
                                       sizeof(typed_pipeline_t));

          // pack-n-send the second (and also last) pipeline width
          // with a normal data completion callback.
          state->typed.machine.Pack (state->typed.pipeline[1],
                                     state->typed.origin_addr,
                                     bytes_remaining - sizeof(typed_pipeline_t));

          _data_model.postMultiPacket (state->msg[3],
                                       send_complete,
                                       (void *) state,
                                       task,
                                       offset,
                                       (void *) & _origin,
                                       sizeof (pami_endpoint_t),
                                       state->typed.pipeline[1],
                                       bytes_remaining - sizeof(typed_pipeline_t));
        }
      else
        {
          // pack-n-send the first full pipeline width with a data
          // completion callback.
          state->typed.machine.Pack (state->typed.pipeline[0],
                                     state->typed.origin_addr,
                                     sizeof(typed_pipeline_t));

          _data_model.postMultiPacket (state->typed.msg[2],
                                       data_complete,
                                       (void *) state,
                                       task,
                                       offset,
                                       (void *) & _origin,
                                       sizeof (pami_endpoint_t),
                                       state->typed.pipeline[0],
                                       sizeof(typed_pipeline_t));

          // pack-n-send the second full pipeline width with a data
          // completion callback.
          state->typed.machine.Pack (state->typed.pipeline[1],
                                     state->typed.origin_addr,
                                     sizeof(typed_pipeline_t));

          _data_model.postMultiPacket (state->msg[3],
                                       data_complete,
                                       (void *) state,
                                       task,
                                       offset,
                                       (void *) & _origin,
                                       sizeof (pami_endpoint_t),
                                       state->typed.pipeline[1],
                                       sizeof(typed_pipeline_t));
        }

#endif
    }

  TRACE_FN_EXIT();
  return PAMI_SUCCESS;
};

template < class T_Model, configuration_t T_Option >
template <bool T_ContiguousCopy>
inline int EagerSimple<T_Model, T_Option>::dispatch_data   (void   * metadata,
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

  pami_endpoint_t origin = *((pami_endpoint_t *) metadata);
  pami_task_t task;
  size_t offset;
  PAMI_ENDPOINT_INFO(origin, task, offset);

  TRACE_FORMAT("origin task = %d, origin offset = %zu, bytes = %zu", task, offset, bytes);

  eager_state_t * state = (eager_state_t *) eager->_data_model.device.getConnection (task, offset);

  // Number of bytes received so far.
  const size_t nbyte = state->target.data.bytes_received;

  // Number of bytes left to copy into the destination buffer
  const size_t nleft = state->target.data.bytes_total - nbyte;

  TRACE_FORMAT("bytes received so far = %zu, bytes yet to receive = %zu, total bytes to receive = %zu", state->target.data.bytes_received, nleft, state->target.data.bytes_total);

  // Copy data from the packet payload into the destination buffer
  const size_t ncopy = MIN(nleft, bytes);

  if (T_ContiguousCopy || likely(state->target.data.is_contiguous_copy_recv))
    Core_memcpy ((uint8_t *)(state->target.info.addr) + nbyte, payload, ncopy);
  else
    {
      Type::TypeMachine * machine = (Type::TypeMachine *) state->target.data.machine;
      machine->Unpack (state->target.info.addr, payload, ncopy);
    }

  state->target.data.bytes_received += ncopy;

  TRACE_FORMAT("nbyte = %zu, bytes = %zu, state->target.data.bytes_total = %zu", nbyte, bytes, state->target.data.bytes_total);

  if ((nbyte + bytes) >= state->target.data.bytes_total)
    {
      // No more data packets will be received on this connection.
      // Clear the connection data and prepare for the next message.
      eager->_data_model.device.clearConnection (task, offset);

      // No more data is to be written to the receive buffer.
      // Invoke the receive done callback.
      if (state->target.info.local_fn)
        state->target.info.local_fn (eager->_context,
                              state->target.info.cookie,
                              PAMI_SUCCESS);

      // Return the receive state object memory to the memory pool.
      eager->freeRecvState (state);

      TRACE_FORMAT("origin task = %d ... receive completed", task);
      TRACE_FN_EXIT();
      return 0;
    }

  TRACE_FORMAT("origin task = %d ... wait for more data", task);
  TRACE_FN_EXIT();
  return 0;
};

template < class T_Model, configuration_t T_Option >
inline void EagerSimple<T_Model, T_Option>::complete_data (pami_context_t   context,
                                                           void           * cookie,
                                                           pami_result_t    result)
{
  TRACE_FN_ENTER();

  eager_state_t * state = (eager_state_t *) cookie;
  EagerSimpleProtocol * eager = (EagerSimpleProtocol *) state->origin.protocol;

  size_t which = state->origin.data.noncontig.start_count & 0x01;
  state->origin.data.noncontig.start_count += 1;

  const size_t bytes_remaining = state->origin.data.noncontig.bytes_remaining;

  if (bytes_remaining <= sizeof(pipeline_t))
    {
      // pack-n-send the last pipeline width with a _send_
      // completion callback.
      state->origin.data.noncontig.machine.Pack (state->origin.data.noncontig.pipeline[which],
                                                 state->origin.data.noncontig.origin_addr,
                                                 bytes_remaining);

      state->origin.data.noncontig.bytes_remaining -= bytes_remaining;

      eager->_data_model.postMultiPacket (state->origin.data.noncontig.state[which],
                                          send_complete,
                                          (void *) state,
                                          0,//task,
                                          0,//offset,
                                          (void *) & eager->_origin,
                                          sizeof (pami_endpoint_t),
                                          state->origin.data.noncontig.pipeline[which],
                                          bytes_remaining);
    }
  else if (bytes_remaining <= sizeof(pipeline_t)*2)
    {
      // pack-n-send this next-to-last full pipeline width _without_ a
      // completion callback.
      state->origin.data.noncontig.machine.Pack (state->origin.data.noncontig.pipeline[which],
                                                 state->origin.data.noncontig.origin_addr,
                                                 sizeof(pipeline_t));

      state->origin.data.noncontig.bytes_remaining -= sizeof(pipeline_t);

      eager->_data_model.postMultiPacket (state->origin.data.noncontig.state[which],
                                          NULL,
                                          (void *) NULL,
                                          0,//task,
                                          0,//offset,
                                          (void *) & eager->_origin,
                                          sizeof (pami_endpoint_t),
                                          state->origin.data.noncontig.pipeline[which],
                                          sizeof(pipeline_t));
    }
  else
    {
      // pack-n-send this next full pipeline width with a pipeline
      // completion callback.
      state->origin.data.noncontig.machine.Pack (state->origin.data.noncontig.pipeline[which],
                                                 state->origin.data.noncontig.origin_addr,
                                                 sizeof(pipeline_t));

      state->origin.data.noncontig.bytes_remaining -= sizeof(pipeline_t);

      eager->_data_model.postMultiPacket (state->origin.data.noncontig.state[which],
                                          complete_data,
                                          (void *) state,
                                          0,//task,
                                          0,//offset,
                                          (void *) & eager->_origin,
                                          sizeof (pami_endpoint_t),
                                          state->origin.data.noncontig.pipeline[which],
                                          sizeof(pipeline_t));
    }

  TRACE_FN_EXIT();
  return;
};

#undef DO_TRACE_ENTEREXIT
#undef DO_TRACE_DEBUG

#endif // __p2p_protocols_send_eager_EagerSimple_data_impl_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
