/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/MultiPt2PtDescriptor.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu2_MultiPt2PtDescriptor_h__
#define __components_devices_bgq_mu2_MultiPt2PtDescriptor_h__

#include "components/devices/bgq/mu2/model/MultiDescriptor.h"

//#include <spi/include/mu/DescriptorBaseXX.h>

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      template <unsigned T_Num>
      class MultiPt2PtDescriptor : public MultiDescriptor<T_Num>
      {
        public:

          inline MultiPt2PtDescriptor() :
              MultiDescriptor<T_Num> ()
          {
            MUSPI_Pt2PtDescriptorInfoFields_t pt2pt;
            pt2pt.Hints_ABCD = 0;
            pt2pt.Skip       = 0;
            pt2pt.Misc1 =
              MUHWI_PACKET_USE_DETERMINISTIC_ROUTING |
              MUHWI_PACKET_DO_NOT_DEPOSIT |
              MUHWI_PACKET_DO_NOT_ROUTE_TO_IO_NODE;
            pt2pt.Misc2 =
              MUHWI_PACKET_VIRTUAL_CHANNEL_DETERMINISTIC;

            size_t i;

            for (i = 0; i < T_Num; i++)
              {
		this->desc[i].setDataPacketType (MUHWI_PT2PT_DATA_PACKET_TYPE);	
		this->desc[i].PacketHeader.NetworkHeader.pt2pt.Byte8.Size=16;
                this->desc[i].setPt2PtFields (&pt2pt);
              }

          };

          inline ~MultiPt2PtDescriptor () {};

          ///
          /// \brief Initialize all descriptors in this container class
          ///
          /// \note The number of descriptors is specified by the \c T_Num
          ///       template parameter.
          ///
          /// \param[in]  dest       Coordinates of destination
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
          inline void initializeDescriptors (MUHWI_Destination_t    dest,
                                             uint64_t               map,
                                             uint8_t                hintsABCD,
                                             uint8_t                hintsE)
          {
            size_t i;
            for (i = 0; i < T_Num; i++)
              {
                this->desc[i].setDestination (dest);
                this->desc[i].setTorusInjectionFIFOMap (map);
                this->desc[i].setHints (hintsABCD, hintsE);
              }
          };

      }; // PAMI::Device::MU::MultiPt2PtDescriptor class

      typedef MultiPt2PtDescriptor<1> Pt2PtDescriptor;

    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace

#endif // __components_devices_bgq_mu2_MultiPt2PtDescriptor_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//

