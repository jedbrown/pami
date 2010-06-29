/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file p2p/protocols/send/eager/EagerSimple.h
/// \brief Simple eager send protocol for reliable devices.
///
/// The EagerSimple class defined in this file uses C++ templates and
/// the device "message" interface - which also uses C++ templates.
///
/// C++ templates require all source code to be #include'd from a header file.
///
#ifndef __p2p_protocols_send_eager_EagerSimple_h__
#define __p2p_protocols_send_eager_EagerSimple_h__

#include "components/memory/MemoryAllocator.h"

#undef TRACE_ERR
#define TRACE_ERR(x)  //fprintf x

namespace PAMI
{
  namespace Protocol
  {
    namespace Send
    {

      ///
      /// \brief Eager simple send protocol class for reliable network devices.
      ///
      /// \tparam T_Model      Packet model class
      /// \tparam T_Device     Packet device class
      /// \tparam T_LongHeader Enable/disable long header support
      /// \tparam T_Connection Connection class
      ///
      /// \see PAMI::Device::Interface::PacketModel
      /// \see PAMI::Device::Interface::PacketDevice
      ///
      template <class T_Model, class T_Device, bool T_LongHeader, class T_Connection>
      class EagerSimple
      {
        protected:

          typedef EagerSimple<T_Model,T_Device,T_LongHeader,T_Connection> EagerSimpleProtocol;

          typedef uint8_t pkt_t[T_Model::packet_model_state_bytes];

          typedef struct __attribute__((__packed__)) protocol_metadata
          {
            size_t             bytes;     ///< Number of bytes of application data being sent
            size_t             metabytes; ///< Number of bytes of application metadata being sent
            void             * va_send;   ///< Virtual address of the send_state_t on the origin
            pami_endpoint_t    origin;    ///< Endpoint that originated the transfer
          } protocol_metadata_t;


          typedef struct send_state
          {
            pkt_t                   pkt[3];
            protocol_metadata_t      metadata;  ///< Eager protocol envelope metadata
            pami_event_function     local_fn;  ///< Application send injection completion callback
            pami_event_function     remote_fn; ///< Application remote receive acknowledgement callback
            void                  * cookie;    ///< Application callback cookie
            struct iovec            v[2];      ///< Iovec array used for transfers
            EagerSimpleProtocol   * eager;    ///< Eager protocol object
          } send_state_t;

          typedef struct recv_state
          {
            pkt_t                   pkt;      ///< packet send state memory
            pami_recv_t             info;     ///< Application receive information.
            size_t                  received; ///< Number of bytes received.
            struct
            {
              uint8_t             * addr;     ///< Address of the long header recv buffer
              size_t                bytes;
              size_t                offset;
            } longheader;
            protocol_metadata_t      metadata; ///< Original eager protocol envelope metadata
            EagerSimpleProtocol   * eager;   ///< Eager protocol object
          } recv_state_t;

        public:

