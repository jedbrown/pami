/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
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

#include <spi/include/mu/DescriptorBaseXX.h>
#include <spi/include/mu/DescriptorWrapperXX.h>
#include <spi/include/mu/Pt2PtMemoryFIFODescriptorXX.h>
#include <spi/include/mu/Pt2PtRemoteGetDescriptorXX.h>
#include <spi/include/mu/Pt2PtDirectPutDescriptorXX.h>
#include <spi/include/kernel/memory.h>

//#include "components/devices/DmaModel.h"
#include "components/devices/myDmaModel.h"
#include "components/devices/bgq/mu/MUDevice.h"
#include "components/devices/bgq/mu/MUInjFifoMessage.h"
#include "components/devices/bgq/mu/MUDescriptorWrapper.h"

#include "Memregion.h"

#ifdef TRACE
#error TRACE already defined!
#else
#define TRACE(x) //fprintf x
#endif

namespace XMI
{
  namespace Device
  {
    namespace MU
    {
      class MUDmaModel : public myInterface::DmaModel<MUDmaModel, MUDevice, sizeof(MUInjFifoMessage)>
      {
        public:

          MUDmaModel (MUDevice & device, xmi_context_t context);

          ~MUDmaModel ();

          inline bool init_impl (size_t origin_rank);

          inline bool postDmaPut_impl (uint8_t              (&obj)[sizeof(MUInjFifoMessage)],
                                       xmi_callback_t     & cb,
                                       size_t               target_rank,
                                       Memregion * local_memregion,
                                       size_t               local_offset,
                                       Memregion * remote_memregion,
                                       size_t               remote_offset,
                                       size_t               bytes);

          inline bool postDmaGet_impl (uint8_t              (&obj)[sizeof(MUInjFifoMessage)],
                                       xmi_callback_t     & cb,
                                       size_t               target_rank,
                                       Memregion * local_memregion,
                                       size_t               local_offset,
                                       Memregion * remote_memregion,
                                       size_t               remote_offset,
                                       size_t               bytes);

          static int dispatch_notify (void   * metadata,
                                      void   * payload,
                                      size_t   bytes,
                                      void   * recv_func_parm)
          {
            xmi_callback_t * cb = (xmi_callback_t *) metadata;
            TRACE((stderr, "MUDmaModel::dispatch_notify() >> cb = %p, cb->function = %p, cb->clientdata = %p\n", cb, cb->function, cb->clientdata));
            cb->function (cb->clientdata, NULL, XMI_SUCCESS);
            TRACE((stderr, "MUDmaModel::dispatch_notify() <<\n"));
            return 0;
          };

        private:
          MUDevice                        & _device;
          MUSPI_Pt2PtDirectPutDescriptor    _dput_desc_model;
          MUSPI_DescriptorWrapper           _wrapper_model; // only needed for direct put?

          MUSPI_Pt2PtRemoteGetDescriptor    _rget_desc_model;
          MUSPI_Pt2PtDirectPutDescriptor    _rput_desc_model;
          MUSPI_Pt2PtMemoryFIFODescriptor   _rmem_desc_model;

          xmi_context_t                     _context;
      };

