/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/MultiMemoryFifoDescriptor.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu2_MultiMemoryFifoDescriptor_h__
#define __components_devices_bgq_mu2_MultiMemoryFifoDescriptor_h__

#include "components/devices/bgq/mu2/model/MultiPt2PtDescriptor.h"

#include <spi/include/mu/Pt2PtMemoryFIFODescriptorXX.h>
//#include <spi/include/mu/DescriptorBaseXX.h>

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      template <unsigned T_Num>
      class MultiMemoryFifoDescriptor : public MultiPt2PtDescriptor<T_Num>
      {
        public:

          inline MultiMemoryFifoDescriptor () :
              MultiPt2PtDescriptor<T_Num> ()
          {
            MUSPI_MemoryFIFODescriptorInfoFields_t memfifo;
            memfifo.Rec_FIFO_Id    = 0;
            memfifo.Rec_Put_Offset = 0;
            memfifo.Interrupt      = MUHWI_DESCRIPTOR_DO_NOT_INTERRUPT_ON_PACKET_ARRIVAL;
            memfifo.SoftwareBit    = 0;

            size_t i;

            for (i = 0; i < T_Num; i++)
              {
                this->desc[i].setMemoryFIFOFields (&memfifo);
              }

          };

          inline ~MultiMemoryFifoDescriptor () {};

          ///
          /// \copydoc MultiPt2PtDescriptor::initializeDescriptors
          ///
          /// \param[in]  rfifo      The reception FIFO Id (0 to 271).
          ///
          inline void initializeDescriptors (MUHWI_Destination_t    dest,
                                             uint64_t               map,
                                             uint8_t                hintsABCD,
                                             uint8_t                hintsE,
                                             uint16_t               rfifo)
          {
            MultiPt2PtDescriptor<T_Num> * parent =
              (MultiPt2PtDescriptor<T_Num> *) this;
            parent->initializeDescriptors (dest, map, hintsABCD, hintsE);

            size_t i;
            for (i = 0; i < T_Num; i++)
              this->desc[i].setRecFIFOId (rfifo);
          };

      }; // PAMI::Device::MU::MultiMemoryFifoDescriptor class

      typedef MultiMemoryFifoDescriptor<1> MemoryFifoDescriptor;

    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace

#endif // __components_devices_bgq_mu2_MultiMemoryFifoDescriptor_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//

