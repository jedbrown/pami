/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file p2p/protocols/fence/deterministic/DeterministicFence.h
/// \brief Deterministic fence implementation
///
/// Only deterministically routed communication will be enced by this protocol.
///
#ifndef __p2p_protocols_fence_deterministic_DeterministicFence_h__
#define __p2p_protocols_fence_deterministic_DeterministicFence_h__

#include <pami.h>

#include "p2p/protocols/Fence.h"
#include "components/memory/MemoryAllocator.h"

#include "util/trace.h"

#define DO_TRACE_ENTEREXIT 0
#define DO_TRACE_DEBUG     0

namespace PAMI
{
  namespace Protocol
  {
    namespace Fence
    {
      ///
      /// \brief Deterministic point-to-point fence implementation.
      ///
      template <class T_PacketModel, class T_DmaModel>
      class DeterministicFence : public Fence
      {
        protected:

          typedef DeterministicFence<T_PacketModel, T_DmaModel> Protocol;

          typedef uint8_t packet_model_state_t[T_PacketModel::packet_model_state_bytes];
          typedef uint8_t dma_model_state_t[T_DmaModel::dma_model_state_bytes];
        
          typedef struct 
          {
            pami_endpoint_t origin;
            uintptr_t       cookie;
          } packet_metadata_t;
        
          typedef struct
          {
            unsigned               active;
            pami_event_function    done_fn;
            void                 * cookie;
            void                 * protocol;
            
            struct
            {
              packet_model_state_t state;
              packet_metadata_t    metadata;
            } pkt;
            
            struct
            {
              dma_model_state_t state;
            } dma;
            
          } origin_state_t;
          
          typedef struct
          {
            struct
            {
              packet_model_state_t   state;
              uintptr_t              payload;
              void                 * protocol;
            } pkt;
            
          } target_state_t;
       
          typedef union
          {
            origin_state_t           origin;
            target_state_t           target;
          } fence_state_t;
 

          T_PacketModel                                   _packet_ack_model;
          T_PacketModel                                   _packet_rts_model;
          T_DmaModel                                      _dma_model;
          typename T_PacketModel::Device                & _packet_device;
          typename T_DmaModel::Device                   & _dma_device;
          MemoryAllocator < sizeof(fence_state_t), 16 >   _allocator;
          pami_endpoint_t                                 _origin;
          pami_context_t                                  _context;
          pami_result_t                                   _status;


          inline DeterministicFence () {};

          inline fence_state_t * allocateState ()
          {
            return (fence_state_t *) _allocator.allocateObject ();
          };

          inline void freeState (fence_state_t * object)
          {
            _allocator.returnObject ((void *) object);
          };

          ///
          /// \brief Single-packet 'rts' packet dispatch.
          ///
          /// \see PAMI::Device::Interface::RecvFunction_t
          ///
          static int rts_dispatch (void   * metadata,
                                   void   * payload,
                                   size_t   bytes,
                                   void   * recv_func_parm,
                                   void   * cookie)
          {
            Protocol * protocol = (Protocol *) recv_func_parm;
            uint8_t stack[T_PacketModel::packet_model_payload_bytes];

            if (T_PacketModel::read_is_required_packet_model)
              {
                payload = (void *) & stack[0];
                protocol->_packet_device.read (payload, bytes, cookie);
              }

            packet_metadata_t * rts = (packet_metadata_t *) payload;

            pami_task_t task;
            size_t offset;
            PAMI_ENDPOINT_INFO(rts->origin, task, offset);
            
            // Attempt to send an 'immediate' ack
            struct iovec data[1];
            data[0].iov_base = (void *) & rts->cookie;
            data[0].iov_len  = sizeof(uintptr_t);
            
            bool posted = protocol->_packet_ack_model.postPacket (task, offset,
                                                                  (void *) NULL, 0,
                                                                  data);
                                                                  
            if (unlikely (posted == false))
              {
                // Allocate memory to maintain the state of the send.
                fence_state_t * fence = protocol->allocateState ();
                fence->target.pkt.payload  = rts->cookie;
                fence->target.pkt.protocol = protocol;
                
                protocol->_packet_ack_model.postPacket (fence->target.pkt.state,
                                                        rts_done, fence,
                                                        task, offset,
                                                        (void *) NULL, 0,
                                                        (void *) & fence->target.pkt.payload,
                                                        sizeof (uintptr_t));
              }
              
            return 0;
          };

          ///
          /// \brief Target 'rts' send completion event callback.
          ///
          static void rts_done (pami_context_t   context,
                                void           * cookie,
                                pami_result_t    result)
          {
            fence_state_t * fence = (fence_state_t *) cookie;
            Protocol * protocol = (Protocol *) fence->target.pkt.protocol;
            protocol->freeState (fence);

            return;
          }

          ///
          /// \brief Single-packet 'ack' packet dispatch.
          ///
          /// \see PAMI::Device::Interface::RecvFunction_t
          ///
          static int ack_dispatch (void   * metadata,
                                   void   * payload,
                                   size_t   bytes,
                                   void   * recv_func_parm,
                                   void   * cookie)
          {
            Protocol * protocol = (Protocol *) recv_func_parm;

            uintptr_t ptr = *((uintptr_t *) payload);
            fence_state_t * fence = (fence_state_t *) ptr;

            if (T_PacketModel::read_is_required_packet_model)
              {
                uint8_t stack[T_PacketModel::packet_model_payload_bytes];
                protocol->_packet_device.read ((void *) & stack[0], bytes, cookie);
                uintptr_t ptr = *((uintptr_t *) & stack[0]);
                fence = (fence_state_t *) ptr;
              }
            
            fence->origin.active--;
            
            if (fence->origin.active == 0)
              {
                if (fence->origin.done_fn)
                  fence->origin.done_fn (protocol->_context,
                                         fence->origin.cookie,
                                         PAMI_SUCCESS);

                protocol->freeState (fence);
              }

            return 0;
          };

