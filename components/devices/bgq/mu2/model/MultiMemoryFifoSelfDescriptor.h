/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/MultiMemoryFifoSelfDescriptor.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu2_MultiMemoryFifoSelfDescriptor_h__
#define __components_devices_bgq_mu2_MultiMemoryFifoSelfDescriptor_h__

#include "components/devices/bgq/mu2/model/MultiMemoryFifoDescriptor.h"

#include "components/devices/bgq/mu2/Context.h"

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      template <unsigned T_Num>
      class MultiMemoryFifoSelfDescriptor : public MultiMemoryFifoDescriptor<T_Num>
      {
        public:

          inline MultiMemoryFifoSelfDescriptor (MU::Context & device) :
              MultiMemoryFifoDescriptor<T_Num> ()
          {
            size_t i;

            for (i = 0; i < T_Num; i++)
              {
                this->desc[i].setRecFIFOId (device.getRecptionFifoIdSelf());
                this->desc[i].setDestination (device.getMuDestinationSelf());

                // In loopback we send only on AM
                this->desc[i].setTorusInjectionFIFOMap (MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_AM);
                this->desc[i].setHints (MUHWI_PACKET_HINT_AM |
                                        MUHWI_PACKET_HINT_B_NONE |
                                        MUHWI_PACKET_HINT_C_NONE |
                                        MUHWI_PACKET_HINT_D_NONE,
                                        MUHWI_PACKET_HINT_E_NONE);
              }

          };

          inline ~MultiMemoryFifoSelfDescriptor () {};

          ///
          /// \brief Initialize all descriptors in this container class
          ///
          /// \note The number of descriptors is specified by the \c T_Num
          ///       template parameter.
          ///
          /// \param[in]  map        Torus injection FIFO map.  A 1 in bit i
          ///                        indicates that the message can be injected
          ///                        into Torus injection FIFO i. i = [0..15]
          /// \param[in]  hintsABCD  Hint bits for network routing.
          ///                        Each bit corresponds to A+, A-, B+, B-,
          ///                        C+, C-, D+, D-.  If a bit is set, it
          ///                        indicates that the packet wants to travel
          ///                        along the corresponding direction.  If
          ///                        all bits are zero, the hardware
          ///                        calculates the hint bits.  Both A+ and A-
          ///                        cannot be set at the same time...same
          ///                        with B, C, and D.  Refer to
          ///                        MUHWI_PACKET_HINT_xx definitions.
          /// \param[in]  hintsE     Hint bits for network routing.
          ///                        Only bits 0 and 1 are used.
          ///                        One of:
          ///                        - MUHWI_PACKET_HINT_E_NONE
          ///                        - MUHWI_PACKET_HINT_EP
          ///                        - MUHWI_PACKET_HINT_EM
          ///                        Only one of these bits can be set.
          ///
          inline void initializeDescriptors (uint64_t               map,
                                             uint8_t                hintsABCD,
                                             uint8_t                hintsE)
          {
            size_t i;

            for (i = 0; i < T_Num; i++)
              {
                this->desc[i].setTorusInjectionFIFOMap (map);
                this->desc[i].setHints (hintsABCD, hintsE);
              }
          };

      }; // PAMI::Device::MU::MultiMemoryFifoSelfDescriptor class

      typedef MultiMemoryFifoSelfDescriptor<1> MemoryFifoSelfDescriptor;

    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace

#endif // __components_devices_bgq_mu2_MultiMemoryFifoSelfDescriptor_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//

