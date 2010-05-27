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

          friend class MU::PacketModelBase<PacketModelMemoryFifoCompletion>;

          /// \see PAMI::Device::Interface::PacketModel::PacketModel
          inline PacketModelMemoryFifoCompletion (MU::Context & context) :
             MU::PacketModelBase<PacketModelMemoryFifoCompletion> (context),
             _done (_context)
          {
            COMPILE_TIME_ASSERT(sizeof(InjectDescriptorMessage<1>) <= packet_model_state_bytes);
            COMPILE_TIME_ASSERT(sizeof(InjectDescriptorMessage<2>) <= packet_model_state_bytes);

            MemoryFifoPacketHeader * hdr =
              (MemoryFifoPacketHeader *) & _done.desc[0].PacketHeader;

            // Set the "notify" system dispatch identifier
            hdr->setSinglePacket (true);
            hdr->setDispatchId (MU::Context::dispatch_system_notify);

            // Set the payload information.
            _done.desc[0].setPayload (0, 0);
          };

          /// \see PAMI::Device::Interface::PacketModel::~PacketModel
          inline ~PacketModelMemoryFifoCompletion () {};

        protected:

          /// \see PAMI::Device::MU::PacketModelBase::processCompletion
          inline void processCompletion_impl (void                * state,
                                              size_t                fnum,
                                              MUSPI_InjFifo_t     * ififo,
                                              size_t                ndesc,
                                              MUHWI_Descriptor_t  * desc,
                                              pami_event_function   fn,
                                              void                * cookie)
          {
            if (likely(ndesc > 1))
              {
                // There is enough space in the injection fifo to add the
                // "ack to self" memory fifo descriptor.

                // Clone the completion model descriptor into the injection fifo
                MemoryFifoDescriptor * done = (MemoryFifoDescriptor *) & desc[1];
                _done.clone (done);

                // Copy the completion function+cookie into the packet header.
                MU::Context::notify_t * hdr =
                  (MU::Context::notify_t *) & done->desc[0].PacketHeader;
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
            _done.desc[0].clone (msg->desc[0]);
            MU::Context::notify_t * hdr =
              (MU::Context::notify_t *) & msg->desc[0].PacketHeader;
            hdr->fn = fn;
            hdr->cookie = cookie;

            InjectDescriptorMessage * msg = (InjectDescriptorMessage *) state;
            new (msg) InjectDescriptorMessage (ififo, & done.desc[0]);
            
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
            _done.desc[0].clone (msg->desc[1]);
            MU::Context::notify_t * hdr =
              (MU::Context::notify_t *) & msg->desc[1].PacketHeader;
            hdr->fn = fn;
            hdr->cookie = cookie;

            return (MU::MessageQueue::Element *) msg;
          };

          MemoryFifoSelfDescriptor _done;

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