          static void dma_done (pami_context_t   context,
                                void           * cookie,
                                pami_result_t    result)
          {
            fence_state_t * fence = (fence_state_t *) cookie;
            Protocol * protocol = (Protocol *) fence->origin.protocol;
            
            fence->origin.active--;
            
            if (fence->origin.active == 0)
              {
                if (fence->origin.done_fn)
                  fence->origin.done_fn (protocol->_context,
                                         fence->origin.cookie,
                                         PAMI_SUCCESS);

                protocol->freeState (fence);
              }

            return;
          }
               
        public:

          ///
          /// \brief DeterministicFence class constructor.
          ///
          inline DeterministicFence (typename T_PacketModel::Device & packet_device,
                                     typename T_DmaModel::Device    & dma_device) :
            _packet_ack_model (packet_device),
            _packet_rts_model (packet_device),
            _dma_model (dma_device, _status),
            _packet_device (packet_device),
            _dma_device (dma_device)
          {
            // ----------------------------------------------------------------
            // Compile-time assertions
            // ----------------------------------------------------------------

            // This protocol only works with reliable networks.
            COMPILE_TIME_ASSERT(T_PacketModel::reliable_packet_model == true);

            // This protocol only works with deterministic models.
            COMPILE_TIME_ASSERT(T_PacketModel::deterministic_packet_model == true);
            
            // This protocol only work with fence-enabled dma models.
            COMPILE_TIME_ASSERT(T_DmaModel::dma_model_fence_supported == true);

            // ----------------------------------------------------------------
            // Compile-time assertions (end)
            // ----------------------------------------------------------------
          };

          ///
          /// \brief DeterministicFence class destructor.
          ///
          /// \note Any class with virtual functions must define a virtual
          ///       destructor.
          ///
          virtual ~DeterministicFence () {};

          /// \note This is required to make "C" programs link successfully with virtual destructors
          inline void operator delete (void * p)
          {
            PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
          };

          pami_result_t initialize (size_t                       dispatch,
                                    pami_endpoint_t              origin,
                                    pami_context_t               context)
          {
            pami_result_t status = PAMI_ERROR;

            _origin  = origin;
            _context = context;

            status = _packet_ack_model.init (dispatch, ack_dispatch, this);

            if (status == PAMI_SUCCESS)
              {
                status = _packet_rts_model.init (dispatch, rts_dispatch, this);
              }

            return status;
          };


          ///
          /// \brief Start a new fence operation to a single endpoint
          ///
          /// \param [in] done_fn Completion callback function
          /// \param [in] cookie  Completion callback cookie
          /// \param [in] target  Fence target endpoint
          ///
          virtual pami_result_t endpoint (pami_event_function   done_fn,
                                          void                * cookie,
                                          pami_endpoint_t       target)
          {
            pami_task_t task;
            size_t offset;
            PAMI_ENDPOINT_INFO(target, task, offset);

            // Verify that this task is addressable by the packet device
            if (unlikely(_packet_device.isPeer(task) == false))
              {
                return PAMI_INVAL;
              }
              
            // Assert that the size of the packet payload area is large
            // enough to transfer the packet fence metadata.
            COMPILE_TIME_ASSERT(sizeof(packet_metadata_t) <= T_PacketModel::packet_model_payload_bytes);

            // Allocate memory to maintain the state of the send.
            fence_state_t * fence = allocateState ();
            fence->origin.done_fn             = done_fn;
            fence->origin.cookie              = cookie;
            fence->origin.protocol            = (void *) this;
            fence->origin.pkt.metadata.origin = _origin;
            fence->origin.pkt.metadata.cookie = (uintptr_t) fence;
            
            fence->origin.active              = 2;

            _packet_rts_model.postPacket (fence->origin.pkt.state,
                                          NULL, NULL,
                                          task, offset,
                                          (void *) NULL, 0,
                                          (void *) & fence->origin.pkt.metadata,
                                          sizeof (packet_metadata_t));
                                          
            _dma_model.postDmaFence (fence->origin.dma.state,
                                     dma_done, (void *) fence,
                                     task, offset);
                                          
            return PAMI_SUCCESS;
          };

          ///
          /// \brief Start a new fence operation to all endpoints
          ///
          /// \param [in] done_fn Completion callback function
          /// \param [in] cookie  Completion callback cookie
          ///
          virtual pami_result_t all (pami_event_function   done_fn,
                                     void                * cookie)
          {
            PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
            return PAMI_UNIMPL;
          };

      }; // PAMI::Protocol::Fence::DeterministicFence class
    };   // PAMI::Protocol::Fence namespace
  };     // PAMI::Protocol namespace
};       // PAMI namespace

#undef DO_TRACE_ENTEREXIT
#undef DO_TRACE_DEBUG

#endif // __p2p_protocols_fence_deterministic_DeterministicFence_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
