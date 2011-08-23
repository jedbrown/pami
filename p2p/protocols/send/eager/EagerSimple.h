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

#include "components/devices/PacketInterface.h"
#include "components/memory/MemoryAllocator.h"
#include "math/Memcpy.x.h"
#include "p2p/protocols/Send.h"

#include "common/type/TypeMachine.h"

#ifndef RETURN_ERR_PAMI
#define RETURN_ERR_PAMI(code, ...) return (code)
#endif

#undef TRACE_ERR
#define TRACE_ERR(x) // fprintf x

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
      /// \tparam T_Option     Send options
      ///
      /// \see PAMI::Device::Interface::PacketModel
      ///
      template < class T_Model, configuration_t T_Option >
      class EagerSimple
      {
        protected:

          typedef EagerSimple<T_Model, T_Option> EagerSimpleProtocol;

          typedef uint8_t model_state_t[T_Model::packet_model_state_bytes];
          typedef uint8_t model_packet_t[T_Model::packet_model_state_bytes];

          typedef struct
          {
            pami_event_function   remote_fn;
            void                * cookie;
            pami_context_t        context;
            bool                  invoke;
          } ack_protocol_t;

          ///
          /// \note The 'short' protocol metadata limits the maximum size of
          ///       the application header + application data to \c USHRT_MAX
          ///       bytes. The effective maximum application header + application
          ///       data size for the short protocol is the minimum of \c USHRT_MAX and
          ///       \c T_Model::packet_model_payload_bytes less the size of the
          ///       \c short_protocol_t metadata structure \em if the size of the
          ///       \c short_protocol_t metadata structure is greater than
          ///       \c T_Model::packet_model_metadata_bytes
          ///
          typedef struct
          {
            uint16_t           bytes;     ///< Number of bytes of application data being sent
            uint16_t           metabytes; ///< Number of bytes of application metadata being sent
            pami_endpoint_t    origin;    ///< Endpoint that originated the transfer
          } short_protocol_t;

          typedef struct
          {
            size_t             bytes;     ///< Number of bytes of application data being sent
            size_t             metabytes; ///< Number of bytes of application metadata being sent
            pami_endpoint_t    origin;    ///< Endpoint that originated the transfer
          } longheader_protocol_t;

          ///
          /// \note The 'eager' protocol metadata structure limits the maximum
          ///       size of the application header to \c USHRT_MAX bytes. The
          ///       effective maximum application header size for the eager
          ///       protocol is the minimum of \c USHRT_MAX and
          ///       \c T_Model::packet_model_payload_bytes less the size of the
          ///       \c eager_protocol_t metadata structure \em if the size of the
          ///       \c eager_protocol_t metadata structure is greater than
          ///       \c T_Model::packet_model_metadata_bytes
          ///
          typedef struct
          {
            size_t             bytes;     ///< Number of bytes of application data being sent
            uint16_t           metabytes; ///< Number of bytes of application metadata being sent
            pami_endpoint_t    origin;    ///< Endpoint that originated the transfer
          } eager_protocol_t;


          typedef struct send_state
          {
            model_state_t           msg[3];
            model_packet_t          pkt;
            union
            {
              ack_protocol_t        ack_protocol;
              short_protocol_t      short_protocol;
              longheader_protocol_t longheader_protocol;
              eager_protocol_t      eager_protocol;
            };
            pami_event_function     local_fn;  ///< Application send injection completion callback
            pami_event_function     remote_fn; ///< Application remote receive acknowledgement callback
            void                  * cookie;    ///< Application callback cookie
            union
            {
              struct iovec          v[2];      ///< Iovec array used for transfers
              struct iovec          v3[3];     ///< Iovec array used for transfers
            };
            EagerSimpleProtocol   * eager;     ///< Eager protocol object
          } send_state_t;

          typedef struct recv_state
          {
            model_state_t           msg;      ///< packet send state memory
            pami_recv_t             info;     ///< Application receive information.
            pami_endpoint_t         origin;
            size_t                  data_size;
            size_t                  header_size;

            size_t                  received; ///< Number of bytes received.
            struct
            {
              uint8_t             * addr;     ///< Address of the long header recv buffer
              size_t                bytes;
              size_t                offset;
            } longheader;
            EagerSimpleProtocol   * eager;   ///< Eager protocol object
            Type::TypeMachine       machine;
            bool                    is_contiguous_copy_recv;
          } recv_state_t;

          static const size_t maximum_short_packet_payload =
            T_Model::packet_model_payload_bytes -
            (sizeof(short_protocol_t) > T_Model::packet_model_metadata_bytes) * sizeof(short_protocol_t);

          static const size_t maximum_eager_packet_payload =
            T_Model::packet_model_payload_bytes -
            (sizeof(eager_protocol_t) > T_Model::packet_model_metadata_bytes) * sizeof(eager_protocol_t);

        public:

          static const size_t recv_immediate_max = MIN(maximum_short_packet_payload, maximum_eager_packet_payload);

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
          template <class T_Device>
          inline EagerSimple (T_Device & device) :
              _envelope_model (device),
              _longheader_envelope_model (device),
              _longheader_message_model (device),
              _data_model (device),
              _ack_model (device),
              _short_model (device)
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
          }


          pami_result_t initialize (size_t                       dispatch,
                                    pami_dispatch_p2p_function   dispatch_fn,
                                    void                       * cookie,
                                    pami_endpoint_t              origin,
                                    pami_context_t               context,
                                    pami_dispatch_hint_t         hint)
          {
            _dispatch_fn = dispatch_fn;
            _cookie      = cookie;
            _origin      = origin;
            _context     = context;

            pami_result_t status = PAMI_ERROR;

            // The models must be registered in reverse order of use in case
            // the remote side is delayed in it's registrations and must save
            // unexpected packets until dispatch registration.
            TRACE_ERR((stderr, "EagerSimple() register ack model dispatch %zu\n", dispatch));

            status = _ack_model.init (dispatch, dispatch_ack, this);
            TRACE_ERR((stderr, "EagerSimple() ack model status = %d\n", status));

            if (status == PAMI_SUCCESS)
              {
                TRACE_ERR((stderr, "EagerSimple() register short model dispatch %zu\n", dispatch));
                status = _short_model.init (dispatch, dispatch_short, this);
                TRACE_ERR((stderr, "EagerSimple() short model status = %d\n", status));

                if (status == PAMI_SUCCESS)
                  {
                    TRACE_ERR((stderr, "EagerSimple() register data model dispatch %zu\n", dispatch));

                    if (hint.recv_contiguous == PAMI_HINT_ENABLE &&
                        hint.recv_copy       == PAMI_HINT_ENABLE)
                      {
                        // Only allow contiguous copy receives
                        status = _data_model.init (dispatch, dispatch_data_message<true>, this);
                      }
                    else
                      {
                        // Allow all typed receives
                        status = _data_model.init (dispatch, dispatch_data_message<false>, this);
                      }

                    TRACE_ERR((stderr, "EagerSimple() data model status = %d\n", status));

                    if (status == PAMI_SUCCESS)
                      {
                        TRACE_ERR((stderr, "EagerSimple() register envelope model dispatch %zu\n", dispatch));

                        if (hint.recv_contiguous == PAMI_HINT_ENABLE)
                          {
                            if (hint.recv_copy == PAMI_HINT_ENABLE)
                              {
                                status = _envelope_model.init (dispatch, dispatch_eager_envelope<true, true>, this);
                              }
                            else
                              {
                                status = _envelope_model.init (dispatch, dispatch_eager_envelope<true, false>, this);
                              }
                          }
                        else
                          {
                            if (hint.recv_copy == PAMI_HINT_ENABLE)
                              {
                                status = _envelope_model.init (dispatch, dispatch_eager_envelope<false, true>, this);
                              }
                            else
                              {
                                status = _envelope_model.init (dispatch, dispatch_eager_envelope<false, false>, this);
                              }
                          }

                        TRACE_ERR((stderr, "EagerSimple() envelope model status = %d\n", status));

                        if (status == PAMI_SUCCESS && !(T_Option & LONG_HEADER_DISABLE))
                          {
                            TRACE_ERR((stderr, "EagerSimple() register 'long header' envelope model dispatch %zu\n", dispatch));
                            status = _longheader_envelope_model.init (dispatch, dispatch_longheader_envelope, this);
                            TRACE_ERR((stderr, "EagerSimple() 'long header' envelope model status = %d\n", status));

                            if (status == PAMI_SUCCESS)
                              {

                                TRACE_ERR((stderr, "EagerSimple() register 'long header' message model dispatch %zu\n", dispatch));

                                if (hint.recv_contiguous == PAMI_HINT_ENABLE)
                                  {
                                    if (hint.recv_copy == PAMI_HINT_ENABLE)
                                      {
                                        status = _longheader_message_model.init (dispatch, dispatch_longheader_message<true, true>, this);
                                      }
                                    else
                                      {
                                        status = _longheader_message_model.init (dispatch, dispatch_longheader_message<true, false>, this);
                                      }
                                  }
                                else
                                  {
                                    if (hint.recv_copy == PAMI_HINT_ENABLE)
                                      {
                                        status = _longheader_message_model.init (dispatch, dispatch_longheader_message<false, true>, this);
                                      }
                                    else
                                      {
                                        status = _longheader_message_model.init (dispatch, dispatch_longheader_message<false, false>, this);
                                      }
                                  }

                                TRACE_ERR((stderr, "EagerSimple() 'long header' message model status = %d\n", status));
                              }
                          }
                      }
                  }
              }

            return status;
          }

          inline pami_result_t short_send (pami_send_immediate_t * send, pami_send_event_t * events, pami_task_t task, size_t offset)
          {
            TRACE_ERR((stderr, ">> EagerSimple::short_send() .. sizeof(short_protocol_t) = %zu\n", sizeof(short_protocol_t)));

            // Allocate memory to maintain the state of the send.
            send_state_t * state = allocateSendState ();

            state->cookie   = events->cookie;
            state->local_fn = events->local_fn;
            state->eager    = this;

            // Specify the protocol metadata to send with the application
            // metadata in the envelope packet.
            state->short_protocol.bytes        = send->data.iov_len;
            state->short_protocol.metabytes    = send->header.iov_len;
            state->short_protocol.origin       = _origin;

            TRACE_ERR((stderr, "   EagerSimple::short_send() .. 'short' protocol special case\n"));

            // This branch should be resolved at compile time and optimized out.
            if (sizeof(short_protocol_t) <= T_Model::packet_model_metadata_bytes)
              {
                TRACE_ERR((stderr, "   EagerSimple::short_send() .. 'short' protocol special case, protocol metadata fits in the packet metadata\n"));

                array_t<iovec, 2> * resized = (array_t<iovec, 2> *) & (send->header);

                _short_model.postPacket (state->msg[0],
                                         send_complete, (void *) state,
                                         task, offset,
                                         (void *) &(state->short_protocol),
                                         sizeof (short_protocol_t),
                                         resized->array);
              }
            else
              {
                TRACE_ERR((stderr, "   EagerSimple::short_send() .. 'short' protocol special case, protocol metadata does not fit in the packet metadata\n"));

                state->v3[0].iov_base = (void *) & (state->short_protocol);
                state->v3[0].iov_len  = sizeof (short_protocol_t);
                state->v3[1].iov_base = send->header.iov_base;
                state->v3[1].iov_len  = send->header.iov_len;
                state->v3[2].iov_base = send->data.iov_base;
                state->v3[2].iov_len  = send->data.iov_len;

                _short_model.postPacket (state->msg[0],
                                         send_complete, (void *) state,
                                         task, offset,
                                         NULL, 0,
                                         &state->v3, 3);
              }

            if (unlikely(events->remote_fn != NULL))
              {
                send_ack_request (task, offset, events->remote_fn, events->cookie);
              }

            TRACE_ERR((stderr, "<< EagerSimple::short_send() .. 'short' protocol special case\n"));
            return PAMI_SUCCESS;
          }


          inline pami_result_t send_header (send_state_t        * state,
                                            pami_task_t           task,
                                            size_t                offset,
                                            struct iovec        & header,
                                            pami_event_function   done_fn)
          {



            TRACE_ERR((stderr, ">> EagerSimple::longheader_send() .. sizeof(longheader_protocol_t) = %zu\n", sizeof(longheader_protocol_t)));

#ifdef ERROR_CHECKS

            if ((T_Option & LONG_HEADER_DISABLE) && (header_bytes > maximum_eager_packet_payload))
              {
                // 'long header' support is disabled, yet the application
                // header will not fit in a single packet.
                TRACE_ERR((stderr, "   EagerSimple::simple_impl() .. Application error: 'long header' support is disabled.\n"));
                return PAMI_INVAL;
              }

#endif

            // ----------------------------------------------------------------
            // Check for a "long header" send protocol
            // ----------------------------------------------------------------
            if (unlikely(!(T_Option & LONG_HEADER_DISABLE) &&
                         (header.iov_len > maximum_eager_packet_payload)))
              {
                // Specify the protocol metadata to send with the application
                // metadata in the envelope packet.
                state->longheader_protocol.bytes        = state->eager_protocol.bytes;
                state->longheader_protocol.metabytes    = header.iov_len;
                state->longheader_protocol.origin       = _origin;


                TRACE_ERR((stderr, "   EagerSimple::longheader_send() .. long header special case, protocol metadata does not fit in the packet metadata\n"));
                _longheader_envelope_model.postPacket (state->msg[0],
                                                       NULL, NULL,
                                                       task, offset,
                                                       (void *) NULL, 0,
                                                       (void *) &(state->longheader_protocol),
                                                       sizeof (longheader_protocol_t));

                _longheader_message_model.postMultiPacket (state->msg[1],
                                                           done_fn,
                                                           (void *) state,
                                                           task,
                                                           offset,
                                                           (void *) &(state->longheader_protocol.origin),
                                                           sizeof (pami_endpoint_t),
                                                           header.iov_base,
                                                           header.iov_len);
              }
            else
              {
                //
                // Send the protocol metadata and the application metadata in the envelope packet
                //
                // This branch should be resolved at compile time and optimized out.
                if (sizeof(eager_protocol_t) <= T_Model::packet_model_metadata_bytes)
                  {
                    TRACE_ERR((stderr, "   EagerSimple::simple_impl() .. protocol metadata fits in the packet metadata\n"));
                    _envelope_model.postPacket (state->msg[0],
                                                done_fn,
                                                (void *) state,
                                                task, offset,
                                                (void *) &(state->eager_protocol),
                                                sizeof (eager_protocol_t),
                                                header.iov_base,
                                                header.iov_len);
                  }
                else
                  {
                    TRACE_ERR((stderr, "   EagerSimple::simple_impl() .. protocol metadata does not fit in the packet metadata\n"));

                    state->v[0].iov_base = (void *) & (state->eager_protocol);
                    state->v[0].iov_len  = sizeof (eager_protocol_t);
                    state->v[1].iov_base = header.iov_base;
                    state->v[1].iov_len  = header.iov_len;

                    _envelope_model.postPacket (state->msg[0],
                                                done_fn,
                                                (void *) state,
                                                task, offset,
                                                NULL, 0,
                                                state->v);
                  }
              }

            return PAMI_SUCCESS;
          }

          template <bool T_Contiguous>
          inline pami_result_t send_data (send_state_t        * state,
                                          pami_task_t           task,
                                          size_t                offset,
                                          pami_send_typed_t   * parameters)
          {
            if (T_Contiguous == true)
              {
                _data_model.postMultiPacket (state->msg[2],
                                             send_complete,
                                             (void *) state,
                                             task,
                                             offset,
                                             (void *) &(state->eager_protocol.origin),
                                             sizeof (pami_endpoint_t),
                                             parameters->send.data.iov_base,
                                             parameters->send.data.iov_len);

              }
            else
              {
                PAMI_abort();
              }

            return PAMI_SUCCESS;
          }

          inline pami_result_t immediate_impl (pami_send_immediate_t * parameters)
          {
            TRACE_ERR((stderr, "EagerSimple::immediate_impl() >>\n"));

#ifdef ERROR_CHECKS

            if (T_Model::packet_model_immediate_bytes <
                (parameters->data.iov_len + parameters->header.iov_len))
              {
                TRACE_ERR((stderr, "Oops! ... %zu < (%zu+%zu) ???\n", T_Model::packet_model_immediate_bytes, parameters->data.iov_len, parameters->header.iov_len));
                return PAMI_INVAL;
              }

#endif

            pami_task_t task;
            size_t offset;
            PAMI_ENDPOINT_INFO(parameters->dest, task, offset);

            // Verify that this task is addressable by this packet device
            if (unlikely(_short_model.device.isPeer(task) == false))
              {
                return PAMI_INVAL;
              }

            // Specify the protocol metadata to send with the application
            // metadata in the packet. This metadata is copied
            // into the network by the device and, therefore, can be placed
            // on the stack.
            short_protocol_t short_protocol;
            short_protocol.bytes     = parameters->data.iov_len;
            short_protocol.metabytes = parameters->header.iov_len;
            short_protocol.origin    = _origin;

            TRACE_ERR((stderr, "EagerSimple::immediate_impl() .. before _send_model.postPacket() .. parameters->header.iov_len = %zu, parameters->data.iov_len = %zu dest:%x\n", parameters->header.iov_len, parameters->data.iov_len, parameters->dest));

            TRACE_ERR((stderr, "EagerSimple::immediate_impl() .. before _send_model.postPacket() .. task = %d, offset = %zu\n", task, offset));

            bool posted = false;

            // This branch should be resolved at compile time and optimized out.
            if (sizeof(short_protocol_t) <= T_Model::packet_model_metadata_bytes)
              {
                // This allows the header+data iovec elements to be treated as a
                // two-element array of iovec structures, and therefore allows the
                // packet model to implement template specialization.
                array_t<struct iovec, 2> * iov = (array_t<struct iovec, 2> *) parameters;

                posted = _short_model.postPacket (task, offset,
                                                  (void *) & short_protocol,
                                                  sizeof (short_protocol_t),
                                                  iov->array);
              }
            else
              {
                iovec iov[3];
                iov[0].iov_base = (void *) & short_protocol;
                iov[0].iov_len  = sizeof (short_protocol_t);
                iov[1].iov_base = parameters->header.iov_base;
                iov[1].iov_len  = parameters->header.iov_len;
                iov[2].iov_base = parameters->data.iov_base;
                iov[2].iov_len  = parameters->data.iov_len;

                posted = _short_model.postPacket (task, offset,
                                                  NULL, 0,
                                                  iov);
              }

            if ((T_Option & QUEUE_IMMEDIATE_DISABLE) && !posted)
              {
                TRACE_ERR((stderr, "EagerSimple::immediate_impl() <<\n"));
                return PAMI_EAGAIN;
              }

            if (unlikely(!posted))
              {
                // For some reason the packet could not be immediately posted.
                // Allocate memory, pack the user data and metadata, and attempt
                // a regular (non-blocking) post.
                TRACE_ERR((stderr, "EagerSimple::immediate_impl() .. immediate post packet unsuccessful.\n"));

                // Allocate memory to maintain the state of the send.
                send_state_t * state = allocateSendState ();

                state->cookie   = NULL;
                state->local_fn = NULL;
                state->eager    = this;

                // Specify the protocol metadata to send with the application
                // metadata in the envelope packet.
                state->short_protocol = short_protocol;

                // This branch should be resolved at compile time and optimized out.
                if (sizeof(short_protocol_t) <= T_Model::packet_model_metadata_bytes)
                  {
                    TRACE_ERR((stderr, "   EagerSimple::immediate_impl() .. 'short' protocol special case, protocol metadata fits in the packet metadata\n"));

                    uint8_t * ptr = (uint8_t *) state->pkt;
                    memcpy (ptr, parameters->header.iov_base, parameters->header.iov_len);
                    ptr += parameters->header.iov_len;
                    memcpy (ptr, parameters->data.iov_base, parameters->data.iov_len);

                    struct iovec iov[1];
                    iov[0].iov_base = (void *) state->pkt;
                    iov[0].iov_len  = parameters->header.iov_len + parameters->data.iov_len;

                    _short_model.postPacket (state->msg[0],
                                             send_complete, (void *) state,
                                             task, offset,
                                             (void *) &(state->short_protocol),
                                             sizeof (short_protocol_t),
                                             iov);
                  }
                else
                  {
                    TRACE_ERR((stderr, "   EagerSimple::immediate_impl() .. 'short' protocol special case, protocol metadata does not fit in the packet metadata\n"));

                    uint8_t * ptr = (uint8_t *) state->pkt;
                    memcpy (ptr, (void *) & (state->short_protocol), sizeof (short_protocol_t));
                    ptr += sizeof (short_protocol_t);
                    memcpy (ptr, parameters->header.iov_base, parameters->header.iov_len);
                    ptr += parameters->header.iov_len;
                    memcpy (ptr, parameters->data.iov_base, parameters->data.iov_len);

                    struct iovec iov[1];
                    iov[0].iov_base = (void *) state->pkt;
                    iov[0].iov_len  = parameters->header.iov_len + parameters->data.iov_len;

                    _short_model.postPacket (state->msg[0],
                                             send_complete, (void *) state,
                                             task, offset,
                                             NULL, 0,
                                             iov);
                  }
              }
            else
              {
                TRACE_ERR((stderr, "EagerSimple::immediate_impl() .. immediate post packet successful.\n"));
              }

            TRACE_ERR((stderr, "EagerSimple::immediate_impl() <<\n"));
            return PAMI_SUCCESS;
          };


          template <bool T_Contiguous>
          inline pami_result_t simple_and_typed_implementation (pami_send_typed_t * parameters)
          {
            TRACE_ERR((stderr, ">> EagerSimple::simple_impl() .. T_Model::packet_model_metadata_bytes = %zu\n", T_Model::packet_model_metadata_bytes));

            pami_task_t task;
            size_t offset;
            PAMI_ENDPOINT_INFO(parameters->send.dest, task, offset);

            // Verify that this task is addressable by this packet device
            if (unlikely(_short_model.device.isPeer (task) == false))
              {
                return PAMI_INVAL;
              }

            const size_t header_bytes = parameters->send.header.iov_len;

            const size_t data_bytes   = parameters->send.data.iov_len;

            const size_t total_bytes  = header_bytes + data_bytes;

            // ----------------------------------------------------------------
            // Check for a "short" send protocol
            // ----------------------------------------------------------------
#ifdef ERROR_CHECKS

            if ((T_Option & RECV_IMMEDIATE_FORCEON) && (total_bytes > maximum_short_packet_payload))
              {
                // 'receive immediate' is forced ON, yet the application
                // header + data will not fit in a single packet.
                TRACE_ERR((stderr, "   EagerSimple::simple_impl() .. Application error: 'recv immediate' forced on.\n"));
                return PAMI_INVAL;
              }

#endif

            //
            //   'immediate receives' are forced on ('long header' is irrelevant),
            //      OR
            //   'immediate receives' are not forced off AND the header + data fit in a single packet
            //
//fprintf (stderr, "(T_Option & RECV_IMMEDIATE_FORCEON) = %d .. (T_Option & RECV_IMMEDIATE_FORCEOFF) = %d .. !(T_Option & RECV_IMMEDIATE_FORCEOFF) = %d\n", (T_Option & RECV_IMMEDIATE_FORCEON), (T_Option & RECV_IMMEDIATE_FORCEOFF), !(T_Option & RECV_IMMEDIATE_FORCEOFF));
//fprintf (stderr, "!(T_Option & RECV_IMMEDIATE_FORCEOFF) && (%zu <= %zu) == %d\n", total_bytes, maximum_short_packet_payload, !(T_Option & RECV_IMMEDIATE_FORCEOFF) && total_bytes);
            if ((T_Contiguous == true) &&    // 'typed' short send not supported
                ((T_Option & RECV_IMMEDIATE_FORCEON) ||
                 (!(T_Option & RECV_IMMEDIATE_FORCEOFF) &&
                  (total_bytes <= maximum_short_packet_payload))))
              {
                return short_send (&parameters->send, &parameters->events, task, offset);
              }

#ifdef ERROR_CHECKS

            if ((T_Option & LONG_HEADER_DISABLE) && (header_bytes > maximum_eager_packet_payload))
              {
                // 'long header' support is disabled, yet the application
                // header will not fit in a single packet.
                TRACE_ERR((stderr, "   EagerSimple::simple_impl() .. Application error: 'long header' support is disabled.\n"));
                return PAMI_INVAL;
              }

#endif

            // ----------------------------------------------------------------
            // Send a single-packet envelope eager message
            // ----------------------------------------------------------------

            // Allocate memory to maintain the state of the send.
            send_state_t * state = allocateSendState ();

            state->cookie   = parameters->events.cookie;
            state->local_fn = parameters->events.local_fn;
            state->eager    = this;



            // Specify the protocol metadata to send with the application
            // metadata in the envelope packet.
            state->eager_protocol.bytes        = data_bytes;
            state->eager_protocol.metabytes    = header_bytes;
            state->eager_protocol.origin       = _origin;

            TRACE_ERR((stderr, "   EagerSimple::simple_impl() .. parameters->send.header.iov_len = %zu, parameters->send.data.iov_len = %zu\n", header_bytes, data_bytes));


            if (unlikely(data_bytes == 0))
              {
                send_header (state, task, offset, parameters->send.header, send_complete);
              }
            else
              {
                send_header (state, task, offset, parameters->send.header, NULL);
                send_data<T_Contiguous> (state, task, offset, parameters);
              }

            if (unlikely(parameters->events.remote_fn != NULL))
              {
                send_ack_request (task, offset, parameters->events.remote_fn, parameters->events.cookie);
              }

            TRACE_ERR((stderr, "<< EagerSimple::simple_impl()\n"));
            return PAMI_SUCCESS;
          };

          inline pami_result_t simple_impl (pami_send_t * parameters)
          {
            // ok to cast since pami_send_t is a subset of pami_send_typed_t.
            return simple_and_typed_implementation<true> ((pami_send_typed_t *) parameters);
          };

          inline pami_result_t typed_impl (pami_send_typed_t * parameters)
          {
            return simple_and_typed_implementation<false> (parameters);
          };

        protected:

          MemoryAllocator < sizeof(send_state_t), 16 > _send_allocator;
          MemoryAllocator < sizeof(recv_state_t), 16 > _recv_allocator;

          T_Model          _envelope_model;
          T_Model          _longheader_envelope_model;
          T_Model          _longheader_message_model;
          T_Model          _data_model;
          T_Model          _ack_model;
          T_Model          _short_model;
          pami_endpoint_t  _origin;
          pami_context_t   _context;

          pami_dispatch_p2p_function _dispatch_fn;
          void                     * _cookie;

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

          template <bool T_Contiguous, bool T_Copy>
          inline void process_envelope (uint8_t            * header,
                                        size_t               header_bytes,
                                        size_t               data_bytes,
                                        pami_endpoint_t      origin,
                                        recv_state_t       * state)
          {
            TRACE_ERR((stderr, ">> EagerSimple::process_envelope() .. origin = 0x%08x, header = %p, header bytes = %zu\n", origin, header, header_bytes));

            state->info.type = (pami_type_t) PAMI_TYPE_BYTE;

            // Invoke the registered dispatch function.
            _dispatch_fn (_context,            // Communication context
                          _cookie,             // Dispatch cookie
                          header,              // Application metadata
                          header_bytes,        // Application metadata bytes
                          NULL,                // No payload data
                          data_bytes,          // Number of msg bytes
                          origin,              // Origin endpoint for the transfer
                          (pami_recv_t *) &(state->info));

            TRACE_ERR((stderr, "   EagerSimple::process_envelope() .. data bytes = %zu, state->info.type = %p\n", data_bytes, state->info.type));

            if (T_Contiguous && T_Copy)
              {
                TRACE_ERR((stderr, "<< EagerSimple::process_envelope() .. 'contiguous copy' receive only.\n"));
                return;
              }
            else if (T_Contiguous)
              {
                state->info.type   = (pami_type_t) PAMI_TYPE_BYTE;
                state->info.offset = 0;
              }
            else if (T_Copy)
              {
                state->info.data_fn     = PAMI_DATA_COPY;
                state->info.data_cookie = NULL;
              }

            Type::TypeCode * type = (Type::TypeCode *) state->info.type;
            PAMI_assert_debugf(type != NULL, "state->info.type == NULL !\n");

#ifdef ERROR_CHECKS

            if (! type->IsCompleted())
              {
                //RETURN_ERR_PAMI(PAMI_INVAL, "Using an incompleted type.\n");
                if (state->info.local_fn != NULL)
                  state->info.local_fn (_context, state->info.cookie, PAMI_INVAL);

                return;
              }

#endif
            state->is_contiguous_copy_recv =
              (state->info.type == PAMI_TYPE_BYTE) &&
              (state->info.data_fn == PAMI_DATA_COPY);

            if (unlikely(!state->is_contiguous_copy_recv))
              {
                // Construct a type machine for this transfer.
                new (&(state->machine)) Type::TypeMachine (type);
                state->machine.SetCopyFunc (state->info.data_fn, state->info.data_cookie);
                state->machine.MoveCursor (state->info.offset);
              }

            TRACE_ERR((stderr, "<< EagerSimple::process_envelope()\n"));
            return;
          }

          // ##################################################################
          // 'acknowledgment' code which provides the 'remote callback'
          // functionality without impacting the normal, performance-critical
          // code path.
          // ##################################################################

          inline void send_ack_request (pami_task_t task, size_t offset, pami_event_function fn, void * cookie)
          {
            ack_protocol_t ack_protocol;
            ack_protocol.remote_fn = fn;
            ack_protocol.cookie = cookie;
            ack_protocol.context = _context;
            ack_protocol.invoke = false;

            struct iovec iov[1] = {{&ack_protocol, sizeof(ack_protocol_t)}};
            bool result =
              _ack_model.postPacket (task, offset, &_origin, sizeof(pami_endpoint_t), iov);

            if (result != true)
              {
                // Allocate memory to maintain the state of the send.
                send_state_t * state = allocateSendState ();

                state->eager = this;
                state->ack_protocol = ack_protocol;

                _ack_model.postPacket (state->msg[0],
                                       ack_done, (void *) state,
                                       task, offset,
                                       &_origin, sizeof(pami_endpoint_t),
                                       (void *) &(state->ack_protocol),
                                       sizeof(ack_protocol_t));
              }
          }

          static void ack_done (pami_context_t   context,
                                void           * cookie,
                                pami_result_t    result)
          {
            TRACE_ERR((stderr, "EagerSimple::ack_done() >> \n"));

            send_state_t * state = (send_state_t *) cookie;
            EagerSimpleProtocol * eager = (EagerSimpleProtocol *) state->eager;
            eager->freeSendState (state);

            TRACE_ERR((stderr, "EagerSimple::ack_done() << \n"));
          }

          static int dispatch_ack (void   * metadata,
                                   void   * payload,
                                   size_t   bytes,
                                   void   * recv_func_parm,
                                   void   * cookie)
          {
            TRACE_ERR((stderr, ">> EagerSimple::dispatch_ack()\n"));
            EagerSimpleProtocol * eager = (EagerSimpleProtocol *) recv_func_parm;
            uint8_t stack[T_Model::packet_model_payload_bytes];

            if (T_Model::read_is_required_packet_model)
              {
                payload = (void *) & stack[0];
                eager->_short_model.device.read (payload, bytes, cookie);
              }

            ack_protocol_t * ack = (ack_protocol_t *) payload;

            if (ack->invoke)
              {
                // This is a 'pong' ack message .. invoke the callback function
                // and return.
                ack->remote_fn (ack->context, ack->cookie, PAMI_SUCCESS);
                return 0;
              }

            pami_task_t task;
            size_t offset;
            PAMI_ENDPOINT_INFO(*((pami_endpoint_t*)metadata), task, offset);

            // Attempt an 'immediate' packet post. If this fails, then a send
            // state must be allocated for this 'pong' ack message.
            ack->invoke = true;
            struct iovec iov[1] = {{payload, sizeof(ack_protocol_t)}};
            bool result = eager->_ack_model.postPacket(task, offset, NULL, 0, iov);

            if (result != true)
              {
                // Allocate memory to maintain the state of the send.
                send_state_t * state = eager->allocateSendState ();

                state->eager = eager;
                state->ack_protocol.remote_fn = ack->remote_fn;
                state->ack_protocol.cookie  = ack->cookie;
                state->ack_protocol.context = ack->context;
                state->ack_protocol.invoke  = true;

                eager->_ack_model.postPacket (state->msg[0],
                                              ack_done, (void *) state,
                                              task, offset,
                                              NULL, 0,
                                              payload,
                                              sizeof(ack_protocol_t));
              }

            TRACE_ERR((stderr, "<< EagerSimple::dispatch_ack()\n"));
            return 0;
          }

          // ##################################################################
          // end 'acknowledgement' code section
          // ##################################################################


          ///
          /// \brief Single-packet 'short' send packet dispatch.
          ///
          /// The short dispatch function is invoked by the message device
          /// to process a single-packet message. The eager simple send protocol
          /// transfers protocol metadata, application metadata, and application
          /// data in a single packet.
          ///
          /// \see PAMI::Device::Interface::RecvFunction_t
          ///
          static int dispatch_short (void   * metadata,
                                     void   * payload,
                                     size_t   bytes,
                                     void   * recv_func_parm,
                                     void   * cookie)
          {
            EagerSimpleProtocol * eager = (EagerSimpleProtocol *) recv_func_parm;
            uint8_t stack[T_Model::packet_model_payload_bytes];

            if (T_Model::read_is_required_packet_model)
              {
                payload = (void *) & stack[0];
                eager->_short_model.device.read (payload, bytes, cookie);
              }

            short_protocol_t * short_protocol;
            void * header;

            // This branch should be resolved at compile time and optimized out.
            if (sizeof(short_protocol_t) > T_Model::packet_model_metadata_bytes)
              {
                short_protocol = (short_protocol_t *) payload;
                header = (void *) (short_protocol + 1);
              }
            else
              {
                short_protocol = (short_protocol_t *) metadata;
                header = payload;
              }

            void * data = (void *) (((uint8_t *) header) + short_protocol->metabytes);

            TRACE_ERR ((stderr, ">> EagerSimple::dispatch_short(), origin = 0x%08x, bytes = %d\n", short_protocol->origin, short_protocol->bytes));

            // Invoke the registered dispatch function.
            eager->_dispatch_fn (eager->_context,           // Communication context
                                 eager->_cookie,            // Dispatch cookie
                                 header,                    // Application metadata
                                 short_protocol->metabytes, // Application metadata bytes
                                 data,                      // Application data
                                 short_protocol->bytes,     // Application data bytes
                                 short_protocol->origin,    // Origin endpoint for the transfer
                                 (pami_recv_t *) NULL);

            TRACE_ERR ((stderr, "<< EagerSimple::dispatch_short()\n"));
            return 0;
          };


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
          static int dispatch_longheader_envelope (void   * metadata,
                                                   void   * payload,
                                                   size_t   bytes,
                                                   void   * recv_func_parm,
                                                   void   * cookie)
          {
            EagerSimpleProtocol * eager = (EagerSimpleProtocol *) recv_func_parm;
            uint8_t stack[T_Model::packet_model_payload_bytes];

            if (T_Model::read_is_required_packet_model)
              {
                payload = (void *) & stack[0];
                eager->_short_model.device.read (payload, bytes, cookie);
              }

            longheader_protocol_t * m = (longheader_protocol_t *) payload;

            TRACE_ERR ((stderr, ">> EagerSimple::dispatch_longheader_envelope(), origin = 0x%08x, m->bytes = %zu\n", m->origin, m->bytes));

            // Allocate a recv state object!
            recv_state_t * state = eager->allocateRecvState ();
            state->eager         = eager;
            state->received      = 0;
            state->data_size     = m->bytes;
            state->header_size   = m->metabytes;
            state->origin        = m->origin;
            state->longheader.offset = 0;

            // Set the eager connection.
            pami_task_t task;
            size_t offset;
            PAMI_ENDPOINT_INFO(m->origin, task, offset);
            eager->_longheader_envelope_model.device.setConnection ((void *)state, task, offset);

            pami_result_t prc;
            prc = __global.heap_mm->memalign((void **) & state->longheader.addr, 0,
                                             state->header_size);
            PAMI_assertf(prc == PAMI_SUCCESS, "alloc of state->longheader.addr failed");
            TRACE_ERR ((stderr, "<< EagerSimple::dispatch_longheader_envelope() .. long header\n"));

            return 0;
          };

          ///
          /// \note The zero-byte application data case is handled by the
          ///       'short' protocol, if the application header will fit in a
          ///       single packet, or by the 'long header' protocol.
          ///
          /// \see dispatch_envelope_direct
          ///
          template <bool T_Contiguous, bool T_Copy>
          static int dispatch_eager_envelope (void   * metadata,
                                              void   * payload,
                                              size_t   bytes,
                                              void   * recv_func_parm,
                                              void   * cookie)
          {
            EagerSimpleProtocol * eager = (EagerSimpleProtocol *) recv_func_parm;
            uint8_t stack[T_Model::packet_model_payload_bytes];

            if (T_Model::read_is_required_packet_model)
              {
                payload = (void *) & stack[0];
                eager->_short_model.device.read (payload, bytes, cookie);
              }

            eager_protocol_t * m;
            void * p;

            // This branch should be resolved at compile time and optimized out.
            if (sizeof(eager_protocol_t) > T_Model::packet_model_metadata_bytes)
              {
                m = (eager_protocol_t *) payload;
                p = (void *) (m + 1);
              }
            else
              {
                m = (eager_protocol_t *) metadata;
                p = payload;
              }

            TRACE_ERR ((stderr, ">> EagerSimple::dispatch_eager_envelope(), origin = 0x%08x, m->bytes = %zu\n", m->origin, m->bytes));

            // Allocate a recv state object!
            recv_state_t * state = eager->allocateRecvState ();

            // This is a short header envelope .. all application metadata
            // has been received.
            eager->process_envelope<T_Contiguous, T_Copy> ((uint8_t *)p, m->metabytes, m->bytes, m->origin, state);

            if (unlikely(m->bytes == 0))
              {
                // No data packets will be received. Invoke the receive
                // completion callback function and return the receive state.

                if (state->info.local_fn)
                  state->info.local_fn (eager->_context, state->info.cookie, PAMI_SUCCESS);

                eager->freeRecvState (state);

                TRACE_ERR ((stderr, "<< EagerSimple::dispatch_eager_envelope()\n"));
                return 0;
              }

            // Complete the receive state initialization
            state->eager         = eager;
            state->received      = 0;
            state->data_size     = m->bytes;
            state->header_size   = m->metabytes;
            state->origin        = m->origin;

            // Set the eager connection.
            pami_task_t task;
            size_t offset;
            PAMI_ENDPOINT_INFO(m->origin, task, offset);
            eager->_envelope_model.device.setConnection ((void *)state, task, offset);

            TRACE_ERR ((stderr, "<< EagerSimple::dispatch_eager_envelope()\n"));
            return 0;
          };

          template <bool T_Contiguous, bool T_Copy>
          static int dispatch_longheader_message (void   * metadata,
                                                  void   * payload,
                                                  size_t   bytes,
                                                  void   * recv_func_parm,
                                                  void   * cookie)
          {
            EagerSimpleProtocol * eager = (EagerSimpleProtocol *) recv_func_parm;
            uint8_t stack[T_Model::packet_model_payload_bytes];

            if (T_Model::read_is_required_packet_model)
              {
                payload = (void *) & stack[0];
                eager->_short_model.device.read (payload, bytes, cookie);
              }

            pami_endpoint_t origin = *((pami_endpoint_t *) metadata);

            TRACE_ERR((stderr, ">> EagerSimple::dispatch_longheader_message(), origin = 0x%08x, bytes = %zu\n", origin, bytes));

            pami_task_t task;
            size_t offset;
            PAMI_ENDPOINT_INFO(origin, task, offset);
            recv_state_t * state = (recv_state_t *) eager->_longheader_message_model.device.getConnection (task, offset);

            size_t n = MIN(bytes, state->header_size - state->longheader.offset);

            eager->_longheader_message_model.device.read ((void *)(state->longheader.addr + state->longheader.offset), n, cookie);
            state->longheader.offset += n;

            if (state->header_size == state->longheader.offset)
              {
                // The entire application metadata has been received. Invoke the
                // registered pami point-to-point dispatch function.

                if (state->data_size == 0)
                  {
                    // No data packets will follow this 'long header' message.
                    // Invoke the dispatch function as an 'immediate' receive.
                    eager->_dispatch_fn (eager->_context,            // Communication context
                                         eager->_cookie,             // Dispatch cookie
                                         state->longheader.addr,     // Application header
                                         state->header_size,         // Application header bytes
                                         NULL,                       // No application data
                                         0,                          // Number of msg bytes
                                         origin,                     // Origin endpoint for the transfer
                                         (pami_recv_t *) NULL);

                    // No data packets will be received on this connection.
                    // Clear the connection data and prepare for the next message.
                    eager->_longheader_message_model.device.clearConnection (task, offset);

                    // Free the malloc'd longheader buffer now that it has been
                    // delivered to the application.
                    __global.heap_mm->free (state->longheader.addr);

                    eager->freeRecvState (state);
                  }
                else
                  {
                    eager->process_envelope<T_Contiguous, T_Copy> (state->longheader.addr,
                                                                   state->header_size,
                                                                   state->data_size,
                                                                   origin,
                                                                   state);

                    // Free the malloc'd longheader buffer now that it has been
                    // delivered to the application.
                    __global.heap_mm->free (state->longheader.addr);
                  }
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
          template <bool T_ContiguousCopy>
          static int dispatch_data_message   (void   * metadata,
                                              void   * payload,
                                              size_t   bytes,
                                              void   * recv_func_parm,
                                              void   * cookie)
          {
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

            TRACE_ERR((stderr, ">> EagerSimple::dispatch_data_message(), origin task = %d, origin offset = %zu, bytes = %zu\n", task, offset, bytes));

            recv_state_t * state = (recv_state_t *) eager->_data_model.device.getConnection (task, offset);

            // Number of bytes received so far.
            const size_t nbyte = state->received;

            // Number of bytes left to copy into the destination buffer
            const size_t nleft = state->data_size - nbyte;

            TRACE_ERR((stderr, "   EagerSimple::dispatch_data_message(), bytes received so far = %zu, bytes yet to receive = %zu, total bytes to receive = %zu\n", state->received, nleft, state->data_size));

            // Copy data from the packet payload into the destination buffer
            size_t ncopy = MIN(nleft, bytes);

            if (T_ContiguousCopy || likely(state->is_contiguous_copy_recv))
              Core_memcpy ((uint8_t *)(state->info.addr) + nbyte, payload, ncopy);
            else
              state->machine.Unpack (state->info.addr, payload, ncopy);

            state->received += ncopy;

            TRACE_ERR((stderr, "   EagerSimple::dispatch_data_message(), nbyte = %zu, bytes = %zu, state->data_bytes = %zu\n", nbyte, bytes, state->data_size));

            if (nbyte + bytes >= state->data_size)
              {
                // No more data packets will be received on this connection.
                // Clear the connection data and prepare for the next message.
                eager->_data_model.device.clearConnection (task, offset);

                // No more data is to be written to the receive buffer.
                // Invoke the receive done callback.
                if (state->info.local_fn)
                  state->info.local_fn (eager->_context,
                                        state->info.cookie,
                                        PAMI_SUCCESS);

                // Return the receive state object memory to the memory pool.
                eager->freeRecvState (state);

                TRACE_ERR((stderr, "<< EagerSimple::dispatch_data_message(), origin task = %d ... receive completed\n", task));
                return 0;
              }

            TRACE_ERR((stderr, "<< EagerSimple::dispatch_data_message(), origin task = %d ... wait for more data\n", task));
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

            eager->freeSendState (state);

            TRACE_ERR((stderr, "EagerSimple::send_complete() << \n"));
            return;
          }

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
