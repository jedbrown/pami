/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/MessageDevice.h
 * \brief ???
 */
#error deprecated
#ifndef __components_devices_MessageDevice_h__
#define __components_devices_MessageDevice_h__

#include "components/devices/PacketModel.h"

namespace XMI
{
  namespace Device
  {
    namespace Interface
    {
      ///
      /// \brief Mesage device interface.
      ///
      /// \todo Need A LOT MORE documentation on this interface and its use
      /// \param T Message device template class
      ///
      template <class T>
      class MessageDevice : public PacketDevice<T>
      {
        public:
          MessageDevice () :
           PacketDevice<T> ()
         {};

          ~MessageDevice() {};
      };
    };
  };
};

#endif // __components_devices_messagedevice_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
