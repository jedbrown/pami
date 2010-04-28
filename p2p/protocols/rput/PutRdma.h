/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file p2p/protocols/rput/PutRdma.h
/// \brief Put protocol devices that implement the 'dma' interface.
///
/// The Put class defined in this file uses C++ templates
/// and the "dma" device interface which also uses C++ templates.
///
/// C++ templates require all source code to be #include'd from a header file
/// which can result in a single very large file.
///
#ifndef __p2p_protocols_rput_PutRdma_h__
#define __p2p_protocols_rput_PutRdma_h__

#include <string.h>

#include "p2p/protocols/RPut.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x) // fprintf x
#endif

namespace PAMI
{
  namespace Protocol
  {
    namespace Put
    {
      template <class T_Model, class T_Device>
      class PutRdma : public RPut
      {
        protected:

          typedef uint8_t msg_t[T_Model::dma_model_state_bytes];

          typedef struct put_state
          {
            msg_t                        msg0;
            msg_t                        msg1;
            pami_event_function          fn_no_deallocate;
            pami_event_function          fn_with_deallocate;
            void                       * cookie;
            PutRdma<T_Model, T_Device> * protocol;
          } put_state_t;

          ///
          /// \brief Local put completion event callback.
          ///
          /// This callback will invoke the application local completion
          /// callback function and \b not free the transfer state memory.
          ///
          static void complete_no_deallocate (pami_context_t   context,
                                              void           * cookie,
                                              pami_result_t    result)
          {
            put_state_t * state = (put_state_t *) cookie;

// This if check can be ignored?
            if (state->fn_no_deallocate != NULL)
              {
                state->fn_no_deallocate (context, state->cookie, PAMI_SUCCESS);
              }

            return;
          }

          ///
          /// \brief Local put completion event callback.
          ///
          /// This callback will invoke the application local completion
          /// callback function and free the transfer state memory.
          ///
          static void complete_with_deallocate (pami_context_t   context,
                                                void           * cookie,
                                                pami_result_t    result)
          {
            put_state_t * state = (put_state_t *) cookie;

// This if check can be ignored?
            if (state->fn_with_deallocate != NULL)
              {
                state->fn_with_deallocate (context, state->cookie, PAMI_SUCCESS);
              }

            state->protocol->_allocator.returnObject ((void *) state);

            return;
          }

        public:

          template <class T_Allocator>
          static PutRdma * generate (T_Device      & device,
                                     T_Allocator   & allocator,
                                     pami_result_t & status)
          {
            TRACE_ERR((stderr, ">> PutRdma::generate()\n"));
            COMPILE_TIME_ASSERT(sizeof(PutRdma) <= T_Allocator::objsize);

            PutRdma * put = (PutRdma *) allocator.allocateObject ();
            new ((void *)put) PutRdma (device, status);

            if (status != PAMI_SUCCESS)
              {
                allocator.returnObject (put);
                put = NULL;
              }

            TRACE_ERR((stderr, "<< PutRdma::generate(), put = %p, status = %d\n", put, status));
            return put;
          }


          inline PutRdma (T_Device & device, pami_result_t & status) :
              _model (device, status),
              _context (device.getContext())
          {
            COMPILE_TIME_ASSERT(T_Model::dma_model_mr_supported == true);
          }

