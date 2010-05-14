/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/MultiDirectPutDescriptor.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu2_MultiDirectPutDescriptor_h__
#define __components_devices_bgq_mu2_MultiDirectPutDescriptor_h__

#include "components/devices/bgq/mu2/model/MultiPt2PtDescriptor.h"

#include <spi/include/mu/Pt2PtDirectPutDescriptorXX.h>

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      template <unsigned T_Num>
      class MultiDirectPutDescriptor : public MultiPt2PtDescriptor<T_Num>
      {
        public:

          inline MultiDirectPutDescriptor () :
              MultiPt2PtDescriptor<T_Num> ()
          {
            MUSPI_DirectPutDescriptorInfoFields dput;
            //dput.Rec_Payload_Base_Address_Id = ResourceManager::BAT_DEFAULT_ENTRY_NUMBER;
            dput.Rec_Payload_Offset          = 0;
            //dput.Rec_Counter_Base_Address_Id = ResourceManager::BAT_SHAREDCOUNTER_ENTRY_NUMBER;
            dput.Rec_Counter_Offset          = 0;
            dput.Pacing                      = MUHWI_PACKET_DIRECT_PUT_IS_NOT_PACED;

            size_t i;

            for (i = 0; i < T_Num; i++)
              {
                this->desc[i].setDirectPutFields (&dput);

                // Use the shared reception counter.
                this->desc[i].setRecCounterBaseAddressInfo (1, 0);
              }
          };

          inline ~MultiDirectPutDescriptor () {};

          ///
          /// \copydoc MultiPt2PtDescriptor::initializeDescriptors
          ///
          /// \param[in] paddr  Physical address of the payload to be sent.
          /// \param[in] bytes  Length of the payload, in bytes.
          /// \param[in] id     The reception payload base address id.
          /// \param[in] offset The reception payload offset from the
          ///                   base address associated with the specified
          ///                   base address id.
          ///
          inline void initializeDescriptors (MUHWI_Destination_t dest,
                                             uint64_t            map,
                                             uint8_t             hintsABCD,
                                             uint8_t             hintsE,
                                             uint64_t            paddr,
                                             uint64_t            bytes,
                                             uint16_t            id,
                                             uint64_t            offset)
          {
            MultiPt2PtDescriptor<T_Num> * parent =
              (MultiPt2PtDescriptor<T_Num> *) this;
            parent->initializeDescriptors (dest, map, hintsABCD, hintsE);

            size_t i;

            for (i = 0; i < T_Num; i++)
              {
                // Set the source buffer address for the direct put.
                this->desc[i].setPayload (paddr, bytes);

                // Set the destination buffer address for the direct put.
                this->desc[i].setRecPayloadBaseAddressInfo (id, offset);
              }
          };

      }; // PAMI::Device::MU::MultiDirectPutDescriptor class

      typedef MultiDirectPutDescriptor<1> DirectPutDescriptor;

    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace

#endif // __components_devices_bgq_mu2_MultiDirectPutDescriptor_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//

