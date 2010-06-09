/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/MemoryFifoCompletion.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu2_MemoryFifoCompletion_h__
#define __components_devices_bgq_mu2_MemoryFifoCompletion_h__

#include "components/devices/bgq/mu2/msg/InjectDescriptorMessage.h"

#include "components/devices/bgq/mu2/trace.h"
#define DO_TRACE_ENTEREXIT 0
#define DO_TRACE_DEBUG     0

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      class MemoryFifoCompletion : public MUSPI_DescriptorBase
      {
        public :

          /// \see PAMI::Device::Interface::PacketModel::PacketModel
          inline MemoryFifoCompletion (MU::Context & context)
          {
            TRACE_FN_ENTER();
            COMPILE_TIME_ASSERT(sizeof(MU::Context::notify_t) <= MemoryFifoPacketHeader::packet_singlepacket_metadata_size);

            // Zero-out the descriptor models before initialization
            memset((void *)this, 0, sizeof(MUSPI_DescriptorBase));


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

            setBaseFields (&base);


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

            setDataPacketType (MUHWI_PT2PT_DATA_PACKET_TYPE);
            PacketHeader.NetworkHeader.pt2pt.Byte8.Size = 16;
            setPt2PtFields (&pt2pt);


            // ----------------------------------------------------------------
            // Set the common memory fifo descriptor fields
            // ----------------------------------------------------------------
            MUSPI_MemoryFIFODescriptorInfoFields_t memfifo;
            memset ((void *)&memfifo, 0, sizeof(memfifo));

            memfifo.Rec_FIFO_Id    = 0;
            memfifo.Rec_Put_Offset = 0;
            memfifo.Interrupt      = MUHWI_DESCRIPTOR_DO_NOT_INTERRUPT_ON_PACKET_ARRIVAL;
            memfifo.SoftwareBit    = 0;

            setMemoryFIFOFields (&memfifo);
            setMessageUnitPacketType (MUHWI_PACKET_TYPE_FIFO);


            // ----------------------------------------------------------------
            // Initializ the memory fifo descriptor to route to "self"
            // ----------------------------------------------------------------
            setRecFIFOId (context.getRecptionFifoIdSelf());
            setDestination (*(context.getMuDestinationSelf()));

            // In loopback we send only on AM
            setTorusInjectionFIFOMap (MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AM);
            setHints (MUHWI_PACKET_HINT_AM |
                      MUHWI_PACKET_HINT_B_NONE |
                      MUHWI_PACKET_HINT_C_NONE |
                      MUHWI_PACKET_HINT_D_NONE,
                      MUHWI_PACKET_HINT_E_NONE);

            // Set the payload information.
            setPayload (0, 0);


            // ----------------------------------------------------------------
            // Set the "notify" system dispatch identifier
            // ----------------------------------------------------------------
            MemoryFifoPacketHeader * hdr =
              (MemoryFifoPacketHeader *) & PacketHeader;
            hdr->setSinglePacket (true);
            hdr->setDispatchId (MU::Context::dispatch_system_notify);

            TRACE_FN_EXIT();
          };

          /// \see PAMI::Device::Interface::PacketModel::~PacketModel
          inline ~MemoryFifoCompletion () {};

          ///
          /// \brief Inject the data mover descriptor(s) and the memory fifo
          ///        completion descriptor into the injection channel.
          ///
          /// \param[in] state   Memory which will, potentially, contain a message object
          /// \param[in] channel Injection channel to use inject into
          /// \param[in] fn      Completion function to invoke
          /// \param[in] cookie  Completion cookie
          /// \param[in] desc    Array of initialized "data mover" descriptors
          ///
          /// \todo Need a single MU SPI to advance the tail by *2* (or more)
          ///       descriptors
          ///
          template <unsigned T_State, unsigned T_Desc>
          inline void inject (uint8_t                (&state)[T_State],
                              InjChannel           * channel,
                              pami_event_function    fn,
                              void                 * cookie,
                              MUSPI_DescriptorBase   (&desc)[T_Desc])
          {
            TRACE_FN_ENTER();
            size_t ndesc = channel->getFreeDescriptorCount ();

            if (likely(ndesc > T_Desc))
              {
                // There is enough space in the injection fifo to add the
                // "ack to self" memory fifo descriptor after the data mover
                // descriptor(s)

                // Clone the completion model descriptor into the injection fifo
                MUSPI_DescriptorBase * d = (MUSPI_DescriptorBase *) desc;
                clone (d[T_Desc]);

                // Copy the completion function+cookie into the packet header.
                MU::Context::notify_t * hdr =
                  (MU::Context::notify_t *) & desc[T_Desc].PacketHeader;
                hdr->fn = fn;
                hdr->cookie = cookie;

                // Advance the injection fifo tail pointer. This action
                // completes the injection operation.
                size_t i;

                for (i = 0; i < T_Desc + 1; i++)
                  channel->injFifoAdvanceDesc (); // see todo

                TRACE_FN_EXIT();
                return;
              }

            // There is not enough space after the data mover descriptor(s) to
            // inject the "ack to self" descriptor.
            //
            // Inject the data mover descriptor(s) and create an "ack to self"
            // message and post it to the channel.
            size_t i;

            for (i = 0; i < T_Desc; i++)
              channel->injFifoAdvanceDesc (); // see todo

            channel->post (createSimpleMessage (state, channel, fn, cookie));
            TRACE_FN_EXIT();
          }

          ///
          /// \brief Create a message containing one or more data mover descriptors
          ///        followed by an "ack to self" memory fifo descriptor.
          ///
          /// \param[in] state   Memory which will contain the message object
          /// \param[in] channel Injection channel to use during message advance
          /// \param[in] fn      Completion function to invoke
          /// \param[in] cookie  Completion cookie
          /// \param[in] desc    Array of initialized "data mover" descriptors
          ///
          /// \return Message to be posted to an injection channel
          ///
          template <unsigned T_State, unsigned T_Desc>
          inline MU::MessageQueue::Element * createDescriptorMessage (uint8_t                (&state)[T_State],
                                                                      InjChannel           * channel,
                                                                      pami_event_function    fn,
                                                                      void                 * cookie,
                                                                      MUSPI_DescriptorBase   (&desc)[T_Desc])
          {
            TRACE_FN_ENTER();
            COMPILE_TIME_ASSERT(sizeof(InjectDescriptorMessage < T_Desc + 1, false > ) <= T_State);

            InjectDescriptorMessage < T_Desc + 1, false > * msg =
              (InjectDescriptorMessage < T_Desc + 1, false > *) state;
            new (msg) InjectDescriptorMessage < T_Desc + 1, false > (channel);

            // Copy the "data mover" descriptor(s) into the message.
            size_t i;

            for (i = 0; i < T_Desc; i++)
              desc[i].clone (msg->desc[i]);

            // Copy the "completion" descriptor into the message and initialize
            // the completion function+cookie in the packet header.
            clone (msg->desc[T_Desc]);
            MU::Context::notify_t * hdr =
              (MU::Context::notify_t *) & msg->desc[T_Desc].PacketHeader;
            hdr->fn = fn;
            hdr->cookie = cookie;

            TRACE_FN_EXIT();
            return (MU::MessageQueue::Element *) msg;
          };

          ///
          /// \brief Create a simple message containing only an "ack to self"
          ///        memory fifo descriptor.
          ///
          /// \param[in] state   Memory which will contain the message object
          /// \param[in] channel Injection channel to use during message advance
          /// \param[in] fn      Completion function to invoke
          /// \param[in] cookie  Completion cookie
          ///
          /// \return Message to be posted to an injection channel
          ///
          template <unsigned T_State>
          inline MU::MessageQueue::Element * createSimpleMessage (uint8_t                (&state)[T_State],
                                                                  InjChannel           * channel,
                                                                  pami_event_function    fn,
                                                                  void                 * cookie)
          {
            TRACE_FN_ENTER();
            COMPILE_TIME_ASSERT(sizeof(InjectDescriptorMessage<1, false>) <= T_State);

            InjectDescriptorMessage<1, false> * msg =
              (InjectDescriptorMessage<1, false> *) state;
            new (msg) InjectDescriptorMessage<1, false> (channel);

            // Copy the "completion" descriptor into the message and initialize
            // the completion function+cookie in the packet header.
            clone (msg->desc[0]);
            MU::Context::notify_t * hdr =
              (MU::Context::notify_t *) & msg->desc[0].PacketHeader;
            hdr->fn = fn;
            hdr->cookie = cookie;

            TRACE_FN_EXIT();
            return (MU::MessageQueue::Element *) msg;
          };

      }; // PAMI::Device::MU::MemoryFifoCompletion class
    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace

#undef  DO_TRACE_ENTEREXIT
#undef  DO_TRACE_DEBUG

#endif // __components_devices_bgq_mu2_MemoryFifoCompletion_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//