          ///
          /// \brief Eager simple send protocol constructor.
          ///
          /// \param[in]  dispatch     Dispatch identifier
          /// \param[in]  dispatch_fn  Point-to-point dispatch callback function
          /// \param[in]  cookie       Opaque application dispatch data
          /// \param[in]  device       Device that implements the message interface
          /// \param[in]  origin       Origin endpoint
          /// \param[in]  context      Origin communcation context
          /// \param[out] status       Constructor status
          ///
          inline EagerSimple (size_t                 dispatch,
                              pami_dispatch_p2p_fn   dispatch_fn,
                              void                 * cookie,
                              T_Device             & device,
                              pami_endpoint_t        origin,
                              pami_context_t         context,
                              pami_result_t        & status) :
              _envelope_model (device),
              _longheader_model (device),
              _data_model (device),
              _ack_model (device),
              _device (device),
              _origin (origin),
              _context (context),
              _dispatch_fn (dispatch_fn),
              _cookie (cookie),
              _connection (device)
          {
            // ----------------------------------------------------------------
            // Compile-time assertions
            // ----------------------------------------------------------------

            // This protocol only works with reliable networks.
            COMPILE_TIME_ASSERT(T_Model::reliable_packet_model == true);

            // This protocol only works with deterministic models.
            COMPILE_TIME_ASSERT(T_Model::deterministic_packet_model == true);

            // Assert that the size of the packet metadata area is large
            // enough to transfer the eager match information. This is used in the
            // various postMultiPacket() calls to transfer long header and data
            // messages.
            COMPILE_TIME_ASSERT(sizeof(pami_endpoint_t) <= T_Model::packet_model_multi_metadata_bytes);

            // Assert that the size of the packet payload area is large
            // enough to transfer a single virtual address. This is used in
            // the postPacket() calls to transfer the ack information.
            COMPILE_TIME_ASSERT(sizeof(void *) <= T_Model::packet_model_payload_bytes);

            // ----------------------------------------------------------------
            // Compile-time assertions (end)
            // ----------------------------------------------------------------

            // The models must be registered in reverse order of use in case
            // the remote side is delayed in it's registrations and must save
            // unexpected packets until dispatch registration.
            TRACE_ERR((stderr, "EagerSimple() register ack model dispatch %zu\n",dispatch));
            status = _ack_model.init (dispatch,
                                      dispatch_ack_direct, this,
                                      dispatch_ack_read, this);
            TRACE_ERR((stderr, "EagerSimple() ack model status = %d\n", status));
            if (status == PAMI_SUCCESS)
              {
                TRACE_ERR((stderr, "EagerSimple() register data model dispatch %zu\n",dispatch));
                status = _data_model.init (dispatch,
                                           dispatch_data_message, this,
                                           dispatch_data_message, this);
                TRACE_ERR((stderr, "EagerSimple() data model status = %d\n", status));
                if (status == PAMI_SUCCESS)
                  {
                    TRACE_ERR((stderr, "EagerSimple() register envelope  model dispatch %zu\n",dispatch));
                    status = _envelope_model.init (dispatch,
                                                   dispatch_envelope_direct, this,
                                                   dispatch_envelope_read, this);
                    TRACE_ERR((stderr, "EagerSimple() envelope model status = %d\n", status));

                    TRACE_ERR((stderr, "EagerSimple() 'long header' support enabled = %d\n", T_LongHeader));
                    if (T_LongHeader == true)
                      {
                        TRACE_ERR((stderr, "EagerSimple() register 'long header'  model dispatch %zu\n",dispatch));
                        status = _longheader_model.init (dispatch,
                                                         dispatch_longheader_message, this,
                                                         dispatch_longheader_message, this);
                        TRACE_ERR((stderr, "EagerSimple() 'long header' model status = %d\n", status));
                      }
                  }
              }
          }

