/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmem/shaddr/BgqShaddrPacketModel.h
 * \brief ???
 */

#ifndef __components_devices_shmem_shaddr_BgqShaddrPacketModel_h__
#define __components_devices_shmem_shaddr_BgqShaddrPacketModel_h__

#include <errno.h>
#include <sys/uio.h>

#include "Arch.h"

#include <pami.h>

#include "components/devices/PacketInterface.h"
#include "components/devices/shmem/ShmemDevice.h"
#include "components/devices/shmem/ShmemPacket.h"
#include "components/devices/shmem/ShmemPacketMessage.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif

namespace PAMI
{
  namespace Device
  {
    namespace Shmem
    {
      template <class T_Device>
      class PhysicalAddressMessage : public PacketMessage<T_Device, PacketWriter<void>, true, false>
      {
        public:

          ///
          /// \brief Payload data for the system shared address packet
          ///
          typedef struct info
          {
            uint64_t paddr; /// Physical address of the "payload"
            uint16_t id;    /// Actual dispatch id to invoke
          } info_t;

          ///
          /// \brief Provide a read-only shared-address global virtual address
          ///        as the payload pointer to the packet dispatch function
          ///
          /// The actual data contained in the packet is the physical address
          /// of the source data and the original dispatch id which identifies
          /// the function and cookie to invoke with the global virtual address
          /// converted from the physical address of the source data.
          ///
          /// \see PAMI::Device::Interface::RecvFunction_t
          ///
          static int intercept_function (void   * metadata,
                                         void   * payload,
                                         size_t   bytes,
                                         void   * recv_func_parm,
                                         void   * cookie)
          {
//fprintf(stderr,"PhysicalAddressMessage::intercept_fn(), recv_func_parm (device*) = %p\n", recv_func_parm);
            info_t * data = (info_t *) payload;
//fprintf(stderr,"PhysicalAddressMessage::intercept_fn(), data->paddr = %ld, data->id = %d\n", data->paddr, data->id);

            void * vaddr = NULL;
            Kernel_Physical2GlobalVirtual ((void *)(data->paddr), &vaddr);

//fprintf(stderr,"PhysicalAddressMessage::intercept_fn(), paddr (%ld) -> vaddr (%p)\n", data->paddr, vaddr);
            T_Device * device = (T_Device *) recv_func_parm;
            device->dispatch (data->id, metadata, vaddr, (size_t)-1);

            return 0;
          }

          ///
          /// \brief Noop completion function to spoof a "required" completion function
          ///
          static void noop_function (pami_context_t context, void * cookie, pami_result_t status)
          {
            return;
          };

          inline PhysicalAddressMessage (pami_event_function   local_fn,
                                         void                * cookie,
                                         T_Device            * device,
                                         size_t                fnum,
                                         uint16_t              dispatch,
                                         void                * metadata,
                                         size_t                metasize,
                                         void                * payload,
                                         size_t                length,
                                         uint16_t              id) :
              PacketMessage<T_Device, PacketWriter<void>, true, false>
                (local_fn==NULL?noop_function:local_fn, cookie, device, fnum, dispatch, metadata, metasize, (void *) &_info, sizeof(info_t))
          {
            _info.id = id;

            // ====================================================
            // Determine the physical address of the source buffer.
            //
            uint32_t rc;
            Kernel_MemoryRegion_t memregion;
            rc = Kernel_CreateMemoryRegion (&memregion, payload, length);
            PAMI_assert ( rc == 0 );

            uint64_t offset = (uint64_t)payload - (uint64_t)memregion.BaseVa;
            _info.paddr = (uint64_t)memregion.BasePa + offset;
            //
            // ====================================================
          };

          inline ~PhysicalAddressMessage () {};

          info_t _info;
      };

