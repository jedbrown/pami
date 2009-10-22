/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/DmaDevice.h
 * \brief ???
 */

#ifndef __components_devices_DmaDevice_h__
#define __components_devices_DmaDevice_h__

#include <sys/uio.h>

namespace XMI
{
  namespace Device
  {
    namespace Interface
    {
      ///
      /// \brief Device interface for all dma devices.
      ///
      /// \param T DMA device template class
      /// \param M DMA message template class
      ///
      template <class T, class M>
      class DmaDevice
      {
        public:
          DmaDevice  () {}
          ~DmaDevice () {}
#if 0
          ///
          /// \brief Post a dma message to the dma device
          ///
          /// \attention All CDI dma device derived classes \b must
          ///            implement the postDmaMessage_impl() method.
          ///
          /// \param msg Message to be queued/advanced
          ///
          /// \todo return type
          ///
          inline XMI_Result postDmaMessage (M * msg);
#endif
      };
#if 0
      template <class T, class M>
      inline XMI_Result Device<T, M>::postDmaMessage (M * msg)
      {
        return static_cast<T*>(this)->postDmaMessage_impl(msg);
      }
#endif
    };
  };
};

#endif // __components_devices_dmadevice_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
