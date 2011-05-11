/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file p2p/protocols/put/PutOverSend.h
/// \brief Put protocol devices that implement the 'packet' interface.
///
/// The Put class defined in this file uses C++ templates
/// and the "packet" device interface which also uses C++ templates.
///
/// C++ templates require all source code to be #include'd from a header file
/// which can result in a single very large file.
///
#ifndef __p2p_protocols_put_PutOverSend_h__
#define __p2p_protocols_put_PutOverSend_h__

#include <string.h>

#include "pami.h"

#include "components/devices/PacketInterface.h"
#include "components/memory/MemoryAllocator.h"
#include "math/Memcpy.x.h"
#include "p2p/protocols/Put.h"

#include "util/trace.h"
#undef  DO_TRACE_ENTEREXIT
#undef  DO_TRACE_DEBUG
#define DO_TRACE_ENTEREXIT 0
#define DO_TRACE_DEBUG     0

namespace PAMI
{
  namespace Protocol
  {
    namespace Put
    {
      template <class T_Model>
      class PutOverSend : public Put
      {
        protected:

          typedef PutOverSend<T_Model> PutOverSendProtocol;

          typedef uint8_t msg_t[T_Model::packet_model_state_bytes];

          typedef struct
          {
            pami_event_function   remote_fn;
            void                * cookie;
            pami_context_t        context;
            bool                  invoke;
          } ack_info_t;

          typedef struct
          {
            struct iovec    dst;
            pami_endpoint_t origin;
          } envelope_t;

          typedef union
          {
            struct
            {
              uint8_t * addr;
              size_t    bytes;
            } recv;

            struct
            {
              msg_t           msg[2];
              struct iovec    metadata;
              struct iovec    iov[2];
              envelope_t      envelope;
              PutOverSendProtocol * protocol;
              pami_event_function   done_fn;
              void                * cookie;
            } send;

            struct
            {
              msg_t           msg;
              ack_info_t  info;
              PutOverSendProtocol * protocol;
            } ack;

          } state_t;

          static const size_t maximum_short_packet_payload =
            T_Model::packet_model_payload_bytes -
            (sizeof(struct iovec) > T_Model::packet_model_metadata_bytes) * sizeof(struct iovec);


          ///
          /// \brief Local put completion event callback.
          ///
          /// This callback will invoke the application local completion
          /// callback function and free the transfer state memory.
          ///
          static void complete_local (pami_context_t   context,
                                      void           * cookie,
                                      pami_result_t    result)
          {
            TRACE_FN_ENTER();
            TRACE_FORMAT("cookie = %p", cookie);

            state_t * state = (state_t *) cookie;
            TRACE_FORMAT("state->send.done_fn = %p, state->send.cookie = %p", state->send.done_fn, state->send.cookie);

            if (state->send.done_fn != NULL)
              {
                state->send.done_fn (context, state->send.cookie, PAMI_SUCCESS);
              }

            TRACE_FORMAT("state->send.protocol = %p", state->send.protocol);
            state->send.protocol->_allocator.returnObject (cookie);

            TRACE_FN_EXIT();
            return;
          }

        public:

          template <class T_Device>
          inline PutOverSend (T_Device & device) :
              _ack_model (device),
              _short_model (device),
              _envelope_model (device),
              _data_model (device)
          {
            TRACE_FN_ENTER();

            // ----------------------------------------------------------------
            // Compile-time assertions (begin)
            // ----------------------------------------------------------------

            // This protocol only works with reliable networks.
            COMPILE_TIME_ASSERT(T_Model::reliable_packet_model == true);

            // This protocol only works with deterministic models.
            COMPILE_TIME_ASSERT(T_Model::deterministic_packet_model == true);

            // Assert that the size of the packet metadata area is large
            // enough to transfer the connection information. This is used by
            // postMultiPacket() to transfer data messages.
            COMPILE_TIME_ASSERT(sizeof(pami_endpoint_t) <= T_Model::packet_model_multi_metadata_bytes);

            // Assert that the size of the packet payload area is large
            // enough to transfer a single virtual address. This is used in
            // the postPacket() calls to transfer the ack information.
            COMPILE_TIME_ASSERT(sizeof(void *) <= T_Model::packet_model_payload_bytes);

            // ----------------------------------------------------------------
            // Compile-time assertions (end)
            // ----------------------------------------------------------------

            TRACE_FN_EXIT();
          }

