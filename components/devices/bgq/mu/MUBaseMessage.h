
#ifndef   __MU_BASE_MESSSAGE__
#define   __MU_BASE_MESSSAGE__

#include "MUBaseMessageT.h"

namespace XMI
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

      }; // XMI::Device::MU::MUBaseMessage class
    };   // XMI::Device::MU namespace
  };     // XMI::Device namespace
};       // XMI namespace

#endif

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
