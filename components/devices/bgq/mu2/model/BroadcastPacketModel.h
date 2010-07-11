/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/model/BroadcastPacketModel.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu2_model_BroadcastPacketModel_h__
#define __components_devices_bgq_mu2_model_BroadcastPacketModel_h__

#include "components/devices/bgq/mu2/model/CollectivePacketModelBase.h"

#include "components/devices/bgq/mu2/trace.h"
#define DO_TRACE_ENTEREXIT 0
#define DO_TRACE_DEBUG     0


namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      class BroadcastPacketModel : public CollectivePacketModelBase<BroadcastPacketModel, MUHWI_COLLECTIVE_TYPE_BROADCAST, MUHWI_PACKET_VIRTUAL_CHANNEL_USER_COMM_WORLD>
      {
        public :

          /// \see PAMI::Device::Interface::CollectivePacketModel::CollectivePacketModel
          inline BroadcastPacketModel (MU::Context & context) :
              CollectivePacketModelBase<BroadcastPacketModel, MUHWI_COLLECTIVE_TYPE_BROADCAST, MUHWI_PACKET_VIRTUAL_CHANNEL_USER_COMM_WORLD> (context)
          {
            TRACE_FN_ENTER();
            TRACE_FN_EXIT();
          };

          /// \see PAMI::Device::Interface::CollectivePacketModel::~CollectivePacketModel
          inline ~BroadcastPacketModel () 
          {
            TRACE_FN_ENTER();
            TRACE_FN_EXIT();
          };

      }; // PAMI::Device::MU::BroadcastPacketModel class
          
    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace

#undef  DO_TRACE_ENTEREXIT
#undef  DO_TRACE_DEBUG

#endif // __components_devices_bgq_mu2_model_BroadcastPacketModel_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