          pami_result_t initialize (size_t          dispatch,
                                    pami_endpoint_t origin,
                                    pami_context_t  context)
          {
            TRACE_FN_ENTER();

            _origin      = origin;
            _context     = context;

            pami_result_t status = PAMI_ERROR;

            // The models must be registered in reverse order of use in case
            // the remote side is delayed in its registrations and must save
            // unexpected packets until dispatch registration.

            TRACE_FORMAT("register ack model dispatch %zu", dispatch);
            status = _ack_model.init (dispatch, dispatch_ack_fn, this);
            TRACE_STRING("ack model registration successful.");

            if (status == PAMI_SUCCESS)
              {
                TRACE_FORMAT("register short model dispatch %zu", dispatch);
                status = _short_model.init (dispatch, dispatch_short_fn, this);
                TRACE_STRING("short model registration successful.");

                if (status == PAMI_SUCCESS)
                  {
                    TRACE_FORMAT("register data model dispatch %zu", dispatch);
                    status = _data_model.init (dispatch, dispatch_data_fn, this);
                    TRACE_STRING("data model registration successful.");

                    if (status == PAMI_SUCCESS)
                      {
                        TRACE_FORMAT("register envelope model dispatch %zu", dispatch);
                        status = _envelope_model.init (dispatch, dispatch_envelope_fn, this);
                        TRACE_STRING("envelope model registration successful.");
                      }
                  }
              }

            TRACE_FN_EXIT();
            return status;
          }

          ///
          /// \brief Start a new contiguous put operation
          ///
          virtual pami_result_t simple (pami_put_simple_t * parameters)
          {
            TRACE_FN_ENTER();

            pami_task_t task;
            size_t offset;
            PAMI_ENDPOINT_INFO(parameters->rma.dest, task, offset);

            // Verify that this task is addressable by this packet device
            if (unlikely(_short_model.device.isPeer (task) == false))
              {
                TRACE_FN_EXIT();
                return PAMI_INVAL;
              }

            // Allocate memory to maintain the origin state of the put operation.
            state_t * state = (state_t *) _allocator.allocateObject();
            TRACE_FORMAT("state = %p", state);
            state->send.envelope.dst.iov_base = parameters->addr.remote;
            state->send.envelope.dst.iov_len  = parameters->rma.bytes;
            state->send.done_fn  = parameters->rma.done_fn;
            state->send.cookie   = parameters->rma.cookie;
            state->send.protocol = this;

            // ----------------------------------------------------------------
            // Check for a "short" put protocol
            // ----------------------------------------------------------------
            if (parameters->rma.bytes <= maximum_short_packet_payload)
              {
                TRACE_FORMAT("'short' put protocol: parameters->rma.bytes = %zu, maximum_short_packet_payload = %zu", parameters->rma.bytes, maximum_short_packet_payload);

                if (sizeof(struct iovec) <= T_Model::packet_model_metadata_bytes)
                  {
                    // Send the "destination" virtual address and data length in
                    // the packet header.
                    _short_model.postPacket (state->send.msg[0],
                                             complete_local, state,
                                             task, offset,
                                             (void *) &(state->send.envelope.dst),
                                             sizeof (struct iovec),
                                             parameters->addr.local,
                                             parameters->rma.bytes);
                  }
                else
                  {
                    // Send the "destination" virtual address and data length in
                    // the packet payload.
                    state->send.iov[0].iov_base = &(state->send.envelope.dst);
                    state->send.iov[0].iov_len  = sizeof(struct iovec);
                    state->send.iov[1].iov_base = parameters->addr.local;
                    state->send.iov[1].iov_len  = parameters->rma.bytes;

                    _short_model.postPacket (state->send.msg[0],
                                             complete_local, state,
                                             task, offset,
                                             (void *) NULL, 0,
                                             state->send.iov);
                  }
              }
            else
              {
                TRACE_FORMAT("'long' put protocol: parameters->rma.bytes = %zu, maximum_short_packet_payload = %zu", parameters->rma.bytes, maximum_short_packet_payload);

                // Send the "destination" virtual address and data length, and the
                // origin endpoint to establish an eager-style connection, in the
                // payload of an envelope packet.
                state->send.envelope.origin = _origin;
                _envelope_model.postPacket (state->send.msg[0],
                                            NULL, NULL,
                                            task, offset,
                                            (void *) NULL, 0,
                                            (void *) &(state->send.envelope),
                                            sizeof (envelope_t));

                TRACE_FORMAT("'long' put protocol, before postMultiPacket(): state->send.done_fn = %p, state->send.cookie = %p", state->send.done_fn, state->send.cookie);
                _data_model.postMultiPacket (state->send.msg[1],
                                             complete_local, state,
                                             task, offset,
                                             (void *) &(state->send.envelope.origin),
                                             sizeof(pami_endpoint_t),
                                             parameters->addr.local,
                                             parameters->rma.bytes);
                TRACE_FORMAT("'long' put protocol, after postMultiPacket():  state->send.done_fn = %p, state->send.cookie = %p", state->send.done_fn, state->send.cookie);
              }

            if (unlikely(parameters->put.rdone_fn != NULL))
              {
                send_ack_request (task, offset,
                                  parameters->put.rdone_fn,
                                  parameters->rma.cookie);
              }

            TRACE_FN_EXIT();
            return PAMI_SUCCESS;
          };

