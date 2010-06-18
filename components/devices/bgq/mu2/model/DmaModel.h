/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/model/DmaModel.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu2_model_DmaModel_h__
#define __components_devices_bgq_mu2_model_DmaModel_h__

#include "components/devices/bgq/mu2/model/DmaModelBase.h"

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      class DmaModel : public MU::DmaModelBase<DmaModel>
      {
        public :

          /// \see PAMI::Device::Interface::DmaModel::DmaModel
          inline DmaModel (MU::Context & device, pami_result_t & status) :
              MU::DmaModelBase<DmaModel> (device, status)
          {};

          /// \see PAMI::Device::Interface::DmaModel::~DmaModel
          inline ~DmaModel () {};

          template <unsigned T_State>
          inline void processCompletion_impl (uint8_t                (&state)[T_State],
                                              InjChannel           & channel,
                                              pami_event_function    fn,
                                              void                 * cookie,
                                              MUSPI_DescriptorBase   (&desc)[1])
          {
            channel.setInjectionDescriptorNotification (fn, cookie, desc);

            // Advance the injection fifo tail pointer. This action
            // completes the injection operation.
            channel.injFifoAdvanceDesc ();
          };

      }; // PAMI::Device::MU::DmaModel class
    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace

#endif // __components_devices_bgq_mu2_DmaModel_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
