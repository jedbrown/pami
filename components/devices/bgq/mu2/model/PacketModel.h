/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/model/PacketModel.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu2_model_PacketModel_h__
#define __components_devices_bgq_mu2_model_PacketModel_h__

#include "components/devices/bgq/mu2/model/PacketModelBase.h"

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      class PacketModel : public MU::PacketModelBase<PacketModel>
      {
        public :

          /// \see PAMI::Device::Interface::PacketModel::PacketModel
          inline PacketModel (MU::Context & context) :
              MU::PacketModelBase<PacketModel> (context)
          {};

          /// \see PAMI::Device::Interface::PacketModel::~PacketModel
          inline ~PacketModel () {};

          template <unsigned T_State, unsigned T_Desc>
          inline void processCompletion_impl (uint8_t                (&state)[T_State],
                                              InjChannel           & channel,
                                              pami_event_function    fn,
                                              void                 * cookie,
                                              MUSPI_DescriptorBase   (&desc)[T_Desc])
          {
            COMPILE_TIME_ASSERT(T_Desc == 1);
            channel.setInjectionDescriptorNotification (fn, cookie, desc);

            // Advance the injection fifo tail pointer. This action
            // completes the injection operation.
            channel.injFifoAdvanceDesc ();
          };

          template <unsigned T_State, unsigned T_Desc>
          inline MU::MessageQueue::Element * createMessage_impl (uint8_t                (&state)[T_State],
                                                                 InjChannel           & channel,
                                                                 pami_event_function    fn,
                                                                 void                 * cookie,
                                                                 MUSPI_DescriptorBase   (&desc)[T_Desc])
          {
            COMPILE_TIME_ASSERT(T_Desc == 1);
            COMPILE_TIME_ASSERT(sizeof(InjectDescriptorMessage<1>) <= T_State);

            InjectDescriptorMessage<1> * msg = (InjectDescriptorMessage<1> *) state;
            new (msg) InjectDescriptorMessage<1> (channel, fn, cookie);

            // Copy the "data mover" descriptor into the message.
            desc[0].clone (msg->desc[0]);

            return (MU::MessageQueue::Element *) msg;
          };
      }; // PAMI::Device::MU::PacketModel class
    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace

#endif // __components_devices_bgq_mu2_PacketModel_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