      bool MUDmaModel::postDmaPut_impl (uint8_t              (&obj)[sizeof(MUInjFifoMessage)],
                                        xmi_callback_t       & cb,
                                        size_t                 target_rank,
                                        Memregion * local_memregion,
                                        size_t                 local_offset,
                                        Memregion * remote_memregion,
                                        size_t                 remote_offset,
                                        size_t                 bytes)
      {
        TRACE((stderr, "MUDmaModel::postDmaPut_impl() >> \n"));

        if (bytes == 0)
          {
            // A zero-byte put is defined to be complete after a dma pingpong. This
            // is accomplished via a zero-byte get operation.
            return postDmaGet_impl (obj, cb, target_rank,
                                    local_memregion, local_offset,
                                    remote_memregion, remote_offset,
                                    0);
          }

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
            TRACE((stderr, "MUDmaModel::postDmaPut_impl() .. after _device.nextInjectionDescriptor(), injfifo = %p, hwi_desc = %p, payloadVa = %p, payloadPa = %p\n", injfifo, hwi_desc, payloadVa, payloadPa));

            MUSPI_DescriptorBase * desc = (MUSPI_DescriptorBase *) hwi_desc;

            // --------------------------------------------------------------------
            // Complete the initialization of the direct put descriptor.
            //
            // Clone the direct put descriptor directly into the injection fifo.
            _dput_desc_model.clone (*desc);

            // Set the source buffer address for the direct put.
            uint64_t pa = (uint64_t) local_memregion->getBasePhysicalAddress ();
            TRACE((stderr, "MUDmaModel::postDmaPut_impl() .. before setPayload(%ld + %zd, %zd)\n", pa, local_offset, bytes));
            desc->setPayload (pa + local_offset, bytes);

            // Set the destination buffer address for the direct put.
            pa = (uint64_t) remote_memregion->getBasePhysicalAddress ();
            desc->setRecPayloadBaseAddressInfo (0, pa + remote_offset);

            // --------------------------------------------------------------------
            // Set the destination torus address and reception fifo.
            // This is terribly inefficient.
            MUHWI_Destination dst;
            size_t addr[BGQ_TDIMS];
            __global.mapping.task2torus (target_rank, addr);
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

            TRACE((stderr, "MUDmaModel::postDmaPut_impl(1) .. after MUSPI_InjFifoAdvanceDesc(), sequenceNum = %ld\n", sequenceNum));

            if (cb.function != NULL)
              {
#ifndef OPTIMIZE_AGGREGATE_LATENCY
                // Check if the descriptor is done.
                TRACE((stderr, "MUDmaModel::postDmaPut_impl(1) .. before MUSPI_CheckDescComplete()\n"));

                uint32_t rc = MUSPI_CheckDescComplete (injfifo, sequenceNum);
                TRACE((stderr, "MUDmaModel::postDmaPut_impl(1) .. after MUSPI_CheckDescComplete(), rc = %d\n", rc));

                if ( rc == 1 )
                  {
                    cb.function (_context, cb.clientdata, XMI_SUCCESS); // Descriptor is done...notify.
                  }
                else
#endif
                  {
                    // The descriptor is not done (or was not checked). Save state
                    // information so that the progress of the decriptor can be checked
                    // later and the callback will be invoked when the descriptor is
                    // complete.
                    MUInjFifoMessage * msg = (MUInjFifoMessage *) obj;
                    new (msg) MUInjFifoMessage (cb.function, cb.clientdata, _context, sequenceNum);

                    // Queue it.
                    _device.addToDoneQ (target_rank, msg->getWrapper());
                  }
              }
          }
        else
          {
            XMI_abort();
          }

        return true;
      };

