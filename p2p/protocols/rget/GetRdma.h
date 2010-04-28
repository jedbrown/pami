/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file p2p/protocols/rget/GetRdma.h
/// \brief Get protocol devices that implement the 'dma' interface.
///
/// The Get class defined in this file uses C++ templates
/// and the "dma" device interface which also uses C++ templates.
///
/// C++ templates require all source code to be #include'd from a header file
/// which can result in a single very large file.
///
#ifndef __p2p_protocols_rget_GetRdma_h__
#define __p2p_protocols_rget_GetRdma_h__

#include <string.h>

#include "p2p/protocols/RGet.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x) // fprintf x
#endif

namespace PAMI
{
  namespace Protocol
  {
    namespace Get
    {
      template <class T_Model, class T_Device>
      class GetRdma : public RGet
      {
        protected:

          typedef uint8_t msg_t[T_Model::dma_model_state_bytes];

          typedef struct get_state
          {
            msg_t                        msg;
            pami_event_function          fn;
            void                       * cookie;
            GetRdma<T_Model, T_Device> * protocol;
          } get_state_t;

          ///
          /// \brief Local get completion event callback.
          ///
          /// This callback will invoke the application local completion
          /// callback function and free the transfer state memory.
          ///
          static void complete (pami_context_t   context,
                                void           * cookie,
                                pami_result_t    result)
          {
            get_state_t * state = (get_state_t *) cookie;

            if (state->fn != NULL)
              {
                state->fn (context, state->cookie, PAMI_SUCCESS);
              }

            state->protocol->_allocator.returnObject ((void *) state);

            return;
          }

        public:

          template <class T_Allocator>
          static GetRdma * generate (T_Device      & device,
                                     T_Allocator   & allocator,
                                     pami_result_t & status)
          {
            TRACE_ERR((stderr, ">> GetRdma::generate()\n"));
            COMPILE_TIME_ASSERT(sizeof(GetRdma) <= T_Allocator::objsize);

            GetRdma * get = (GetRdma *) allocator.allocateObject ();
            new ((void *)get) GetRdma (device, status);

            if (status != PAMI_SUCCESS)
              {
                allocator.returnObject (get);
                get = NULL;
              }

            TRACE_ERR((stderr, "<< GetRdma::generate(), get = %p, status = %d\n", get, status));
            return get;
          }


          inline GetRdma (T_Device & device, pami_result_t & status) :
              _model (device, status),
              _context (device.getContext())
          {
            COMPILE_TIME_ASSERT(T_Model::dma_model_mr_supported == true);
          }

          ///
          /// \brief Start a new contiguous rdma get operation
          ///
          inline pami_result_t simple (pami_rget_simple_t * parameters)
          {
            TRACE_ERR((stderr, ">> GetRdma::simple()\n"));
            pami_task_t task;
            size_t offset;
            PAMI_ENDPOINT_INFO(parameters->rma.dest, task, offset);

            TRACE_ERR((stderr, "   GetRdma::simple(), attempt an 'immediate' rget transfer.\n"));
            if (_model.postDmaGet (task, offset,
                                   parameters->rma.bytes,
                                   (Memregion *) parameters->rdma.local.mr,
                                   parameters->rdma.local.offset,
                                   (Memregion *) parameters->rdma.remote.mr,
                                   parameters->rdma.remote.offset))
              {
                TRACE_ERR((stderr, "   GetRdma::simple(), 'immediate' rget transfer was successful, invoke callback.\n"));
                if (parameters->rma.done_fn)
                  parameters->rma.done_fn (_context, parameters->rma.cookie, PAMI_SUCCESS);

                TRACE_ERR((stderr, "<< GetRdma::simple(), PAMI_SUCCESS\n"));
                return PAMI_SUCCESS;
              }

            TRACE_ERR((stderr, "   GetRdma::simple(), 'immediate' rget transfer was not successful, allocate rdma get state memory.\n"));

            // Allocate memory to maintain the state of the get operation.
            get_state_t * state =
              (get_state_t *) _allocator.allocateObject();

            state->fn       = parameters->rma.done_fn;
            state->cookie   = parameters->rma.cookie;
            state->protocol = this;

            TRACE_ERR((stderr, "   GetRdma::simple(), attempt a 'non-blocking' rget transfer.\n"));
            _model.postDmaGet (state->msg,
                               complete,
                               (void *) state,
                               task, offset,
                               parameters->rma.bytes,
                               (Memregion *) parameters->rdma.local.mr,
                               parameters->rdma.local.offset,
                               (Memregion *) parameters->rdma.remote.mr,
                               parameters->rdma.remote.offset);

            TRACE_ERR((stderr, "<< GetRdma::simple(), PAMI_SUCCESS\n"));
            return PAMI_SUCCESS;
          };

        protected:

          T_Model                                   _model;
          MemoryAllocator < sizeof(get_state), 16 > _allocator;
          pami_context_t                            _context;
      };
    };
  };
};
#undef TRACE_ERR
#endif /* __p2p_protocols_rget_GetRdma_h__ */

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