        protected:

          static int dispatch_short_fn (void   * metadata,
                                        void   * payload,
                                        size_t   bytes,
                                        void   * recv_func_parm,
                                        void   * cookie)
          {
            TRACE_FN_ENTER();

            PutOverSendProtocol * protocol = (PutOverSendProtocol *) recv_func_parm;

            if (sizeof(struct iovec) <= T_Model::packet_model_metadata_bytes)
              {
                // The "destination" virtual address and data length are located
                // in the packet header.
                struct iovec * iov = (struct iovec *) metadata;

                if (T_Model::read_is_required_packet_model)
                  protocol->_short_model.device.read (iov->iov_base, iov->iov_len, cookie);
                else
                  Core_memcpy (iov->iov_base, payload, iov->iov_len);
              }
            else
              {
                // The "destination" virtual address and data length are located
                // in the packet payload.
                if (T_Model::read_is_required_packet_model)
                  {
                    struct iovec iov;
                    protocol->_short_model.device.read (&iov, sizeof(struct iovec), cookie);
                    protocol->_short_model.device.read (iov.iov_base, iov.iov_len, cookie);
                  }
                else
                  {
                    struct iovec * iov = (struct iovec *) payload;
                    void * src = (void *) (iov + 1);
                    Core_memcpy (iov->iov_base, src, iov->iov_len);
                  }
              }

            TRACE_FN_EXIT();
            return 0;
          }

          static int dispatch_envelope_fn (void   * metadata,
                                           void   * payload,
                                           size_t   bytes,
                                           void   * recv_func_parm,
                                           void   * cookie)
          {
            TRACE_FN_ENTER();

            PutOverSendProtocol * protocol = (PutOverSendProtocol *) recv_func_parm;
            uint8_t stack[T_Model::packet_model_payload_bytes];

            if (T_Model::read_is_required_packet_model)
              {
                payload = (void *) & stack[0];
                protocol->_envelope_model.device.read (payload, bytes, cookie);
              }

            // Allocate memory to maintain the state of the send.
            state_t * state = (state_t *) protocol->_allocator.allocateObject();

            envelope_t * envelope = (envelope_t *) payload;
            state->recv.addr  = (uint8_t *) envelope->dst.iov_base;
            state->recv.bytes = envelope->dst.iov_len;

            // Set the deterministic connection.
            pami_task_t task;
            size_t offset;
            PAMI_ENDPOINT_INFO(envelope->origin, task, offset);
            protocol->_envelope_model.device.setConnection ((void *)state, task, offset);

            TRACE_FN_EXIT();
            return 0;
          }

          static int dispatch_data_fn (void   * metadata,
                                       void   * payload,
                                       size_t   bytes,
                                       void   * recv_func_parm,
                                       void   * cookie)
          {
            TRACE_FN_ENTER();

            PutOverSendProtocol * protocol = (PutOverSendProtocol *) recv_func_parm;

            pami_endpoint_t origin = *((pami_endpoint_t *)metadata);
            pami_task_t task;
            size_t offset;
            PAMI_ENDPOINT_INFO(origin, task, offset);
            state_t * state = (state_t *) protocol->_data_model.device.getConnection (task, offset);

            size_t nbytes = MIN(bytes, state->recv.bytes);

            if (T_Model::read_is_required_packet_model)
              protocol->_data_model.device.read (state->recv.addr, nbytes, cookie);
            else
              Core_memcpy (state->recv.addr, payload, nbytes);

            state->recv.addr  += nbytes;
            state->recv.bytes -= nbytes;

            if (state->recv.bytes == 0)
              protocol->_data_model.device.clearConnection (task, offset);

            TRACE_FN_EXIT();
            return 0;
          }


          // ##################################################################
          // 'acknowledgment' code which provides the 'remote callback'
          // functionality without impacting the normal, performance-critical
          // code path.
          // ##################################################################