          ///
          /// \brief Start a new contiguous rdma put operation
          ///
          inline pami_result_t simple (pami_rput_simple_t * parameters)
          {
            TRACE_ERR((stderr, ">> PutRdma::simple()\n"));
            pami_task_t task;
            size_t offset;
            PAMI_ENDPOINT_INFO(parameters->rma.dest, task, offset);

            TRACE_ERR((stderr, "   PutRdma::simple(), attempt an 'immediate' rput transfer.\n"));
            if (_model.postDmaPut (task, offset,
                                   parameters->rma.bytes,
                                   (Memregion *) parameters->rdma.local.mr,
                                   parameters->rdma.local.offset,
                                   (Memregion *) parameters->rdma.remote.mr,
                                   parameters->rdma.remote.offset))
              {
                TRACE_ERR((stderr, "   PutRdma::simple(), 'immediate' rput transfer was successful, invoke callback.\n"));
                if (parameters->rma.done_fn)
                  parameters->rma.done_fn (_context, parameters->rma.cookie, PAMI_SUCCESS);
                  
                if (parameters->put.rdone_fn)
                {
                  TRACE_ERR((stderr, "   PutRdma::simple(), attempt an 'immediate' rput zero-byte transfer.\n"));
                  if (_model.postDmaPut (task, offset, 0,
                                         (Memregion *) parameters->rdma.local.mr,
                                         parameters->rdma.local.offset,
                                         (Memregion *) parameters->rdma.remote.mr,
                                         parameters->rdma.remote.offset))
                    {
                      parameters->put.rdone_fn (_context, parameters->rma.cookie, PAMI_SUCCESS);
                    }
                  else
                    {
                      TRACE_ERR((stderr, "   PutRdma::simple(), 'immediate' rput zero-byte transfer was not successful, allocate rdma put state memory.\n"));

                      // Allocate memory to maintain the state of the put operation.
                      put_state_t * state =
                        (put_state_t *) _allocator.allocateObject();

                      state->fn_with_deallocate = parameters->put.rdone_fn;
                      state->cookie             = parameters->rma.cookie;
                      state->protocol           = this;

                      TRACE_ERR((stderr, "   PutRdma::simple(), attempt a 'non-blocking' rput zero-byte transfer.\n"));
                      _model.postDmaPut (state->msg0,
                                         complete_with_deallocate,
                                         (void *) state,
                                         task, offset, 0,
                                         (Memregion *) parameters->rdma.local.mr,
                                         parameters->rdma.local.offset,
                                         (Memregion *) parameters->rdma.remote.mr,
                                         parameters->rdma.remote.offset);
                    }
                }

                TRACE_ERR((stderr, "<< PutRdma::simple(), PAMI_SUCCESS\n"));
                return PAMI_SUCCESS;
              }

            TRACE_ERR((stderr, "   PutRdma::simple(), 'immediate' rput transfer was not successful, allocate rdma put state memory.\n"));

            // Allocate memory to maintain the state of the put operation.
            put_state_t * state =
              (put_state_t *) _allocator.allocateObject();

            state->cookie   = parameters->rma.cookie;
            state->protocol = this;

            TRACE_ERR((stderr, "   PutRdma::simple(), attempt a 'non-blocking' rput transfer.\n"));
            if (likely(parameters->rma.done_fn != NULL))
            {
              if (unlikely(parameters->put.rdone_fn != NULL))
              {
                state->fn_no_deallocate = parameters->rma.done_fn;
                _model.postDmaPut (state->msg0,
                                   complete_no_deallocate,
                                   (void *) state,
                                   task, offset,
                                   parameters->rma.bytes,
                                   (Memregion *) parameters->rdma.local.mr,
                                   parameters->rdma.local.offset,
                                   (Memregion *) parameters->rdma.remote.mr,
                                   parameters->rdma.remote.offset);

                // A zero-byte put initiates a fence operation
                state->fn_with_deallocate = parameters->put.rdone_fn;
                _model.postDmaPut (state->msg1,
                                   complete_with_deallocate,
                                   (void *) state,
                                   task, offset, 0,
                                   (Memregion *) parameters->rdma.local.mr,
                                   parameters->rdma.local.offset,
                                   (Memregion *) parameters->rdma.remote.mr,
                                   parameters->rdma.remote.offset);
              }
              else
              {
                state->fn_with_deallocate = parameters->rma.done_fn;
                _model.postDmaPut (state->msg0,
                                   complete_with_deallocate,
                                   (void *) state,
                                   task, offset,
                                   parameters->rma.bytes,
                                   (Memregion *) parameters->rdma.local.mr,
                                   parameters->rdma.local.offset,
                                   (Memregion *) parameters->rdma.remote.mr,
                                   parameters->rdma.remote.offset);
              }
            }
            else
            {
              if (likely(parameters->put.rdone_fn != NULL))
              {
                _model.postDmaPut (state->msg0,
                                   NULL, NULL,
                                   task, offset,
                                   parameters->rma.bytes,
                                   (Memregion *) parameters->rdma.local.mr,
                                   parameters->rdma.local.offset,
                                   (Memregion *) parameters->rdma.remote.mr,
                                   parameters->rdma.remote.offset);

                // A zero-byte put initiates a fence operation
                state->fn_with_deallocate = parameters->put.rdone_fn;
                _model.postDmaPut (state->msg1,
                                   complete_with_deallocate,
                                   (void *) state,
                                   task, offset, 0,
                                   (Memregion *) parameters->rdma.local.mr,
                                   parameters->rdma.local.offset,
                                   (Memregion *) parameters->rdma.remote.mr,
                                   parameters->rdma.remote.offset);
              }
              else
              {
                // huh? no callbacks specified?
              }
            }

            TRACE_ERR((stderr, "<< PutRdma::simple(), PAMI_SUCCESS\n"));
            return PAMI_SUCCESS;
          };

        protected:

          T_Model                                   _model;
          MemoryAllocator < sizeof(put_state), 16 > _allocator;
          pami_context_t                            _context;
      };
    };
  };
};
#undef TRACE_ERR
#endif /* __p2p_protocols_rput_PutRdma_h__ */

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