      bool MUDmaModel::postDmaGet_impl (uint8_t              (&obj)[sizeof(MUInjFifoMessage)],
                                        xmi_callback_t       & cb,
                                        size_t                 target_rank,
                                        Memregion * local_memregion,
                                        size_t                 local_offset,
                                        Memregion * remote_memregion,
                                        size_t                 remote_offset,
                                        size_t                 bytes)
      {
        TRACE((stderr, "MUDmaModel::postDmaGet_impl() >> \n"));

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
            TRACE((stderr, "MUDmaModel::postDmaGet_impl() .. after _device.nextInjectionDescriptor(), injfifo = %p, hwi_desc = %p, payloadVa = %p, payloadPa = %p\n", injfifo, hwi_desc, payloadVa, payloadPa));

            MUSPI_DescriptorBase * desc = (MUSPI_DescriptorBase *) hwi_desc;
            MUSPI_DescriptorBase * payload_desc = (MUSPI_DescriptorBase *) payloadVa;

            // --------------------------------------------------------------------
            // Complete the initialization of the direct put descriptor (payload).
            //
            // Clone the direct put descriptor into the packet payload.
            _rput_desc_model.clone (payload_desc[0]);

            // Set the payload of the direct put descriptor to be the physical
            // address of the source buffer on the remote node (from the user's
            // memory region).
            uint64_t pa = (uint64_t) remote_memregion->getBasePhysicalAddress ();
            payload_desc[0].setPayload (pa + remote_offset, bytes);

            // Set the destination buffer address for the remote direct put.
            pa = (uint64_t) local_memregion->getBasePhysicalAddress ();
            payload_desc[0].setRecPayloadBaseAddressInfo (0, pa + local_offset);



            // --------------------------------------------------------------------
            // Complete the initialization of the memory fifo descriptor (payload).
            //
            // Clone the memory fifo descriptor into the packet payload.
            _rmem_desc_model.clone (payload_desc[1]);

            // Set the user callback information in the unused portion of the
            // packet header for a single packet transfer. The remote memfifo send
            // operation will contain 0 bytes of data in the payload because all
            // information is being packed in the header.
            xmi_callback_t * callback = (xmi_callback_t *) & payload_desc[1].PacketHeader.messageUnitHeader.Packet_Types.Memory_FIFO.Put_Offset_LSB;
            callback->function   = cb.function;
            callback->clientdata = cb.clientdata;

#if 1
// Mambo HACK! send one byte of whatever!
            TRACE((stderr, "MUDmaModel::postDmaGet_impl() .. MAMBO HACK .. set payload (%p) and bytes(1) for remote memory fifo descriptor.\n", (void *)pa));
            payload_desc[1].setPayload (pa, 1);
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
            __global.mapping.task2torus (target_rank, addr);
            dst.Destination.A_Destination = addr[0];
            dst.Destination.B_Destination = addr[1];
            dst.Destination.C_Destination = addr[2];
            dst.Destination.D_Destination = addr[3];
            dst.Destination.E_Destination = addr[4];
            desc->setDestination (dst);

            // Set the appropriate rget inj fifo id based on the channel and rank.
            uint16_t id = _device.getRgetInjFifoId (target_rank);
            TRACE((stderr, "MUDmaModel::postDmaGet_impl() .. after _device.getRgetInjFifoId(%zd), id = %d\n", target_rank, id));
            desc->setRemoteGetInjFIFOId (id);

            // Set the payload of the remote get descriptor.
            if (bytes == 0 && cb.function != NULL)
              {
                // Perform a memfifo "reflect" operation.
                desc->setPayload ((uint64_t)payloadPa + sizeof (MUHWI_Descriptor_t), sizeof (MUHWI_Descriptor_t));
              }
            else if (cb.function != NULL)
              {
                // Send both the direct put and memfifo "reflect" descriptors.
                desc->setPayload ((uint64_t)payloadPa, sizeof (MUHWI_Descriptor_t) + sizeof (MUHWI_Descriptor_t));
              }
            else
              {
                // Do not send the memfifo descriptor since the user did not request
                // completion notification.
                desc->setPayload ((uint64_t)payloadPa, sizeof (MUHWI_Descriptor_t));
              }



            // --------------------------------------------------------------------
            // Advance the injection fifo descriptor tail which actually enables
            // the MU hardware to process the descriptor and send the packet
            // on the torus.
            uint64_t sequenceNum = MUSPI_InjFifoAdvanceDesc (injfifo);
          }
        else
          {
            XMI_abort();
#if 0
            // Construct a message and post to the device to be processed later.
            new (obj) MUInjFifoMessage (cb);
            obj->setSourceBuffer (payload, bytes);

            // Initialize the descriptor directly in the injection fifo.
            MUSPI_DescriptorBase * desc = obj->getDescriptor ();
            initializeDescriptor (desc, target_rank, 0, 0);

            // Copy the metadata into the network header in the descriptor.
            MemoryFifoPacketHeader_t * hdr =
              (MemoryFifoPacketHeader_t *) & (desc->PacketHeader);
            XMI_assert(metasize <= _device.getPacketMetadataSize());
            memcpy((void *) &hdr->dev.singlepkt.metadata, metadata, metasize); // <-- replace with an optimized MUSPI function.

            // Add this message to the send queue to be processed when there is
            // space available in the injection fifo.
            _device.addToSendQ (target_rank, (Queueing::QueueElem *)obj);
#endif
          }

        TRACE((stderr, "MUDmaModel::postDmaGet_impl() << \n"));
        return true;

      }; // XMI::Device::MU::MUDmaModel class
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
