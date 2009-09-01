/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file p2p/protocol/send/eager/EagerSimple.h
/// \brief Simple eager send protocol for reliable devices.
///
/// The EagerSimple class defined in this file uses C++ templates and
/// the device "message" interface - which also uses C++ templates.
///
/// C++ templates require all source code to be #include'd from a header file.
///
#ifndef __xmi_p2p_protocol_send_eager_eagersimple_h__
#define __xmi_p2p_protocol_send_eager_eagersimple_h__

#include "../Simple.h"

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
      /// \brief Eager simple send protocol class.
      ///
      /// \param T_Model   Template packet model class
      /// \param T_Device  Template packet device class
      ///
      /// \see Packet::Model
      /// \see Packet::Device
      ///
      template <class T_Model, class T_Device, class T_Message>
      class EagerSimple : public XMI::Protocol::Send::Simple
      {
        protected:

          typedef struct __attribute__((__packed__)) short_metadata
          {
            size_t   fromRank;
            size_t   bytes;
            uint8_t  metasize;
        } short_metadata_t;

          typedef struct send_state
          {
            T_Message            msg[2];
            xmi_event_function * local_fn;
            xmi_event_function * remote_fn;
            void               * cookie;
          } send_state_t;

          typedef struct recv_state
          {
            xmi_recv_t info;     ///< Application receive information.
            size_t     received; ///< Number of bytes received.
            size_t     sndlen;   ///< Number of bytes being sent from the origin rank.
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
          /// \param[out] status       Constructor status
          ///
          inline EagerSimple (xmi_dispatch_t             dispatch,
                              xmi_dispatch_callback_fn   dispatch_fn,
                              void                     * cookie,
                              T_Device                 & device,
                              size_t                     origin_task,
                              xmi_result_t             & status) :
              XMI::Protocol::Send::Simple (),
              _envelope_model (device),
              _data_model (device),
              _ack_model (device),
              _msgDevice (device),
              _pktsize (device.getPacketPayloadSize ()),
              _fromRank (origin_task),
              _dispatch_fn (dispatch_fn),
              _cookie (cookie)
          {
            size_t mbytes = device->getPacketMetadataSize ();
            TRACE_ERR((stderr, "Eager(), sizeof(short_metadata_t) = %zd, mbytes = %zd, _pktsize = %zd\n", sizeof(short_metadata_t), mbytes, _pktsize));

            if (mbytes < sizeof(short_metadata_t))
              {
                // Not handled right now !!
                assert(0);
                return;
              }

            //_cb_null.function   = NULL;
            //_cb_null.clientdata = NULL;

            TRACE_ERR((stderr, "Eager() [0] status = %d\n", status));
            status = _envelope_model.init (dispatch_envelope_direct, this,
                                           dispatch_envelope_read, this);
            TRACE_ERR((stderr, "Eager() [1] success = %d\n", success));

            if (status == XMI_SUCCESS)
              {
                status = _data_model.init (dispatch_data_direct, this,
                                            dispatch_data_read, this);
                TRACE_ERR((stderr, "Eager() [2] success = %d\n", success));

                if (status == XMI_SUCCESS)
                  {
                    status = _ack_model.init (dispatch_ack_direct, this,
                                               dispatch_ack_read, this);
                    TRACE_ERR((stderr, "Eager() [3] success = %d\n", success));

                    //if (status == XMI_SUCCESS) status = XMI_SUCCESS;

                    //return;
                  }
              }

            //status = XMI_ERROR;
          }

          ///
          /// \brief Start a new simple send eager message.
          ///
          /// \see XMI::Protocol::Send:Simple::start
          ///
          virtual xmi_result_t start (xmi_event_function   local_fn,
                                      xmi_event_function   remote_fn,
                                      void               * cookie,
                                      size_t               peer,
                                      void               * src,
                                      size_t               bytes,
                                      void               * msginfo,
                                      size_t               mbytes)
          {
            //T_Message * msg = (T_Message *) request;

            short_metadata_t metadata;
            metadata.fromRank = _fromRank;
            metadata.bytes    = bytes;
            //metadata.metasize = mbytes;

            send_state_t * state = allocateSendState ();
            state->local_fn  = local_fn;
            state->remote_fn = remote_fn;
            state->cookie    = cookie;

            _envelope_model.postPacket (&(state->msg[0]),
                                        NULL,
                                        NULL,
                                        peer,
                                        (void *) &metadata,
                                        sizeof (short_metadata_t),
                                        msginfo,
                                        mbytes);

            _data_model.postMessage (&(state->msg[1]),
                                     NULL,
                                     NULL,
                                     peer,
                                     (void *) &metadata.fromRank,
                                     sizeof (size_t),
                                     src,
                                     bytes);

            return XMI_SUCCESS;
          };

          inline T_Device * getDevice()
          {
            return _msgDevice;
          };

        protected:
          inline send_state_t * allocateSendState ()
          {
            return (send_state_t *) _send_allocator[_context].allocateObject ();
          }

          inline void freeSendState (send_state_t * object)
          {
            _send_allocator[_context].freeObject ((void *) object);
          }

          inline recv_state_t * allocateRecvState ()
          {
            return (recv_state_t *) _recv_allocator[_context].allocateObject ();
          }

          inline void freeRecvState (recv_state_t * object)
          {
            _recv_allocator[_context].freeObject ((void *) object);
          }

          /// Maximum of 100 contexts
          static MemoryAllocator < sizeof(send_state_t), 16 > _send_allocator[100];
          static MemoryAllocator < sizeof(recv_state_t), 16 > _recv_allocator[100];

          T_Model         _envelope_model;
          T_Model         _data_model;
          T_Model         _ack_model;
          T_Device      & _msgDevice;
          //DCMF::Mapping & _mapping;
          size_t          _pktsize;
          size_t          _fromRank;

          size_t          _context; // Id .. not object

          xmi_dispatch_callback_fn   _dispatch_fn;
          void                     * _cookie;
          //CM_Callback_t   _cb_null;


          static int dispatch_ack_direct (void         * metadata,
                                          void         * payload,
                                          size_t         bytes,
                                          void         * recv_func_parm)
          {
            send_state_t * state = (send_state_t *) metadata;

            xmi_event_function local_fn  = state->local_fn;
            xmi_event_function remote_fn = state->remote_fn;
            void               * cookie  = state->cookie;

            EagerSimple<T_Model, T_Device, T_Message> * eager =
              (EagerSimple<T_Model, T_Device, T_Message> *) recv_func_parm;
            eager->freeSendState (state);

#warning callbacks must provide the context
            if (local_fn)  local_fn  (0, cookie, XMI_SUCCESS);
            if (remote_fn) remote_fn (0, cookie, XMI_SUCCESS);

            freeSendState (state);

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
          /// \brief Direct multi-packet send envelope packet callback.
          ///
          /// The eager packet factory will register this dispatch function
          /// if and only if the device \b does provide direct access to
          /// data which has already been read from the network by the device.
          ///
          /// The dispatch function will invoke the registered user callback
          /// with the 16-byte (CMQuad) aligned data pointers provided by
          /// the CDI packet device.
          ///
          /// The envelope dispatch function is invoked by the packet device
          /// to process the first packet of a multi-packet message. The CDI
          /// eager protocol sends two iovec elements in the first packet for
          /// the number of data bytes to follow and the msginfo metadata.
          ///
          /// \note The CDI packet device \b must provide 16-byte aligned
          ///       iov_base fields in the iovec structure.
          ///
          /// \see DCMF::CDI::RecvFunction_t
          ///
          static int dispatch_envelope_direct (void         * metadata,
                                               void         * payload,
                                               size_t         bytes,
                                               void         * recv_func_parm)
          {
            short_metadata_t * m = (short_metadata_t *) metadata;

            //TRACE_ERR ((stderr, "(%zd) dispatch_envelope_direct(), m->quads = %d, m->fromRank = %zd, m->bytes = %zd\n", DCMF_Messager_rank(), m->quads, m->fromRank, m->bytes)); fflush(stderr);

            EagerSimple<T_Model, T_Device, T_Message> * eager =
              (EagerSimple<T_Model, T_Device, T_Message> *) recv_func_parm;

            // This packet device provides the data buffer(s) for the message.
            // The dispatch function must provide the (CMQuad) aligned pointer(s)
            // to the registered user callback.
            assert(eager->getDevice()->getConnection (m->fromRank) == NULL);

            // Allocate a recv state object!
            recv_state_t * state = eager->allocateRecvState ();

            eager->dispatch_fn (eager->context,
                                eager->cookie,
                                m->fromRank, // origin rank
                                payload,     // payload contain only msginfo metadata
                                bytes,       // number of bytes of metadata
                                NULL,        // No payload data
                                0,           // No payload data
                                (xmi_recv_t *) state);

            state->received = 0;
            state->sndlen = m->bytes;

            assert(state->recv.kind == XMI_AM_KIND_SIMPLE);

            eager->getDevice()->setConnection (m->fromRank, (void *)state);

            return 0;
          };

          ///
          /// \brief Direct multi-packet send data packet callback.
          ///
          /// Copies incoming packet data from the device to the destination buffer.
          ///
          /// The eager packet factory will register this dispatch function
          /// if and only if the device \b does provide direct access to
          /// data which has already been read from the network by the device.
          ///
          /// The data dispatch function is invoked by the packet device
          /// to process the subsequent packets of a multi-packet message
          /// after the first envelope packet. The CDI eager protocol sends
          /// one iovec element in the data packets for the user data buffer.
          ///
          /// \note The CDI packet device \b must provide 16-byte aligned
          ///       iov_base fields in the iovec structure.
          ///
          /// \see DCMF::CDI::RecvFunction_t
          ///
          static int dispatch_data_direct   (void   * metadata,
                                             void   * payload,
                                             size_t   bytes,
                                             void   * recv_func_parm)
          {
            EagerSimple<T_Model, T_Device, T_Message> * eager =
              (EagerSimple<T_Model, T_Device, T_Message> *) recv_func_parm;

            size_t fromRank = *((size_t *)metadata);
//fprintf (stderr, "dispatch_data_direct(), fromRank = %zd, bytes = %zd\n", fromRank, bytes);

            // This packet device provides the data buffer(s) for the message.
            // The dispatch function must provide the (CMQuad) aligned pointer(s)
            // to the registered user callback.

            recv_state_t * state = (recv_state_t *) eager->getDevice()->getConnection (fromRank);
            assert(state != NULL);

            size_t nbyte = state->received;
            size_t nleft = state->recv.data.simple.bytes - nbyte;
            size_t ncopy = bytes;

//fprintf (stderr, "(%zd) dispatch_data_direct(), fromRank = %zd, state->received = %zd, state->rcvlen = %zd, nleft = %zd, bytes = %zd\n", DCMF_Messager_rank(), fromRank, state->received, state->rcvlen, nleft, bytes);
            if (nleft > 0)
              {
                // Copy data from the packet buffer into the receive buffer.
                if (nleft < ncopy) ncopy = nleft;

                ncopy = nleft > bytes ? bytes : nleft;
                memcpy (state->recv.data.simple.addr + nbyte, payload, ncopy);
              }

//fprintf (stderr, "dispatch_data_direct(), nbyte = %zd, ncopy = %zd\n", nbyte, ncopy);
            if ((nbyte + ncopy) == state->sndlen)
              {
//fprintf (stderr, "dispatch_data_direct(), setConnection = NULL\n");
                eager->getDevice()->setConnection (fromRank, NULL);


//fprintf (stderr, "dispatch_data_direct(),  after setConnection\n");
                if (state->recv.local_fn) state->recv.local_fn (0, state->recv.cookie, XMI_SUCCESS);

                // Send the acknowledgement.
                eager->_ack_model.postPacket (&(state->msg),
                                       receive_complete,
                                       (void *) state,
                                       fromRank,
                                       (void *) &(state->origin_object),
                                       sizeof (void *),
                                       NULL,
                                       0);

                return 0;
              }

            state->received += ncopy;

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

          static void receive_complete (xmi_context_t   context,
                                        void          * cookie,
                                        xmi_result_t    result)
          {
            recv_state_t * state = (recv_state_t *) cookie;
            EagerSimple<T_Model, T_Device, T_Message> * eager =
              (EagerSimple<T_Model, T_Device, T_Message> *) state->factory;

            eager->freeRecvState (state);

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