          inline pami_result_t simple_impl (pami_send_t * parameters)
          {
            TRACE_ERR((stderr, ">> EagerSimple::simple_impl() .. sizeof(protocol_metadata_t) = %zu, T_Model::packet_model_metadata_bytes = %zu\n", sizeof(protocol_metadata_t), T_Model::packet_model_metadata_bytes));

            pami_task_t task;
            size_t offset;
            PAMI_ENDPOINT_INFO(parameters->send.dest,task,offset);

            // Verify that this task is addressable by this packet device
            if (!_device.isPeer (task)) return PAMI_ERROR;

            // Allocate memory to maintain the state of the send.
            send_state_t * state = allocateSendState ();

            state->cookie   = parameters->events.cookie;
            state->local_fn = parameters->events.local_fn;
            state->eager    = this;

            // Specify the protocol metadata to send with the application
            // metadata in the envelope packet.
            state->metadata.bytes        = parameters->send.data.iov_len;
            state->metadata.metabytes    = parameters->send.header.iov_len;
            state->metadata.origin       = _origin;

            // Set the acknowledgement information to the virtual address of
            // send state object on the origin task if a local callback of the
            // remote receive completion event is requested. If this is set to
            // NULL no acknowledgement will be received by the origin task.
            if (parameters->events.remote_fn != NULL)
              {
                state->metadata.va_send = state;
                state->remote_fn = parameters->events.remote_fn;
              }
            else
              {
                state->metadata.va_send = NULL;
                state->remote_fn = NULL;
              }

            if (unlikely(parameters->send.data.iov_len == 0))
              {
                // In the unlikely event that this eager (i.e., multi-packet)
                // protocol is being used to send zero bytes of application
                // data, the envelope message completion must clean up the
                // protocol resources and invoke the application local done
                // callback function, as there will be no data message.
                TRACE_ERR((stderr, "   EagerSimple::simple_impl() .. zero-byte data special case\n"));

                // This branch should be resolved at compile time and optimized out.
                if (sizeof(protocol_metadata_t) <= T_Model::packet_model_metadata_bytes)
                  {
                    TRACE_ERR((stderr, "   EagerSimple::simple_impl() .. zero-byte data special case, protocol metadata fits in the packet metadata\n"));

#ifdef ERROR_CHECKS
                    if (T_LongHeader==false && unlikely(parameters->send.header.iov_len > T_Model::packet_model_payload_bytes))
                    {
                      // "'long header' support is disabled.\n"
                      return PAMI_INVAL;
                    }
#endif

                    // Short circuit evaluation of the constant expression will
                    // allow the compiler to optimize out this branch when long
                    // header support is disabled.
                    //
                    // When long header support is enabled the compiler should
                    // respect the 'unlikely if' conditional.
                    if (T_LongHeader == true && unlikely(parameters->send.header.iov_len > T_Model::packet_model_payload_bytes))
                      {
                        TRACE_ERR((stderr, "   EagerSimple::simple_impl() .. zero-byte data special case, protocol metadata fits in the packet metadata, application metadata does not fit in a single packet payload\n"));

                        // "long header" with zero bytes of application data ? Weird.
                        _envelope_model.postPacket (state->pkt[0],
                                                    NULL,
                                                    NULL,
                                                    task,
                                                    offset,
                                                    (void *) &(state->metadata),
                                                    sizeof (protocol_metadata_t),
                                                    (void *) NULL, 0);

                        _longheader_model.postMultiPacket (state->pkt[1],
                                                           send_complete,
                                                           (void *) state,
                                                           task,
                                                           offset,
                                                           (void *) &(state->metadata.origin),
                                                           sizeof (pami_endpoint_t),
                                                           parameters->send.header.iov_base,
                                                           parameters->send.header.iov_len);
                      }
                    else
                      {
                        TRACE_ERR((stderr, "   EagerSimple::simple_impl() .. zero-byte data special case, protocol metadata fits in the packet metadata, application metadata does fit in a single packet payload\n"));

                        // Single packet header with zero bytes of application data.
                        _envelope_model.postPacket (state->pkt[0],
                                                    send_complete,
                                                    (void *) state,
                                                    task,
                                                    offset,
                                                    (void *) &(state->metadata),
                                                    sizeof (protocol_metadata_t),
                                                    parameters->send.header.iov_base,
                                                    parameters->send.header.iov_len);
                      }
                  }
                else
                  {
                    TRACE_ERR((stderr, "   EagerSimple::simple_impl() .. zero-byte data special case, protocol metadata does not fit in the packet metadata\n"));
                    //PAMI_assertf((parameters->send.header.bytes + sizeof(protocol_metadata_t)) <= T_Model::packet_model_payload_bytes, "Unable to fit protocol metadata (%zu) and application metadata (%zu) within the payload of a single packet (%zu)\n", sizeof(protocol_metadata_t), parameters->send.header.bytes, T_Model::packet_model_payload_bytes);

#ifdef ERROR_CHECKS
                    if (T_LongHeader==false && unlikely(parameters->send.header.iov_len > (T_Model::packet_model_payload_bytes - sizeof(protocol_metadata_t))))
                    {
                      // "'long header' support is disabled.\n"
                      TRACE_ERR((stderr, "   EagerSimple::simple_impl() .. error .. 'long header' support is disabled.\n"));
                      return PAMI_INVAL;
                    }
#endif

                    // Short circuit evaluation of the constant expression will
                    // allow the compiler to optimize out this branch when long
                    // header support is disabled.
                    //
                    // When long header support is enabled the compiler should
                    // respect the 'unlikely if' conditional.
                    if (T_LongHeader == true && unlikely(parameters->send.header.iov_len > (T_Model::packet_model_payload_bytes - sizeof(protocol_metadata_t))))
                      {
                        TRACE_ERR((stderr, "   EagerSimple::simple_impl() .. zero-byte data special case, protocol metadata does not fit in the packet metadata, protocol + application metadata does not fit in a single packet payload\n"));

                        // "long header" with zero bytes of application data ? Weird.
                        _envelope_model.postPacket (state->pkt[0],
                                                    NULL,
                                                    NULL,
                                                    task,
                                                    offset,
                                                    NULL, 0,
                                                    (void *) &(state->metadata),
                                                    sizeof (protocol_metadata_t));

                        _longheader_model.postMultiPacket (state->pkt[1],
                                                           send_complete,
                                                           (void *) state,
                                                           task,
                                                           offset,
                                                           (void *) &(state->metadata.origin),
                                                           sizeof (pami_endpoint_t),
                                                           parameters->send.header.iov_base,
                                                           parameters->send.header.iov_len);
                      }
                    else
                      {
                        TRACE_ERR((stderr, "   EagerSimple::simple_impl() .. zero-byte data special case, protocol metadata does not fit in the packet metadata, protocol + application metadata does fit in a single packet payload\n"));

                        // Single packet header with zero bytes of application data.
                        state->v[0].iov_base = (void *) &(state->metadata);
                        state->v[0].iov_len  = sizeof (protocol_metadata_t);
                        state->v[1].iov_base = parameters->send.header.iov_base;
                        state->v[1].iov_len  = parameters->send.header.iov_len;
                        _envelope_model.postPacket (state->pkt[0],
                                                    send_complete,
                                                    (void *) state,
                                                    task,
                                                    offset,
                                                    NULL, 0,
                                                    state->v);
                      }
                  }
              }
            else
              {
                TRACE_ERR((stderr, "   EagerSimple::simple_impl() .. parameters->send.header.iov_len = %zu, parameters->send.data.iov_len = %zu\n", parameters->send.header.iov_len, parameters->send.data.iov_len));

                // This branch should be resolved at compile time and optimized out.
                if (sizeof(protocol_metadata_t) <= T_Model::packet_model_metadata_bytes)
                  {
                    TRACE_ERR((stderr, "   EagerSimple::simple_impl() .. protocol metadata (%zu bytes) sent in the packet metadata (%zu bytes available).\n", sizeof(protocol_metadata_t), T_Model::packet_model_metadata_bytes));
#ifdef ERROR_CHECKS
                    if (T_LongHeader==false && unlikely(parameters->send.header.iov_len > T_Model::packet_model_payload_bytes))
                    {
                      // "'long header' support is disabled.\n"
                      TRACE_ERR((stderr, "   EagerSimple::simple_impl() .. ERROR. 'long header' support is disabled.\n"));
                      return PAMI_INVAL;
                    }
#endif

                    // Short circuit evaluation of the constant expression will
                    // allow the compiler to optimize out this branch when long
                    // header support is disabled.
                    //
                    // When long header support is enabled the compiler should
                    // respect the 'unlikely if' conditional.
                    if (T_LongHeader == true && unlikely(parameters->send.header.iov_len > T_Model::packet_model_payload_bytes))
                      {
                        // Application metadata does not fit in a single packet.
                        // Send a "long header" message.
                        TRACE_ERR((stderr, "   EagerSimple::simple_impl() .. application metadata (%zu bytes) is too large for a single packet (%zu bytes of payload). Send a 'long header'.\n", parameters->send.header.iov_len, T_Model::packet_model_payload_bytes));

                        _envelope_model.postPacket (state->pkt[0],
                                                    NULL,
                                                    NULL,
                                                    task,
                                                    offset,
                                                    (void *) &(state->metadata),
                                                    sizeof (protocol_metadata_t),
                                                    (void *) NULL, 0);

                        _longheader_model.postMultiPacket (state->pkt[1],
                                                           NULL,
                                                           NULL,
                                                           task,
                                                           offset,
                                                           (void *) &(state->metadata.origin),
                                                           sizeof (pami_endpoint_t),
                                                           parameters->send.header.iov_base,
                                                           parameters->send.header.iov_len);
                      }
                    else
                      {
                        TRACE_ERR((stderr, "   EagerSimple::simple_impl() .. application metadata (%zu bytes) sent in a single packet (%zu bytes of payload).\n", parameters->send.header.iov_len, T_Model::packet_model_payload_bytes));
                        _envelope_model.postPacket (state->pkt[0],
                                                    NULL,
                                                    NULL,
                                                    task,
                                                    offset,
                                                    (void *) &(state->metadata),
                                                    sizeof (protocol_metadata_t),
                                                    parameters->send.header.iov_base,
                                                    parameters->send.header.iov_len);
                      }
                  }
                else
                  {
                    TRACE_ERR((stderr, "   EagerSimple::simple_impl() .. protocol metadata (%zu bytes) is too large for the packet metadata (%zu bytes available). Protocol metadata will be sent in the packet payload.\n", sizeof(protocol_metadata_t), T_Model::packet_model_metadata_bytes));
#ifdef ERROR_CHECKS
                    if (T_LongHeader==false && unlikely(parameters->send.header.iov_len > (T_Model::packet_model_payload_bytes - sizeof(protocol_metadata_t))))
                    {
                      // "'long header' support is disabled.\n"
                      TRACE_ERR((stderr, "   EagerSimple::simple_impl() .. ERROR. 'long header' support is disabled.\n"));
                      return PAMI_INVAL;
                    }
#endif

                    // Short circuit evaluation of the constant expression will
                    // allow the compiler to optimize out this branch when long
                    // header support is disabled.
                    //
                    // When long header support is enabled the compiler should
                    // respect the 'unlikely if' conditional.
                    if (T_LongHeader == true && unlikely(parameters->send.header.iov_len > (T_Model::packet_model_payload_bytes - sizeof(protocol_metadata_t))))
                      {
                        // Protocol metadata + application metadata does not fit in
                        // a single packet. Send a "long header" message.
                        TRACE_ERR((stderr, "   EagerSimple::simple_impl() .. protocol metadata (%zu bytes) + application metadata (%zu bytes) is too large for a single packet (%zu bytes of payload). Send a 'long header'.\n", sizeof(protocol_metadata_t), parameters->send.header.iov_len, T_Model::packet_model_payload_bytes));

                        _envelope_model.postPacket (state->pkt[0],
                                                    NULL,
                                                    NULL,
                                                    task,
                                                    offset,
                                                    NULL,
                                                    0,
                                                    (void *) &(state->metadata),
                                                    sizeof (protocol_metadata_t));

                        _longheader_model.postMultiPacket (state->pkt[1],
                                                           NULL,
                                                           NULL,
                                                           task,
                                                           offset,
                                                           (void *) &(state->metadata.origin),
                                                           sizeof (pami_endpoint_t),
                                                           parameters->send.header.iov_base,
                                                           parameters->send.header.iov_len);
                      }
                    else
                      {
                        TRACE_ERR((stderr, "   EagerSimple::simple_impl() .. protocol metadata (%zu bytes) + application metadata (%zu bytes) sent in a single packet (%zu bytes of payload).\n", sizeof(protocol_metadata_t), parameters->send.header.iov_len, T_Model::packet_model_payload_bytes));

                        state->v[0].iov_base = (void *) &(state->metadata);
                        state->v[0].iov_len  = sizeof (protocol_metadata_t);
                        state->v[1].iov_base = parameters->send.header.iov_base;
                        state->v[1].iov_len  = parameters->send.header.iov_len;

                        _envelope_model.postPacket (state->pkt[0],
                                                    NULL, NULL,
                                                    task, offset,
                                                    NULL, 0,
                                                    state->v);
                      }
                  }

                TRACE_ERR((stderr, "   EagerSimple::simple_impl() .. send the application data.\n"));
                _data_model.postMultiPacket (state->pkt[2],
                                             send_complete,
                                             (void *) state,
                                             task,
                                             offset,
                                             (void *) &(state->metadata.origin),
                                             sizeof (pami_endpoint_t),
                                             parameters->send.data.iov_base,
                                             parameters->send.data.iov_len);
              }

            TRACE_ERR((stderr, "<< EagerSimple::simple_impl()\n"));
            return PAMI_SUCCESS;
          };