          ///
          /// \brief Process an incomming 'ack' packet
          ///
          /// This dispatch function is invoked on both the target and origin
          /// tasks of an acknowledgement communication flow.
          ///
          /// A dispatch on the target task will inject the ack 'pong' packet,
          /// while a dispatch on the origin task will examine the information
          /// in the ack packet and invoke the ack completion event function.
          ///
          /// \see PAMI::Device::Interface::RecvFunction_t
          ///
          static int dispatch_ack_fn (void   * metadata,
                                      void   * payload,
                                      size_t   bytes,
                                      void   * recv_func_parm,
                                      void   * cookie)
          {
            TRACE_FN_ENTER();

            PutOverSendProtocol * protocol = (PutOverSendProtocol *) recv_func_parm;
            uint8_t stack[T_Model::packet_model_payload_bytes];

            if (T_Model::read_is_required_packet_model)
              {
                payload = (void *) & stack[0];
                protocol->_ack_model.device.read (payload, bytes, cookie);
              }

            ack_info_t * info = (ack_info_t *) payload;

            if (info->invoke)
              {
                // This is a 'pong' ack message .. invoke the callback function
                // and return.
                info->remote_fn (info->context, info->cookie, PAMI_SUCCESS);

                TRACE_FN_EXIT();
                return 0;
              }

            pami_task_t task;
            size_t offset;
            PAMI_ENDPOINT_INFO(*((pami_endpoint_t*)metadata), task, offset);

            // Send the ack information back to the origin, however change the
            // value of the 'invoke' field so that the origin dispatch will
            // invoke the event function.
            info->invoke = true;

            // Attempt an 'immediate' packet post. If this fails, then a send
            // state must be allocated for this 'pong' ack message.
            struct iovec iov[1] = {{payload, sizeof(ack_info_t)}};
            bool result = protocol->_ack_model.postPacket(task, offset, NULL, 0, iov);

            if (result != true)
              {
                // Allocate memory to maintain the state of the send.
                state_t * state = (state_t *) protocol->_allocator.allocateObject();

                state->send.protocol = protocol;
                state->ack.info = *info;

                protocol->_ack_model.postPacket (state->send.msg[0],
                                                 ack_done, (void *) state,
                                                 task, offset,
                                                 NULL, 0,
                                                 (void *) &(state->ack.info),
                                                 sizeof(ack_info_t));
              }

            TRACE_FN_EXIT();
            return 0;
          }


          ///
          /// \brief Initiate an independent 'ack' communication flow
          ///
          /// Used only by the ack request originiator.
          ///
          /// \param [in] task   Target task
          /// \param [in] offset Target context offset
          /// \param [in] fn     Event function to invoke after ack completes
          /// \param [in] cookie Event function cookie
          ///
          inline void send_ack_request (pami_task_t task, size_t offset,
                                        pami_event_function fn, void * cookie)
          {
            TRACE_FN_ENTER();

            ack_info_t info;
            info.remote_fn = fn;
            info.cookie    = cookie;
            info.context   = _context;
            info.invoke    = false;

            // It is safe to place this iovec array on the stack as it is only
            // used by the 'immediate' post packet function below.
            struct iovec iov[1] = {{&info, sizeof(ack_info_t)}};

            bool result =
              _ack_model.postPacket (task, offset, &_origin, sizeof(pami_endpoint_t), iov);

            if (result != true)
              {
                // The 'immediate' post packet failed. Allocate memory to
                // maintain the origin state of the ack. The state object is
                // deallocated after the ack request packet is injected.
                state_t * state = (state_t *) _allocator.allocateObject();

                state->ack.protocol = this;
                state->ack.info     = info;

                _ack_model.postPacket (state->ack.msg,
                                       ack_done, (void *) state,
                                       task, offset,
                                       &_origin, sizeof(pami_endpoint_t),
                                       (void *) &(state->ack.info),
                                       sizeof(ack_info_t));
              }

            TRACE_FN_EXIT();
          }

          ///
          /// \brief Deallocate the send object used to inject an ack packet
          ///
          static void ack_done (pami_context_t   context,
                                void           * cookie,
                                pami_result_t    result)
          {
            TRACE_FN_ENTER();

            state_t * state = (state_t *) cookie;
            state->ack.protocol->_allocator.returnObject (cookie);

            TRACE_FN_EXIT();
          }

          // ##################################################################
          // end 'acknowledgement' code section
          // ##################################################################

          T_Model                                   _ack_model;
          T_Model                                   _short_model;
          T_Model                                   _envelope_model;
          T_Model                                   _data_model;
          MemoryAllocator < sizeof(state_t), 16 > _allocator;
          pami_context_t                            _context;
          pami_endpoint_t                           _origin;
      };
    };
  };
};
#undef DO_TRACE_ENTEREXIT
#undef DO_TRACE_DEBUG
#endif // __p2p_protocols_put_PutOverSend_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
