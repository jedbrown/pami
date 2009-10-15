/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
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
#ifndef __xmi_p2p_protocol_send_eager_eagersimple_h__
#define __xmi_p2p_protocol_send_eager_eagersimple_h__

#include "components/memory/MemoryAllocator.h"

#include "p2p/protocols/send/eager/EagerConnection.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif

namespace XMI
{
  namespace Protocol
  {
    namespace Send
    {

      ///
      /// \brief Eager simple send protocol class for reliable network devices.
      ///
      /// \tparam T_Model   Template packet model class
      /// \tparam T_Device  Template packet device class
      ///
      /// \see XMI::Device::Interface::PacketModel
      /// \see XMI::Device::Interface::PacketDevice
      ///
      template <class T_Model, class T_Device>
      class EagerSimple
      {
        protected:

          typedef uint8_t pkt_t[T_Model::packet_model_state_bytes];
          typedef uint8_t msg_t[T_Model::message_model_state_bytes];

          typedef struct __attribute__((__packed__)) short_metadata
          {
            size_t         bytes;     ///< Number of bytes of application data being sent
            size_t         metabytes; ///< Number of bytes of application metadata being sent
            void         * ackinfo;   ///< a.k.a. send_state_t *
            xmi_task_t     fromRank;  ///< Sender global task identifier
        } short_metadata_t;


          typedef struct send_state
          {
            pkt_t                   pkt;
            msg_t                   msg[2];
            short_metadata_t        metadata;  ///< Eager protocol envelope metadata
            xmi_event_function      local_fn;  ///< Application send injection completion callback
            xmi_event_function      remote_fn; ///< Application remote receive acknowledgement callback
            void                  * cookie;    ///< Application callback cookie
            EagerSimple < T_Model,
            T_Device > * eager;    ///< Eager protocol object
          } send_state_t;

          typedef struct recv_state
          {
            pkt_t                   pkt;
            xmi_recv_t              info;     ///< Application receive information.
            size_t                  received; ///< Number of bytes received.
            struct
            {
              uint8_t             * addr;
              size_t                bytes;
              size_t                offset;
            } longheader;
            short_metadata_t        metadata; ///< Original eager protocol envelope metadata
            EagerSimple < T_Model,
            T_Device > * eager;   ///< Eager protocol object
          } recv_state_t;

        public:

          ///
          /// \brief Eager simple send protocol constructor.
          ///
          /// \param[in]  dispatch     Dispatch identifier
          /// \param[in]  dispatch_fn  Dispatch callback function
          /// \param[in]  cookie       Opaque application dispatch data
          /// \param[in]  device       Device that implements the message interface
          /// \param[in]  origin_task  Origin task identifier
          /// \param[in]  context      Communication context
          /// \param[out] status       Constructor status
          ///
          inline EagerSimple (size_t                     dispatch,
                              xmi_dispatch_callback_fn   dispatch_fn,
                              void                     * cookie,
                              T_Device                 & device,
                              xmi_task_t                 origin_task,
                              xmi_context_t              context,
                              size_t                     contextid,
                              xmi_result_t             & status) :
              _envelope_model (device, context),
              _longheader_model (device, context),
              _data_model (device, context),
              _ack_model (device, context),
              _device (device),
              _fromRank (origin_task),
              _context (context),
              _contextid (contextid),
              _dispatch_fn (dispatch_fn),
              _cookie (cookie),
              _connection ((void **)NULL),
              _connection_manager (device)
          {
            // ----------------------------------------------------------------
            // Compile-time assertions
            // ----------------------------------------------------------------

            // This protocol only works with reliable networks.
            COMPILE_TIME_ASSERT(T_Model::reliable_packet_model == true);

            // This protcol only works with deterministic models.
            COMPILE_TIME_ASSERT(T_Model::deterministic_packet_model == true);

            // Assert that the size of the packet metadata area is large
            // enough to transfer a single xmi_task_t. This is used in the
            // various postMessage() calls to transfer long header and data
            // messages.
            COMPILE_TIME_ASSERT(sizeof(xmi_task_t) <= T_Model::message_model_metadata_bytes);

            // Assert that the size of the packet payload area is large
            // enough to transfer a single virtual address. This is used in
            // the postPacket() calls to transfer the ack information.
            COMPILE_TIME_ASSERT(sizeof(void *) <= T_Model::packet_model_payload_bytes);

            // ----------------------------------------------------------------
            // Compile-time assertions
            // ----------------------------------------------------------------


            _connection = _connection_manager.getConnectionArray (context);

            TRACE_ERR((stderr, "EagerSimple() [0]\n"));
            status = _envelope_model.init (dispatch,
                                           dispatch_envelope_direct, this,
                                           dispatch_envelope_read, this);
            TRACE_ERR((stderr, "EagerSimple() [1] status = %d\n", status));

            if (status == XMI_SUCCESS)
              {
                status = _data_model.init (dispatch,
                                           dispatch_data_message, this,
                                           dispatch_data_message, this);
                TRACE_ERR((stderr, "EagerSimple() [2] status = %d\n", status));

                if (status == XMI_SUCCESS)
                  {
                    status = _ack_model.init (dispatch,
                                              dispatch_ack_direct, this,
                                              dispatch_ack_read, this);
                    TRACE_ERR((stderr, "EagerSimple() [3] status = %d\n", status));

                    if (status == XMI_SUCCESS)
                      {
                        status = _longheader_model.init (dispatch,
                                                         dispatch_longheader_message, this,
                                                         dispatch_longheader_message, this);
                        TRACE_ERR((stderr, "EagerSimple() [4] status = %d\n", status));
                      }
                  }
              }
          }