        protected:

          MemoryAllocator < sizeof(send_state_t), 16 > _send_allocator;
          MemoryAllocator < sizeof(recv_state_t), 16 > _recv_allocator;

          T_Model          _envelope_model;
          T_Model          _longheader_model;
          T_Model          _data_model;
          T_Model          _ack_model;
          T_Device       & _device;
          pami_endpoint_t  _origin;
          pami_context_t   _context;

          pami_dispatch_p2p_fn   _dispatch_fn;
          void                     * _cookie;

          T_Connection              _connection;

          inline send_state_t * allocateSendState ()
          {
            return (send_state_t *) _send_allocator.allocateObject ();
          }

          inline void freeSendState (send_state_t * object)
          {
            _send_allocator.returnObject ((void *) object);
          }

          inline recv_state_t * allocateRecvState ()
          {
            return (recv_state_t *) _recv_allocator.allocateObject ();
          }

          inline void freeRecvState (recv_state_t * object)
          {
            _recv_allocator.returnObject ((void *) object);
          }

          inline void process_envelope (protocol_metadata_t * metadata,
                                        uint8_t            * header,
                                        recv_state_t       * state)
          {
            TRACE_ERR((stderr, ">> EagerSimple::process_envelope() .. origin = 0x%08x, header = %p, header bytes = %zu\n", metadata->origin, header, metadata->metabytes));

            // Invoke the registered dispatch function.
            _dispatch_fn (_context,            // Communication context
                              _cookie,             // Dispatch cookie
                              header,              // Application metadata
                              metadata->metabytes, // Application metadata bytes
                              NULL,                // No payload data
                              metadata->bytes,     // Number of msg bytes
                              metadata->origin,    // Origin endpoint for the transfer
                              (pami_recv_t *) &(state->info));

            TRACE_ERR((stderr, "   EagerSimple::process_envelope() .. metadata->bytes = %zu, state->info.type = %p\n", metadata->bytes, state->info.type));

            // Only contiguous receives are implemented
            PAMI_assertf(state->info.type == PAMI_BYTE, "[%5d:%s] %s() - Only contiguous receives are implemented.\n", __LINE__, __FILE__, __FUNCTION__);

            if (unlikely(metadata->bytes == 0))
              {
                // No data packets will follow this envelope packet. Invoke the
                // recv done callback and, if an acknowledgement packet was
                // requested send the acknowledgement. Otherwise return the recv
                // state memory which was allocated above.
                TRACE_ERR((stderr, "   EagerSimple::process_envelope() .. state->info.local_fn = %p\n", state->info.local_fn));

                if (state->info.local_fn)
                  state->info.local_fn (_context, state->info.cookie, PAMI_SUCCESS);

                TRACE_ERR((stderr, "   EagerSimple::process_envelope() .. state->metadata.va_send = %p\n", state->metadata.va_send));
                pami_task_t task;
                size_t offset;
                PAMI_ENDPOINT_INFO(metadata->origin,task,offset);

                if (unlikely(state->metadata.va_send != NULL))
                  {
                    _ack_model.postPacket (state->pkt,
                                           receive_complete,
                                           (void *) state,
                                           task,
                                           offset,
                                           NULL, 0,
                                           (void *) &(state->metadata.va_send),
                                           sizeof (send_state_t *));
                  }
                else
                  {
                    _connection.clear (metadata->origin);
                    freeRecvState (state);
                  }
              }

            TRACE_ERR((stderr, "<< EagerSimple::process_envelope()\n"));
            return;
          }

