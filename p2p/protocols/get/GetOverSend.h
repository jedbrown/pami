/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file p2p/protocols/get/GetOverSend.h
/// \brief Get protocol devices that implement the 'packet' interface.
///
/// The GetOverSend class defined in this file uses C++ templates
/// and the "packet" device interface which also uses C++ templates.
///
/// C++ templates require all source code to be #include'd from a header file
/// which can result in a single very large file.
///
#ifndef __p2p_protocols_get_GetOverSend_h__
#define __p2p_protocols_get_GetOverSend_h__

#include <string.h>

#include "pami.h"

#include "components/devices/PacketInterface.h"
#include "components/memory/MemoryAllocator.h"
#include "math/Memcpy.x.h"
#include "p2p/protocols/Get.h"

#include "util/trace.h"
#undef  DO_TRACE_ENTEREXIT
#undef  DO_TRACE_DEBUG
#define DO_TRACE_ENTEREXIT 1
#define DO_TRACE_DEBUG     1

namespace PAMI
{
  namespace Protocol
  {
    namespace Get
    {
      template <class T_Model>
      class GetOverSend : public Get
      {
        protected:

          typedef GetOverSend<T_Model> GetOverSendProtocol;

          typedef uint8_t msg_t[T_Model::packet_model_state_bytes];

          typedef struct
          {
            void                * src;      ///< src buffer virtual address on target
            void                * state;    ///< state object virtual address on origin
            size_t                bytes;    ///< bytes to transfer
            size_t                origin_task;
            size_t                origin_offset; 
          } rts_t;

          typedef struct 
          {
            msg_t                 msg;
            rts_t                 rts;
            
            uint8_t             * dst;
            size_t                bytes_remaining;

            pami_event_function   fn;       ///< completion function
            void                * cookie;   ///< completion cookie
            
            GetOverSendProtocol * protocol;

          } state_t;


        public:

          template <class T_Device>
          inline GetOverSend (T_Device & device) :
              _data_model (device),
              _rts_model (device)
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
            // enough to transfer the virtual address of the origin state object.
            // This is used by postMultiPacket() to transfer data messages.
            COMPILE_TIME_ASSERT(sizeof(void *) <= T_Model::packet_model_multi_metadata_bytes);

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

            _context     = context;

            PAMI_ENDPOINT_INFO(origin, _origin_task, _origin_offset);

            pami_result_t status = PAMI_ERROR;

            // The models must be registered in reverse order of use in case
            // the remote side is delayed in its registrations and must save
            // unexpected packets until dispatch registration.

            TRACE_FORMAT("register data model dispatch %zu", dispatch);
            status = _data_model.init (dispatch, dispatch_data_fn, this);
            TRACE_STRING("data model registration successful.");

            if (status == PAMI_SUCCESS)
              {
                TRACE_FORMAT("register rts model dispatch %zu", dispatch);
                status = _rts_model.init (dispatch, dispatch_rts_fn, this);
                TRACE_STRING("rts model registration successful.");
              }

            TRACE_FN_EXIT();
            return status;
          }

          ///
          /// \brief Start a new contiguous get operation
          ///
          virtual pami_result_t get (pami_get_simple_t * parameters)
          {
            TRACE_FN_ENTER();

            pami_task_t task;
            size_t offset;
            PAMI_ENDPOINT_INFO(parameters->rma.dest, task, offset);

            // Verify that this task is addressable by this packet device
            if (unlikely(_rts_model.device.isPeer (task) == false))
              {
                errno = EHOSTUNREACH;
                TRACE_FN_EXIT();
                return PAMI_CHECK_ERRNO;
              }

            // Allocate memory to maintain the origin state of the operation.
            state_t * state = (state_t *) _allocator.allocateObject();
            TRACE_FORMAT("state = %p", state);
            
            state->rts.src           = parameters->addr.remote;
            state->rts.bytes         = parameters->rma.bytes;
            state->rts.state         = (void *) state;
            state->rts.origin_task   = _origin_task;
            state->rts.origin_offset = _origin_offset;
            
            state->dst               = (uint8_t *) parameters->addr.local;
            state->bytes_remaining   = parameters->rma.bytes;
            
            state->fn                = parameters->rma.done_fn;
            state->cookie            = parameters->rma.cookie;
            
            _rts_model.postPacket (state->msg,
                                   NULL, NULL,
                                   task, offset,
                                   (void *) NULL, 0,
                                   (void *) &(state->rts),
                                   sizeof (rts_t));

            TRACE_FN_EXIT();
            return PAMI_SUCCESS;
          };

        protected:

          static void target_done (pami_context_t   context,
                                   void           * cookie,
                                   pami_result_t    result)
          {
            TRACE_FN_ENTER();
            TRACE_FORMAT("cookie = %p", cookie);

            state_t * state = (state_t *) cookie;
            TRACE_FORMAT("state->protocol = %p", state->protocol);
            state->protocol->_allocator.returnObject (cookie);

            TRACE_FN_EXIT();
            return;
          }

          static int dispatch_rts_fn (void   * metadata,
                                      void   * payload,
                                      size_t   bytes,
                                      void   * recv_func_parm,
                                      void   * cookie)
          {
            TRACE_FN_ENTER();

            GetOverSendProtocol * protocol = (GetOverSendProtocol *) recv_func_parm;

            // Allocate memory to maintain the target state of the operation.
            state_t * state = (state_t *) protocol->_allocator.allocateObject();
            TRACE_FORMAT("state = %p", state);

            rts_t * rts = (rts_t *) payload;
            state->protocol = protocol;
            
            protocol->_data_model.postMultiPacket (state->msg,
                                                   target_done, (void *) state,
                                                   rts->origin_task, rts->origin_offset,
                                                   (void *) &(rts->state), sizeof(void *),
                                                   rts->src, rts->bytes);

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

            GetOverSendProtocol * protocol = (GetOverSendProtocol *) recv_func_parm;

            state_t * state = (state_t *) *((state_t **) metadata);

            size_t nbytes = MIN(bytes, state->bytes_remaining);

            if (T_Model::read_is_required_packet_model)
              protocol->_data_model.device.read (state->dst, nbytes, cookie);
            else
              Core_memcpy (state->dst, payload, nbytes);

            state->dst  += nbytes;
            state->bytes_remaining -= nbytes;

            if (state->bytes_remaining == 0)
            {
              TRACE_FORMAT("protocol = %p", protocol);
              if (state->fn != NULL)
              {
                state->fn (protocol->_context, state->cookie, PAMI_SUCCESS);
              }

              protocol->_allocator.returnObject (state);
            }

            TRACE_FN_EXIT();
            return 0;
          }

          T_Model                                 _data_model;
          T_Model                                 _rts_model;
          MemoryAllocator < sizeof(state_t), 16 > _allocator;
          pami_context_t                          _context;
          size_t                                  _origin_task;
          size_t                                  _origin_offset;
      };
    };
  };
};
#undef DO_TRACE_ENTEREXIT
#undef DO_TRACE_DEBUG
#endif // __p2p_protocols_get_GetOverSend_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