      ///
      /// \brief Packet model interface implementation for shared memory.
      ///
      /// The shared memory packet model contains the shared memory
      /// packet header which is block-copied into a shared memory message
      /// when it is generated.
      ///
      /// The model also registers the dispatch function with the shared
      /// memory device and stores the dispatch id in the packet header.
      ///
      /// \see ShmemPacketDevice
      ///
      template <class T_Device>
      class BgqShaddrPacketModel : public Interface::PacketModel < BgqShaddrPacketModel<T_Device> >
      {
        public:
          ///
          /// \brief Construct a shared memory device packet model.
          ///
          /// \param [in] device  Shared memory device
          ///
          BgqShaddrPacketModel (T_Device & device) :
              Interface::PacketModel < BgqShaddrPacketModel<T_Device> > (device),
              _shmem_model (device),
              device (device)
          {
            //COMPILE_TIME_ASSERT(sizeof(Shmem::PacketMessage<T_Device>) == sizeof(MultiPacketMessage<T_Device>));
          };

          static const bool   reliable_packet_model             = T_Device::reliable;
          static const bool   deterministic_packet_model        = T_Device::deterministic;
          static const bool   read_is_required_packet_model     = false;

          static const size_t packet_model_metadata_bytes       = T_Device::metadata_size;
          static const size_t packet_model_multi_metadata_bytes = T_Device::metadata_size;
          static const size_t packet_model_payload_bytes        = T_Device::payload_size;
          static const size_t packet_model_immediate_bytes      = T_Device::payload_size;
          static const size_t packet_model_state_bytes          = PacketModel<T_Device>::packet_model_state_bytes;

          pami_result_t init_impl (size_t                      dispatch,
                                   Interface::RecvFunction_t   recv_func,
                                   void                      * recv_func_parm)
          {
//fprintf(stderr, "BgqShaddrPacketModel::init_impl(%zu, %p, %p, %p, %p)\n", dispatch, direct_recv_func, direct_recv_func_parm, read_recv_func, read_recv_func_parm);
            pami_result_t status = PAMI_ERROR;

//fprintf(stderr, "BgqShaddrPacketModel::init_impl(), device.shaddr._shaddr_packet_dispatch = %d, &device = %p\n", device.shaddr._shaddr_packet_dispatch, &device);
            if (device.system_ro_put_dispatch != (uint16_t) -1)
              status = PAMI_SUCCESS;
            else
              status = device.registerSystemRecvFunction (PhysicalAddressMessage<T_Device>::intercept_function,
                                                           &device,
                                                           device.system_ro_put_dispatch);
            if (status == PAMI_SUCCESS)
              return _shmem_model.init (dispatch,
                                        recv_func,
                                        recv_func_parm);

            return status;
          };

          template <unsigned T_StateBytes>
          inline bool postPacket_impl (uint8_t               (&state)[T_StateBytes],
                                       pami_event_function   fn,
                                       void                * cookie,
                                       size_t                target_task,
                                       size_t                target_offset,
                                       void                * metadata,
                                       size_t                metasize,
                                       struct iovec_t      * iov,
                                       size_t                niov)
          {
            TRACE_ERR((stderr, ">> PacketModel::postPacket_impl(1)\n"));

            bool result =
              _shmem_model.postPacket (state, fn, cookie,
                                       target_task, target_offset,
                                       metadata, metasize,
                                       iov, niov);

            TRACE_ERR((stderr, "<< PacketModel::postPacket_impl(1), return %d\n", result));
            return result;
          };

          template <unsigned T_StateBytes, unsigned T_Niov>
          inline bool postPacket_impl (uint8_t               (&state)[T_StateBytes],
                                       pami_event_function   fn,
                                       void                * cookie,
                                       size_t                target_task,
                                       size_t                target_offset,
                                       void                * metadata,
                                       size_t                metasize,
                                       struct iovec          (&iov)[T_Niov])
          {
            TRACE_ERR((stderr, ">> PacketModel::postPacket_impl(2), T_Niov = %d\n", T_Niov));

            bool result =
              _shmem_model.postPacket (state, fn, cookie,
                                       target_task, target_offset,
                                       metadata, metasize, iov);

            TRACE_ERR((stderr, "<< PacketModel::postPacket_impl(2), return %d\n", result));
            return result;
          };

