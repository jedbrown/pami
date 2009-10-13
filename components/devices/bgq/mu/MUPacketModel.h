/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu/MUPacketModel.h
 * \brief ???
 */
#ifndef __devices_bgq_mu_packet_model_h__
#define __devices_bgq_mu_packet_model_h__

#include <hwi/include/bqc/MU_PacketHeader.h>

#include <spi/include/mu/DescriptorBaseXX.h>
#include <spi/include/mu/DescriptorWrapperXX.h>
#include <spi/include/mu/Pt2PtMemoryFIFODescriptorXX.h>
#include <spi/include/kernel/memory.h>

#include "components/devices/MessageModel.h"

#include "components/devices/bgq/mu/MUDevice.h"
#include "components/devices/bgq/mu/MUInjFifoMessage.h"
#include "components/devices/bgq/mu/Dispatch.h"
#include "components/devices/bgq/mu/MUDescriptorWrapper.h"

#ifdef TRACE
#undef TRACE
#endif
#define TRACE(x) //fprintf x


//#define OPTIMIZE_AGGREGATE_LATENCY

namespace XMI
{
  namespace Device
  {
    namespace MU
    {
      class MUPacketModel : public Interface::MessageModel<MUPacketModel, MUDevice>
      {
        public:

          /// \see XMI::Device::Interface::PacketModel::PacketModel
          /// \see XMI::Device::Interface::MessageModel::MessageModel
          MUPacketModel (MUDevice & device, xmi_context_t context);

          /// \see XMI::Device::Interface::PacketModel::~PacketModel
          /// \see XMI::Device::Interface::MessageModel::~MessageModel
          ~MUPacketModel ();

          static const bool   deterministic_packet_model   = true;
          static const bool   reliable_packet_model        = true;
          static const size_t packet_model_metadata_bytes  = MUDevice::packet_metadata_size;
          static const size_t packet_model_payload_bytes   = MUDevice::payload_size;
          static const size_t packet_model_state_bytes     = sizeof(MUInjFifoMessage);

          static const bool   deterministic_message_model  = true;
          static const bool   reliable_message_model       = true;
          static const size_t message_model_metadata_bytes = MUDevice::message_metadata_size;
          static const size_t message_model_payload_bytes  = MUDevice::payload_size;
          static const size_t message_model_state_bytes    = sizeof(MUInjFifoMessage);

          /// \see XMI::Device::Interface::PacketModel::init
          xmi_result_t init_impl (size_t                      dispatch,
                                  Interface::RecvFunction_t   direct_recv_func,
                                  void                      * direct_recv_func_parm,
                                  Interface::RecvFunction_t   read_recv_func,
                                  void                      * read_recv_func_parm);

          /// \see XMI::Device::Interface::PacketModel::postPacket
          inline bool postPacket_impl (uint8_t              state[MUPacketModel::packet_model_state_bytes],
                                       xmi_event_function   fn,
                                       void               * cookie,
                                       size_t               target_rank,
                                       void               * metadata,
                                       size_t               metasize,
                                       void               * payload,
                                       size_t               bytes);

          /// \see XMI::Device::Interface::PacketModel::postPacket
          inline bool postPacket_impl (uint8_t              state[MUPacketModel::packet_model_state_bytes],
                                       xmi_event_function   fn,
                                       void               * cookie,
                                       size_t               target_rank,
                                       void               * metadata,
                                       size_t               metasize,
                                       void               * payload0,
                                       size_t               bytes0,
                                       void               * payload1,
                                       size_t               bytes1);

          /// \see XMI::Device::Interface::PacketModel::postPacket
          inline bool postPacket_impl (uint8_t              state[MUPacketModel::packet_model_state_bytes],
                                       xmi_event_function   fn,
                                       void               * cookie,
                                       size_t               target_rank,
                                       void               * metadata,
                                       size_t               metasize,
                                       struct iovec       * iov,
                                       size_t               niov);

          /// \see XMI::Device::Interface::PacketModel::postPacketImmediate
          inline bool postPacketImmediate_impl (size_t   target_rank,
                                                void   * metadata,
                                                size_t   metasize,
                                                void   * payload0,
                                                size_t   bytes0,
                                                void   * payload1,
                                                size_t   bytes1);

