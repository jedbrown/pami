/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/MultiRemoteInjectDescriptor.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu2_MultiRemoteInjectDescriptor_h__
#define __components_devices_bgq_mu2_MultiRemoteInjectDescriptor_h__

#include "components/devices/bgq/mu2/model/MultiPt2PtDescriptor.h"

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      template <unsigned T_Num>
      class MultiRemoteInjectDescriptor : public MultiPt2PtDescriptor<T_Num>
      {
        public:

          inline MultiRemoteInjectDescriptor () :
              MultiPt2PtDescriptor<T_Num> ()
          {
            MUSPI_RemoteGetDescriptorInfoFields_t rget;
            rget.Type             = MUHWI_PACKET_TYPE_GET;
            rget.Rget_Inj_FIFO_Id = 0;

            size_t i;

            for (i = 0; i < T_Num; i++)
              {
                this->desc[i].setRemoteGetFields (&rget);
              }
          };

          inline ~MultiRemoteInjectDescriptor () {};

          ///
          /// \copydoc MultiPt2PtDescriptor::initializeDescriptors
          ///
          /// \param[in] paddr  Physical address of the payload to be sent.
          /// \param[in] bytes  Length of the payload, in bytes.
          ///
          inline void initializeDescriptors (MUHWI_Destination_t dest,
                                             uint64_t            map,
                                             uint8_t             hintsABCD,
                                             uint8_t             hintsE,
                                             uint64_t            paddr,
                                             uint64_t            bytes)
          {
            MultiPt2PtDescriptor<T_Num> * parent =
              (MultiPt2PtDescriptor<T_Num> *) this;
            parent->initializeDescriptors (dest, map, hintsABCD, hintsE);

            size_t i;

            for (i = 0; i < T_Num; i++)
              {
                // Set the source buffer address for the direct put.
                this->desc[i].setPayload (paddr, bytes);
              }
          };

      }; // PAMI::Device::MU::MultiRemoteInjectDescriptor class

      typedef MultiRemoteInjectDescriptor<1> RemoteInjectDescriptor;

    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace

#endif // __components_devices_bgq_mu2_MultiRemoteInjectDescriptor_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//