          static int dispatch_ack_direct (void   * metadata,
                                          void   * payload,
                                          size_t   bytes,
                                          void   * recv_func_parm,
                                          void   * cookie)
          {
            TRACE_ERR((stderr, ">> EagerSimple::dispatch_ack_direct()\n"));
            send_state_t * state = *((send_state_t **) payload);

            pami_event_function   remote_fn = state->remote_fn;
            void               * fn_cookie = state->cookie;
            TRACE_ERR((stderr, "   EagerSimple::dispatch_ack_direct() .. state = %p, remote_fn = %p\n", state, remote_fn));

            EagerSimpleProtocol * eager = (EagerSimpleProtocol *) recv_func_parm;

            eager->freeSendState (state);

            if (remote_fn) remote_fn (eager->_context, fn_cookie, PAMI_SUCCESS);

            TRACE_ERR((stderr, "<< EagerSimple::dispatch_ack_direct()\n"));
            return 0;
          }

          static int dispatch_ack_read (void   * metadata,
                                        void   * payload,
                                        size_t   bytes,
                                        void   * recv_func_parm,
                                        void   * cookie)
          {
            TRACE_ERR((stderr, ">> EagerSimple::dispatch_ack_read()\n"));

            EagerSimpleProtocol * pf = (EagerSimpleProtocol *) recv_func_parm;

            // This packet device DOES NOT provide the data buffer(s) for the
            // message and the data must be read on to the stack before the
            // recv callback is invoked.
            PAMI_assert_debugf(payload == NULL, "The 'read only' packet device did not invoke dispatch with payload == NULL (%p)\n", payload);

            uint8_t stackData[T_Model::packet_model_payload_bytes];
            void * p = (void *) & stackData[0];
            pf->_device.read (p, bytes, cookie);

            dispatch_ack_direct (metadata, p, bytes, recv_func_parm, cookie);

            TRACE_ERR((stderr, "<< EagerSimple::dispatch_ack_read()\n"));
            return 0;
          }