          /// \see XMI::Device::Interface::MessageModel::postMessage
          inline bool postMessage_impl (uint8_t              state[MUPacketModel::message_model_state_bytes],
                                        xmi_event_function   fn,
                                        void               * cookie,
                                        size_t               target_rank,
                                        void               * metadata,
                                        size_t               metasize,
                                        void               * payload,
                                        size_t               bytes);

        protected:

          inline void initializeDescriptor (MUSPI_DescriptorBase * desc,
                                            size_t                 target_rank,
                                            uint64_t               payloadPa,
                                            size_t                 bytes);
        private:
          MUDevice                        & _device;
          MUDescriptorWrapper               _wrapper_model;
          MUSPI_Pt2PtMemoryFIFODescriptor   _desc_model;
          xmi_context_t                     _context;
      };

      void MUPacketModel::initializeDescriptor (MUSPI_DescriptorBase * desc,
                                                size_t                 target_rank,
                                                uint64_t               payloadPa,
                                                size_t                 bytes)
      {
        TRACE((stderr, ">> initializeDescriptor(%p, %zd, %p, %zd)\n", desc, target_rank, (void *)payloadPa, bytes));
        // Clone the model descriptor.
        _desc_model.clone (*desc);

        // --------------------------------------------------------------------
        // Set the destination torus address and reception fifo.
        // This is terribly inefficient.
        MUHWI_Destination dst;
        size_t addr[7];
        _device.sysdep->mapping.task2torus ((xmi_task_t)target_rank, addr);
        dst.Destination.A_Destination = addr[0];
        dst.Destination.B_Destination = addr[1];
        dst.Destination.C_Destination = addr[2];
        dst.Destination.D_Destination = addr[3];
        dst.Destination.E_Destination = addr[4];
        desc->setDestination (dst);

        // Assuming t is the recv grp id ... what about 'p' coordintate?
        //TRACE((stderr, "MUPacketModel::initializeDescriptor() .. _device.getRecFifoIdForDescriptor(%zd) = %d\n", network.n_torus.coords[5], _device.getRecFifoIdForDescriptor(network.n_torus.coords[5])));
        desc->setRecFIFOId (_device.getRecFifoIdForDescriptor(addr[5]));

        // TODO - Calculate the best torusInjectionFifoMap.
        // For now, hard code to A-minus direction.
        desc->setTorusInjectionFIFOMap (MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AM);
#if 1
        // TODO - Calculate the best torus hints.
        // For now, hard code to A-minus direction.
        desc->setHints ( MUHWI_PACKET_HINT_AM |
                         MUHWI_PACKET_HINT_B_NONE |
                         MUHWI_PACKET_HINT_C_NONE |
                         MUHWI_PACKET_HINT_D_NONE,
                         MUHWI_PACKET_HINT_E_NONE );
#endif
        // --------------------------------------------------------------------

        TRACE((stderr, "   initializeDescriptor(), before desc->setPayload()\n"));
        //desc->dump();

        desc->setPayload (payloadPa, bytes);

        TRACE((stderr, "   initializeDescriptor(),  after desc->setPayload()\n"));
        //desc->dump();

        TRACE((stderr, "<< initializeDescriptor(%p, %zd, %p, %zd)\n", desc, target_rank, (void *)payloadPa, bytes));
      }