          ///
          /// \brief Start a new simple send eager operation.
          ///
          /// \see XMI::Protocol::Send:simple
          ///
          inline xmi_result_t simple_impl (xmi_event_function   local_fn,
                                           xmi_event_function   remote_fn,
                                           void               * cookie,
                                           xmi_task_t           peer,
                                           void               * src,
                                           size_t               bytes,
                                           void               * msginfo,
                                           size_t               mbytes)
          {
            TRACE_ERR((stderr, "EagerSimple::simple_impl() >> sizeof(short_metadata_t) = %zd, T_Model::packet_model_metadata_bytes = %zd\n", sizeof(short_metadata_t), T_Model::packet_model_metadata_bytes));

            // Allocate memory to maintain the state of the send.
            send_state_t * state = allocateSendState ();

            state->cookie   = cookie;
            state->local_fn = local_fn;
            state->eager    = this;

            // Specify the protocol metadata to send with the application
            // metadata in the envelope packet.
            state->metadata.fromRank  = _fromRank;
            state->metadata.bytes     = bytes;
            state->metadata.metabytes = mbytes;

            // Set the acknowledgement information to the virtual address of
            // send state object on the origin task if a local callback of the
            // remote receive completion event is requested. If this is set to
            // NULL no acknowledgement will be received by the origin task.
            if (remote_fn != NULL)
              {
                state->metadata.ackinfo = state;
                state->remote_fn = remote_fn;
              }
            else
              {
                state->metadata.ackinfo = NULL;
                state->remote_fn = NULL;
              }

            if (unlikely(bytes == 0))
              {
                // In the unlikely event that this eager (i.e., multi-packet)
                // protocol is being used to send zero bytes of application
                // data, the envelope message completion must clean up the
                // protocol resources and invoke the application local done
                // callback function, as there will be no data message.
                TRACE_ERR((stderr, "EagerSimple::simple_impl() .. zero-byte data special case\n"));

                // This branch should be resolved at compile time and optimized out.
                if (sizeof(short_metadata_t) <= T_Model::packet_model_metadata_bytes)
                  {
                    TRACE_ERR((stderr, "EagerSimple::simple_impl() .. zero-byte data special case, protocol metadata fits in the packet metadata\n"));

                    if (unlikely(mbytes > T_Model::packet_model_payload_bytes))
                      {
                        TRACE_ERR((stderr, "EagerSimple::simple_impl() .. zero-byte data special case, protocol metadata fits in the packet metadata, application metadata does not fit in a single packet payload\n"));

                        // "long header" with zero bytes of application data ? Weird.
                        _envelope_model.postPacket (state->pkt,
                                                    NULL,
                                                    NULL,
                                                    peer,
                                                    (void *) &(state->metadata),
                                                    sizeof (short_metadata_t),
                                                    (void *) NULL,
                                                    0);

                        _longheader_model.postMessage (state->msg[0],
                                                       send_complete,
                                                       (void *) state,
                                                       peer,
                                                       (void *) &(state->metadata.fromRank),
                                                       sizeof (xmi_task_t),
                                                       msginfo,
                                                       mbytes);
                      }
                    else
                      {
                        TRACE_ERR((stderr, "EagerSimple::simple_impl() .. zero-byte data special case, protocol metadata fits in the packet metadata, application metadata does fit in a single packet payload\n"));

                        // Single packet header with zero bytes of application data.
                        _envelope_model.postPacket (state->pkt,
                                                    send_complete,
                                                    (void *) state,
                                                    peer,
                                                    (void *) &(state->metadata),
                                                    sizeof (short_metadata_t),
                                                    msginfo,
                                                    mbytes);
                      }
                  }
                else
                  {
                    TRACE_ERR((stderr, "EagerSimple::simple_impl() .. zero-byte data special case, protocol metadata does not fit in the packet metadata\n"));
                    //XMI_assertf((mbytes + sizeof(short_metadata_t)) <= T_Model::packet_model_payload_bytes, "Unable to fit protocol metadata (%zd) and application metadata (%zd) within the payload of a single packet (%zd)\n", sizeof(short_metadata_t), mbytes, T_Model::packet_model_payload_bytes);

                    if (unlikely(mbytes > (T_Model::packet_model_payload_bytes - sizeof(short_metadata_t))))
                      {
                        TRACE_ERR((stderr, "EagerSimple::simple_impl() .. zero-byte data special case, protocol metadata does not fit in the packet metadata, protocol + application metadata does not fit in a single packet payload\n"));

                        // "long header" with zero bytes of application data ? Weird.
                        _envelope_model.postPacket (state->pkt,
                                                    NULL,
                                                    NULL,
                                                    peer,
                                                    NULL,
                                                    0,
                                                    (void *) &(state->metadata),
                                                    sizeof (short_metadata_t));

                        _longheader_model.postMessage (state->msg[0],
                                                       send_complete,
                                                       (void *) state,
                                                       peer,
                                                       (void *) &(state->metadata.fromRank),
                                                       sizeof (xmi_task_t),
                                                       msginfo,
                                                       mbytes);
                      }
                    else
                      {
                        TRACE_ERR((stderr, "EagerSimple::simple_impl() .. zero-byte data special case, protocol metadata does not fit in the packet metadata, protocol + application metadata does fit in a single packet payload\n"));

                        // Single packet header with zero bytes of application data.
                        _envelope_model.postPacket (state->pkt,
                                                    send_complete,
                                                    (void *) state,
                                                    peer,
                                                    NULL,
                                                    0,
                                                    (void *) &(state->metadata),
                                                    sizeof (short_metadata_t),
                                                    msginfo,
                                                    mbytes);
                      }
                  }
              }
            else
              {
                TRACE_ERR((stderr, "EagerSimple::simple_impl() .. before _envelope_model.postPacket() .. bytes = %zd\n", bytes));

                // This branch should be resolved at compile time and optimized out.
                if (sizeof(short_metadata_t) <= T_Model::packet_model_metadata_bytes)
                  {
                    if (unlikely(mbytes > T_Model::packet_model_payload_bytes))
                      {
                        // Application metadata does not fit in a single packet.
                        // Send a "long header" message.
                        _envelope_model.postPacket (state->pkt,
                                                    NULL,
                                                    NULL,
                                                    peer,
                                                    (void *) &(state->metadata),
                                                    sizeof (short_metadata_t),
                                                    (void *)NULL,
                                                    0);

                        _longheader_model.postMessage (state->msg[0],
                                                       NULL,
                                                       NULL,
                                                       peer,
                                                       (void *) &(state->metadata.fromRank),
                                                       sizeof (xmi_task_t),
                                                       msginfo,
                                                       mbytes);
                      }
                    else
                      {
                        _envelope_model.postPacket (state->pkt,
                                                    NULL,
                                                    NULL,
                                                    peer,
                                                    (void *) &(state->metadata),
                                                    sizeof (short_metadata_t),
                                                    msginfo,
                                                    mbytes);
                      }
                  }
                else
                  {
                    if (unlikely(mbytes > (T_Model::packet_model_payload_bytes - sizeof(short_metadata_t))))
                      {
                        // Protocol metadata + application metadata does not fit in
                        // a single packet. Send a "long header" message.
                        _envelope_model.postPacket (state->pkt,
                                                    NULL,
                                                    NULL,
                                                    peer,
                                                    (void *) &(state->metadata),
                                                    sizeof (short_metadata_t),
                                                    (void *)NULL,
                                                    0);

                        _longheader_model.postMessage (state->msg[0],
                                                       NULL,
                                                       NULL,
                                                       peer,
                                                       (void *) &(state->metadata.fromRank),
                                                       sizeof (xmi_task_t),
                                                       msginfo,
                                                       mbytes);
                      }
                    else
                      {
                        _envelope_model.postPacket (state->pkt,
                                                    NULL,
                                                    NULL,
                                                    peer,
                                                    NULL,
                                                    0,
                                                    (void *) &(state->metadata),
                                                    sizeof (short_metadata_t),
                                                    msginfo,
                                                    mbytes);
                      }
                  }

                TRACE_ERR((stderr, "EagerSimple::simple_impl() .. before _data_model.postPacket()\n"));
                _data_model.postMessage (state->msg[1],
                                         send_complete,
                                         (void *) state,
                                         peer,
                                         (void *) &(state->metadata.fromRank),
                                         sizeof (xmi_task_t),
                                         src,
                                         bytes);
              }

            TRACE_ERR((stderr, "EagerSimple::simple_impl() <<\n"));
            return XMI_SUCCESS;
          };