          template <unsigned T_StateBytes>
          inline bool postPacket_impl (uint8_t              (&state)[T_StateBytes],
                                       pami_event_function  fn,
                                       void               * cookie,
                                       size_t               target_task,
                                       size_t               target_offset,
                                       void               * metadata,
                                       size_t               metasize,
                                       void               * payload,
                                       size_t               length)
          {
            TRACE_ERR((stderr, ">> PacketModel::postPacket_impl(0)\n"));

            COMPILE_TIME_ASSERT(sizeof(PhysicalAddressMessage<T_Device>) <= packet_model_state_bytes);

            size_t fnum = device.fnum (device.task2peer(target_task), target_offset);

            COMPILE_TIME_ASSERT(sizeof(PhysicalAddressMessage<T_Device>) <= packet_model_state_bytes);
            PhysicalAddressMessage<T_Device> * msg = (PhysicalAddressMessage<T_Device> *) state;
            new (msg) PhysicalAddressMessage<T_Device> (fn, cookie, &device, fnum,
                                                        device.system_ro_put_dispatch,
                                                        metadata, metasize, payload, length,
                                                        _shmem_model.getDispatchId());

            if (unlikely(device.isSendQueueEmpty (fnum)))
              {
                if (likely(device._fifo[fnum].producePacket(msg->_writer)))
                  {
                    if (unlikely(fn == NULL)) return false;

                    // Create a "completion message" on the done queue and wait
                    // until the target task has completed the put operation.
                    COMPILE_TIME_ASSERT(T_Device::completion_work_size <= packet_model_state_bytes);

                    size_t sequence = device._fifo[fnum].lastPacketProduced();
                    array_t<uint8_t, T_Device::completion_work_size> * resized =
                      (array_t<uint8_t, T_Device::completion_work_size> *) state;

                    device.postCompletion (resized->array, fn, cookie, fnum, sequence);

                    TRACE_ERR((stderr, "<< PacketModel::postPacket_impl(0), return false\n"));
                    return false;
                  }
              }

            // Send queue is not empty or not all packets were written to the
            // fifo. Construct a message and post to device
            TRACE_ERR((stderr, "   PacketModel::postPacket_impl(0), post message to device\n"));

            device.post (fnum, msg);

            TRACE_ERR((stderr, "<< PacketModel::postPacket_impl(0), return false\n"));
            return false;
          };

          template <unsigned T_Niov>
          inline bool postPacket_impl (size_t         target_task,
                                       size_t         target_offset,
                                       void         * metadata,
                                       size_t         metasize,
                                       struct iovec   (&iov)[T_Niov])
          {
            TRACE_ERR((stderr, ">> PacketModel::postPacket_impl(\"immediate\")\n"));

            bool result =
              _shmem_model.postPacket (target_task, target_offset,
                                       metadata, metasize, iov);

            TRACE_ERR((stderr, "<< PacketModel::postPacket_impl(\"immediate\"), return %d\n", result));
            return result;
          };

          template <unsigned T_StateBytes>
          inline bool postMultiPacket_impl (uint8_t               (&state)[T_StateBytes],
                                            pami_event_function   fn,
                                            void                * cookie,
                                            size_t                target_task,
                                            size_t                target_offset,
                                            void                * metadata,
                                            size_t                metasize,
                                            void                * payload,
                                            size_t                length)
          {
            TRACE_ERR((stderr, ">> PacketModel::postMultiPacket_impl()\n"));

            bool result = postPacket_impl (state, fn, cookie,
                                           target_task, target_offset,
                                           metadata, metasize, payload, length);

            TRACE_ERR((stderr, "<< PacketModel::postMultiPacket_impl(), return %d\n", result));
            return result;
          };

        protected:

          PAMI::Device::Shmem::PacketModel<T_Device>   _shmem_model;
          
        public:
        
          T_Device                                   & device;

      };  // PAMI::Device::Shmem::BgqShaddrPacketModel class
    };    // PAMI::Device::Shmem namespace
  };      // PAMI::Device namespace
};        // PAMI namespace
#undef TRACE_ERR
#endif // __components_devices_shmem_shaddr_BgqShaddrPacketModel_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
