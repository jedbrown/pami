/**
 * \file components/devices/bgq/mu/MUBaseMessage.h
 * \brief ???
 */

#ifndef __components_devices_bgq_mu_MUBaseMessage_h__
#define __components_devices_bgq_mu_MUBaseMessage_h__

#include "MUBaseMessageT.h"

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {

      class MUBaseMessage : public MUBaseMessageT<MUBaseMessage>
      {
        public:
          MUBaseMessage () : MUBaseMessageT <MUBaseMessage> () {}

          ///
          /// \brief Advance This Message
          ///
          /// This function posts the descriptor for this message to the
          /// DMA fifo(s).
          ///
          /// \param[in]  fifo  Pointer to a list of DMA fifo number to be
          ///                   used by the advance function.
          /// \param[in]  nfifos  Number of fifo numbers in the fifo list.
          ///
          virtual bool advance(unsigned *fnum, unsigned nfifos) = 0;

          inline void start_impl () {}

          inline void reset_impl () {}

      }; // PAMI::Device::MU::MUBaseMessage class
    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace

#endif

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
