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
      /// \tparam T_Message Template packet message class
      ///
      /// \see XMI::Device::Interface::PacketModel
      /// \see XMI::Device::Interface::PacketDevice
      ///
      template <class T_Model, class T_Device, class T_Message>
      class EagerSimple
      {
        protected:

          typedef struct send_state
          {
            T_Message            msg[2];
            xmi_event_function   local_fn;
            xmi_event_function   remote_fn;
            void               * cookie;
            EagerSimple<T_Model, T_Device, T_Message> * eager;
          } send_state_t;

          typedef struct recv_state
          {
            T_Message      msg;
            send_state_t * ackinfo;
            EagerSimple<T_Model, T_Device, T_Message> * eager;
            xmi_recv_t     info;     ///< Application receive information.
            size_t         received; ///< Number of bytes received.
            size_t         sndlen;   ///< Number of bytes being sent from the origin rank.
          } recv_state_t;

          typedef struct __attribute__((__packed__)) short_metadata
          {
            xmi_task_t     fromRank;
            size_t         bytes;
            send_state_t * ackinfo;
        } short_metadata_t;


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
              _data_model (device, context),
              _ack_model (device, context),
              _msgDevice (device),
              _pktsize (device.getPacketPayloadSize ()),
              _fromRank (origin_task),
              _context (context),
              _dispatch_fn (dispatch_fn),
              _cookie (cookie),
              _connection ((void **)NULL),
              _connection_manager (device),
              _contextid (contextid)
          {
            _connection = _connection_manager.getConnectionArray (context);

            TRACE_ERR((stderr, "EagerSimple() [0]\n"));
            status = _envelope_model.init (dispatch,
                                           dispatch_envelope_direct, this,
                                           dispatch_envelope_read, this);
            TRACE_ERR((stderr, "EagerSimple() [1] status = %d\n", status));

            if (status == XMI_SUCCESS)
              {
                status = _data_model.init (dispatch,
                                           dispatch_data_direct, this,
                                           dispatch_data_read, this);
                TRACE_ERR((stderr, "EagerSimple() [2] status = %d\n", status));

                if (status == XMI_SUCCESS)
                  {
                    status = _ack_model.init (dispatch,
                                              dispatch_ack_direct, this,
                                              dispatch_ack_read, this);
                    TRACE_ERR((stderr, "EagerSimple() [3] status = %d\n", status));
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
            TRACE_ERR((stderr, "EagerSimple::simple_impl() >>\n"));

            // Allocate memory to maintain the state of the send.
            send_state_t * state = allocateSendState ();

            state->cookie   = cookie;
            state->local_fn = local_fn;
            state->eager    = this;

            // Specify the protocol metadata to send with the application
            // metadata in the envelope packet. This metadata is copied
            // into the network by the device and, therefore, can be placed
            // on the stack.
            short_metadata_t metadata;
            metadata.fromRank = _fromRank;
            metadata.bytes    = bytes;

            // Set the acknowledgement information to the virtual address of
            // send state object on the origin task if a local callback of the
            // remote receive completion event is requested. If this is set to
            // NULL no acknowledgement will be received by the origin task.
            if (remote_fn != NULL)
              {
                metadata.ackinfo = state;
                state->remote_fn = remote_fn;
              }
            else
              {
                metadata.ackinfo = NULL;
                state->remote_fn = NULL;
              }

            if (bytes == 0)
              {
                // In the unlikely event that this eager (i.e., multi-packet)
                // protocol is being used to send zero bytes of application
                // data, the envelope message completion must clean up the
                // protocol resources and invoke the application local done
                // callback function, as there will be no data message.
                TRACE_ERR((stderr, "EagerSimple::simple_impl() .. before _envelope_model.postPacket() .. bytes = %zd\n", bytes));
                _envelope_model.postPacket (&(state->msg[0]),
                                            send_complete,
                                            (void *) state,
                                            peer,
                                            (void *) &metadata,
                                            sizeof (short_metadata_t),
                                            msginfo,
                                            mbytes);
              }
            else
              {
                TRACE_ERR((stderr, "EagerSimple::simple_impl() .. before _envelope_model.postPacket() .. bytes = %zd\n", bytes));
                _envelope_model.postPacket (&(state->msg[0]),
                                            NULL,
                                            NULL,
                                            peer,
                                            (void *) &metadata,
                                            sizeof (short_metadata_t),
                                            msginfo,
                                            mbytes);

                TRACE_ERR((stderr, "EagerSimple::simple_impl() .. before _data_model.postPacket()\n"));
                _data_model.postMessage (&(state->msg[1]),
                                         send_complete,
                                         (void *) state,
                                         peer,
                                         (void *) &metadata.fromRank,
                                         sizeof (xmi_task_t),
                                         src,
                                         bytes);
              }

            TRACE_ERR((stderr, "EagerSimple::simple_impl() <<\n"));
            return XMI_SUCCESS;
          };

        protected:
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
            size_t peer = _msgDevice.task2peer (task);
            TRACE_ERR((stderr, ">> EagerSimple::setConnection(%zd, %p) .. _connection[%zd] = %p\n", task, arg, peer, _connection[peer]));
            XMI_assert(_connection[peer] == NULL);
            _connection[peer] = arg;
            TRACE_ERR((stderr, "<< EagerSimple::setConnection(%zd, %p)\n", task, arg));
          }

          inline void * getConnection (xmi_task_t task)
          {
            size_t peer = _msgDevice.task2peer (task);
            TRACE_ERR((stderr, ">> EagerSimple::getConnection(%zd) .. _connection[%zd] = %p\n", task, peer, _connection[peer]));
            XMI_assert(_connection[peer] != NULL);
            TRACE_ERR((stderr, "<< EagerSimple::getConnection(%zd) .. _connection[%zd] = %p\n", task, peer, _connection[peer]));
            return _connection[peer];
          }

          inline void clearConnection (xmi_task_t task)
          {
            size_t peer = _msgDevice.task2peer (task);
            TRACE_ERR((stderr, ">> EagerSimple::clearConnection(%zd) .. _connection[%zd] = %p\n", task, peer, _connection[peer]));
            _connection[peer] = NULL;
            TRACE_ERR((stderr, "<< EagerSimple::clearConnection(%zd) .. _connection[%zd] = %p\n", task, peer, _connection[peer]));
          }


          MemoryAllocator < sizeof(send_state_t), 16 > _send_allocator;
          MemoryAllocator < sizeof(recv_state_t), 16 > _recv_allocator;

          T_Model         _envelope_model;
          T_Model         _data_model;
          T_Model         _ack_model;
          T_Device      & _msgDevice;
          size_t          _pktsize;
          xmi_task_t      _fromRank;

          xmi_context_t   _context;

          xmi_dispatch_callback_fn   _dispatch_fn;
          void                     * _cookie;

          void ** _connection;
          // Support up to 100 unique contexts.
          //static eager_connection_t _eager_connection[];
          EagerConnection<T_Device> _connection_manager;

          size_t      _contextid;

          static int dispatch_ack_direct (void         * metadata,
                                          void         * payload,
                                          size_t         bytes,
                                          void         * recv_func_parm)
          {
            TRACE_ERR((stderr, "EagerSimple::dispatch_ack_direct() >> \n"));
            send_state_t * state = *((send_state_t **) metadata);

            xmi_event_function remote_fn = state->remote_fn;
            void               * cookie  = state->cookie;
            TRACE_ERR((stderr, "EagerSimple::dispatch_ack_direct() .. state = %p, remote_fn = %p\n", state, remote_fn));

            EagerSimple<T_Model, T_Device, T_Message> * eager =
              (EagerSimple<T_Model, T_Device, T_Message> *) recv_func_parm;

            eager->freeSendState (state);

            if (remote_fn) remote_fn (eager->_context, cookie, XMI_SUCCESS);

            TRACE_ERR((stderr, "EagerSimple::dispatch_ack_direct() << \n"));
            return 0;
          }

          static int dispatch_ack_read (void         * metadata,
                                        void         * payload,
                                        size_t         bytes,
                                        void         * recv_func_parm)
          {
#if 0
            send_state_t * state = (send_state_t *) metadata;

            xmi_event_function * local_fn  = state->local_fn;
            xmi_event_function * remote_fn = state->remote_fn;
            void               * cookie    = state->cookie;

            Eager<T_Model, T_Device, T_Message> * eager =
              (Eager<T_Model, T_Device, T_Message> *) recv_func_parm;
            eager->freeSendState (state);

            if (local_fn)  local_fn  (0, cookie, XMI_SUCCESS);

            if (remote_fn) remote_fn (0, cookie, XMI_SUCCESS);

#endif
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
          static int dispatch_envelope_direct (void         * metadata,
                                               void         * payload,
                                               size_t         bytes,
                                               void         * recv_func_parm)
          {
            short_metadata_t * m = (short_metadata_t *) metadata;

            TRACE_ERR ((stderr, ">> EagerSimple::dispatch_envelope_direct(), m->fromRank = %zd, m->bytes = %zd, m->ackinfo = %p\n", m->fromRank, m->bytes, m->ackinfo));

            EagerSimple<T_Model, T_Device, T_Message> * eager =
              (EagerSimple<T_Model, T_Device, T_Message> *) recv_func_parm;

            // Allocate a recv state object!
            recv_state_t * state = eager->allocateRecvState ();
            state->eager = eager;
            state->ackinfo = m->ackinfo;

            // Invoke the registered dispatch function.
            eager->_dispatch_fn.p2p (eager->_context, // Communication context
                                     eager->_contextid,
                                     eager->_cookie,  // Dispatch cookie
                                     m->fromRank,     // Origin (sender) rank
                                     payload,         // Application metadata
                                     bytes,           // Metadata bytes
                                     NULL,            // No payload data
                                     m->bytes,        // Number of msg bytes
                                     (xmi_recv_t *) &(state->info));

            // Only handle simple receives .. until the non-contiguous support
            // is available
            XMI_assert(state->info.kind == XMI_AM_KIND_SIMPLE);

            if (m->bytes == 0) // Move this special case to another dispatch funtion to improve latency in the common case.
              {
                // No data packets will follow this envelope packet. Invoke the
                // recv done callback and, if an acknowledgement packet was
                // requested send the acknowledgement. Otherwise return the recv
                // state memory which was allocated above.
                TRACE_ERR((stderr, "   EagerSimple::dispatch_envelope_direct() .. state->info.local_fn = %p\n", state->info.local_fn));

                if (state->info.local_fn)
                  state->info.local_fn (eager->_context,
                                        state->info.cookie,
                                        XMI_SUCCESS);

                if (state->ackinfo != NULL)
                  {
                    eager->_ack_model.postPacket (&(state->msg),
                                                  receive_complete,
                                                  (void *) state,
                                                  m->fromRank,
                                                  (void *) &(state->ackinfo),
                                                  sizeof (send_state_t *),
                                                  (void *)NULL,
                                                  0);
                  }
                else
                  {
                    eager->freeRecvState (state);
                  }
              }
            else
            {
              state->received = 0;
              state->sndlen = m->bytes;

              // Set the eager connection.
              eager->setConnection (m->fromRank, (void *)state);
            }

            TRACE_ERR ((stderr, "<< EagerSimple::dispatch_envelope_direct()\n"));

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
          static int dispatch_data_direct   (void   * metadata,
                                             void   * payload,
                                             size_t   bytes,
                                             void   * recv_func_parm)
          {
            EagerSimple<T_Model, T_Device, T_Message> * eager =
              (EagerSimple<T_Model, T_Device, T_Message> *) recv_func_parm;

            xmi_task_t fromRank = *((xmi_task_t *)metadata);
            TRACE_ERR((stderr, ">> EagerSimple::dispatch_data_direct(), fromRank = %zd, bytes = %zd\n", fromRank, bytes));

            recv_state_t * state = (recv_state_t *) eager->getConnection (fromRank);
            XMI_assert_debug(state != NULL);

            // Number of bytes received so far.
            size_t nbyte = state->received;

            // Number of bytes left to copy into the destination buffer
            size_t nleft = state->info.data.simple.bytes - nbyte;

            TRACE_ERR((stderr, "   EagerSimple::dispatch_data_direct(), bytes received so far = %zd, bytes yet to receive = %zd, total bytes to receive = %zd, total bytes being sent = %zd\n", state->received, nleft, state->info.data.simple.bytes, state->sndlen));

            if (nleft > 0) // nleft should never be zero .. right?
              {
                // Copy data from the packet buffer into the receive buffer.
                if (nleft < bytes)
                {
                  memcpy ((uint8_t *)(state->info.data.simple.addr) + nbyte, payload, nleft);

                  // Update the receive state to prepate for another data packet.
                  state->received += nleft;
                }
                else
                {
                  memcpy ((uint8_t *)(state->info.data.simple.addr) + nbyte, payload, bytes);

                  // Update the receive state to prepate for another data packet.
                  state->received += bytes;
                }
              }

            TRACE_ERR((stderr, "   EagerSimple::dispatch_data_direct(), nbyte = %zd\n", nbyte));

            if (nbyte+bytes >= state->sndlen)
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

              if (state->ackinfo != NULL)
              {
                // Send an acknowledgement if requested.
                eager->_ack_model.postPacket (&(state->msg),
                                              receive_complete,
                                              (void *) state,
                                              fromRank,
                                              (void *) &(state->ackinfo),
                                              sizeof (send_state_t *),
                                              (void *)NULL,
                                              0);
              }
              else
              {
                // Otherwise, return the receive state object memory to
                // the memory pool.
                eager->freeRecvState (state);
              }
              TRACE_ERR((stderr, "<< dispatch_data_direct(), fromRank = %zd ... receive completed\n", fromRank));
              return 0;
            }

            TRACE_ERR((stderr, "<< dispatch_data_direct(), fromRank = %zd ... wait for more data\n", fromRank));
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
                                             void   * recv_func_parm)
          {
#if 0
            TRACE_ERR((stderr, "(%zd) EagerFactory::dispatch_envelope_read() .. \n", DCMF_Messager_rank()));

            EagerFactory<T_Model, T_Device, T_Message> * pf =
              (EagerFactory<T_Model, T_Device, T_Message> *) recv_func_parm;

            // This packet device DOES NOT provide the data buffer(s) for the
            // message and the data must be read on to the stack before the
            // recv callback is invoked.

            uint8_t stackData[pf->getDevice()->getPacketPayloadSize()];
            void * p = (void *) & stackData[0];
            pf->getDevice()->readData(channel, (char *) p, bytes);

            dispatch_envelope_direct (channel, metadata, p, bytes, recv_func_parm);
#else
            assert(0);
#endif
            return 0;
          };


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
                                             void   * recv_func_parm)
          {
#if 0
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
#else
            assert(0);
#endif
            return 0;
          };

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

            EagerSimple<T_Model, T_Device, T_Message> * eager =
              (EagerSimple<T_Model, T_Device, T_Message> *) state->eager;

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
            EagerSimple<T_Model, T_Device, T_Message> * eager =
              (EagerSimple<T_Model, T_Device, T_Message> *) state->eager;

            eager->freeRecvState (state);

            TRACE_ERR((stderr, "EagerSimple::receive_complete() << \n"));
            return;
          };

        private:

          static inline void compile_time_assert ()
          {
            // This protocol only works with reliable networks.
            COMPILE_TIME_ASSERT(T_Device::reliable_network == true);

            // This protcol only works with deterministic models.
            COMPILE_TIME_ASSERT(T_Model::deterministic == true);

            // Verify that there is enough space for the protocol metadata.
            COMPILE_TIME_ASSERT(sizeof(short_metadata_t) <= T_Device::packet_metadata_size);
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
