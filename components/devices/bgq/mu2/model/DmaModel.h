/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/DmaModel.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu2_DmaModel_h__
#define __components_devices_bgq_mu2_DmaModel_h__

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
          {
// !!!!
// Initialize rput model: destination = self, dynamic routing, etc
// !!!!
            MUSPI_DescriptorBase * rput =
              (MUSPI_DescriptorBase *) & _rput.desc[0].PacketHeader;

            // Set the destination to the coordinates of this mu context
            rput->setDestination (*(_device.getMuDestinationSelf()));
          };

          /// \see PAMI::Device::Interface::DmaModel::~DmaModel
          inline ~DmaModel () {};

          inline size_t
          initializePayloadDescriptors_impl (void                * vaddr,
                                             uint64_t              local_dst_pa,
                                             uint64_t              remote_src_pa,
                                             size_t                bytes,
                                             size_t                from_task,
                                             size_t                from_offset,
                                             pami_event_function   local_fn,
                                             void                * cookie)
          {
            // Retreive the route information back to mu context "self"
            uint64_t map;
            uint8_t  hintsABCD;
            uint8_t  hintsE;

            _device.pinInformation (from_task,
                                    from_offset,
                                    map,
                                    hintsABCD,
                                    hintsE);

            // Clone the remote direct put model descriptor into the payload
            DirectPutDescriptor * clone = (DirectPutDescriptor *) vaddr;
            _rput.clone (clone);

            MUSPI_DescriptorBase * rput = (MUSPI_DescriptorBase *) & clone->desc[0];

            // Set the payload of the direct put descriptor to be the physical
            // address of the source buffer on the remote node (from the user's
            // memory region).
            rput->setPayload (remote_src_pa, bytes);

            // Set the destination buffer address for the remote direct put.
            rput->setRecPayloadBaseAddressInfo (0, local_dst_pa);

            rput->setTorusInjectionFIFOMap (map);
            rput->setHints (hintsABCD, hintsE);
// !!!!
// Allocate completion counter, set counter in rput descriptor, set completion function and cookie
// !!!!

            return sizeof(MUHWI_Descriptor_t);
          };

        protected:

          DirectPutDescriptor _rput;

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