      bool MUPacketModel::postPacket_impl (uint8_t              state[MUPacketModel::packet_model_state_bytes],
                                           xmi_event_function   fn,
                                           void               * cookie,
                                           size_t               target_rank,
                                           void               * metadata,
                                           size_t               metasize,
                                           void               * payload,
                                           size_t               bytes)
      {
        TRACE((stderr, "MUPacketModel::postPacket_impl(1) >> \n"));

        MUSPI_InjFifo_t    * injfifo;
        MUHWI_Descriptor_t * hwi_desc;
        void               * payloadVa;
        void               * payloadPa;

#ifdef ENABLE_MAMBO_WORKAROUNDS
#warning    Mambo can not handle zero-byte payloads.
            if (bytes == 0 || payload == NULL)
            {
              payload = metadata;
              bytes = 1;
            }
#endif

        if (_device.nextInjectionDescriptor (target_rank,
                                             &injfifo,
                                             &hwi_desc,
                                             &payloadVa,
                                             &payloadPa))
          {
            TRACE((stderr, "MUPacketModel::postPacket_impl(1) .. after _device.nextInjectionDescriptor(), injfifo = %p, hwi_desc = %p, payloadVa = %p, payloadPa = %p\n", injfifo, hwi_desc, payloadVa, payloadPa));
            MUSPI_DescriptorBase * desc = (MUSPI_DescriptorBase *) hwi_desc;

            // Initialize the descriptor directly in the injection fifo.
            initializeDescriptor (desc, target_rank, (uint64_t) payloadPa, bytes);

            // Enable the "single packet message" bit.
            desc->setSoftwareBit (1);

            // Pack the input buffer into the packet payload.
            memcpy (payloadVa, payload, bytes);

            // Copy the metadata into the network header in the descriptor.
            if (metasize > 0)
              {
                MemoryFifoPacketHeader_t * hdr =
                  (MemoryFifoPacketHeader_t *) & desc->PacketHeader;
                  
                //XMI_assert(metasize <= _device.getPacketMetadataSize());
                XMI_assert_debugf(metasize <= packet_model_metadata_bytes, "metasize = %zd, packet_model_metadata_bytes = %zd\n", metasize, packet_model_metadata_bytes);
                memcpy((void *) &hdr->dev.singlepkt.metadata, metadata, metasize); // <-- replace with an optimized MUSPI function.
              }

            // Advance the injection fifo descriptor tail which actually enables
            // the MU hardware to process the descriptor and send the packet
            // on the torus.
            TRACE((stderr, "MUPacketModel::postPacket_impl(1) .. before MUSPI_InjFifoAdvanceDesc()\n"));
            desc->dump();
            uint64_t sequenceNum = MUSPI_InjFifoAdvanceDesc (injfifo);
            TRACE((stderr, "MUPacketModel::postPacket_impl(1) .. after MUSPI_InjFifoAdvanceDesc(), sequenceNum = %ld\n", sequenceNum));

            if (fn != NULL)
              {
                fn (_context, cookie, XMI_SUCCESS);
              }
          }
        else
          {
            // Construct a message and post to the device to be processed later.
            MUInjFifoMessage * obj = (MUInjFifoMessage *) state;
            new (obj) MUInjFifoMessage (fn, cookie, _context);
            obj->setSourceBuffer (payload, bytes);

            // Initialize the descriptor directly in the injection fifo.
            MUSPI_DescriptorBase * desc = obj->getDescriptor ();
            initializeDescriptor (desc, target_rank, 0, 0);

            // Copy the metadata into the network header in the descriptor.
            if (metasize > 0)
              {
                MemoryFifoPacketHeader_t * hdr =
                  (MemoryFifoPacketHeader_t *) & (desc->PacketHeader);
                XMI_assert(metasize <= packet_model_metadata_bytes);
                memcpy((void *) &hdr->dev.singlepkt.metadata, metadata, metasize); // <-- replace with an optimized MUSPI function.
              }

            // Add this message to the send queue to be processed when there is
            // space available in the injection fifo.
            _device.addToSendQ (target_rank, (QueueElem *)obj);
          }

        TRACE((stderr, "MUPacketModel::postPacket_impl(1) << \n"));
        return true;
      };


