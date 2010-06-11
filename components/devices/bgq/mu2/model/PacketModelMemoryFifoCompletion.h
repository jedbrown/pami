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
#include "components/devices/bgq/mu2/model/MemoryFifoCompletion.h"
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
      class PacketModelMemoryFifoCompletion : public MU::PacketModelBase<PacketModelMemoryFifoCompletion>
      {
        public :

          /// \see PAMI::Device::Interface::PacketModel::PacketModel
          inline PacketModelMemoryFifoCompletion (MU::Context & context) :
              MU::PacketModelBase<PacketModelMemoryFifoCompletion> (context),
              _completion (context)
          {
          };

          /// \see PAMI::Device::Interface::PacketModel::~PacketModel
          inline ~PacketModelMemoryFifoCompletion () {};

          ///
          /// \todo Need a single MU SPI to advance the tail by *2* (or more)
          ///       descriptors
          ///
          /// \see PAMI::Device::MU::PacketModelBase::processCompletion
          template <unsigned T_State>
          inline void processCompletion_impl (uint8_t                (&state)[T_State],
                                              InjChannel           & channel,
                                              pami_event_function    fn,
                                              void                 * cookie,
                                              MUSPI_DescriptorBase   (&desc)[1])
          {
            TRACE_FN_ENTER();
            _completion.inject (state, channel, fn, cookie, desc);
            TRACE_FN_EXIT();
          }

          /// \see PAMI::Device::MU::PacketModelBase::createMessage
          template <unsigned T_State>
          inline MU::MessageQueue::Element * createMessage_impl (uint8_t                (&state)[T_State],
                                                                 InjChannel           & channel,
                                                                 pami_event_function    fn,
                                                                 void                 * cookie,
                                                                 MUSPI_DescriptorBase   (&desc)[1])
          {
            TRACE_FN_ENTER();
            MU::MessageQueue::Element * msg =
              _completion.createDescriptorMessage (state, channel, fn, cookie, desc);
            TRACE_FN_EXIT();
            return msg;
          };

          MemoryFifoCompletion _completion;

      }; // PAMI::Device::MU::PacketModelMemoryFifoCompletion class
    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace

#undef  DO_TRACE_ENTEREXIT
#undef  DO_TRACE_DEBUG

#endif // __components_devices_bgq_mu2_PacketModelMemoryFifoCompletion_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//