          ///
          /// \brief Direct multi-packet send envelope packet dispatch.
          ///
          /// The eager simple send protocol will register this dispatch
          /// function if and only if the device \b does provide direct access
          /// to data which has already been read from the network by the
          /// device.
          ///
          /// The envelope dispatch function is invoked by the message device
          /// to process the first packet of a multi-packet message. The eager
          /// simple send protocol transfers protocol metadata and application
          /// metadata in a single packet. Application data will arrive in
          /// subsequent eager simple send data packets and will be processed
          /// by the data dispatch function.
          ///
          /// \see PAMI::Device::Interface::RecvFunction_t
          ///
          static int dispatch_envelope_direct (void   * metadata,
                                               void   * payload,
                                               size_t   bytes,
                                               void   * recv_func_parm,
                                               void   * cookie)
          {
            protocol_metadata_t * m;
            void * p;

            // This branch should be resolved at compile time and optimized out.
            if (sizeof(protocol_metadata_t) > T_Model::packet_model_metadata_bytes)
              {
                m = (protocol_metadata_t *) payload;
                p = (void *) (m + 1);
              }
            else
              {
                m = (protocol_metadata_t *) metadata;
                p = payload;
              }

            TRACE_ERR ((stderr, ">> EagerSimple::dispatch_envelope_direct(), origin = 0x%08x, m->bytes = %zu, m->va_send = %p\n", m->origin, m->bytes, m->va_send));

            EagerSimpleProtocol * eager = (EagerSimpleProtocol *) recv_func_parm;

            // Allocate a recv state object!
            recv_state_t * state = eager->allocateRecvState ();
            state->eager              = eager;
            state->received           = 0;
            state->metadata.va_send   = m->va_send;
            state->metadata.bytes     = m->bytes;
            state->metadata.metabytes = m->metabytes;
            state->metadata.origin    = m->origin;

            // Set the eager connection.
            eager->_connection.set (m->origin, (void *)state);

            // The compiler will optimize out this constant expression
            // conditional and include this code block only when the long
            // header support is enabled.
            if (T_LongHeader == true)
              {
                // Check for long header
                size_t header_bytes = m->metabytes;

                size_t pbytes = (sizeof(protocol_metadata_t) > T_Model::packet_model_metadata_bytes) ? sizeof(protocol_metadata_t) : 0;

                TRACE_ERR ((stderr, "   EagerSimple::dispatch_envelope_direct() .. header_bytes = %zu, T_Model::packet_model_payload_bytes = %zu, pbytes = %zu\n", header_bytes, T_Model::packet_model_payload_bytes, pbytes));

                if (unlikely((header_bytes) > (T_Model::packet_model_payload_bytes - pbytes)))
                  {
                    state->longheader.addr   = (uint8_t *) malloc(header_bytes);
                    state->longheader.bytes  = header_bytes;
                    state->longheader.offset = 0;
                    TRACE_ERR ((stderr, "<< EagerSimple::dispatch_envelope_direct() .. long header\n"));
                    return 0;
                  }
              }

            // This is a short header envelope .. all application metadata
            // has been received.
            eager->process_envelope (m, (uint8_t *)p, state);

            TRACE_ERR ((stderr, "<< EagerSimple::dispatch_envelope_direct()\n"));
            return 0;
          };