      bool MUPacketModel::postPacket_impl (uint8_t              state[MUPacketModel::packet_model_state_bytes],
                                           xmi_event_function   fn,
                                           void               * cookie,
                                           size_t               target_rank,
                                           void               * metadata,
                                           size_t               metasize,
                                           void               * payload0,
                                           size_t               bytes0,
                                           void               * payload1,
                                           size_t               bytes1)
      {
        TRACE((stderr, "MUPacketModel::postPacket_impl(2) >> \n"));

        MUSPI_InjFifo_t    * injfifo;
        MUHWI_Descriptor_t * hwi_desc;
        void               * payloadVa;
        void               * payloadPa;

#ifdef ENABLE_MAMBO_WORKAROUNDS
#warning    Mambo can not handle zero-byte payloads.
            if (bytes0+bytes1 == 0)
            {
              payload0 = metadata;
              bytes0 = 1;
              payload1 = metadata;
              bytes1 = 1;
            }
#endif

        TRACE((stderr, "MUPacketModel::postPacket_impl(2) .. before nextInjectionDescriptor()\n"));
        if (_device.nextInjectionDescriptor (target_rank,
                                             &injfifo,
                                             &hwi_desc,
                                             &payloadVa,
                                             &payloadPa))
          {
            TRACE((stderr, "MUPacketModel::postPacket_impl(2) .. after _device.nextInjectionDescriptor(), injfifo = %p, hwi_desc = %p, payloadVa = %p, payloadPa = %p\n", injfifo, hwi_desc, payloadVa, payloadPa));
            MUSPI_DescriptorBase * desc = (MUSPI_DescriptorBase *) hwi_desc;

            // Initialize the descriptor directly in the injection fifo.
            initializeDescriptor (desc, target_rank, (uint64_t) payloadPa, bytes0 + bytes1);

            // Enable the "single packet message" bit.
            desc->setSoftwareBit (1);

            // Pack the input buffers into the packet payload.
            memcpy (payloadVa, payload0, bytes0);
            memcpy ((void *)((uint8_t*)payloadVa + bytes0), payload1, bytes1);

            // Copy the metadata into the network header in the descriptor.
            if (metasize > 0)
              {
                MemoryFifoPacketHeader_t * hdr =
                  (MemoryFifoPacketHeader_t *) & desc->PacketHeader;
                XMI_assert(metasize <= packet_model_metadata_bytes);
                memcpy((void *) &hdr->dev.singlepkt.metadata, metadata, metasize); // <-- replace with an optimized MUSPI function.
              }

            // Advance the injection fifo descriptor tail which actually enables
            // the MU hardware to process the descriptor and send the packet
            // on the torus.
            uint64_t sequenceNum = MUSPI_InjFifoAdvanceDesc (injfifo);

            TRACE((stderr, "MUPacketModel::postPacket_impl(2) .. after MUSPI_InjFifoAdvanceDesc(), sequenceNum = %ld\n", sequenceNum));

            if (fn != NULL)
              {
                fn (_context, cookie, XMI_SUCCESS); // Descriptor is done...notify.
              }
          }
        else
          {
            TRACE((stderr, "MUPacketModel::postPacket_impl(2) .. after nextInjectionDescriptor(), no space in fifo\n"));
            // Construct a message and post to the device to be processed later.
            MUInjFifoMessage * obj = (MUInjFifoMessage *) state;
            new (obj) MUInjFifoMessage (fn, cookie, _context);
            TRACE((stderr, "MUPacketModel::postPacket_impl(2) .. before setSourceBuffer\n"));
            obj->setSourceBuffer (payload0, bytes0, payload0, bytes0);

            // Initialize the descriptor directly in the injection fifo.
            TRACE((stderr, "MUPacketModel::postPacket_impl(2) .. before getDescriptor\n"));
            MUSPI_DescriptorBase * desc = obj->getDescriptor ();
            TRACE((stderr, "MUPacketModel::postPacket_impl(2) .. before initializeDescriptor\n"));
            initializeDescriptor (desc, target_rank, 0, 0);

            // Copy the metadata into the network header in the descriptor.
            if (metasize > 0)
              {
                TRACE((stderr, "MUPacketModel::postPacket_impl(2) .. copy metadata\n"));
                MemoryFifoPacketHeader_t * hdr =
                  (MemoryFifoPacketHeader_t *) & (desc->PacketHeader);
                XMI_assert(metasize <= packet_model_metadata_bytes);
                memcpy((void *) &hdr->dev.singlepkt.metadata, metadata, metasize); // <-- replace with an optimized MUSPI function.
              }

            TRACE((stderr, "MUPacketModel::postPacket_impl(2) .. before addToSendQ()\n"));

            // Add this message to the send queue to be processed when there is
            // space available in the injection fifo.
            _device.addToSendQ (target_rank, (QueueElem *) obj);
          }

        TRACE((stderr, "MUPacketModel::postPacket_impl(2) << \n"));
        return true;
      };

      bool MUPacketModel::postPacket_impl (uint8_t              state[MUPacketModel::packet_model_state_bytes],
                                           xmi_event_function   fn,
                                           void               * cookie,
                                           size_t               target_rank,
                                           void               * metadata,
                                           size_t               metasize,
                                           struct iovec       * iov,
                                           size_t               niov)
      {
        XMI_abort();
      };

