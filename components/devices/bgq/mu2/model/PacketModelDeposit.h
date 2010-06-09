/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/PacketModelDeposit.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu2_PacketModelDeposit_h__
#define __components_devices_bgq_mu2_PacketModelDeposit_h__

#include "components/devices/bgq/mu2/model/PacketModel.h"

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      class PacketModelDeposit : public MU::PacketModel
      {
        public :

          /// \see PAMI::Device::Interface::PacketModel::PacketModel
          inline PacketModelDeposit (MU::Context & context) :
              MU::PacketModel (context)
          {
            _singlepkt.setDeposit(MUHWI_PACKET_DEPOSIT);
            _multipkt.setDeposit(MUHWI_PACKET_DEPOSIT);
          };

          /// \see PAMI::Device::Interface::PacketModel::~PacketModel
          inline ~PacketModelDeposit () {};

      }; // PAMI::Device::MU::PacketModelDeposit class
    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace

#endif // __components_devices_bgq_mu2_PacketModelDeposit_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//