          ///
          /// \brief Read-access multi-packet send envelope packet callback.
          ///
          /// Reads incoming bytes from the device to a temporary buffer and
          /// invokes the registered receive callback.
          ///
          /// The eager packet factory will register this dispatch function
          /// if and only if the device \b does \b not provide access to
          /// data which has already been read from the network by the device.
          ///
          /// The dispatch function must read the data onto the stack and
          /// then provide 16-byte (CMQuad) aligned data pointer(s) to the
          /// registered user callback.
          ///
          /// The envelope dispatch function is invoked by the packet device
          /// to process the first packet of a multi-packet message. The CDI
          /// eager protocol sends two iovec elements in the first packet for
          /// the number of data bytes to follow and the msginfo metadata.
          ///
          /// \note The iov_base fields in the iovec structure will be \c NULL
          ///
          /// \see DCMF::CDI::RecvFunction_t
          /// \see dispatch_data_read
          ///
          static int dispatch_envelope_read (void   * metadata,
                                             void   * payload,
                                             size_t   bytes,
                                             void   * recv_func_parm,
                                             void   * cookie)
          {
            TRACE_ERR((stderr, ">> EagerSimple::dispatch_envelope_read()\n"));

            EagerSimpleProtocol * pf = (EagerSimpleProtocol *) recv_func_parm;

            // This packet device DOES NOT provide the data buffer(s) for the
            // message and the data must be read on to the stack before the
            // recv callback is invoked.
            PAMI_assert_debugf(payload == NULL, "The 'read only' packet device did not invoke dispatch with payload == NULL (%p)\n", payload);

            uint8_t stackData[T_Model::packet_model_payload_bytes];
            void * p = (void *) & stackData[0];
            pf->_device.read (p, bytes, cookie);

            dispatch_envelope_direct (metadata, p, bytes, recv_func_parm, cookie);

            TRACE_ERR((stderr, "<< EagerSimple::dispatch_envelope_read()\n"));
            return 0;
          };

          static int dispatch_longheader_message (void   * metadata,
                                                  void   * payload,
                                                  size_t   bytes,
                                                  void   * recv_func_parm,
                                                  void   * cookie)
          {
            // This assertion of a constant expression should be optimized out
            // by the compiler when long header support is enabled.
            PAMI_assertf(T_LongHeader == true, "'long header' support is not enabled.");

            EagerSimpleProtocol * eager = (EagerSimpleProtocol *) recv_func_parm;

            pami_endpoint_t origin = *((pami_endpoint_t *) metadata);

            TRACE_ERR((stderr, ">> EagerSimple::dispatch_longheader_message(), origin task = %d, origin offset = %zu, bytes = %zu\n", task, offset, bytes));

            recv_state_t * state = (recv_state_t *) eager->_connection.get (origin);

            size_t n = MIN(bytes, state->longheader.bytes - state->longheader.offset);

            eager->_device.read ((void *)(state->longheader.addr + state->longheader.offset), n, cookie);
            state->longheader.offset += n;

            if (state->longheader.bytes == state->longheader.offset)
              {
                // The entire application metadata has been received. Invoke the
                // registered pami point-to-point dispatch function.

                eager->process_envelope (&state->metadata, state->longheader.addr, state);

                // Free the malloc'd longheader buffer now that it has been
                // delivered to the application.
                free (state->longheader.addr);
              }

            TRACE_ERR((stderr, "<< EagerSimple::dispatch_longheader_message()\n"));
            return 0;
          };

