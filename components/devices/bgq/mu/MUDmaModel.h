/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu/MUDmaModel.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu_MUDmaModel_h__
#define __components_devices_bgq_mu_MUDmaModel_h__

#include <hwi/include/bqc/MU_PacketHeader.h>

#include <spi/include/mu/Descriptor.h>
#include <spi/include/mu/Descriptor_inlines.h>
#include <spi/include/mu/DescriptorBaseXX.h>
#include <spi/include/mu/DescriptorWrapperXX.h>
#include <spi/include/mu/Pt2PtMemoryFIFODescriptorXX.h>
#include <spi/include/mu/Pt2PtRemoteGetDescriptorXX.h>
#include <spi/include/mu/Pt2PtDirectPutDescriptorXX.h>
#include <spi/include/kernel/memory.h>

#include "components/devices/DmaInterface.h"
#include "components/devices/bgq/mu/MUDevice.h"
#include "components/devices/bgq/mu/msg/ShortInjFifoMessage.h"
#include "components/devices/bgq/mu/MUDescriptorWrapper.h"
#include "components/devices/bgq/mu/ResourceManager.h"

#include "Memregion.h"

#ifdef TRACE
#error TRACE already defined!
#else
#define TRACE(x) //fprintf x
#endif

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      template <bool T_Ordered>
      class MUDmaModel : public Interface::DmaModel < MUDmaModel<T_Ordered>, MUDevice, sizeof(ShortInjFifoMessage) >
      {
        public:

          inline MUDmaModel (MUDevice & device, pami_result_t & result) :
              Interface::DmaModel < MUDmaModel<T_Ordered>, MUDevice, sizeof(ShortInjFifoMessage) > (device, result),
              _device (device),
              _wrapper_model (),
              _context (device.getContext())
          {
            result = PAMI_SUCCESS;

            // --------------------------------------------------------------------------
            // Initialize (as much as possible) the rget descriptor model.
            // --------------------------------------------------------------------------
            MUSPI_BaseDescriptorInfoFields_t base;
            base.Pre_Fetch_Only   = MUHWI_DESCRIPTOR_PRE_FETCH_ONLY_NO;
            base.Payload_Address  = 0;
            base.Message_Length   = 0;
            base.Torus_FIFO_Map   = 0;
            base.Dest.Destination.Destination = 0;

            MUSPI_Pt2PtDescriptorInfoFields_t pt2pt;
            pt2pt.Hints_ABCD = 0;
            pt2pt.Misc1      = MUHWI_PACKET_USE_DETERMINISTIC_ROUTING |
                               MUHWI_PACKET_DO_NOT_DEPOSIT |
                               MUHWI_PACKET_DO_NOT_ROUTE_TO_IO_NODE;
            pt2pt.Misc2      = MUHWI_PACKET_VIRTUAL_CHANNEL_DETERMINISTIC;
            pt2pt.Skip       = 0;

            MUSPI_RemoteGetDescriptorInfoFields_t rget;
            rget.Type             = MUHWI_PACKET_TYPE_GET;
            rget.Rget_Inj_FIFO_Id = 0;

            // Set the rget descriptor model.
            _rget_desc_model.setBaseFields (&base);
            _rget_desc_model.setPt2PtFields (&pt2pt);
            _rget_desc_model.setRemoteGetFields( &rget );

#ifdef ENABLE_MAMBO_WORKAROUNDS
            // These are requried in order to work around mambo bugs

            // TODO - Calculate the best torusInjectionFifoMap.
            // For now, hard code to A-minus direction.
            _rget_desc_model.setTorusInjectionFIFOMap (MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AM);

            // TODO - Calculate the best torus hints.
            // For now, hard code to A-minus direction.
            _rget_desc_model.setHints ( MUHWI_PACKET_HINT_AM |
                                        MUHWI_PACKET_HINT_B_NONE |
                                        MUHWI_PACKET_HINT_C_NONE |
                                        MUHWI_PACKET_HINT_D_NONE,
                                        MUHWI_PACKET_HINT_E_NONE );
#endif



            // --------------------------------------------------------------------------
            // Initialize (as much as possible) the direct put descriptor model which
            // will be sent as payload of the rget descriptor.
            //
            // This direct put descriptor specifies a deterministically routed transfer
            // so that a deterministically routed memory fifo transfer can follow and
            // notify the receiving rank (a.k.a. the "origin" rank in a remote get
            // operation) that the direct put has completed
            // --------------------------------------------------------------------------
            MUSPI_DirectPutDescriptorInfoFields rput;
            rput.Rec_Payload_Base_Address_Id = ResourceManager::BAT_DEFAULT_ENTRY_NUMBER;
            rput.Rec_Payload_Offset          = 0;
            rput.Rec_Counter_Base_Address_Id = ResourceManager::BAT_SHAREDCOUNTER_ENTRY_NUMBER;
            rput.Rec_Counter_Offset          = 0;
            rput.Pacing                      = MUHWI_PACKET_DIRECT_PUT_IS_NOT_PACED;

            _rput_desc_model.setBaseFields (&base);
            _rput_desc_model.setPt2PtFields (&pt2pt);
            _rput_desc_model.setDirectPutFields( &rput );

            // The destination of the remote direct put is this (local) rank.
            MUHWI_Destination dst;
            size_t addr[BGQ_TDIMS];
            size_t local_rank = __global.mapping.task ();
            __global.mapping.task2torus (local_rank, addr);
            dst.Destination.A_Destination = addr[0];
            dst.Destination.B_Destination = addr[1];
            dst.Destination.C_Destination = addr[2];
            dst.Destination.D_Destination = addr[3];
            dst.Destination.E_Destination = addr[4];
            _rput_desc_model.setDestination (dst);

            _rput_desc_model.setPt2PtMisc1 (MUHWI_PACKET_USE_DETERMINISTIC_ROUTING |
                                            MUHWI_PACKET_DO_NOT_DEPOSIT |
                                            MUHWI_PACKET_DO_NOT_ROUTE_TO_IO_NODE);

            _rput_desc_model.setPt2PtMisc2 (MUHWI_PACKET_VIRTUAL_CHANNEL_DETERMINISTIC);

            // Use the shared reception counter.
            _rput_desc_model.setRecCounterBaseAddressInfo (1, 0);

#ifdef ENABLE_MAMBO_WORKAROUNDS
            // These are requried in order to work around mambo bugs

            // TODO - Calculate the best torusInjectionFifoMap.
            // For now, hard code to A-minus direction.
            _rput_desc_model.setTorusInjectionFIFOMap (MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AM);

            // TODO - Calculate the best torus hints.
            // For now, hard code to A-minus direction.
            _rput_desc_model.setHints ( MUHWI_PACKET_HINT_AM |
                                        MUHWI_PACKET_HINT_B_NONE |
                                        MUHWI_PACKET_HINT_C_NONE |
                                        MUHWI_PACKET_HINT_D_NONE,
                                        MUHWI_PACKET_HINT_E_NONE );
#endif



            // --------------------------------------------------------------------------
            // Initialize (as much as possible) the memory fifo descriptor model which
            // will be sent as payload of the rget descriptor.
            //
            // See the direct put comments above.
            // --------------------------------------------------------------------------
            MUSPI_MemoryFIFODescriptorInfoFields_t memfifo;
            memfifo.Rec_FIFO_Id       = 0;
            memfifo.Rec_Put_Offset    = 0;
            memfifo.Interrupt         = MUHWI_DESCRIPTOR_DO_NOT_INTERRUPT_ON_PACKET_ARRIVAL;
            memfifo.SoftwareBit       = 1; // Specifies a "single packet" send
            memfifo.SoftwareBytes[0]  = 0;
            memfifo.SoftwareBytes[1]  = 0;
            memfifo.SoftwareBytes[2]  = 0;
            memfifo.SoftwareBytes[3]  = 0;
            memfifo.SoftwareBytes[4]  = 0;
            memfifo.SoftwareBytes[5]  = 0;
            memfifo.SoftwareBytes[6]  = 0;
            memfifo.SoftwareBytes[7]  = 0;
            memfifo.SoftwareBytes[8]  = 0;
            memfifo.SoftwareBytes[9]  = 0;
            memfifo.SoftwareBytes[10] = 0;
            memfifo.SoftwareBytes[11] = 0;
            memfifo.SoftwareBytes[12] = 0;
            memfifo.SoftwareBytes[13] = 0;

            // Register a memfifo dispatch id and write it into the remote memfifo
            // descriptor model.
            uint16_t dispatch_id;
            _device.registerPacketHandler (255,
                                           (Interface::RecvFunction_t) dispatch_notify,
                                           (void *) this, dispatch_id);
            *((uint16_t *)&memfifo.SoftwareBytes[12]) = dispatch_id;

            _rmem_desc_model.setBaseFields (&base);
            _rmem_desc_model.setPt2PtFields (&pt2pt);
            _rmem_desc_model.setMemoryFIFOFields (&memfifo);

            // The destination of the remote memfifo send is this (local) rank.
            _rmem_desc_model.setDestination (dst);

#ifdef ENABLE_MAMBO_WORKAROUNDS
            // These are requried in order to work around mambo bugs

            // TODO - Calculate the best torusInjectionFifoMap.
            // For now, hard code to A-minus direction.
            _rmem_desc_model.setTorusInjectionFIFOMap (MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AM);

            // TODO - Calculate the best torus hints.
            // For now, hard code to A-minus direction.
            _rmem_desc_model.setHints ( MUHWI_PACKET_HINT_AM |
                                        MUHWI_PACKET_HINT_B_NONE |
                                        MUHWI_PACKET_HINT_C_NONE |
                                        MUHWI_PACKET_HINT_D_NONE,
                                        MUHWI_PACKET_HINT_E_NONE );
#endif



            // --------------------------------------------------------------------------
            // Initialize (as much as possible) the direct put descriptor model.
            //
            // This direct put descriptor is different than the "rget dput" descriptor
            // and is used to directly send data from this local rank to that remote rank.
            // --------------------------------------------------------------------------
            _dput_desc_model.setBaseFields (&base);
            _dput_desc_model.setPt2PtFields (&pt2pt);
            _dput_desc_model.setDirectPutFields( &rput );

            _dput_desc_model.setPt2PtMisc1 (MUHWI_PACKET_USE_DETERMINISTIC_ROUTING |
                                            MUHWI_PACKET_DO_NOT_DEPOSIT |
                                            MUHWI_PACKET_DO_NOT_ROUTE_TO_IO_NODE);

            _dput_desc_model.setPt2PtMisc2 (MUHWI_PACKET_VIRTUAL_CHANNEL_DETERMINISTIC);

            // Use the shared reception counter.
            _dput_desc_model.setRecCounterBaseAddressInfo (1, 0);

#ifdef ENABLE_MAMBO_WORKAROUNDS
            // These are requried in order to work around mambo bugs

            // TODO - Calculate the best torusInjectionFifoMap.
            // For now, hard code to A-minus direction.
            _dput_desc_model.setTorusInjectionFIFOMap (MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AM);

            // TODO - Calculate the best torus hints.
            // For now, hard code to A-minus direction.
            _dput_desc_model.setHints ( MUHWI_PACKET_HINT_AM |
                                        MUHWI_PACKET_HINT_B_NONE |
                                        MUHWI_PACKET_HINT_C_NONE |
                                        MUHWI_PACKET_HINT_D_NONE,
                                        MUHWI_PACKET_HINT_E_NONE );
#endif
          };

          inline ~MUDmaModel () {};

          /// \see Device::Interface::DmaModel::getVirtualAddressSupported
          static const size_t dma_model_va_supported = true;

          /// \see Device::Interface::DmaModel::getMemoryRegionSupported
          static const size_t dma_model_mr_supported = true;

          /// \see Device::Interface::DmaModel::getDmaTransferStateBytes
          static const size_t dma_model_state_bytes  = sizeof(ShortInjFifoMessage);

          inline bool postDmaPut_impl (size_t                target_task,
                                       size_t                target_offset,
                                       size_t                bytes,
                                       void                * local,
                                       void                * remote);

          inline bool postDmaPut_impl (uint8_t               (&state)[sizeof(ShortInjFifoMessage)],
                                       pami_event_function   local_fn,
                                       void                * cookie,
                                       size_t                target_task,
                                       size_t                target_offset,
                                       size_t                bytes,
                                       void                * local,
                                       void                * remote);

          inline bool postDmaPut_impl (size_t                target_task,
                                       size_t                target_offset,
                                       size_t                bytes,
                                       Memregion           * local_memregion,
                                       size_t                local_offset,
                                       Memregion           * remote_memregion,
                                       size_t                remote_offset);

          inline bool postDmaPut_impl (uint8_t               (&state)[sizeof(ShortInjFifoMessage)],
                                       pami_event_function   local_fn,
                                       void                * cookie,
                                       size_t                target_task,
                                       size_t                target_offset,
                                       size_t                bytes,
                                       Memregion           * local_memregion,
                                       size_t                local_offset,
                                       Memregion           * remote_memregion,
                                       size_t                remote_offset);

          inline bool postDmaGet_impl (size_t                target_task,
                                       size_t                target_offset,
                                       size_t                bytes,
                                       void                * local,
                                       void                * remote);

          inline bool postDmaGet_impl (uint8_t               (&state)[sizeof(ShortInjFifoMessage)],
                                       pami_event_function   local_fn,
                                       void                * cookie,
                                       size_t                target_task,
                                       size_t                target_offset,
                                       size_t                bytes,
                                       void                * local,
                                       void                * remote);

          inline bool postDmaGet_impl (size_t                target_task,
                                       size_t                target_offset,
                                       size_t                bytes,
                                       Memregion           * local_memregion,
                                       size_t                local_offset,
                                       Memregion           * remote_memregion,
                                       size_t                remote_offset);

          inline bool postDmaGet_impl (uint8_t              (&state)[sizeof(ShortInjFifoMessage)],
                                       pami_event_function   local_fn,
                                       void                * cookie,
                                       size_t                target_task,
                                       size_t                target_offset,
                                       size_t                bytes,
                                       Memregion           * local_memregion,
                                       size_t                local_offset,
                                       Memregion           * remote_memregion,
                                       size_t                remote_offset);

        protected:
          static int dispatch_notify (void   * metadata,
                                      void   * payload,
                                      size_t   bytes,
                                      void   * recv_func_parm)
          {
            pami_callback_t * cb = (pami_callback_t *) metadata;
            TRACE((stderr, ">> MUDmaModel::dispatch_notify(), cb = %p, cb->function = %p, cb->clientdata = %p\n", cb, cb->function, cb->clientdata));
            cb->function (NULL, cb->clientdata, PAMI_SUCCESS);
            TRACE((stderr, "<< MUDmaModel::dispatch_notify()\n"));
            return 0;
          };

          inline void initializeRputDescriptor (MUSPI_DescriptorBase * desc,
                                                uint64_t               remote_dst_pa,
                                                uint64_t               local_src_pa,
                                                size_t                 bytes,
                                                size_t                 target_task);

          inline void initializeRgetDescriptorAndPayload (MUSPI_DescriptorBase * desc,
                                                          MUSPI_DescriptorBase * payload,
                                                          uint64_t               remote_src_pa,
                                                          uint64_t               local_dst_pa,
                                                          size_t                 bytes,
                                                          pami_event_function    local_fn,
                                                          void                 * cookie,
                                                          size_t                 target_task,
                                                          void                 * payloadPa);

        private:
          MUDevice                        & _device;
          MUSPI_Pt2PtDirectPutDescriptor    _dput_desc_model;
          MUSPI_DescriptorWrapper           _wrapper_model; // only needed for direct put?

          MUSPI_Pt2PtRemoteGetDescriptor    _rget_desc_model;
          MUSPI_Pt2PtDirectPutDescriptor    _rput_desc_model;
          MUSPI_Pt2PtMemoryFIFODescriptor   _rmem_desc_model;

          pami_context_t                     _context;
      };

      template <bool T_Ordered>
      bool MUDmaModel<T_Ordered>::postDmaPut_impl (size_t                target_task,
                                                   size_t                target_offset,
                                                   size_t                bytes,
                                                   void                * local,
                                                   void                * remote)
      {
        PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
        return false;
      }

      template <bool T_Ordered>
      bool MUDmaModel<T_Ordered>::postDmaPut_impl (uint8_t               (&state)[sizeof(ShortInjFifoMessage)],
                                                   pami_event_function   local_fn,
                                                   void                * cookie,
                                                   size_t                target_task,
                                                   size_t                target_offset,
                                                   size_t                bytes,
                                                   void                * local,
                                                   void                * remote)
      {
        PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
        return false;
      }

      template <bool T_Ordered>
      void MUDmaModel<T_Ordered>::initializeRputDescriptor (MUSPI_DescriptorBase * desc,
                                                            uint64_t               remote_dst_pa,
                                                            uint64_t               local_src_pa,
                                                            size_t                 bytes,
                                                            size_t                 target_task)
      {
            // --------------------------------------------------------------------
            // Complete the initialization of the direct put descriptor.
            //
            // Clone the direct put descriptor directly into the injection fifo.
            _dput_desc_model.clone (*desc);

            // Set the source buffer address for the direct put.
            desc->setPayload (local_src_pa, bytes);

            // Set the destination buffer address for the direct put.
            desc->setRecPayloadBaseAddressInfo (0, remote_dst_pa);

            // --------------------------------------------------------------------
            // Set the destination torus address and reception fifo.
            // This is terribly inefficient.
            MUHWI_Destination dst;
            size_t addr[BGQ_TDIMS];
            __global.mapping.task2torus (target_task, addr);
            dst.Destination.A_Destination = addr[0];
            dst.Destination.B_Destination = addr[1];
            dst.Destination.C_Destination = addr[2];
            dst.Destination.D_Destination = addr[3];
            dst.Destination.E_Destination = addr[4];
            desc->setDestination (dst);
      }

      template <bool T_Ordered>
      bool MUDmaModel<T_Ordered>::postDmaPut_impl (size_t                target_task,
                                                   size_t                target_offset,
                                                   size_t                bytes,
                                                   Memregion           * local_memregion,
                                                   size_t                local_offset,
                                                   Memregion           * remote_memregion,
                                                   size_t                remote_offset)
      {
        // The MU device *could* immediately complete the put operation
        // *if* the source data is copied into a payload lookaside buffer
        // as is done for immediate packet operations.
        //
        // For now return 'false' until this optimization is implemented.
        return false;
      }

      template <bool T_Ordered>
      bool MUDmaModel<T_Ordered>::postDmaPut_impl (uint8_t               (&state)[sizeof(ShortInjFifoMessage)],
                                                   pami_event_function   local_fn,
                                                   void                * cookie,
                                                   size_t                target_task,
                                                   size_t                target_offset,
                                                   size_t                bytes,
                                                   Memregion           * local_memregion,
                                                   size_t                local_offset,
                                                   Memregion           * remote_memregion,
                                                   size_t                remote_offset)
      {
        TRACE((stderr, ">> MUDmaModel::postDmaPut_impl('memory region')\n"));

        if (bytes == 0)
          {
            // A zero-byte put is defined to be complete after a dma pingpong. This
            // is accomplished via a zero-byte get operation.
            return postDmaGet_impl (state, local_fn, cookie, target_task, target_offset, 0,
                                    local_memregion, local_offset,
                                    remote_memregion, remote_offset);
          }

        MUSPI_InjFifo_t    * injfifo;
        MUHWI_Descriptor_t * hwi_desc;
        void               * payloadVa;
        void               * payloadPa;

        if (_device.nextInjectionDescriptor (target_task,
                                             &injfifo,
                                             &hwi_desc,
                                             &payloadVa,
                                             &payloadPa))
          {
            TRACE((stderr, "   MUDmaModel::postDmaPut_impl('memory region') .. after _device.nextInjectionDescriptor(), injfifo = %p, hwi_desc = %p, payloadVa = %p, payloadPa = %p\n", injfifo, hwi_desc, payloadVa, payloadPa));

            MUSPI_DescriptorBase * desc = (MUSPI_DescriptorBase *) hwi_desc;

            // --------------------------------------------------------------------
            // Complete the initialization of the direct put descriptor.
            //
            // Clone the direct put descriptor directly into the injection fifo.
            _dput_desc_model.clone (*desc);

            // Set the source buffer address for the direct put.
            uint64_t pa = (uint64_t) local_memregion->getBasePhysicalAddress ();
            TRACE((stderr, "   MUDmaModel::postDmaPut_impl('memory region') .. before setPayload(%ld + %zu, %zu)\n", pa, local_offset, bytes));
            desc->setPayload (pa + local_offset, bytes);

            // Set the destination buffer address for the direct put.
            pa = (uint64_t) remote_memregion->getBasePhysicalAddress ();
            desc->setRecPayloadBaseAddressInfo (0, pa + remote_offset);

            // --------------------------------------------------------------------
            // Set the destination torus address and reception fifo.
            // This is terribly inefficient.
            MUHWI_Destination dst;
            size_t addr[BGQ_TDIMS];
            __global.mapping.task2torus (target_task, addr);
            dst.Destination.A_Destination = addr[0];
            dst.Destination.B_Destination = addr[1];
            dst.Destination.C_Destination = addr[2];
            dst.Destination.D_Destination = addr[3];
            dst.Destination.E_Destination = addr[4];
            desc->setDestination (dst);

            // --------------------------------------------------------------------
            // Advance the injection fifo descriptor tail which actually enables
            // the MU hardware to process the descriptor and send the packet
            // on the torus.
            uint64_t sequenceNum = MUSPI_InjFifoAdvanceDesc (injfifo);

            TRACE((stderr, "   MUDmaModel::postDmaPut_impl('memory region') .. after MUSPI_InjFifoAdvanceDesc(), sequenceNum = %ld\n", sequenceNum));

            if (local_fn != NULL)
              {
#ifndef OPTIMIZE_AGGREGATE_LATENCY
                // Check if the descriptor is done.
                TRACE((stderr, "   MUDmaModel::postDmaPut_impl('memory region') .. before MUSPI_CheckDescComplete()\n"));

                uint32_t rc = MUSPI_CheckDescComplete (injfifo, sequenceNum);
                TRACE((stderr, "   MUDmaModel::postDmaPut_impl('memory region') .. after MUSPI_CheckDescComplete(), rc = %d\n", rc));

                if ( rc == 1 )
                  {
                    local_fn (_context, cookie, PAMI_SUCCESS); // Descriptor is done...notify.
                  }
                else
#endif
                  {
                    // The descriptor is not done (or was not checked). Save state
                    // information so that the progress of the decriptor can be checked
                    // later and the callback will be invoked when the descriptor is
                    // complete.
                    InjFifoMessage * msg = (InjFifoMessage *) state;
                    new (msg) InjFifoMessage (local_fn, cookie, _context, sequenceNum);

                    // Queue it.
                    _device.addToDoneQ (target_task, msg->getWrapper());
                  }
              }
          }
        else
          {
            // Construct a message and post to the device to be processed later.
            InjFifoMessage * msg = (InjFifoMessage *) state;
            new (msg) InjFifoMessage ();

            MUSPI_DescriptorBase * desc = (MUSPI_DescriptorBase *) msg->getDescriptor();

            uint64_t remote_dst_pa =
              ((uint64_t) remote_memregion->getBasePhysicalAddress ()) + remote_offset;
            uint64_t local_src_pa =
              ((uint64_t) local_memregion->getBasePhysicalAddress ()) + local_offset;

            initializeRputDescriptor (desc, remote_dst_pa, local_src_pa,
                                      bytes, target_task);

            // Add this message to the send queue to be processed when there is
            // space available in the injection fifo.
            _device.addToSendQ (target_task, (Queue::Element *)msg);
          }

        return true;
      };

      template <bool T_Ordered>
      bool MUDmaModel<T_Ordered>::postDmaGet_impl (size_t                target_task,
                                                   size_t                target_offset,
                                                   size_t                bytes,
                                                   void                * local,
                                                   void                * remote)
      {
        // get via MU can never be completed "immediately".
        return false;
      }

      template <bool T_Ordered>
      bool MUDmaModel<T_Ordered>::postDmaGet_impl (uint8_t               (&state)[sizeof(ShortInjFifoMessage)],
                                                   pami_event_function   local_fn,
                                                   void                * cookie,
                                                   size_t                target_task,
                                                   size_t                target_offset,
                                                   size_t                bytes,
                                                   void                * local,
                                                   void                * remote)
      {
        PAMI_abortf("%s<%d>\n", __FILE__, __LINE__);
        return false;
      }

      template <bool T_Ordered>
      void MUDmaModel<T_Ordered>::initializeRgetDescriptorAndPayload (MUSPI_DescriptorBase * desc,
                                                                      MUSPI_DescriptorBase * payload,
                                                                      uint64_t               remote_src_pa,
                                                                      uint64_t               local_dst_pa,
                                                                      size_t                 bytes,
                                                                      pami_event_function    local_fn,
                                                                      void                 * cookie,
                                                                      size_t                 target_task,
                                                                      void                 * payloadPa)
      {
            // --------------------------------------------------------------------
            // Complete the initialization of the direct put descriptor (payload).
            //
            // Clone the direct put descriptor into the packet payload.
            _rput_desc_model.clone (payload[0]);

            // Set the payload of the direct put descriptor to be the physical
            // address of the source buffer on the remote node (from the user's
            // memory region).
            payload[0].setPayload (remote_src_pa, bytes);

            // Set the destination buffer address for the remote direct put.
            payload[0].setRecPayloadBaseAddressInfo (0, local_dst_pa);

#ifdef MU_GET_COMPLETION_VIA_MEMFIFO_REFLECT
            // --------------------------------------------------------------------
            // Complete the initialization of the memory fifo descriptor (payload).
            //
            // Clone the memory fifo descriptor into the packet payload.
            _rmem_desc_model.clone (payload[1]);

            // Set the user callback information in the unused portion of the
            // packet header for a single packet transfer. The remote memfifo send
            // operation will contain 0 bytes of data in the payload because all
            // information is being packed in the header.
            pami_callback_t * callback = (pami_callback_t *) & payload[1].PacketHeader.messageUnitHeader.Packet_Types.Memory_FIFO.Put_Offset_LSB;
            callback->function   = local_fn;
            callback->clientdata = cookie;

#ifdef ENABLE_MAMBO_WORKAROUNDS
            TRACE((stderr, "   MUDmaModel::postDmaGet_impl('memory region') .. MAMBO HACK .. set payload (%p) and bytes(1) for remote memory fifo descriptor.\n", (void *)local_pa));
            payload[1].setPayload (local_pa, 1);
#endif
#endif
            // --------------------------------------------------------------------
            // Complete the initialization of the remote get descriptor.
            //
            // Clone the model descriptor.
            _rget_desc_model.clone (*desc);

            // Set the destination torus address and reception fifo.
            // This is terribly inefficient.
            MUHWI_Destination dst;
            size_t addr[BGQ_TDIMS];
            __global.mapping.task2torus (target_task, addr);
            dst.Destination.A_Destination = addr[0];
            dst.Destination.B_Destination = addr[1];
            dst.Destination.C_Destination = addr[2];
            dst.Destination.D_Destination = addr[3];
            dst.Destination.E_Destination = addr[4];
            desc->setDestination (dst);

            // Set the appropriate rget inj fifo id based on the channel and rank.
            uint16_t id = _device.getRgetInjFifoId (target_task);
            TRACE((stderr, "   MUDmaModel::postDmaGet_impl('memory region') .. after _device.getRgetInjFifoId(%d), id = %d\n", target_task, id));
            desc->setRemoteGetInjFIFOId (id);

            // Set the payload of the remote get descriptor.
            if (bytes == 0 && local_fn != NULL)
              {
#ifdef MU_GET_COMPLETION_VIA_MEMFIFO_REFLECT
                // Perform a memfifo "reflect" operation.
                desc->setPayload ((uint64_t)payloadPa + sizeof (MUHWI_Descriptor_t), sizeof (MUHWI_Descriptor_t));
#else
                PAMI_abortf("%s<%d> .. counter-based completion notification is unimplemented.\n", __FILE__, __LINE__);
#endif
              }
            else if (local_fn != NULL)
              {
#ifdef MU_GET_COMPLETION_VIA_MEMFIFO_REFLECT
                // Send both the direct put and memfifo "reflect" descriptors.
                desc->setPayload ((uint64_t)payloadPa, sizeof (MUHWI_Descriptor_t) + sizeof (MUHWI_Descriptor_t));
#else
                PAMI_abortf("%s<%d> .. counter-based completion notification is unimplemented.\n", __FILE__, __LINE__);
#endif
              }
            else
              {
                // Do not send the memfifo descriptor since the user did not request
                // completion notification.
                desc->setPayload ((uint64_t)payloadPa, sizeof (MUHWI_Descriptor_t));
              }
      }


      template <bool T_Ordered>
      bool MUDmaModel<T_Ordered>::postDmaGet_impl (size_t                target_task,
                                                   size_t                target_offset,
                                                   size_t                bytes,
                                                   Memregion           * local_memregion,
                                                   size_t                local_offset,
                                                   Memregion           * remote_memregion,
                                                   size_t                remote_offset)
      {
        // rget via MU can never be completed "immediately".
        return false;
      }


      template <bool T_Ordered>
      bool MUDmaModel<T_Ordered>::postDmaGet_impl (uint8_t              (&state)[sizeof(ShortInjFifoMessage)],
                                                   pami_event_function   local_fn,
                                                   void                * cookie,
                                                   size_t                target_task,
                                                   size_t                target_offset,
                                                   size_t                bytes,
                                                   Memregion           * local_memregion,
                                                   size_t                local_offset,
                                                   Memregion           * remote_memregion,
                                                   size_t                remote_offset)
      {
        TRACE((stderr, ">> MUDmaModel::postDmaGet_impl('memory region')\n"));

        uint64_t remote_src_pa = (uint64_t) remote_memregion->getBasePhysicalAddress ();
        remote_src_pa += remote_offset;

        uint64_t local_dst_pa = (uint64_t) local_memregion->getBasePhysicalAddress ();
        local_dst_pa += local_offset;

        MUSPI_InjFifo_t    * injfifo;
        MUHWI_Descriptor_t * hwi_desc;
        void               * payloadVa;
        void               * payloadPa;

        if (_device.nextInjectionDescriptor (target_task,
                                             &injfifo,
                                             &hwi_desc,
                                             &payloadVa,
                                             &payloadPa))
          {
            TRACE((stderr, "   MUDmaModel::postDmaGet_impl('memory region') .. after _device.nextInjectionDescriptor(), injfifo = %p, hwi_desc = %p, payloadVa = %p, payloadPa = %p\n", injfifo, hwi_desc, payloadVa, payloadPa));

            MUSPI_DescriptorBase * desc = (MUSPI_DescriptorBase *) hwi_desc;
            MUSPI_DescriptorBase * payload_desc = (MUSPI_DescriptorBase *) payloadVa;

            initializeRgetDescriptorAndPayload (desc, payload_desc,
                                                remote_src_pa, local_dst_pa,
                                                bytes, local_fn, cookie, target_task,
                                                payloadPa);

            // --------------------------------------------------------------------
            // Advance the injection fifo descriptor tail which actually enables
            // the MU hardware to process the descriptor and send the packet
            // on the torus.
            MUSPI_InjFifoAdvanceDesc (injfifo);
          }
        else
          {
            // Construct a message and post to the device to be processed later.
            ShortInjFifoMessage * msg = (ShortInjFifoMessage *) state;
            new (msg) ShortInjFifoMessage ();


            MUSPI_DescriptorBase * desc = (MUSPI_DescriptorBase *) msg->getDescriptor();
            MUSPI_DescriptorBase * payload_desc = (MUSPI_DescriptorBase *) msg->getPayload();

            Kernel_MemoryRegion_t kmr;
            Kernel_CreateMemoryRegion (&kmr, (void *) payload_desc, sizeof (MUSPI_DescriptorBase) * 2);

            initializeRgetDescriptorAndPayload (desc, payload_desc,
                                                remote_src_pa, local_dst_pa,
                                                bytes, local_fn, cookie, target_task,
                                                kmr.BasePa);

            // Add this message to the send queue to be processed when there is
            // space available in the injection fifo.
            _device.addToSendQ (target_task, (Queue::Element *)msg);
          }

        TRACE((stderr, "<< MUDmaModel::postDmaGet_impl('memory region')\n"));
        return true;

      }; // PAMI::Device::MU::MUDmaModel class
    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace

#undef TRACE

#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
