/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/msg/InjectDescriptorMessage.h
 * \brief ???
 */

#ifndef __components_devices_bgq_mu2_msg_InjectDescriptorMessage_h__
#define __components_devices_bgq_mu2_msg_InjectDescriptorMessage_h__

#include "components/devices/bgq/mu2/msg/MessageQueue.h"

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      class InjectDescriptorMessage : public MessageQueue::Element
      {
        public:

          inline InjectDescriptorMessage (MUSPI_InjFifo_t      * injfifo,
                                          MUSPI_DescriptorBase * desc) :
              _injfifo (injfifo)
          {
            desc->clone (_desc);
          };

          inline ~InjectDescriptorMessage () {};

          ///
          /// \brief Inject descriptor message element advance method
          ///
          /// \retval true  Message complete
          /// \retval false Message incomplete and must remain on the queue
          ///
          virtual bool advance ()
          {
            uint64_t n = MUSPI_InjFifoInject (_injfifo, (void *) & _desc);
            return  (n != (uint64_t) - 1);
          }

        protected:

          MUSPI_DescriptorBase   _desc;
          MUSPI_InjFifo_t      * _injfifo;

      }; // class     PAMI::Device::MU::InjectDescriptorMessage
    };   // namespace PAMI::Device::MU
  };     // namespace PAMI::Device
};       // namespace PAMI


#endif // __components_devices_bgq_mu2_msg_InjectDescriptorMessage_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//

