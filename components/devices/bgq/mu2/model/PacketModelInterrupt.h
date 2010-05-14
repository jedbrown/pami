/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/PacketModelInterrupt.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu2_PacketModelInterrupt_h__
#define __components_devices_bgq_mu2_PacketModelInterrupt_h__

#include "components/devices/bgq/mu2/model/PacketModelBase.h"

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      class PacketModelInterrupt : public MU::PacketModelBase<PacketModelInterrupt>
      {
        public :

          /// \see PAMI::Device::Interface::PacketModel::PacketModel
          inline PacketModelInterrupt (MU::Context & device) :
             MU::PacketModelBase<PacketModelInterrupt> (device)
          {
            _singlepkt.desc[0].setInterrupt(MUHWI_DESCRIPTOR_INTERRUPT_ON_PACKET_ARRIVAL);
            _multipkt.desc[0].setInterrupt(MUHWI_DESCRIPTOR_INTERRUPT_ON_PACKET_ARRIVAL);
          };

          /// \see PAMI::Device::Interface::PacketModel::~PacketModel
          inline ~PacketModelInterrupt () {};

      }; // PAMI::Device::MU::PacketModelInterrupt class
    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace

#endif // __components_devices_bgq_mu2_PacketModelInterrupt_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//