        protected:

          MemoryAllocator < sizeof(send_state_t), 16 > _send_allocator;
          MemoryAllocator < sizeof(recv_state_t), 16 > _recv_allocator;

          T_Model         _envelope_model;
          T_Model         _longheader_model;
          T_Model         _data_model;
          T_Model         _ack_model;
          T_Device      & _device;
          xmi_task_t      _fromRank;
          xmi_context_t   _context;
          size_t          _contextid;

          xmi_dispatch_callback_fn   _dispatch_fn;
          void                     * _cookie;

          void **                   _connection;
          EagerConnection<T_Device> _connection_manager;

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

          inline void setConnection (xmi_task_t task, void * arg)
          {
            size_t peer = _device.task2peer (task);
            TRACE_ERR((stderr, ">> EagerSimple::setConnection(%zd, %p) .. _connection[%zd] = %p\n", (size_t)task, arg, peer, _connection[peer]));
            XMI_assert_debug(_connection[peer] == NULL);
            _connection[peer] = arg;
            TRACE_ERR((stderr, "<< EagerSimple::setConnection(%zd, %p)\n", (size_t)task, arg));
          }

          inline void * getConnection (xmi_task_t task)
          {
            size_t peer = _device.task2peer (task);
            TRACE_ERR((stderr, ">> EagerSimple::getConnection(%zd) .. _connection[%zd] = %p\n", (size_t)task, peer, _connection[peer]));
            XMI_assert_debug(_connection[peer] != NULL);
            TRACE_ERR((stderr, "<< EagerSimple::getConnection(%zd) .. _connection[%zd] = %p\n", (size_t)task, peer, _connection[peer]));
            return _connection[peer];
          }

