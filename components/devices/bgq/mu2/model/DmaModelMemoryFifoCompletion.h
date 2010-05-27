/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/DmaModelMemoryFifoCompletion.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu2_DmaModelMemoryFifoCompletion_h__
#define __components_devices_bgq_mu2_DmaModelMemoryFifoCompletion_h__

#include "components/devices/bgq/mu2/model/DmaModelBase.h"

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      class DmaModelMemoryFifoCompletion : public MU::DmaModelBase<DmaModelMemoryFifoCompletion>
      {

        public :

          friend class MU::DmaModelBase<DmaModelMemoryFifoCompletion>;

          /// \see PAMI::Device::Interface::DmaModel::DmaModel
          inline DmaModelMemoryFifoCompletion (MU::Context & context, pami_result_t & status) :
              MU::DmaModelBase<DmaModelMemoryFifoCompletion> (context, status)
          {
            COMPILE_TIME_ASSERT(sizeof(MU::Context::notify_t) <= MemoryFifoPacketHeader::packet_singlepacket_metadata_size);
            COMPILE_TIME_ASSERT((sizeof(MUSPI_DescriptorBase)*2) <= MU::Context::LOOKASIDE_PAYLOAD_SIZE);

            // Zero-out the descriptor models before initialization
            memset((void *)&_ack_to_self, 0, sizeof(_ack_to_self));

            // ----------------------------------------------------------------
            // Set the common base descriptor fields
            // ----------------------------------------------------------------
            MUSPI_BaseDescriptorInfoFields_t base;
            memset((void *)&base, 0, sizeof(base));

            base.Pre_Fetch_Only  = MUHWI_DESCRIPTOR_PRE_FETCH_ONLY_NO;
            base.Payload_Address = 0;
            base.Message_Length  = 0;
            base.Torus_FIFO_Map  = 0;
            base.Dest.Destination.Destination = 0;

            _ack_to_self.setBaseFields (&base);


            // ----------------------------------------------------------------
            // Set the common point-to-point descriptor fields
            // ----------------------------------------------------------------
            MUSPI_Pt2PtDescriptorInfoFields_t pt2pt;
            memset((void *)&pt2pt, 0, sizeof(pt2pt));

            pt2pt.Hints_ABCD = 0;
            pt2pt.Skip       = 0;
            pt2pt.Misc1 =
              MUHWI_PACKET_USE_DETERMINISTIC_ROUTING |
              MUHWI_PACKET_DO_NOT_DEPOSIT |
              MUHWI_PACKET_DO_NOT_ROUTE_TO_IO_NODE;
            pt2pt.Misc2 =
              MUHWI_PACKET_VIRTUAL_CHANNEL_DETERMINISTIC;

            _ack_to_self.setDataPacketType (MUHWI_PT2PT_DATA_PACKET_TYPE);
            _ack_to_self.PacketHeader.NetworkHeader.pt2pt.Byte8.Size = 16;
            _ack_to_self.setPt2PtFields (&pt2pt);


            // ----------------------------------------------------------------
            // Set the memory fifo descriptor fields
            // ----------------------------------------------------------------
            MUSPI_MemoryFIFODescriptorInfoFields_t memfifo;
            memset ((void *)&memfifo, 0, sizeof(memfifo));

            memfifo.Rec_FIFO_Id    = 0;
            memfifo.Rec_Put_Offset = 0;
            memfifo.Interrupt      = MUHWI_DESCRIPTOR_DO_NOT_INTERRUPT_ON_PACKET_ARRIVAL;
            memfifo.SoftwareBit    = 0;

            _ack_to_self.setMemoryFIFOFields (&memfifo);
            _ack_to_self.setMessageUnitPacketType (MUHWI_PACKET_TYPE_FIFO);
            _ack_to_self.setRecFIFOId (context.getRecptionFifoIdSelf());
            _ack_to_self.setDestination (*(context.getMuDestinationSelf()));


            // ----------------------------------------------------------------
            // Set the "notify" system dispatch identifier
            // ----------------------------------------------------------------
            MemoryFifoPacketHeader * hdr =
              (MemoryFifoPacketHeader *) & _ack_to_self.PacketHeader;
            hdr->setSinglePacket (true);
            hdr->setDispatchId (MU::Context::dispatch_system_notify);
          };

          /// \see PAMI::Device::Interface::DmaModel::~DmaModel
          inline ~DmaModelMemoryFifoCompletion () {};

          inline size_t initializeRemoteGetPayload (void                * vaddr,
                                                    uint64_t              local_dst_pa,
                                                    uint64_t              remote_src_pa,
                                                    size_t                bytes,
                                                    size_t                from_task,
                                                    size_t                from_offset,
                                                    pami_event_function   local_fn,
                                                    void                * cookie)
          {
            // ----------------------------------------------------------------
            // Initialize the "data mover" descriptor in the rget payload
            // ----------------------------------------------------------------
            MU::DmaModelBase<DmaModelMemoryFifoCompletion> * parent =
              (MU::DmaModelBase<DmaModelMemoryFifoCompletion> *) this;

            size_t pbytes =
              parent->initializeRemoteGetPayload (vaddr, local_dst_pa, remote_src_pa,
                                                  bytes, from_task, from_offset,
                                                  local_fn, cookie);


            // ----------------------------------------------------------------
            // Initialize the "ack to self" descriptor in the rget payload
            // ----------------------------------------------------------------
            MUSPI_DescriptorBase * desc = (MUSPI_DescriptorBase *) vaddr;
            _ack_to_self.clone (desc[1]);

            desc[1].setTorusInjectionFIFOMap (desc[0].Torus_FIFO_Map);
            desc[1].setHints (desc[0].PacketHeader.NetworkHeader.pt2pt.Hints,
                              desc[0].PacketHeader.NetworkHeader.pt2pt.Byte2.Byte2); // is this right for 'E' hints?


            // ----------------------------------------------------------------
            // Copy the completion function+cookie into the packet header.
            // ----------------------------------------------------------------
            MU::Context::notify_t * hdr =
              (MU::Context::notify_t *) & desc[1].PacketHeader;
            hdr->fn = local_fn;
            hdr->cookie = cookie;

            return pbytes + sizeof(MUHWI_Descriptor_t);
          };

        protected:

          MUSPI_DescriptorBase _rput;
          MUSPI_DescriptorBase _ack_to_self;

      }; // PAMI::Device::MU::DmaModelMemoryFifoCompletion class
    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace

#endif // __components_devices_bgq_mu2_DmaModelMemoryFifoCompletion_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//




