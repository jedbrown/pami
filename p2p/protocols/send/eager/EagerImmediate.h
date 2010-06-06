/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
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
#ifndef __p2p_protocols_send_eager_EagerImmediate_h__
#define __p2p_protocols_send_eager_EagerImmediate_h__

#include "components/memory/MemoryAllocator.h"

#include "util/common.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x) // fprintf x
#endif

namespace PAMI
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
      /// \see PAMI::Device::Interface::PacketModel
      /// \see PAMI::Device::Interface::PacketDevice
      ///
      template <class T_Model, class T_Device>
      class EagerImmediate
      {
        protected:

          typedef uint8_t pkt_t[T_Model::packet_model_state_bytes];

          ///
          /// \brief Protocol metadata structure for immediate sends
          ///
          typedef struct __attribute__((__packed__)) protocol_metadata
          {
            uint16_t        databytes; ///< Number of bytes of data
            uint16_t        metabytes; ///< Number of bytes of metadata
            pami_endpoint_t origin;    ///< Origin endpoint for transfer  
          } protocol_metadata_t;

          ///
          /// \brief Shadow the \c pami_send_immediate_t parameter structure
          ///
          /// This allows the header+data iovec elements to be treated as a
          /// two-element array of iovec structures, and therefore allows the
          /// packet model to implement template specialization.
          ///
          typedef struct
          {
            struct iovec iov[2];
          } parameters_iov_t;

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
            uint8_t                    data[T_Model::packet_model_payload_bytes]; ///< Packed data
            pkt_t                      pkt;
            protocol_metadata_t        metadata;
            EagerImmediate<T_Model,
                           T_Device> * pf;  ///< Eager immediate protocol
          } send_t;

        public:

          ///
          /// \brief Eager immediate send protocol constructor.
          ///
          /// \param[in]  dispatch     Dispatch identifier
          /// \param[in]  dispatch_fn  Dispatch callback function
          /// \param[in]  cookie       Opaque application dispatch data
          /// \param[in]  origin       Origin endpoint
          /// \param[in]  device       Device that implements the message interface
          /// \param[out] status       Constructor status
          ///
          inline EagerImmediate (size_t                      dispatch,
                                 pami_dispatch_callback_fn   dispatch_fn,
                                 void                      * cookie,
                                 T_Device                  & device,
                                 pami_endpoint_t             origin,
                                 pami_result_t             & status) :
              _send_model (device),
              _context (device.getContext()),
              _dispatch_fn (dispatch_fn),
              _cookie (cookie),
              _device (device),
              _origin (origin)
          {
            // ----------------------------------------------------------------
            // Compile-time assertions
            // ----------------------------------------------------------------

            // This protocol only works with reliable networks.
            COMPILE_TIME_ASSERT(T_Model::reliable_packet_model == true);

            // This protcol only works with deterministic models.
            COMPILE_TIME_ASSERT(T_Model::deterministic_packet_model == true);

            // Ensure there is enough space in the packet metadata
            COMPILE_TIME_ASSERT(sizeof(protocol_metadata_t) <= T_Model::packet_model_metadata_bytes);

            // ----------------------------------------------------------------
            // Compile-time assertions
            // ----------------------------------------------------------------

            TRACE_ERR((stderr, "EagerImmediate() register 'immediate' model\n"));
            status = _send_model.init (dispatch,
                                       dispatch_send_direct, this,
                                       dispatch_send_read, this);
            TRACE_ERR((stderr, "EagerImmediate() 'immediate' model status = %d\n", status));
          }

          inline pami_result_t immediate_impl (pami_send_immediate_t * parameters)
          {
            TRACE_ERR((stderr, "EagerImmediate::immediate_impl() >>\n"));

            pami_task_t task;
            size_t offset;
            PAMI_ENDPOINT_INFO(parameters->dest,task,offset);

            // Verify that this task is addressable by this packet device
            if (!_device.isPeer(task)) return PAMI_ERROR;

            // Specify the protocol metadata to send with the application
            // metadata in the packet. This metadata is copied
            // into the network by the device and, therefore, can be placed
            // on the stack.
            protocol_metadata_t metadata;
            metadata.databytes = parameters->data.iov_len;
            metadata.metabytes = parameters->header.iov_len;
            metadata.origin    = _origin;

            TRACE_ERR((stderr, "EagerImmediate::immediate_impl() .. before _send_model.postPacket() .. parameters->header.iov_len = %zu, parameters->data.iov_len = %zu dest:%x\n", parameters->header.iov_len, parameters->data.iov_len, parameters->dest));

            TRACE_ERR((stderr, "EagerImmediate::immediate_impl() .. before _send_model.postPacket() .. task = %d, offset = %zu\n", task, offset));

            // This shadow pointer allows template specialization on the iovecs
            parameters_iov_t * const p = (parameters_iov_t *) parameters;

            bool posted =
              _send_model.postPacket (task, offset,
                                      (void *) &metadata,
                                      sizeof (protocol_metadata_t),
                                      p->iov);

            if (unlikely(!posted))
            {
              // For some reason the packet could not be immediately posted.
              // Allocate memory, pack the user data and metadata, and attempt
              // a regular (non-blocking) post.
              TRACE_ERR((stderr, "EagerImmediate::immediate_impl() .. immediate post packet unsuccessful.\n"));

              // Allocate memory for a protocol send state object. A pointer to
              // this object is used as the "cookie" for the send_complete()
              // event function.
              send_t * send = (send_t *) _allocator.allocateObject ();

              // Save a pointer to the protocol in order to later free the
              // protocol send state memory allocated above.
              send->pf = this;

              // Copy the application header and application data into a
              // temporary buffer in the protocol send state object.
              memcpy (&(send->data[0]), parameters->header.iov_base, metadata.metabytes);
              memcpy (&(send->data[metadata.metabytes]), parameters->data.iov_base, metadata.databytes);

              // Copy the metadata off the stack because this stack frame will
              // disappear when this method returns and the model send state
              // will be left pointing to garbage.
              send->metadata.databytes = metadata.databytes;
              send->metadata.metabytes = metadata.metabytes;
              send->metadata.origin    = metadata.origin;

              // Do the send!
              TRACE_ERR((stderr, "EagerImmediate::immediate_impl() .. post packet after failure, dest:0x%08x \n",parameters->dest));

              _send_model.postPacket (send->pkt,
                                      send_complete,
                                      send,
                                      task,
                                      offset,
                                      (void *) &(send->metadata),
                                      sizeof (protocol_metadata_t),
                                      (void *) &(send->data[0]),
                                      metadata.databytes+metadata.metabytes);
            }
            else
            {
              TRACE_ERR((stderr, "EagerImmediate::immediate_impl() .. immediate post packet successful.\n"));
            }

            TRACE_ERR((stderr, "EagerImmediate::immediate_impl() <<\n"));
            return PAMI_SUCCESS;
          };

        protected:

          inline void freeSendState (send_t * object)
          {
            _allocator.returnObject ((void *) object);
          }

          MemoryAllocator < sizeof(send_t), 16 > _allocator;

          T_Model                     _send_model;

          pami_context_t              _context;
          pami_dispatch_callback_fn   _dispatch_fn;
          void                      * _cookie;
          T_Device                  & _device;
          pami_endpoint_t             _origin;

          ///
          /// \brief Direct single-packet send dispatch.
          ///
          /// The eager immediate send protocol will register this dispatch
          /// function if and only if the device \b does provide direct access
          /// to data which has already been read from the network by the
          /// device.
          ///
          /// Protocol metadata, application metadata, and application data
          /// are all delivered as a single contiguous buffer.
          ///
          /// \see PAMI::Device::Interface::RecvFunction_t
          ///
          static int dispatch_send_direct (void   * metadata,
                                           void   * payload,
                                           size_t   bytes,
                                           void   * recv_func_parm,
                                           void   * cookie)
          {
            protocol_metadata_t * const m = (protocol_metadata_t *) metadata;

            TRACE_ERR ((stderr, ">> EagerImmediate::dispatch_send_direct(), m->databytes = %d, m->metabytes = %d\n", m->databytes, m->metabytes));

            EagerImmediate<T_Model, T_Device> * const send =
              (EagerImmediate<T_Model, T_Device> *) recv_func_parm;

            uint8_t * data = (uint8_t *)payload;

            // Invoke the registered dispatch function.
            send->_dispatch_fn.p2p (send->_context,   // Communication context
                                    send->_cookie,    // Dispatch cookie
                                    (void *) data,    // Application metadata
                                    m->metabytes,     // Metadata bytes
                                    (void *) (data + m->metabytes),  // payload data
                                    m->databytes,     // Total number of bytes
                                    m->origin,        // Origin endpoint for the transfer
                                    (pami_recv_t *) NULL);

            TRACE_ERR ((stderr, "<< EagerImmediate::dispatch_send_direct()\n"));
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
                                         void   * recv_func_parm,
                                         void   * cookie)
          {
            TRACE_ERR((stderr, ">> EagerImmediate::dispatch_send_read()\n"));

            EagerImmediate<T_Model, T_Device> * pf =
              (EagerImmediate<T_Model, T_Device> *) recv_func_parm;

            // This packet device DOES NOT provide the data buffer(s) for the
            // message and the data must be read on to the stack before the
            // recv callback is invoked.
            PAMI_assert_debugf(payload == NULL, "The 'read only' packet device did not invoke dispatch with payload == NULL (%p)\n", payload);

            uint8_t stackData[T_Model::packet_model_payload_bytes];
            void * p = (void *) & stackData[0];
            pf->_device.read (p, bytes, cookie);

            dispatch_send_direct (metadata, p, bytes, recv_func_parm, cookie);

            TRACE_ERR((stderr, "<< EagerImmediate::dispatch_send_read()\n"));
            return 0;
          };

          ///
          /// \brief Local send completion event callback.
          ///
          /// This callback will free the send state memory.
          ///
          static void send_complete (pami_context_t   context,
                                     void          * cookie,
                                     pami_result_t    result)
          {
            TRACE_ERR((stderr, "EagerImmediate::send_complete() >> \n"));
            send_t * state = (send_t *) cookie;

            EagerImmediate<T_Model, T_Device> * pf =
              (EagerImmediate<T_Model, T_Device> *) state->pf;

            pf->freeSendState (state);

            TRACE_ERR((stderr, "EagerImmediate::send_complete() << \n"));
            return;
          }
      };
    };
  };
};
#undef TRACE_ERR
#endif // __pami_p2p_protocol_send_eager_eagerimmediate_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