      bool MUPacketModel::postPacketImmediate_impl (size_t   target_rank,
                                                    void   * metadata,
                                                    size_t   metasize,
                                                    void   * payload0,
                                                    size_t   bytes0,
                                                    void   * payload1,
                                                    size_t   bytes1)
      {
        TRACE((stderr, "MUPacketModel::postPacketImmediate_impl(2) >> \n"));

        MUSPI_InjFifo_t    * injfifo;
        MUHWI_Descriptor_t * hwi_desc;
        void               * payloadVa;
        void               * payloadPa;

#ifdef ENABLE_MAMBO_WORKAROUNDS
#warning    Mambo can not handle zero-byte payloads.
            if (bytes0+bytes1 == 0)
            {
              payload0 = metadata;
              bytes0 = 1;
              payload1 = metadata;
              bytes1 = 1;
            }
#endif

        if (_device.nextInjectionDescriptor (target_rank,
                                             &injfifo,
                                             &hwi_desc,
                                             &payloadVa,
                                             &payloadPa))
        {
          TRACE((stderr, "MUPacketModel::postPacketImmediate_impl(2) .. after _device.nextInjectionDescriptor(), injfifo = %p, hwi_desc = %p, payloadVa = %p, payloadPa = %p\n", injfifo, hwi_desc, payloadVa, payloadPa));
          MUSPI_DescriptorBase * desc = (MUSPI_DescriptorBase *) hwi_desc;

          // Initialize the descriptor directly in the injection fifo.
          initializeDescriptor (desc, target_rank, (uint64_t) payloadPa, bytes0 + bytes1);

          // Enable the "single packet message" bit.
          desc->setSoftwareBit (1);

          // Pack the input buffers into the packet payload.
          memcpy (payloadVa, payload0, bytes0);
          memcpy ((void *)((uint8_t*)payloadVa + bytes0), payload1, bytes1);

          // Copy the metadata into the network header in the descriptor.
          if (metasize > 0)
          {
            MemoryFifoPacketHeader_t * hdr =
              (MemoryFifoPacketHeader_t *) & desc->PacketHeader;
            XMI_assert(metasize <= packet_model_metadata_bytes);
            memcpy((void *) &hdr->dev.singlepkt.metadata, metadata, metasize); // <-- replace with an optimized MUSPI function.
          }

          // Advance the injection fifo descriptor tail which actually enables
          // the MU hardware to process the descriptor and send the packet
          // on the torus.
          uint64_t sequenceNum = MUSPI_InjFifoAdvanceDesc (injfifo);

          TRACE((stderr, "MUPacketModel::postPacketImmediate_impl(2) .. after MUSPI_InjFifoAdvanceDesc(), sequenceNum = %ld\n", sequenceNum));

          return true;
        }
        return false;
      };

