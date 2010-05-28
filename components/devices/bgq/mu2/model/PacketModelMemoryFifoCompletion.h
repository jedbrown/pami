/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/PacketModelMemoryFifoCompletion.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu2_PacketModelMemoryFifoCompletion_h__
#define __components_devices_bgq_mu2_PacketModelMemoryFifoCompletion_h__

#include "components/devices/bgq/mu2/model/PacketModelBase.h"
#include "components/devices/bgq/mu2/msg/InjectDescriptorMessage.h"

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      class PacketModelMemoryFifoCompletion : public MU::PacketModelBase<PacketModelMemoryFifoCompletion>
      {
        public :

          /// \see PAMI::Device::Interface::PacketModel::PacketModel
          inline PacketModelMemoryFifoCompletion (MU::Context & context) :
              MU::PacketModelBase<PacketModelMemoryFifoCompletion> (context)
          {
            COMPILE_TIME_ASSERT(sizeof(InjectDescriptorMessage<1>) <= packet_model_state_bytes);
            COMPILE_TIME_ASSERT(sizeof(InjectDescriptorMessage<2>) <= packet_model_state_bytes);
            COMPILE_TIME_ASSERT(sizeof(MU::Context::notify_t) <= MemoryFifoPacketHeader::packet_singlepacket_metadata_size);

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
            // Set the common memory fifo descriptor fields
            // ----------------------------------------------------------------
            MUSPI_MemoryFIFODescriptorInfoFields_t memfifo;
            memset ((void *)&memfifo, 0, sizeof(memfifo));

            memfifo.Rec_FIFO_Id    = 0;
            memfifo.Rec_Put_Offset = 0;
            memfifo.Interrupt      = MUHWI_DESCRIPTOR_DO_NOT_INTERRUPT_ON_PACKET_ARRIVAL;
            memfifo.SoftwareBit    = 0;

            _ack_to_self.setMemoryFIFOFields (&memfifo);
            _ack_to_self.setMessageUnitPacketType (MUHWI_PACKET_TYPE_FIFO);


            // ----------------------------------------------------------------
            // Initializ the memory fifo descriptor to route to "self"
            // ----------------------------------------------------------------
            _ack_to_self.setRecFIFOId (context.getRecptionFifoIdSelf());
            _ack_to_self.setDestination (*(context.getMuDestinationSelf()));

            // In loopback we send only on AM
            _ack_to_self.setTorusInjectionFIFOMap (MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AM);
            _ack_to_self.setHints (MUHWI_PACKET_HINT_AM |
                                   MUHWI_PACKET_HINT_B_NONE |
                                   MUHWI_PACKET_HINT_C_NONE |
                                   MUHWI_PACKET_HINT_D_NONE,
                                   MUHWI_PACKET_HINT_E_NONE);

            // Set the payload information.
            _ack_to_self.setPayload (0, 0);


            // ----------------------------------------------------------------
            // Set the "notify" system dispatch identifier
            // ----------------------------------------------------------------
            MemoryFifoPacketHeader * hdr =
              (MemoryFifoPacketHeader *) & _ack_to_self.PacketHeader;
            hdr->setSinglePacket (true);
            hdr->setDispatchId (MU::Context::dispatch_system_notify);
          };

          /// \see PAMI::Device::Interface::PacketModel::~PacketModel
          inline ~PacketModelMemoryFifoCompletion () {};

          /// \see PAMI::Device::MU::PacketModelBase::processCompletion
          inline void processCompletion_impl (void                 * state,
                                              size_t                 fnum,
                                              MUSPI_InjFifo_t      * ififo,
                                              size_t                 ndesc,
                                              MUSPI_DescriptorBase * desc,
                                              pami_event_function    fn,
                                              void                 * cookie)
          {
            if (likely(ndesc > 1))
              {
                // There is enough space in the injection fifo to add the
                // "ack to self" memory fifo descriptor.

                // Clone the completion model descriptor into the injection fifo
                _ack_to_self.clone (desc[1]);

                // Copy the completion function+cookie into the packet header.
                MU::Context::notify_t * hdr =
                  (MU::Context::notify_t *) & desc[1].PacketHeader;
                hdr->fn = fn;
                hdr->cookie = cookie;

                // Advance the injection fifo tail pointer. This action
                // completes the injection operation.
// !!!!
// Need a single MU SPI to advance the tail by *2* (or more) descriptors
// !!!!
                MUSPI_InjFifoAdvanceDesc (ififo);
                MUSPI_InjFifoAdvanceDesc (ififo);

                return;
              }

            InjectDescriptorMessage<1> * msg = (InjectDescriptorMessage<1> *) state;
            new (msg) InjectDescriptorMessage<1> (ififo);

            // Copy the "completion" descriptor into the message and initialize
            // the completion function+cookie in the packet header.
            _ack_to_self.clone (msg->desc[0]);
            MU::Context::notify_t * hdr =
              (MU::Context::notify_t *) & msg->desc[0].PacketHeader;
            hdr->fn = fn;
            hdr->cookie = cookie;

            //InjectDescriptorMessage<1> * msg = (InjectDescriptorMessage<1> *) state;
            //new (msg) InjectDescriptorMessage (ififo, & done.desc[0]);

            _context.post (fnum, msg);
          }

          inline MU::MessageQueue::Element * createMessage_impl (void                 * state,
                                                                 MUSPI_DescriptorBase & desc,
                                                                 MUSPI_InjFifo_t      * ififo,
                                                                 pami_event_function    fn,
                                                                 void                 * cookie)
          {
            InjectDescriptorMessage<2> * msg = (InjectDescriptorMessage<2> *) state;
            new (msg) InjectDescriptorMessage<2> (ififo);

            // Copy the "data mover" descriptor into the message.
            desc.clone (msg->desc[0]);

            // Copy the "completion" descriptor into the message and initialize
            // the completion function+cookie in the packet header.
            _ack_to_self.clone (msg->desc[1]);
            MU::Context::notify_t * hdr =
              (MU::Context::notify_t *) & msg->desc[1].PacketHeader;
            hdr->fn = fn;
            hdr->cookie = cookie;

            return (MU::MessageQueue::Element *) msg;
          };

          MUSPI_DescriptorBase _ack_to_self;

      }; // PAMI::Device::MU::PacketModelMemoryFifoCompletion class
    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace

#endif // __components_devices_bgq_mu2_PacketModelMemoryFifoCompletion_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//




