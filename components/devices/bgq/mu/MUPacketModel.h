/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu/MUPacketModel.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu_MUPacketModel_h__
#define __components_devices_bgq_mu_MUPacketModel_h__

#include <hwi/include/bqc/MU_PacketHeader.h>

#include <spi/include/mu/DescriptorBaseXX.h>
#include <spi/include/mu/DescriptorWrapperXX.h>
#include <spi/include/mu/Pt2PtMemoryFIFODescriptorXX.h>
#include <spi/include/kernel/memory.h>

#include "components/devices/PacketInterface.h"

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
      class MUPacketModel : public Interface::PacketModel<MUPacketModel, MUDevice, sizeof(MUInjFifoMessage)>
      {
        public:

          /// \see XMI::Device::Interface::PacketModel::PacketModel
          /// \see XMI::Device::Interface::MessageModel::MessageModel
          MUPacketModel (MUDevice & device);

          /// \see XMI::Device::Interface::PacketModel::~PacketModel
          /// \see XMI::Device::Interface::MessageModel::~MessageModel
          ~MUPacketModel ();

          static const bool   deterministic_packet_model         = true;
          static const bool   reliable_packet_model              = true;
          static const size_t packet_model_metadata_bytes        = MUDevice::packet_metadata_size;
          static const size_t packet_model_multi_metadata_bytes  = MUDevice::packet_metadata_size;
          static const size_t packet_model_payload_bytes         = MUDevice::payload_size;
          static const size_t packet_model_state_bytes           = sizeof(MUInjFifoMessage);

          /// \see XMI::Device::Interface::PacketModel::init
          xmi_result_t init_impl (size_t                      dispatch,
                                  Interface::RecvFunction_t   direct_recv_func,
                                  void                      * direct_recv_func_parm,
                                  Interface::RecvFunction_t   read_recv_func,
                                  void                      * read_recv_func_parm);

          /// \see XMI::Device::Interface::PacketModel::postPacket
          inline bool postPacket_impl (uint8_t              (&state)[MUPacketModel::packet_model_state_bytes],
                                       xmi_event_function   fn,
                                       void               * cookie,
                                       xmi_task_t           target_task,
                                       size_t               target_offset,
                                       void               * metadata,
                                       size_t               metasize,
                                       struct iovec       * iov,
                                       size_t               niov);

          /// \see XMI::Device::Interface::PacketModel::postPacket
          template <unsigned T_Niov>
          inline bool postPacket_impl (uint8_t              (&state)[MUPacketModel::packet_model_state_bytes],
                                       xmi_event_function   fn,
                                       void               * cookie,
                                       xmi_task_t           target_task,
                                       size_t               target_offset,
                                       void               * metadata,
                                       size_t               metasize,
                                       struct iovec         (&iov)[T_Niov]);

          /// \see XMI::Device::Interface::PacketModel::postPacket
          inline bool postPacket_impl (uint8_t              (&state)[MUPacketModel::packet_model_state_bytes],
                                       xmi_event_function   fn,
                                       void               * cookie,
                                       xmi_task_t           target_task,
                                       size_t               target_offset,
                                       void               * metadata,
                                       size_t               metasize,
                                       void               * payload,
                                       size_t               length);

          /// \see XMI::Device::Interface::PacketModel::postPacket
          template <unsigned T_Niov>
          inline bool postPacket_impl (xmi_task_t     target_task,
                                       size_t         target_offset,
                                       void         * metadata,
                                       size_t         metasize,
                                       struct iovec   (&iov)[T_Niov]);

          /// \see XMI::Device::Interface::PacketModel::postMultiPacket
          inline bool postMultiPacket_impl (uint8_t              (&state)[MUPacketModel::packet_model_state_bytes],
                                            xmi_event_function   fn,
                                            void               * cookie,
                                            xmi_task_t           target_task,
                                            size_t               target_offset,
                                            void               * metadata,
                                            size_t               metasize,
                                            void               * payload,
                                            size_t               length);

        protected:

          inline void initializeDescriptor (MUSPI_DescriptorBase * desc,
                                            xmi_task_t             target_task,
                                            size_t                 target_offset,
                                            uint64_t               payloadPa,
                                            size_t                 bytes);
        private:
          MUDevice                        & _device;
          MUDescriptorWrapper               _wrapper_model;
          MUSPI_Pt2PtMemoryFIFODescriptor   _desc_model;
//          xmi_client_t                     _client;
          xmi_context_t                     _context;
      };

      void MUPacketModel::initializeDescriptor (MUSPI_DescriptorBase * desc,
                                                xmi_task_t             target_task,
                                                size_t                 target_offset,
                                                uint64_t               payloadPa,
                                                size_t                 bytes)
      {
        TRACE((stderr, ">> initializeDescriptor(%p, %u, %zu, %p, %zd)\n", desc, target_task, target_offset, (void *)payloadPa, bytes));
        // Clone the model descriptor.
        _desc_model.clone (*desc);
#warning set reception fifo based on the target context id (target_offset)
        // --------------------------------------------------------------------
        // Set the destination torus address and reception fifo.
        // This is terribly inefficient.
        MUHWI_Destination dst;
        size_t addr[BGQ_TDIMS + BGQ_LDIMS];
        __global.mapping.task2global ((xmi_task_t)target_task, addr);
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

        TRACE((stderr, "<< initializeDescriptor(%p, %d, %p, %zd)\n", desc, target_task, (void *)payloadPa, bytes));
      }

      bool MUPacketModel::postPacket_impl (uint8_t              (&state)[MUPacketModel::packet_model_state_bytes],
                                           xmi_event_function   fn,
                                           void               * cookie,
                                           xmi_task_t           target_task,
                                           size_t               target_offset,
                                           void               * metadata,
                                           size_t               metasize,
                                           struct iovec       * iov,
                                           size_t               niov)
      {
        XMI_abort();
      };


      template <unsigned T_Niov>
      bool MUPacketModel::postPacket_impl (uint8_t              (&state)[MUPacketModel::packet_model_state_bytes],
                                           xmi_event_function   fn,
                                           void               * cookie,
                                           xmi_task_t           target_task,
                                           size_t               target_offset,
                                           void               * metadata,
                                           size_t               metasize,
                                           struct iovec         (&iov)[T_Niov])
      {
        TRACE((stderr, "MUPacketModel::postPacket_impl(%d) >> \n", T_Niov));

        MUSPI_InjFifo_t    * injfifo;
        MUHWI_Descriptor_t * hwi_desc;
        void               * payloadVa;
        void               * payloadPa;

        size_t tbytes = 0;
        unsigned i;
        for (i=0; i<T_Niov; i++) tbytes += iov[i].iov_len;

#ifdef ENABLE_MAMBO_WORKAROUNDS
#warning    Mambo can not handle zero-byte payloads.
            if (tbytes == 0)
            {
              iov[0].iov_base = metadata;
              iov[0].iov_len  = 1;
            }
#endif

        TRACE((stderr, "MUPacketModel::postPacket_impl(%d) .. before nextInjectionDescriptor()\n", T_Niov));
        if (_device.nextInjectionDescriptor (target_task,
                                             &injfifo,
                                             &hwi_desc,
                                             &payloadVa,
                                             &payloadPa))
          {
            TRACE((stderr, "MUPacketModel::postPacket_impl(%d) .. after _device.nextInjectionDescriptor(), injfifo = %p, hwi_desc = %p, payloadVa = %p, payloadPa = %p\n", T_Niov, injfifo, hwi_desc, payloadVa, payloadPa));
            MUSPI_DescriptorBase * desc = (MUSPI_DescriptorBase *) hwi_desc;

            // Initialize the descriptor directly in the injection fifo.
            initializeDescriptor (desc, target_task, target_offset, (uint64_t) payloadPa, tbytes);

            // Enable the "single packet message" bit.
            desc->setSoftwareBit (1);

            // Pack the input buffers into the packet payload.
            uint8_t * data = (uint8_t *) payloadVa;
            for (i=0; i<T_Niov; i++)
            {
              memcpy (data, iov[i].iov_base, iov[i].iov_len);
              data += iov[i].iov_len;
            }

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

            TRACE((stderr, "MUPacketModel::postPacket_impl(%d) .. after MUSPI_InjFifoAdvanceDesc(), sequenceNum = %ld\n", T_Niov, sequenceNum));
            sequenceNum = 0; // just to stop warnings

            if (fn != NULL)
              {
                fn (_context, cookie, XMI_SUCCESS); // Descriptor is done...notify.
              }
          }
        else
          {
            TRACE((stderr, "MUPacketModel::postPacket_impl(%d) .. after nextInjectionDescriptor(), no space in fifo\n", T_Niov));
            // Construct a message and post to the device to be processed later.
            MUInjFifoMessage * obj = (MUInjFifoMessage *) state;
            new (obj) MUInjFifoMessage (fn, cookie, _context);
            TRACE((stderr, "MUPacketModel::postPacket_impl(%d) .. before setSourceBuffer\n", T_Niov));
            obj->setSourceBuffer (iov);

            // Initialize the descriptor directly in the injection fifo.
            TRACE((stderr, "MUPacketModel::postPacket_impl(%d) .. before getDescriptor\n", T_Niov));
            MUSPI_DescriptorBase * desc = obj->getDescriptor ();
            TRACE((stderr, "MUPacketModel::postPacket_impl(%d) .. before initializeDescriptor\n", T_Niov));
            initializeDescriptor (desc, target_task, target_offset, 0, 0);

            // Copy the metadata into the network header in the descriptor.
            if (metasize > 0)
              {
                TRACE((stderr, "MUPacketModel::postPacket_impl(%d) .. copy metadata\n", T_Niov));
                MemoryFifoPacketHeader_t * hdr =
                  (MemoryFifoPacketHeader_t *) & (desc->PacketHeader);
                XMI_assert(metasize <= packet_model_metadata_bytes);
                memcpy((void *) &hdr->dev.singlepkt.metadata, metadata, metasize); // <-- replace with an optimized MUSPI function.
              }

            TRACE((stderr, "MUPacketModel::postPacket_impl(%d) .. before addToSendQ()\n", T_Niov));

            // Add this message to the send queue to be processed when there is
            // space available in the injection fifo.
#warning send queue must be based on task+offset
            _device.addToSendQ (target_task, (QueueElem *) obj);
          }

        TRACE((stderr, "MUPacketModel::postPacket_impl(%d) << \n", T_Niov));
        return true;
      };

      bool MUPacketModel::postPacket_impl (uint8_t              (&state)[MUPacketModel::packet_model_state_bytes],
                                           xmi_event_function   fn,
                                           void               * cookie,
                                           xmi_task_t           target_task,
                                           size_t               target_offset,
                                           void               * metadata,
                                           size_t               metasize,
                                           void               * payload,
                                           size_t               length)
      {
        TRACE((stderr, "MUPacketModel::postPacket_impl(single) .. %p, %zu >> \n", payload, length));

        MUSPI_InjFifo_t    * injfifo;
        MUHWI_Descriptor_t * hwi_desc;
        void               * payloadVa;
        void               * payloadPa;

#ifdef ENABLE_MAMBO_WORKAROUNDS
#warning    Mambo can not handle zero-byte payloads.
            if (length == 0)
            {
              payload = metadata;
              length  = 1;
            }
#endif

        TRACE((stderr, "MUPacketModel::postPacket_impl(single) .. before nextInjectionDescriptor()\n"));
        if (_device.nextInjectionDescriptor (target_task,
                                             &injfifo,
                                             &hwi_desc,
                                             &payloadVa,
                                             &payloadPa))
          {
            TRACE((stderr, "MUPacketModel::postPacket_impl(single) .. after _device.nextInjectionDescriptor(), injfifo = %p, hwi_desc = %p, payloadVa = %p, payloadPa = %p\n", injfifo, hwi_desc, payloadVa, payloadPa));
            MUSPI_DescriptorBase * desc = (MUSPI_DescriptorBase *) hwi_desc;

            // Initialize the descriptor directly in the injection fifo.
            initializeDescriptor (desc, target_task, target_offset, (uint64_t) payloadPa, length);

            // Enable the "single packet message" bit.
            desc->setSoftwareBit (1);

            // Pack the input buffers into the packet payload.
            memcpy (payloadVa, payload, length);

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

            TRACE((stderr, "MUPacketModel::postPacket_impl(single) .. after MUSPI_InjFifoAdvanceDesc(), sequenceNum = %ld\n", sequenceNum));
            sequenceNum = 0; // just to stop warnings

            if (fn != NULL)
              {
                fn (_context, cookie, XMI_SUCCESS); // Descriptor is done...notify.
              }
          }
        else
          {
            TRACE((stderr, "MUPacketModel::postPacket_impl(single) .. after nextInjectionDescriptor(), no space in fifo\n"));
            // Construct a message and post to the device to be processed later.
            MUInjFifoMessage * obj = (MUInjFifoMessage *) state;
            new (obj) MUInjFifoMessage (fn, cookie, _context);
            TRACE((stderr, "MUPacketModel::postPacket_impl(single) .. before setSourceBuffer\n"));
            obj->setSourceBuffer (payload, length);

            // Initialize the descriptor directly in the injection fifo.
            TRACE((stderr, "MUPacketModel::postPacket_impl(single) .. before getDescriptor\n"));
            MUSPI_DescriptorBase * desc = obj->getDescriptor ();
            TRACE((stderr, "MUPacketModel::postPacket_impl(single) .. before initializeDescriptor\n"));
            initializeDescriptor (desc, target_task, target_offset, 0, 0);

            // Copy the metadata into the network header in the descriptor.
            if (metasize > 0)
              {
                TRACE((stderr, "MUPacketModel::postPacket_impl(single) .. copy metadata\n"));
                MemoryFifoPacketHeader_t * hdr =
                  (MemoryFifoPacketHeader_t *) & (desc->PacketHeader);
                XMI_assert(metasize <= packet_model_metadata_bytes);
                memcpy((void *) &hdr->dev.singlepkt.metadata, metadata, metasize); // <-- replace with an optimized MUSPI function.
              }

            TRACE((stderr, "MUPacketModel::postPacket_impl(single) .. before addToSendQ()\n"));

            // Add this message to the send queue to be processed when there is
            // space available in the injection fifo.
            _device.addToSendQ (target_task, (QueueElem *) obj);
          }

        TRACE((stderr, "MUPacketModel::postPacket_impl(single) << \n"));
        return true;
      };

      template <unsigned T_Niov>
      bool MUPacketModel::postPacket_impl (xmi_task_t     target_task,
                                           size_t         target_offset,
                                           void         * metadata,
                                           size_t         metasize,
                                           struct iovec   (&iov)[T_Niov])
      {
        TRACE((stderr, "MUPacketModel::postPacket_impl(%d) >> \n", T_Niov));

        MUSPI_InjFifo_t    * injfifo;
        MUHWI_Descriptor_t * hwi_desc;
        void               * payloadVa;
        void               * payloadPa;

        size_t tbytes = 0;
        unsigned i;
        for (i=0; i<T_Niov; i++) tbytes += iov[i].iov_len;

#ifdef ENABLE_MAMBO_WORKAROUNDS
#warning    Mambo can not handle zero-byte payloads.
            if (tbytes == 0)
            {
              iov[0].iov_base = metadata;
              iov[0].iov_len  = 1;
            }
#endif

        if (_device.nextInjectionDescriptor (target_task,
                                             &injfifo,
                                             &hwi_desc,
                                             &payloadVa,
                                             &payloadPa))
        {
          TRACE((stderr, "MUPacketModel::postPacketImmediate_impl(%d) .. after _device.nextInjectionDescriptor(), injfifo = %p, hwi_desc = %p, payloadVa = %p, payloadPa = %p\n", T_Niov, injfifo, hwi_desc, payloadVa, payloadPa));
          MUSPI_DescriptorBase * desc = (MUSPI_DescriptorBase *) hwi_desc;

          // Initialize the descriptor directly in the injection fifo.
          initializeDescriptor (desc, target_task, target_offset, (uint64_t) payloadPa, tbytes);

          // Enable the "single packet message" bit.
          desc->setSoftwareBit (1);

          // Pack the input buffers into the packet payload.
          uint8_t * data = (uint8_t *) payloadVa;
          for (i=0; i<T_Niov; i++)
          {
            memcpy (data, iov[i].iov_base, iov[i].iov_len);
            data += iov[i].iov_len;
          }

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

          TRACE((stderr, "MUPacketModel::postPacketImmediate_impl(%d) .. after MUSPI_InjFifoAdvanceDesc(), sequenceNum = %ld\n", T_Niov, sequenceNum));
          sequenceNum = 0; // just to stop warnings

          return true;
        }
        return false;
      };

      bool MUPacketModel::postMultiPacket_impl (uint8_t              (&state)[MUPacketModel::packet_model_state_bytes],
                                                xmi_event_function   fn,
                                                void               * cookie,
                                                xmi_task_t           target_task,
                                                size_t               target_offset,
                                                void               * metadata,
                                                size_t               metasize,
                                                void               * payload,
                                                size_t               length)
      {
        TRACE((stderr, "MUPacketModel::postMultiPacket_impl() >> \n"));

#ifdef ENABLE_MAMBO_WORKAROUNDS
#warning    Mambo can not handle zero-byte payloads.
            if (length == 0)
            {
              payload = metadata;
              length = 1;
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
        rc = Kernel_CreateMemoryRegion (&memRegion, payload, length);
        XMI_assert ( rc == 0 );

        uint64_t paddr = (uint64_t)memRegion.BasePa +
                         ((uint64_t)payload - (uint64_t)memRegion.BaseVa);

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
            TRACE((stderr, "MUPacketModel::postMultiPacket_impl() .. after _device.nextInjectionDescriptor(), injfifo = %p, hwi_desc = %p, payloadVa = %p, payloadPa = %p\n", injfifo, hwi_desc, payloadVa, payloadPa));
            MUSPI_DescriptorBase * desc = (MUSPI_DescriptorBase *) hwi_desc;

            // Initialize the descriptor directly in the injection fifo.
            initializeDescriptor (desc, target_task, target_offset, paddr, length);

            // Copy the metadata into the network header in the descriptor.
            if (metasize > 0)
              {
                MemoryFifoPacketHeader_t * hdr =
                  (MemoryFifoPacketHeader_t *) & desc->PacketHeader;
                XMI_assert(metasize <= packet_model_multi_metadata_bytes);
                memcpy((void *) &hdr->dev.multipkt.metadata, metadata, metasize); // <-- replace with an optimized MUSPI function.
              }

            // Advance the injection fifo descriptor tail which actually enables
            // the MU hardware to process the descriptor and send the packet
            // on the torus.
            uint64_t sequenceNum = MUSPI_InjFifoAdvanceDesc (injfifo);

            TRACE((stderr, "MUPacketModel::postMultiPacket_impl() .. after MUSPI_InjFifoAdvanceDesc(), sequenceNum = %ld, fn = %p\n", sequenceNum, fn));

            if (fn != NULL)
              {
#ifndef OPTIMIZE_AGGREGATE_LATENCY
                // Check if the descriptor is done.
                TRACE((stderr, "MUPacketModel::postMultiPacket_impl() .. before MUSPI_CheckDescComplete()\n"));

                uint32_t rc = MUSPI_CheckDescComplete (injfifo, sequenceNum);
                TRACE((stderr, "MUPacketModel::postMultiPacket_impl() .. after MUSPI_CheckDescComplete(), rc = %d, fn = %p\n", rc, fn));

                if ( rc == 1 )
                  {
                    fn (_context, cookie, XMI_SUCCESS); // Descriptor is done...notify.
                  }
                else
#endif
                  {
                    MUInjFifoMessage * obj = (MUInjFifoMessage *) state;
                    TRACE((stderr, "MUPacketModel::postMultiPacket_impl() .. descriptor is not done, create message (%p) and add to send queue\n", obj));
                    // The descriptor is not done (or was not checked). Save state
                    // information so that the progress of the decriptor can be checked
                    // later and the callback will be invoked when the descriptor is
                    // complete.
                    new (obj) MUInjFifoMessage (fn, cookie, _context, sequenceNum);

                    // Queue it.
                    _device.addToDoneQ (target_task, obj->getWrapper());
                  }
              }
          }
        else
          {
            // Construct a message and post to the device to be processed later.
            MUInjFifoMessage * obj = (MUInjFifoMessage *) state;
            new (obj) MUInjFifoMessage (fn, cookie, _context);
            obj->setSourceBuffer (payload, length);

            // Initialize the descriptor directly in the injection fifo.
            MUSPI_DescriptorBase * desc = obj->getDescriptor ();
            initializeDescriptor (desc, target_task, target_offset, paddr, length);

            // Copy the metadata into the network header in the descriptor.
            if (metasize > 0)
              {
                MemoryFifoPacketHeader_t * hdr =
                  (MemoryFifoPacketHeader_t *) & (desc->PacketHeader);
                XMI_assert(metasize <= packet_model_multi_metadata_bytes);
                memcpy((void *) &hdr->dev.multipkt.metadata, metadata, metasize); // <-- replace with an optimized MUSPI function.
              }

            // Add this message to the send queue to be processed when there is
            // space available in the injection fifo.
            _device.addToSendQ (target_task, (QueueElem *) obj);
          }

        TRACE((stderr, "MUPacketModel::postMultiPacket_impl() << \n"));
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