          ///
          /// \brief Direct multi-packet send data packet callback.
          ///
          /// Copies incoming packet data from the device to the destination buffer.
          ///
          /// The eager simple send protocol will register this dispatch function
          /// if and only if the device \b does provide direct access to
          /// data which has already been read from the network by the device.
          ///
          /// The data dispatch function is invoked by the packet device
          /// to process the subsequent packets of a multi-packet message
          /// after the first envelope packet.
          ///
          /// \see PAMI::Device::Interface::RecvFunction_t
          ///
          static int dispatch_data_message   (void   * metadata,
                                              void   * payload,
                                              size_t   bytes,
                                              void   * recv_func_parm,
                                              void   * cookie)
          {
            EagerSimpleProtocol * eager = (EagerSimpleProtocol *) recv_func_parm;

            pami_endpoint_t origin = *((pami_endpoint_t *) metadata);
            pami_task_t task;
            size_t offset;
            PAMI_ENDPOINT_INFO(origin,task,offset);

            TRACE_ERR((stderr, ">> EagerSimple::dispatch_data_message(), origin task = %d, origin offset = %zu, bytes = %zu\n", task, offset, bytes));

            recv_state_t * state = (recv_state_t *) eager->_connection.get (origin);

            // Number of bytes received so far.
            size_t nbyte = state->received;

            // Number of bytes left to copy into the destination buffer
            size_t nleft = state->metadata.bytes - nbyte;

            TRACE_ERR((stderr, "   EagerSimple::dispatch_data_message(), bytes received so far = %zu, bytes yet to receive = %zu, total bytes to receive = %zu, total bytes being sent = %zu\n", state->received, nleft, state->longheader.bytes, state->metadata.bytes));

            // Copy data from the packet payload into the destination buffer
            size_t ncopy = MIN(nleft,bytes);
            memcpy ((uint8_t *)(state->info.addr) + nbyte, payload, ncopy);
            state->received += ncopy;

            TRACE_ERR((stderr, "   EagerSimple::dispatch_data_message(), nbyte = %zu\n", nbyte));

            if (nbyte + bytes >= state->metadata.bytes)
              {
                // No more data packets will be received on this connection.
                // Clear the connection data and prepare for the next message.
                eager->_connection.clear (origin);

                // No more data is to be written to the receive buffer.
                // Invoke the receive done callback.
                if (state->info.local_fn)
                  state->info.local_fn (eager->_context,
                                        state->info.cookie,
                                        PAMI_SUCCESS);

                if (state->metadata.va_send != NULL)
                  {
                    eager->_ack_model.postPacket (state->pkt,
                                                  receive_complete,
                                                  (void *) state,
                                                  task,
                                                  offset,
                                                  (void *) NULL,
                                                  0,
                                                  (void *) &(state->metadata.va_send),
                                                  sizeof (send_state_t *));
                  }
                else
                  {
                    // Otherwise, return the receive state object memory to
                    // the memory pool.
                    eager->freeRecvState (state);
                  }

                TRACE_ERR((stderr, "<< dispatch_data_message(), origin task = %d ... receive completed\n", task));
                return 0;
              }

            TRACE_ERR((stderr, "<< dispatch_data_message(), origin task = %d ... wait for more data\n", task));
            return 0;
          };

          ///
          /// \brief Local send completion event callback.
          ///
          /// This callback will invoke the application local completion
          /// callback function and, if notification of remote receive
          /// completion is not required, free the send state memory.
          ///
          static void send_complete (pami_context_t   context,
                                     void           * cookie,
                                     pami_result_t    result)
          {
            TRACE_ERR((stderr, "EagerSimple::send_complete() >> \n"));
            send_state_t * state = (send_state_t *) cookie;

            EagerSimpleProtocol * eager = (EagerSimpleProtocol *) state->eager;

            if (state->local_fn != NULL)
              {
                state->local_fn (eager->_context, state->cookie, PAMI_SUCCESS);
              }

            if (state->remote_fn == NULL)
              {
                eager->freeSendState (state);
              }

            TRACE_ERR((stderr, "EagerSimple::send_complete() << \n"));
            return;
          }

          ///
          /// \brief Local receive completion event callback.
          ///
          /// This callback will invoke the application local receive
          /// completion callback and free the receive state object
          /// memory.
          ///
          static void receive_complete (pami_context_t   context,
                                        void           * cookie,
                                        pami_result_t    result)
          {
            TRACE_ERR((stderr, "EagerSimple::receive_complete() >> \n"));
            recv_state_t * state = (recv_state_t *) cookie;
            EagerSimpleProtocol * eager = (EagerSimpleProtocol *) state->eager;

            eager->_connection.clear (state->metadata.origin);
            eager->freeRecvState (state);

            TRACE_ERR((stderr, "EagerSimple::receive_complete() << \n"));
            return;
          };
      };
    };
  };
};
#undef TRACE_ERR
#endif // __pami_p2p_protocol_send_eager_eagersimple_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
