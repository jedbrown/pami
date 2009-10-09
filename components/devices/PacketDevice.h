/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/PacketDevice.h
 * \brief ???
 */

#ifndef __components_devices_packetdevice_h__
#define __components_devices_packetdevice_h__

#include "sys/xmi.h"

namespace XMI
{
  namespace Device
  {
    namespace Interface
    {
      ///
      /// \brief Packet device dispatch receive function signature
      ///
      /// \param[in] metadata       Pointer to network header metadata that is
      ///                           not part of the normal packet payload.
      /// \param[in] payload        Pointer to the raw packet payload.
      /// \param[in] bytes          Number of valid bytes of packet payload.
      /// \param[in] recv_func_parm Registered dispatch clientdata
      /// \param[in] cookie         Device cookie
      ///
      /// \todo Define return value(s)
      ///
      /// \return ???
      ///
      typedef int (*RecvFunction_t) (void   * metadata,
                                     void   * payload,
                                     size_t   bytes,
                                     void   * recv_func_parm,
                                     void   * cookie);

      ///
      /// \brief Packet device interface.
      ///
      /// \todo Need A LOT MORE documentation on this interface and its use
      /// \param T Packet device template class
      ///
      template <class T>
      class PacketDevice
      {
        public:
          PacketDevice() {}
          ~PacketDevice() {}

          ///
          /// \brief Read data from a packet device
          ///
          /// \attention All packet device derived classes \b must
          ///            implement the readData_impl() method.
          ///
          /// \param[in] dst      Destination buffer
          /// \param[in] bytes    Number of bytes to read
          /// \param[in] cookie   Device cookie
          ///
          /// \todo Define return value(s)
          ///
          /// \return 0 on success, !0 on failure
          ///
          inline int read (void * dst, size_t bytes, void * cookie);
      };

      template <class T>
      inline int PacketDevice<T>::read (void * dst, size_t bytes, void * cookie)
      {
        return static_cast<T*>(this)->read_impl(dst, bytes, cookie);
      }
    };
  };
};

#endif // __components_devices_packetdevice_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