          inline void clearConnection (xmi_task_t task)
          {
            size_t peer = _device.task2peer (task);
            TRACE_ERR((stderr, ">> EagerSimple::clearConnection(%zd) .. _connection[%zd] = %p\n", (size_t)task, peer, _connection[peer]));
            _connection[peer] = NULL;
            TRACE_ERR((stderr, "<< EagerSimple::clearConnection(%zd) .. _connection[%zd] = %p\n", (size_t)task, peer, _connection[peer]));
          }

          inline void process_envelope (short_metadata_t * metadata,
                                        uint8_t          * header,
                                        recv_state_t     * state)
          {
            TRACE_ERR((stderr, ">> EagerSimple::process_envelope()\n"));

            // Invoke the registered dispatch function.
            _dispatch_fn.p2p (_context,            // Communication context
                              _contextid,          // Context index
                              _cookie,             // Dispatch cookie
                              metadata->fromRank,  // Origin (sender) rank
                              header,              // Application metadata
                              metadata->metabytes, // Application metadata bytes
                              NULL,                // No payload data
                              metadata->bytes,     // Number of msg bytes
                              (xmi_recv_t *) &(state->info));

            // Only handle simple receives .. until the non-contiguous support
            // is available
            XMI_assert(state->info.kind == XMI_AM_KIND_SIMPLE);

            TRACE_ERR((stderr, "   EagerSimple::process_envelope() .. metadata->bytes = %zd\n", metadata->bytes));

            if (unlikely(metadata->bytes == 0))
              {
                // No data packets will follow this envelope packet. Invoke the
                // recv done callback and, if an acknowledgement packet was
                // requested send the acknowledgement. Otherwise return the recv
                // state memory which was allocated above.
                TRACE_ERR((stderr, "   EagerSimple::process_envelope() .. state->info.local_fn = %p\n", state->info.local_fn));

                if (state->info.local_fn)
                  state->info.local_fn (_context,
                                        state->info.cookie,
                                        XMI_SUCCESS);

                TRACE_ERR((stderr, "   EagerSimple::process_envelope() .. state->metadata.ackinfo = %p\n", state->metadata.ackinfo));

                if (unlikely(state->metadata.ackinfo != NULL))
                  {
                    _ack_model.postPacket (state->pkt,
                                           receive_complete,
                                           (void *) state,
                                           metadata->fromRank,
                                           (void *) NULL,
                                           0,
                                           (void *) &(state->metadata.ackinfo),
                                           sizeof (send_state_t *));
                  }
                else
                  {
                    clearConnection (metadata->fromRank);
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

            xmi_event_function   remote_fn = state->remote_fn;
            void               * fn_cookie = state->cookie;
            TRACE_ERR((stderr, "   EagerSimple::dispatch_ack_direct() .. state = %p, remote_fn = %p\n", state, remote_fn));

            EagerSimple<T_Model, T_Device> * eager =
              (EagerSimple<T_Model, T_Device> *) recv_func_parm;

            eager->freeSendState (state);

            if (remote_fn) remote_fn (eager->_context, fn_cookie, XMI_SUCCESS);

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

            EagerSimple<T_Model, T_Device> * pf =
              (EagerSimple<T_Model, T_Device> *) recv_func_parm;

            // This packet device DOES NOT provide the data buffer(s) for the
            // message and the data must be read on to the stack before the
            // recv callback is invoked.
            XMI_assert_debugf(payload == NULL, "The 'read only' packet device did not invoke dispatch with payload == NULL (%p)\n", payload);

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
          /// \see XMI::Device::Interface::RecvFunction_t
          ///
          static int dispatch_envelope_direct (void   * metadata,
                                               void   * payload,
                                               size_t   bytes,
                                               void   * recv_func_parm,
                                               void   * cookie)
          {
            short_metadata_t * m;
            void * p;

            // This branch should be resolved at compile time and optimized out.
            if (sizeof(short_metadata_t) > T_Model::packet_model_metadata_bytes)
              {
                m = (short_metadata_t *) payload;
                p = (void *) (m + 1);
              }
            else
              {
                m = (short_metadata_t *) metadata;
                p = payload;
              }

            TRACE_ERR ((stderr, ">> EagerSimple::dispatch_envelope_direct(), m->fromRank = %zd, m->bytes = %zd, m->ackinfo = %p\n", (size_t)(m->fromRank), m->bytes, m->ackinfo));

            EagerSimple<T_Model, T_Device> * eager =
              (EagerSimple<T_Model, T_Device> *) recv_func_parm;

            // Allocate a recv state object!
            recv_state_t * state = eager->allocateRecvState ();
            state->eager = eager;
            state->received = 0;
            state->metadata.ackinfo   = m->ackinfo;
            state->metadata.bytes     = m->bytes;
            state->metadata.fromRank  = m->fromRank;
            state->metadata.metabytes = m->metabytes;

            // Set the eager connection.
            eager->setConnection (m->fromRank, (void *)state);

            // Check for long header
            size_t header_bytes = m->metabytes;

            if (unlikely((header_bytes) > (T_Model::packet_model_payload_bytes -
                                           sizeof(short_metadata_t) > T_Model::packet_model_metadata_bytes ? sizeof(short_metadata_t) : 0)))
              {
                state->longheader.addr   = (uint8_t *) malloc(header_bytes);
                state->longheader.bytes  = header_bytes;
                state->longheader.offset = 0;
                TRACE_ERR ((stderr, "<< EagerSimple::dispatch_envelope_direct() .. long header\n"));
                return 0;
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

            EagerSimple<T_Model, T_Device> * pf =
              (EagerSimple<T_Model, T_Device> *) recv_func_parm;

            // This packet device DOES NOT provide the data buffer(s) for the
            // message and the data must be read on to the stack before the
            // recv callback is invoked.
            XMI_assert_debugf(payload == NULL, "The 'read only' packet device did not invoke dispatch with payload == NULL (%p)\n", payload);

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
            EagerSimple<T_Model, T_Device> * eager =
              (EagerSimple<T_Model, T_Device> *) recv_func_parm;

            xmi_task_t fromRank = *((xmi_task_t *)metadata);
            TRACE_ERR((stderr, ">> EagerSimple::dispatch_longheader_message(), fromRank = %zd, bytes = %zd\n", (size_t)fromRank, bytes));

            recv_state_t * state = (recv_state_t *) eager->getConnection (fromRank);
            XMI_assert_debug(state != NULL);

            size_t n = MIN(bytes, state->longheader.bytes - state->longheader.offset);

            eager->_device.read ((void *)(state->longheader.addr + state->longheader.offset), n, cookie);
            state->longheader.offset += n;

            if (state->longheader.bytes == state->longheader.offset)
              {
                // The entire application metadata has been received. Invoke the
                // registered xmi point-to-point dispatch function.

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
          /// \see XMI::Device::Interface::RecvFunction_t
          ///
          static int dispatch_data_message   (void   * metadata,
                                              void   * payload,
                                              size_t   bytes,
                                              void   * recv_func_parm,
                                              void   * cookie)
          {
            EagerSimple<T_Model, T_Device> * eager =
              (EagerSimple<T_Model, T_Device> *) recv_func_parm;

            xmi_task_t fromRank = *((xmi_task_t *)metadata);
            TRACE_ERR((stderr, ">> EagerSimple::dispatch_data_message(), fromRank = %zd, bytes = %zd\n", (size_t)fromRank, bytes));

            recv_state_t * state = (recv_state_t *) eager->getConnection (fromRank);
            XMI_assert_debug(state != NULL);

            // Number of bytes received so far.
            size_t nbyte = state->received;

            // Number of bytes left to copy into the destination buffer
            size_t nleft = state->info.data.simple.bytes - nbyte;

            TRACE_ERR((stderr, "   EagerSimple::dispatch_data_message(), bytes received so far = %zd, bytes yet to receive = %zd, total bytes to receive = %zd, total bytes being sent = %zd\n", state->received, nleft, state->info.data.simple.bytes, state->metadata.bytes));

            if (nleft > 0)
              {
                // Copy data from the packet buffer into the receive buffer.
                if (nleft < bytes)
                  {
                    //memcpy ((uint8_t *)(state->info.data.simple.addr) + nbyte, payload, nleft);
                    eager->_device.read ((uint8_t *)(state->info.data.simple.addr) + nbyte, nleft, cookie);

                    // Update the receive state to prepate for another data packet.
                    state->received += nleft;
                  }
                else
                  {
                    //memcpy ((uint8_t *)(state->info.data.simple.addr) + nbyte, payload, bytes);
                    eager->_device.read ((uint8_t *)(state->info.data.simple.addr) + nbyte, bytes, cookie);

                    // Update the receive state to prepate for another data packet.
                    state->received += bytes;
                  }
              }

            TRACE_ERR((stderr, "   EagerSimple::dispatch_data_message(), nbyte = %zd\n", nbyte));

            if (nbyte + bytes >= state->metadata.bytes)
              {
                // No more data packets will be received on this connection.
                // Clear the connection data and prepare for the next message.
                eager->clearConnection (fromRank);

                // No more data is to be written to the receive buffer.
                // Invoke the receive done callback.
                if (state->info.local_fn)
                  state->info.local_fn (eager->_context,
                                        state->info.cookie,
                                        XMI_SUCCESS);

                if (state->metadata.ackinfo != NULL)
                  {
                    eager->_ack_model.postPacket (state->pkt,
                                                  receive_complete,
                                                  (void *) state,
                                                  fromRank,
                                                  (void *) NULL,
                                                  0,
                                                  (void *) &(state->metadata.ackinfo),
                                                  sizeof (send_state_t *));
                  }
                else
                  {
                    // Otherwise, return the receive state object memory to
                    // the memory pool.
                    eager->freeRecvState (state);
                  }

                TRACE_ERR((stderr, "<< dispatch_data_message(), fromRank = %zd ... receive completed\n", (size_t)fromRank));
                return 0;
              }

            TRACE_ERR((stderr, "<< dispatch_data_message(), fromRank = %zd ... wait for more data\n", (size_t)fromRank));
            return 0;
          };

#if 0
          ///
          /// \brief Read-access multi-packet send data packet callback.
          ///
          /// Reads incoming bytes from the device to the destination buffer.
          ///
          /// The eager packet factory will register this dispatch function
          /// if and only if the device \b does \b not provide access to
          /// data which has already been read from the network by the device.
          ///
          /// The data dispatch function is invoked by the packet device
          /// to process the subsequent packets of a multi-packet message
          /// after the first envelope packet. The CDI eager protocol sends
          /// one iovec element in the data packets for the user data buffer.
          ///
          /// \note The iov_base fields in the iovec structure will be \c NULL
          ///
          /// \see DCMF::CDI::RecvFunction_t
          /// \see dispatch_envelope_read
          ///
          static int dispatch_data_read     (void   * metadata,
                                             void   * payload,
                                             size_t   bytes,
                                             void   * recv_func_parm,
                                             void   * cookie)
          {
            TRACE_ERR((stderr, "EagerFactory::dispatch_data_read() .. \n"));

            EagerFactory<T_Model, T_Device, T_Message> * pf =
              (EagerFactory<T_Model, T_Device, T_Message> *) recv_func_parm;

            // This packet device DOES NOT provide the data buffer(s) for the
            // message and the data must be read on to the stack before the
            // recv callback is invoked.

            uint8_t stackData[pf->getDevice()->getPacketPayloadSize()];
            void * p = (void *) & stackData[0];
            pf->getDevice()->readData(channel, (char *) p, bytes);

            dispatch_data_read (channel, metadata, p, bytes, recv_func_parm);
            return 0;
          };
#endif

          ///
          /// \brief Local send completion event callback.
          ///
          /// This callback will invoke the application local completion
          /// callback function and, if notification of remote receive
          /// completion is not required, free the send state memory.
          ///
          static void send_complete (xmi_context_t   context,
                                     void          * cookie,
                                     xmi_result_t    result)
          {
            TRACE_ERR((stderr, "EagerSimple::send_complete() >> \n"));
            send_state_t * state = (send_state_t *) cookie;

            EagerSimple<T_Model, T_Device> * eager =
              (EagerSimple<T_Model, T_Device> *) state->eager;

            if (state->local_fn != NULL)
              {
                state->local_fn (eager->_context, state->cookie, XMI_SUCCESS);
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
          static void receive_complete (xmi_context_t   context,
                                        void          * cookie,
                                        xmi_result_t    result)
          {
            TRACE_ERR((stderr, "EagerSimple::receive_complete() >> \n"));
            recv_state_t * state = (recv_state_t *) cookie;
            EagerSimple<T_Model, T_Device> * eager =
              (EagerSimple<T_Model, T_Device> *) state->eager;

            eager->freeRecvState (state);

            TRACE_ERR((stderr, "EagerSimple::receive_complete() << \n"));
            return;
          };
      };
    };
  };
};
#undef TRACE_ERR
#endif // __xmi_p2p_protocol_send_eager_eagersimple_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