      bool MUPacketModel::postMessage_impl (uint8_t              state[MUPacketModel::message_model_state_bytes],
                                            xmi_event_function   fn,
                                            void               * cookie,
                                            size_t               target_rank,
                                            void               * metadata,
                                            size_t               metasize,
                                            void               * payload,
                                            size_t               bytes)
      {
        TRACE((stderr, "MUPacketModel::postMessage_impl() >> \n"));

#ifdef ENABLE_MAMBO_WORKAROUNDS
#warning    Mambo can not handle zero-byte payloads.
            if (bytes == 0)
            {
              payload = metadata;
              bytes = 1;
            }
#endif

        // Determine the physical address of the source buffer.
        //
        // TODO - need to have a different way of finding/pinning the buffer that
        //        does not involve a syscall or memory region. If we use a memory
        //        region, when is the region destroyed?  It should be *after* the
        //        descriptor is completed, right?
        uint32_t rc;
        Kernel_MemoryRegion_t memRegion; // Memory region associated with the buffer.
        rc = Kernel_CreateMemoryRegion (&memRegion, payload, bytes);
        XMI_assert ( rc == 0 );

        uint64_t paddr = (uint64_t)memRegion.BasePa +
                         ((uint64_t)payload - (uint64_t)memRegion.BaseVa);

        MUSPI_InjFifo_t    * injfifo;
        MUHWI_Descriptor_t * hwi_desc;
        void               * payloadVa;
        void               * payloadPa;

        if (_device.nextInjectionDescriptor (target_rank,
                                             &injfifo,
                                             &hwi_desc,
                                             &payloadVa,
                                             &payloadPa))
          {
            TRACE((stderr, "MUPacketModel::postMessage_impl() .. after _device.nextInjectionDescriptor(), injfifo = %p, hwi_desc = %p, payloadVa = %p, payloadPa = %p\n", injfifo, hwi_desc, payloadVa, payloadPa));
            MUSPI_DescriptorBase * desc = (MUSPI_DescriptorBase *) hwi_desc;

            // Initialize the descriptor directly in the injection fifo.
            initializeDescriptor (desc, target_rank, paddr, bytes);

            // Copy the metadata into the network header in the descriptor.
            if (metasize > 0)
              {
                MemoryFifoPacketHeader_t * hdr =
                  (MemoryFifoPacketHeader_t *) & desc->PacketHeader;
                XMI_assert(metasize <= message_model_metadata_bytes);
                memcpy((void *) &hdr->dev.multipkt.metadata, metadata, metasize); // <-- replace with an optimized MUSPI function.
              }

            // Advance the injection fifo descriptor tail which actually enables
            // the MU hardware to process the descriptor and send the packet
            // on the torus.
            uint64_t sequenceNum = MUSPI_InjFifoAdvanceDesc (injfifo);

            TRACE((stderr, "MUPacketModel::postMessage_impl() .. after MUSPI_InjFifoAdvanceDesc(), sequenceNum = %ld, fn = %p\n", sequenceNum, fn));

            if (fn != NULL)
              {
#ifndef OPTIMIZE_AGGREGATE_LATENCY
                // Check if the descriptor is done.
                TRACE((stderr, "MUPacketModel::postMessage_impl() .. before MUSPI_CheckDescComplete()\n"));

                uint32_t rc = MUSPI_CheckDescComplete (injfifo, sequenceNum);
                TRACE((stderr, "MUPacketModel::postMessage_impl() .. after MUSPI_CheckDescComplete(), rc = %d, fn = %p\n", rc, fn));

                if ( rc == 1 )
                  {
                    fn (_context, cookie, XMI_SUCCESS); // Descriptor is done...notify.
                  }
                else
#endif
                  {
                    MUInjFifoMessage * obj = (MUInjFifoMessage *) state;
                    TRACE((stderr, "MUPacketModel::postMessage_impl() .. descriptor is not done, create message (%p) and add to send queue\n", obj));
                    // The descriptor is not done (or was not checked). Save state
                    // information so that the progress of the decriptor can be checked
                    // later and the callback will be invoked when the descriptor is
                    // complete.
                    new (obj) MUInjFifoMessage (fn, cookie, _context, sequenceNum);

                    // Queue it.
                    _device.addToDoneQ (target_rank, obj->getWrapper());
                  }
              }
          }
        else
          {
            // Construct a message and post to the device to be processed later.
            MUInjFifoMessage * obj = (MUInjFifoMessage *) state;
            new (obj) MUInjFifoMessage (fn, cookie, _context);
            obj->setSourceBuffer (payload, bytes);

            // Initialize the descriptor directly in the injection fifo.
            MUSPI_DescriptorBase * desc = obj->getDescriptor ();
            initializeDescriptor (desc, target_rank, paddr, bytes);

            // Copy the metadata into the network header in the descriptor.
            if (metasize > 0)
              {
                MemoryFifoPacketHeader_t * hdr =
                  (MemoryFifoPacketHeader_t *) & (desc->PacketHeader);
                XMI_assert(metasize <= message_model_metadata_bytes);
                memcpy((void *) &hdr->dev.multipkt.metadata, metadata, metasize); // <-- replace with an optimized MUSPI function.
              }

            // Add this message to the send queue to be processed when there is
            // space available in the injection fifo.
            _device.addToSendQ (target_rank, (QueueElem *) obj);
          }

        TRACE((stderr, "MUPacketModel::postMessage_impl() << \n"));
        return true;

      }; // XMI::Device::MU::MUPacketModel class
    };   // XMI::Device::MU namespace
  };     // XMI::Device namespace
};       // XMI namespace

#undef TRACE

#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
