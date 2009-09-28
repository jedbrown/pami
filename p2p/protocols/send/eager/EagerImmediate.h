/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file p2p/protocols/send/eager/EagerImmediate.h
/// \brief Immediate eager send protocol for reliable devices.
///
/// The EagerImmediate class defined in this file uses C++ templates and
/// the device "message" interface - which also uses C++ templates.
///
/// C++ templates require all source code to be #include'd from a header file.
///
#ifndef __xmi_p2p_protocol_send_eager_eagerimmediate_h__
#define __xmi_p2p_protocol_send_eager_eagerimmediate_h__

#include "components/memory/MemoryAllocator.h"

#include "util/common.h"

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
      class EagerImmediate
      {
        protected:

          ///
          /// \brief Sender-side state structure for immediate sends.
          ///
          /// If the immediate post to the device fails due to unavailable
          /// network resources, memory is allocated for this structure to
          /// pack the source metadata and source data into a single
          /// contiguous buffer which can be posted to the device as a
          /// non-blocking transfer.
          ///
          typedef struct send
          {
            uint8_t                     data[T_Device::packet_payload_size]; ///< Packed data
            T_Message                   msg; ///< Device message state object
            EagerImmediate<T_Model,
                           T_Device,
                           T_Message> * pf;  ///< Eager immediate protocol
          } send_t;

          ///
          /// \brief Protocol metadata structure for immediate sends
          ///
          typedef struct __attribute__((__packed__)) protocol_metadata
          {
            size_t         fromRank;  ///< Origin task id
            uint16_t       databytes; ///< Number of bytes of data
            uint16_t       metabytes; ///< Number of bytes of metadata
          } protocol_metadata_t;

        public:

          ///
          /// \brief Eager immediate send protocol constructor.
          ///
          /// \param[in]  dispatch     Dispatch identifier
          /// \param[in]  dispatch_fn  Dispatch callback function
          /// \param[in]  cookie       Opaque application dispatch data
          /// \param[in]  device       Device that implements the message interface
          /// \param[in]  origin_task  Origin task identifier
          /// \param[in]  context      Communication context
          /// \param[out] status       Constructor status
          ///
          inline EagerImmediate (size_t                     dispatch,
                                 xmi_dispatch_callback_fn   dispatch_fn,
                                 void                     * cookie,
                                 T_Device                 & device,
                                 size_t                     origin_task,
                                 xmi_context_t              context,
                                 xmi_result_t             & status) :
              _send_model (device, context),
              _fromRank (origin_task),
              _context (context),
              _dispatch_fn (dispatch_fn),
              _cookie (cookie)
          {
            TRACE_ERR((stderr, "EagerImmediate() [0]\n"));
            status = _send_model.init (dispatch_send_direct, this,
                                       dispatch_send_read, this);
            TRACE_ERR((stderr, "EagerImmediate() [1] status = %d\n", status));
          }

          ///
          /// \brief Start a new simple send eager operation.
          ///
          /// \see XMI::Protocol::Send::immediate
          ///
          inline xmi_result_t immediate_impl (size_t   peer,
                                       void   * src,
                                       size_t   bytes,
                                       void   * msginfo,
                                       size_t   mbytes)
          {
            TRACE_ERR((stderr, "EagerImmediate::immediate_impl() >>\n"));

            // Specify the protocol metadata to send with the application
            // metadata in the packet. This metadata is copied
            // into the network by the device and, therefore, can be placed
            // on the stack.
            protocol_metadata_t metadata;
            metadata.fromRank  = _fromRank;
            metadata.databytes = bytes;
            metadata.metabytes = mbytes;

            TRACE_ERR((stderr, "EagerImmediate::immediate_impl() .. before _send_model.postPacket() .. bytes = %zd\n", bytes));
            bool posted =
              _send_model.postPacketImmediate (peer,
                                               (void *) &metadata,
                                               sizeof (protocol_metadata_t),
                                               msginfo,
                                               mbytes,
                                               src,
                                               bytes);

            if (!posted)
            {
              // For some reason the packet could not be immediately posted.
              // Allocate memory, copy the user data and metadata, and attempt
              // a regular (non-blocking) post.
              send_t * send = (send_t *) _allocator.allocateObject ();
              send->pf = this;
              memcpy (&(send->data[0]), msginfo, mbytes);
              memcpy (&(send->data[mbytes]), src, bytes);

              _send_model.postPacket (&(send->msg),
                                      send_complete,
                                      send,
                                      peer,
                                      (void *) &metadata,
                                      sizeof (protocol_metadata_t),
                                      &(send->data[0]),
                                      mbytes+bytes);
            }

            TRACE_ERR((stderr, "EagerImmediate::immediate_impl() <<\n"));
            return XMI_SUCCESS;
          };

        protected:

          inline void freeSendState (send_t * object)
          {
            _allocator.returnObject ((void *) object);
          }

          MemoryAllocator < sizeof(send_t), 16 > _allocator;

          T_Model         _send_model;
          size_t          _fromRank;

          xmi_context_t              _context;
          xmi_dispatch_callback_fn   _dispatch_fn;
          void                     * _cookie;

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
          static int dispatch_send_direct (void         * metadata,
                                           void         * payload,
                                           size_t         bytes,
                                           void         * recv_func_parm)
          {
            protocol_metadata_t * m = (protocol_metadata_t *) metadata;

            TRACE_ERR ((stderr, "dispatch_send_direct(), m->fromRank = %zd, m->databytes = %zd, m->metabytes = %zd\n", m->fromRank, m->databytes, m->metabytes));

            EagerImmediate<T_Model, T_Device, T_Message> * send =
              (EagerImmediate<T_Model, T_Device, T_Message> *) recv_func_parm;

            uint8_t * data = (uint8_t *)payload;
            xmi_recv_t recv; // used only to provide a non-null recv object to the dispatch function.


            // Invoke the registered dispatch function.
            send->_dispatch_fn.p2p (send->_context, // Communication context
                                    send->_cookie,  // Dispatch cookie
                                    m->fromRank,    // Origin (sender) rank
                                    (void *) data,  // Application metadata
                                    m->metabytes,   // Metadata bytes
                                    (void *) (data + m->metabytes),            // payload data
                                    m->databytes,   // Payload data bytes
                                    (xmi_recv_t *) &recv);

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
          static int dispatch_send_read (void   * metadata,
                                         void   * payload,
                                         size_t   bytes,
                                         void   * recv_func_parm)
          {
            TRACE_ERR((stderr, "EagerImmediate::dispatch_send_read() .. \n"));
#if 0
            EagerImmediate<T_Model, T_Device, T_Message> * pf =
              (EagerImmediate<T_Model, T_Device, T_Message> *) recv_func_parm;

            // This packet device DOES NOT provide the data buffer(s) for the
            // message and the data must be read on to the stack before the
            // recv callback is invoked.

            uint8_t stackData[pf->getDevice()->getPacketPayloadSize()];
            void * p = (void *) & stackData[0];
            pf->getDevice()->readData((void *) p, bytes);

            dispatch_send_direct (metadata, p, bytes, recv_func_parm);
#else
            XMI_abort();
#endif
            return 0;
          };

          ///
          /// \brief Local send completion event callback.
          ///
          /// This callback will free the send state memory.
          ///
          static void send_complete (xmi_context_t   context,
                                     void          * cookie,
                                     xmi_result_t    result)
          {
            TRACE_ERR((stderr, "EagerImmediate::send_complete() >> \n"));
            send_t * state = (send_t *) cookie;

            EagerImmediate<T_Model, T_Device, T_Message> * pf =
              (EagerImmediate<T_Model, T_Device, T_Message> *) state->pf;

            pf->freeSendState (state);

            TRACE_ERR((stderr, "EagerImmediate::send_complete() << \n"));
            return;
          }

        private:

          static inline void compile_time_assert ()
          {
            // This protocol only works with reliable networks.
            COMPILE_TIME_ASSERT(T_Device::reliable_network == true);

            // This protcol only works with deterministic models.
            COMPILE_TIME_ASSERT(T_Model::deterministic == true);

            // Verify that there is enough space for the protocol metadata.
            COMPILE_TIME_ASSERT(sizeof(protocol_metadata_t) <= T_Device::packet_metadata_size);
          };
      };
    };
  };
};
#undef TRACE_ERR
#endif // __xmi_p2p_protocol_send_eager_eagerimmediate_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
