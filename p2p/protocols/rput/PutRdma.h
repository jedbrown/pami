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
      template <class T_Model, class T_Device>
      class PutRdma : public RPut
      {
        protected:

          typedef PutRdma<T_Model, T_Device> Protocol;

          typedef uint8_t model_state_t[T_Model::dma_model_state_bytes];

          typedef struct
          {
            model_state_t         state;
            pami_event_function   fn;
            void                * cookie;
            Protocol            * protocol;
          } state_t;

          void start_remotefn (pami_task_t           task,
                               size_t                offset,
                               pami_event_function   fn,
                               void                * cookie,
                               Memregion           * local_mr,
                               Memregion           * remote_mr)
          {
            TRACE_FN_ENTER();

            // Allocate memory to maintain the state of the put operation.
            state_t * put =
              (state_t *) _allocator.allocateObject();

            put->fn       = fn;
            put->cookie   = cookie;
            put->protocol = this;

            _model.postDmaPut (put->state,
                               complete_remotefn, (void *) put,
                               task, offset, 0,
                               local_mr, 0, remote_mr, 0);

            TRACE_FN_EXIT();
          };

          static void complete_remotefn (pami_context_t   context,
                                         void           * cookie,
                                         pami_result_t    result)
          {
            TRACE_FN_ENTER();

            state_t * put = (state_t *) cookie;

            put->fn (context, put->cookie, PAMI_SUCCESS);
            put->protocol->_allocator.returnObject ((void *) put);

            TRACE_FN_EXIT();
          };

          inline void start_simple (pami_task_t           task,
                                    size_t                offset,
                                    size_t                bytes,
                                    Memregion           * local_mr,
                                    size_t                local_offset,
                                    Memregion           * remote_mr,
                                    size_t                remote_offset,
                                    pami_event_function   done_fn,
                                    void                * cookie)
          {
            TRACE_FN_ENTER();

            if (_model.postDmaPut (task, offset, bytes,
                                   local_mr, local_offset,
                                   remote_mr, remote_offset))
              {
                if (done_fn)
                  done_fn (_context, cookie, PAMI_SUCCESS);
              }
            else
              {
                state_t * put =
                  (state_t *) _allocator.allocateObject();

                put->fn       = done_fn;
                put->cookie   = cookie;
                put->protocol = this;

                _model.postDmaPut (put->state,
                                   complete_simple, (void *) put,
                                   task, offset, bytes,
                                   local_mr, local_offset,
                                   remote_mr, remote_offset);
              }

            TRACE_FN_EXIT();
          };

          static void complete_simple (pami_context_t   context,
                                       void           * cookie,
                                       pami_result_t    result)
          {
            TRACE_FN_ENTER();

            state_t * put = (state_t *) cookie;

            if (put->fn)
              put->fn (context, put->cookie, PAMI_SUCCESS);

            put->protocol->_allocator.returnObject ((void *) put);

            TRACE_FN_EXIT();
          };


        public:

          template <class T_MemoryManager>
          static PutRdma * generate (T_Device        & device,
                                     pami_context_t    context,
                                     T_MemoryManager * mm,
                                     pami_result_t   & status)
          {
            TRACE_FN_ENTER();

            void * protocol = NULL;
            status = mm->memalign((void **) & protocol, 16, sizeof(PutRdma));
            PAMI_assert_alwaysf(status == PAMI_SUCCESS, "Failed to get memory for rdma put protocol");
            new (protocol) PutRdma (device, context, status);

            TRACE_FN_EXIT();

            return (PutRdma *) protocol;;
          }


          inline PutRdma (T_Device & device, pami_context_t context, pami_result_t & status) :
              _model (device, status),
              _device (device),
              _context (context)
          {
            TRACE_FN_ENTER();

            COMPILE_TIME_ASSERT(T_Model::dma_model_mr_supported == true);

            TRACE_FN_EXIT();
          }

          ///
          /// \brief Start a new contiguous rdma put operation
          ///
          inline pami_result_t simple (pami_rput_simple_t * parameters)
          {
            TRACE_FN_ENTER();

            pami_task_t task;
            size_t offset;
            PAMI_ENDPOINT_INFO(parameters->rma.dest, task, offset);

            // Verify that this task is addressable by this dma device
            if (unlikely(_device.isPeer(task) == false)) return PAMI_ERROR;

            start_simple (task, offset,
                          parameters->rma.bytes,
                          (Memregion *) parameters->rdma.local.mr,
                          parameters->rdma.local.offset,
                          (Memregion *) parameters->rdma.remote.mr,
                          parameters->rdma.remote.offset,
                          parameters->rma.done_fn,
                          parameters->rma.cookie);

            if (unlikely(parameters->put.rdone_fn != NULL))
              start_remotefn (task, offset,
                              parameters->put.rdone_fn,
                              parameters->rma.cookie,
                          (Memregion *) parameters->rdma.local.mr,
                          (Memregion *) parameters->rdma.remote.mr);

            TRACE_FN_EXIT();

            return PAMI_SUCCESS;
          };

        protected:

          T_Model                                   _model;
          T_Device                                & _device;
          MemoryAllocator < sizeof(state_t), 16 > _allocator;
          pami_context_t                            _context;
      };
    };
  };
};
#undef DO_TRACE_ENTEREXIT
#undef DO_TRACE_DEBUG

#endif /* __p2p_protocols_rput_PutRdma_h__ */

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
