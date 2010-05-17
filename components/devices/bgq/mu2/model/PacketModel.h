/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/PacketModel.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu2_PacketModel_h__
#define __components_devices_bgq_mu2_PacketModel_h__

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
          inline PacketModel (MU::Context & device) :
             MU::PacketModelBase<PacketModel> (device)
          {
          };

          /// \see PAMI::Device::Interface::PacketModel::~PacketModel
          inline ~PacketModel () {};

          /// \see PAMI::Device::MU::PacketModelBase::processCompletion
          inline void processCompletion_impl (void                * state,
                                              size_t                fnum,
                                              MUSPI_InjFifo_t     * ififo,
                                              size_t                ndesc,
                                              MUHWI_Descriptor_t  * desc,
                                              pami_event_function   fn,
                                              void                * cookie)
          {
            // Advance the injection fifo tail pointer. This action
            // completes the injection operation.
            uint64_t sequenceNum = 0; // suppress warning 
            sequenceNum = MUSPI_InjFifoAdvanceDesc (ififo);
// !!!!
// add counter completion notification stuff
// !!!!
          }

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




